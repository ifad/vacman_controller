require 'spec_helper'

describe VacmanController::Token do
  let(:dpx_filename) { 'sample_dpx/VDP0000000.dpx' }
  let(:transport_key) { '11111111111111111111111111111111' }

  let(:tokens) do
    VacmanController::Token.import dpx_filename, transport_key
  end

  let(:token) { tokens.first }

  describe '#serial' do
    subject { token.serial }

    it { is_expected.to eq('VDP0000000') }
  end

  describe '#app_name' do
    subject { token.app_name }

    it { is_expected.to eq('RESPONLY    ') }
  end

  describe '#inspect' do
    subject { token.inspect }

    it 'returns a printable version of the token' do
      is_expected.to match(/serial="VDP0000000" app_name="RESPONLY    "/)
    end
  end

  describe '#to_h' do
    subject { token.to_h }

    it 'returns the token low-level hash' do
      is_expected.to be_a(Hash)
    end

    it { is_expected.to have_key('serial') }
    it { is_expected.to have_key('app_name') }
    it { is_expected.to have_key('blob') }
    it { is_expected.to have_key('flags1') }
    it { is_expected.to have_key('flags2') }
    it { is_expected.to have_key('sv') }

    it 'does not change across invocations' do
      id = token.to_h.object_id

      expect(token.to_h.object_id).to be(id)
    end
  end

  describe '#verify!' do
    it { expect(token.verify!(token.generate)).to be(true) }

    it do
      expect { token.verify!('111111') }.to \
        raise_error(VacmanController::Error, /Validation Failed/)
    end

    it 'does not crash when large strings are passed' do
      expect { token.verify!('1' * 10000).to raise_error(VacmanController::Error) }
    end

    it { expect { token.verify!(token.generate) }.to change { token.to_h } }
  end

  describe '#verify' do
    it { expect(token.verify(token.generate)).to be(true) }

    it { expect(token.verify('111111')).to be(false) }

    it { expect { token.verify(token.generate) }.to change { token.to_h } }

    context 'lockout' do
      before { expect(VacmanController::Kernel['IThreshold']).to eq(3) }

      # This is not completely clear at the moment, but it seems that
      # DiagLevel is driving the lockout value and not IThreshold.
      #
      before { VacmanController::Kernel['DiagLevel'] = VacmanController::Kernel['IThreshold'] }

      it 'allows 2 invalid OTPs without locking' do
        2.times { expect(token.verify('000000')).to be(false) }

        expect(token.properties.error_count).to eq(2)

        expect(token.verify(token.generate)).to be(true)

        expect(token.properties.error_count).to eq(0)
      end

      it 'locks out after 3 attempts' do
        otp = token.generate

        # Lock it out
        3.times { expect(token.verify('000000')).to be(false) }
        expect(token.properties.error_count).to eq(3)

        # Verify lockout
        expect(token.verify(otp)).to be(false)
        expect(token.properties.error_count).to eq(4)

        # Reset it
        expect(token.reset!).to be(true)
        expect(token.properties.error_count).to eq(0)

        # Verify it clicks
        expect(token.verify(token.generate)).to be(true)
        expect(token.properties.error_count).to eq(0)
      end
    end
  end

  describe '#generate' do
    subject { token.generate }

    it 'generates OTPs if allowed' do
      is_expected.to match(/\A[0-9]{6}\Z/)
    end
  end

  describe '#activation' do
    subject { token.activation }

    context 'on tokens that support activation data generation' do
      let(:dpx_filename) { 'sample_dpx/Demo_DP4MobileES.dpx' }

      it { expect(subject).to eq(['0000001', '20419498810810562569']) }

      it { expect { subject }.to_not change { token.to_h } }
      it { expect { token.verify('1234') }.to_not change { token.to_h['sv'] } }
    end

    context 'on tokens that do not support activation data generation' do
      it { expect { subject }.to raise_error(VacmanController::Error, /Invalid Static Vector Length/) }
      it { expect { subject rescue nil }.to_not change { token.to_h } }
    end
  end

  context 'on tokens that support PINs' do
    let(:dpx_filename) { 'sample_dpx/Demo_GO6.dpx' }

    describe '#set_pin' do
      context 'given a valid PIN' do
        let(:pin) { 3137 }

        it { expect(token.set_pin(pin)).to be(true) }

        it { expect { token.set_pin(pin) }.to change { token.to_h } }

        ## Works only on certain circumstances
        ##
        context 'vtoken' do
          before { token.set_pin(pin) }

          it { expect(token.verify("3137#{token.generate}")).to be(true) }
          it { expect(token.verify("0000#{token.generate}")).to be(false) }
        end if ENV['AAL2vtoken']
      end

      context 'given an invalid PIN' do
        let(:pin) { [1, 1234567890, 'fo'].shuffle.first }

        it { expect { token.set_pin(pin) }.to raise_error(VacmanController::Error) }

        it { expect { token.set_pin(pin) rescue nil }.to_not change { token.to_h } }
      end
    end

    describe '#enable_pin!' do
      it { expect(token.enable_pin!).to be(true) }
    end

    describe '#disable_pin!' do
      it { expect(token.disable_pin!).to be(true) }
    end

    describe '#force_pin_change!' do
      it { expect(token.force_pin_change!).to be(true) }
    end
  end

  context 'on tokens that do not support PINs' do
    describe '#set_pin' do
      it { expect { token.set_pin(1234) }.to raise_error(/PIN not supported/i) }
    end

    describe '#enable_pin!' do
      it { expect { token.enable_pin! }.to raise_error(/invalid property value/i) }
    end

    describe '#disable_pin!' do
      it { expect { token.disable_pin! }.to raise_error(/invalid property value/i) }
    end

    describe '#force_pin_change!' do
      it { expect { token.force_pin_change! }.to raise_error(/invalid property value/i) }
    end
  end

  describe '#reset!' do
    it { expect(token.reset!).to be(true) }

    context do
      before { 5.times { token.verify('foobar') } }
      subject { token.reset! }
      it { expect { subject }.to change { token.properties.error_count }.from(5).to(0) }
    end
  end

  describe '#disable!' do
    it { expect(token.disable!).to be(true) }

    context do
      before { token.disable! }

      if ENV['AAL2vtoken']
        it { expect(token.verify(token.generate)).to be(false) }
      else
        it { expect { token.verify(token.generate) }.to raise_error(/Application disabled/) }
      end
    end
  end

  describe '#enable_primary_only!' do
    it { expect(token.enable_primary_only!).to be(true) }

    context do
      before { token.enable_primary_only! }

      it { expect(token.verify(token.generate)).to be(true) }
      it { expect { token.generate }.to_not raise_error }
    end
  end

  describe '#reset_error_count!' do
    before { token.verify('foobar') }
    subject { token.reset_error_count! }
    it { expect { subject }.to change { token.properties.error_count }.from(1).to(0) }
  end

  ## Don't have demo tokens to test this...
  #
  #describe '#enable_backup_only!' do
  #  it { expect(token.enable_backup_only!).to be(true) }
  #
  #  context do
  #    before { token.enable_backup_only! }
  #
  #    it { expect(token.verify(token.generate)).to be(false) }
  #  end
  #end
  describe '#enable_backup_only!' do
    it { expect { token.enable_backup_only! }.to raise_error(/error 517/) }
  end

  ## Don't have demo tokens to test this...
  #
  #describe '#enable!' do
  #  it { expect(token.enable!).to be(true) }
  #
  #  context do
  #    before { token.enable! }
  #
  #    it { expect(token.verify(token.generate)).to be(true) }
  #  end
  #end
  describe '#enable!' do
    it { expect { token.enable! }.to raise_error(/error 517/) }
  end


  describe '#properties' do
    it { expect(token.properties).to be_a(VacmanController::Token::Properties) }
  end
end

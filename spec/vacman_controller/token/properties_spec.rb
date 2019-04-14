require 'spec_helper'

describe VacmanController::Token::Properties do
  let(:dpx_filename) { 'sample_dpx/VDP0000000.dpx' }
  let(:transport_key) { '11111111111111111111111111111111' }

  let(:tokens) do
    VacmanController::Token.import dpx_filename, transport_key
  end

  let(:token) { tokens.first }

  describe '.names' do
    subject { described_class.names }

    it { is_expected.to be_frozen }
    it { is_expected.to be_a(Array) }

    it { is_expected.to include('token_model') }
    it { is_expected.to include('pin_enabled') }
    it { is_expected.to include('pin_ch_on') }
    it { is_expected.to include('use_count') }
  end

  describe '#inspect' do
    it { expect(token.properties.inspect).to be_a(String) }
  end

  describe '#all' do
    subject { token.properties.all }

    it { expect(subject).to be_a(Hash) }
    it { expect(subject.keys).to eq(described_class.names) }
  end

  describe '[]' do
    it { expect(token.properties[:last_time_used]).to eq(Time.utc(1970)) }
    it { expect(token.properties[:virtual_token_grace_period]).to be(nil) }

    it { expect(token.properties[:pin_min_len]).to be(0) }
    it { expect(token.properties[:use_count]).to be(0) }

    it { expect(token.properties[:pin_supported]).to be(false) }
    it { expect(token.properties[:pin_enabled]).to be(false) }
    it { expect(token.properties[:sync_windows]).to be(true) }

    it { expect(token.properties[:virtual_token_type]).to eq('PRIMARY') }
    it { expect(token.properties[:code_word]).to eq('00005200') }

    context 'on a pin-enabled token' do
      let(:dpx_filename) { 'sample_dpx/Demo_GO6.dpx' }

      it { expect(token.properties[:last_time_used]).to eq(Time.utc(1970)) }

      it { expect(token.properties[:pin_min_len]).to be(4) }

      it { expect(token.properties[:pin_supported]).to be(true) }
      it { expect(token.properties[:pin_enabled]).to be(true) }
      it { expect(token.properties[:sync_windows]).to be(true) }

      it { expect(token.properties[:virtual_token_type]).to be(nil) }
      it { expect(token.properties[:code_word]).to eq('00005200') }
    end

    context 'when inserting the correct OTP' do
      subject { token.verify(token.generate) }

      it { expect { subject }.to change { token.properties[:use_count] }.by(1) }
      it { expect { subject }.to change { token.properties[:last_time_used] } }
    end

    context 'on a write-only property' do
      it { expect { token.properties[:token_status] }.to raise_error(/Invalid property/) }
    end
  end

  describe '[]=' do
    context 'on an integer bounded property' do
      let(:prop) { :last_time_used }

      context 'within bounds' do
        subject { token.properties[prop] = 631152000 }

        it { expect { subject }.to_not raise_error }
        it { expect { subject }.to change { token.properties[prop] } }

        context do
          before { subject }
          it { expect(token.properties[prop]).to eq(Time.utc(1990)) }
        end
      end

      context 'outside bounds' do
        subject { token.properties[prop] = 123 }

        it { expect { subject }.to raise_error(/Invalid #{prop} value provided: 123/) }

        it { expect { subject rescue nil }.to_not change { token.properties[prop] } }
      end
    end

    context 'on an enumerated property' do
      let(:prop) { :token_status }
      subject { token.properties[prop] = value }

      context 'with a valid value' do
        let(:value) { :disabled }
        it { expect { subject }.to_not raise_error }
      end

      context 'with a bogus value' do
        let(:value) { :foobar }
        it { expect { subject }.to raise_error(/cannot be set to :foobar/) }
      end
    end

    context 'on a readonly property' do
      subject { token.properties[:time_based_algo] = 1 }

      it { expect { subject }.to raise_error(/Invalid or read-only property/) }
    end
  end

  describe '#method_missing' do
    context 'on a PIN-enabled token' do
      let(:dpx_filename) { 'sample_dpx/Demo_GO6.dpx' }

      it { expect(token.properties.pin_supported).to be(true) }

      it { expect(token.properties.pin_enabled).to be(true) }

      context 'pin_enabled' do
        before { token.properties.pin_enabled = false }

        subject { token.properties.pin_enabled = true }

        it { expect { subject }.to change { token.properties.pin_enabled }.from(false).to(true) }

        after { token.properties.pin_enabled = true }
      end

      context 'pin_change_forced = true' do
        subject { token.properties.pin_change_forced = true }

        it { expect { subject }.to change { token.properties.pin_change_forced }.from(false).to(true) }
      end

      context 'pin_change_forced = false' do
        subject { token.properties.pin_change_forced = false }

        it { expect { subject }.to raise_error(/cannot be set to false/) }
      end

      context 'pin_minimum_length' do
        before { token.properties.pin_minimum_length = 3 }
        subject { token.properties.pin_minimum_length = 8 }
        it { expect { subject }.to change { token.properties.pin_minimum_length }.from(3).to(8) }
      end
    end

    context 'on a token without PINs' do
      it { expect(token.properties.pin_supported).to be(false) }

      it { expect(token.properties.pin_enabled).to be(false) }

      it { expect { token.properties.pin_enabled = true }.to raise_error(/Invalid property value/) }
    end

    it { expect(token.properties.last_time_used).to eq(Time.utc(1970)) }

    it { expect(token.properties.virtual_token_grace_period).to be(nil) }

    context 'error_count' do
      it { expect(token.properties.error_count).to be(0) }

      context 'when resetting' do
        before { token.verify('foobar') }
        subject { token.properties.error_count = 0 }
        it { expect { subject }.to change { token.properties.error_count }.from(1).to(0) }
      end

      context 'when setting a specific value' do
        subject { token.properties.error_count = 123 }
        it { expect { subject }.to raise_error(/error_count cannot be set to 123/) }
      end
    end

    it { expect(token.properties.auth_mode).to be(:response_only) }

    context 'last_time_shift' do
      before { token.properties.last_time_shift = 0 }
      subject { token.properties.last_time_shift = 10 }
      it { expect { subject }.to change { token.properties.last_time_shift }.from(0).to(10) }
    end

    context 'virtual_token_grace_period' do
      before { token.properties.virtual_token_grace_period = 1 }

      subject { token.properties.virtual_token_grace_period = 320 }
      it { expect { subject }.to change { token.properties.virtual_token_grace_period } }
           ## Doesn't work due do clock approximations done in AAL2.
           #.from(Time.now.utc.round + 86400).to(Time.now.utc.round + 320 * 86400) }

      context 'read approximation' do
        let(:time_difference) {
          (Time.now.utc.round + 320 * 86400) -
          token.properties.virtual_token_grace_period
        }

        before { subject }

        it { expect(time_difference).to be < 2 }
      end
    end

    context 'virtual_token_remain_use' do
      before { token.properties.virtual_token_remain_use = 1 }
      subject { token.properties.virtual_token_remain_use = 220 }
      it { expect { subject }.to change { token.properties.virtual_token_remain_use } }
    end

    context 'event_value' do
      # Don't have event-based tokens to test with. Test the error case
      subject { token.properties.event_value = 123 }
      it { expect { subject }.to raise_error(/error 517/) }
    end
  end
end

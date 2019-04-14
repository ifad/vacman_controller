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
    context 'on a writeable property' do
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

    context 'on a readonly property' do
      let(:prop) { :time_based_algo }

      subject { token.properties[prop] = 1 }

      it { expect { subject }.to raise_error(/Invalid property/) }
    end
  end

  describe '#method_missing' do
    it { expect(token.properties.pin_enabled).to be(false) }

    it { expect(token.properties.last_time_used).to eq(Time.utc(1970)) }
  end
end

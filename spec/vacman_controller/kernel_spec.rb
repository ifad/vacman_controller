require 'rspec'
require 'vacman_controller'

describe VacmanController::Kernel do

  describe '.version' do
    subject { described_class.version }

    it { is_expected.to be_a(Hash) }

    it { is_expected.to have_key('version') }
    it { is_expected.to have_key('bitness') }
    it { is_expected.to have_key('type') }
  end

  describe '.property_names' do
    subject { described_class.property_names }

    it { is_expected.to be_a(Array) }

    it { is_expected.to include('ITimeWindow') }
    it { is_expected.to include('DiagLevel') }
  end

  describe '.inspect' do
    subject { described_class.inspect }

    it { is_expected.to be_a(String) }
    it { is_expected.to match(/version="[0-9\.]+?" bitness="[0-9]+?" type=".+?" parameters={.+?}/) }
  end

  describe '.all' do
    subject { described_class.all }

    it { is_expected.to be_a(Hash) }
    it { expect(subject.keys).to eq(described_class.property_names) }
  end

  describe '[]' do
    context 'on a valid property' do
      it { expect(described_class['DiagLevel']).to eq(0) }
      it { expect(described_class['ITimeWindow']).to eq(30) }
    end

    context 'on a bogus property' do
      it { expect { described_class['Foo'] }.to raise_error(/Invalid kernel param Foo/) }
    end
  end

  describe '[]=' do
    context 'on a valid property' do
      subject { described_class['ITimeWindow'] = 60 }
      after { described_class['ITimeWindow'] = 30 }

      it { expect { subject }.to_not raise_error }

      it { expect { subject }.to change { described_class['ITimeWindow'] } }
    end

    context 'on a bogus property' do
      subject { described_class['Foo'] = 60 }

      it { expect { subject }.to raise_error(/Invalid kernel param Foo/) }
    end
  end

end

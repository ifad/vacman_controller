require 'rspec'
require 'vacman_controller'

describe VacmanController do
  let(:dpx_filename) { 'sample_dpx/VDP0000000.dpx' }
  let(:transport_key) { '11111111111111111111111111111111' }

  let(:hashes) do
    VacmanController.import dpx_filename, transport_key
  end

  describe '.import' do
    it 'imports the right number of tokens' do
      expect(hashes.count).to eq(20)
    end

    it 'have the right serials' do
      expect(hashes.select {|e| e['serial'] =~ /VDP000000[01]/}.count).to eq(2)
    end

    context 'given an invalid key' do
      let(:transport_key) { '00000000000000000000000000000000' }

      it { expect { hashes }.to raise_error(VacmanController::Error, /invalid transport key/) }
    end

    context 'given a non-existing file' do
      let(:dpx_filename) { 'nonexistant' }

      it { expect { hashes }.to raise_error(VacmanController::Error, /cannot open DPX file/) }
    end
  end

  describe '.kernel' do
    it { expect(described_class.kernel).to be(VacmanController::Kernel) }
  end

end

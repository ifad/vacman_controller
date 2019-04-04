require 'rspec'
require 'vacman_controller'

describe VacmanController::Token do
  let(:dpx_filename) { 'sample_dpx/VDP0000000.dpx' }
  let(:transport_key) { '11111111111111111111111111111111' }

  let(:tokens) do
    VacmanController::Token.import dpx_filename, transport_key
  end

  let(:token) { tokens.first }

  describe '#serial' do
    it 'returns the token serial number' do
      expect(token.serial).to eq('VDP0000000')
    end
  end

  describe '#app_name' do
    it 'returns the token app name' do
      expect(token.app_name).to eq('RESPONLY    ')
    end
  end

  describe '#generate' do
    it 'generates OTPs if allowed' do
      expect(token.generate).to match(/\A[0-9]{6}\Z/)
    end
  end

  describe "#verify!" do
    it "verifies a valid key ok" do
      expect(token.verify!(token.generate)).to be(true)
    end

    it "does NOT verify a invalid key and raise Error" do
      expect {
        token.verify!('111111')
      }.to raise_error(VacmanController::Error, /Validation Failed/)
    end

    it "does not crash when large strings are passed" do
      expect { token.verify!('1' * 10000).to raise_error(VacmanController::Error) }
    end
  end

  describe "#verify" do
    it "does NOT verify a invalid key and return false" do
      expect(token.verify('111111')).to be(false)
    end
  end

  describe "too many false password attempts will lock the digipass" do
    it "allows two invalid OTPs without locking" do
      2.times do
        expect(token.verify('000000')).to be(false)
      end

      expect(token.verify(token.generate)).to be(true)
    end
  end
end

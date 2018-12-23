RSpec.describe Linenoise do
  it "has a version number" do
    expect(Linenoise::VERSION).to be_a(String)
  end

  it "has a gem version number" do
    expect(Linenoise::GEM_VERSION).to be_a(String)
  end

  describe "#completion_proc=" do
    it "raises error when passed value doesn't implement #call" do
      expect { described_class.completion_proc = 1 }
        .to raise_error(ArgumentError, "argument must respond to `call'")
    end
  end
end

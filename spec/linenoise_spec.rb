RSpec.describe Linenoise do
  it "has a version number" do
    expect(Linenoise::VERSION).to be_a(String)
  end

  it "has a gem version number" do
    expect(Linenoise::GEM_VERSION).to be_a(String)
  end
end

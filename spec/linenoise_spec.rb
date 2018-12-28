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

  describe "#hint_proc=" do
    it "raises error when passed value doesn't implement #call" do
      expect { described_class.hint_proc = 1 }
        .to raise_error(ArgumentError, "argument must respond to `call'")
    end
  end

  describe "#hint_color=" do
    after { Linenoise.hint_color = Linenoise::DEFAULT }

    it "sets hint color" do
      Linenoise.hint_color = Linenoise::RED
      expect(Linenoise.hint_color).to eq(Linenoise::RED)
    end

    it "raises error if color is not in range" do
      expect { Linenoise.hint_color = -1 }
        .to raise_error(ArgumentError, "color '-1' is not in range (31-37)")
    end

    it "raises error if color is not an integer" do
      expect { Linenoise.hint_color = 'salamat' }
        .to raise_error(TypeError, 'hint color is not an Integer')
    end
  end

  describe "#hint_bold=" do
    after { Linenoise.hint_bold = false }

    it "sets hint color" do
      Linenoise.hint_bold = true
      expect(Linenoise.hint_bold?).to be_truthy
    end
  end

  describe "#multiline?" do
    after { Linenoise.multiline = true }

    it "is `true` by default" do
      expect(Linenoise).to be_multiline
    end

    it "can be set to `false`" do
      Linenoise.multiline = false
      expect(Linenoise).not_to be_multiline
    end
  end
end

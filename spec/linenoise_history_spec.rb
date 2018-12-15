RSpec.describe Linenoise::HISTORY do
  after { subject.clear }

  describe "#push" do
    it "appends to history" do
      subject.max_len = 3

      subject << "123"
      expect(subject.size).to eq(1)

      subject.push("1", "2")
      expect(subject.size).to eq(3)

      subject.push("3", "4")
      expect(subject.size).to eq(3)
    end
  end

  describe "#save" do
    let(:filename) { 'history_file' }

    after { File.delete(filename) }

    it "saves history" do
      expect(File.exist?(filename)).not_to be_truthy
      subject.save(filename)
      expect(File.exist?(filename)).to be_truthy
    end
  end

  describe "#load" do
    let(:filename) { 'history_file' }

    before do
      File.open(filename, 'w+') { |f| f.puts("1\n2\n") }
    end

    after { File.delete(filename) }

    it "loads history" do
      subject.load(filename)
      expect(subject.size).to eq(2)
    end
  end
end

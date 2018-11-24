require 'rspec/core/rake_task'
require 'rake/extensiontask'

RSpec::Core::RakeTask.new(:spec)
task default: :spec

Rake::ExtensionTask.new do |ext|
  ext.name = 'linenoise'
  ext.ext_dir = 'ext/linenoise'
end

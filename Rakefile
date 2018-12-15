require 'rspec/core/rake_task'
require 'rake/extensiontask'

RSpec::Core::RakeTask.new(:spec)
task default: :spec

Rake::ExtensionTask.new do |ext|
  ext.name = 'linenoise'
  ext.ext_dir = 'ext/linenoise'
  ext.lib_dir = 'lib/linenoise'
end

task console: :compile do
  require_relative './lib/linenoise'
  require 'pry'

  Pry.start
end

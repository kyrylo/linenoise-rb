require './lib/linenoise/version'

Gem::Specification.new do |s|
  s.name = 'linenoise'
  s.version = Linenoise::GEM_VERSION
  s.authors = ['Kyrylo Silin']
  s.email = ['silin@kyrylo.org']

  s.summary = ''
  s.description = ''
  s.homepage = 'https://github.com/kyrylo/linenoise'
  s.license = 'MIT'

  s.metadata['homepage_uri'] = s.homepage
  s.metadata['source_code_uri'] = s.homepage
  s.metadata['changelog_uri'] = File.join(s.homepage, 'blob/master/CHANGELOG.md')

  s.require_path = 'lib'
  s.files = ['lib/linenoise.rb', *Dir.glob('lib/**/*')]
  s.test_files = Dir.glob('spec/**/*')

  s.required_ruby_version = '>= 2.1'

  s.add_development_dependency 'rake', '~> 12.3'
  s.add_development_dependency 'rake-compiler', '~> 1.0'
  s.add_development_dependency 'rspec', '~> 3.8'
end

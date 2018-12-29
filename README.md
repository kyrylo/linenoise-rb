Linenoise Ruby
==============

[![CircleCI](https://circleci.com/gh/kyrylo/linenoise-rb.svg?style=svg)](https://circleci.com/gh/kyrylo/linenoise-rb)
[![Gem Version](https://badge.fury.io/rb/linenoise.svg)](http://badge.fury.io/rb/linenoise)
[![Documentation Status](http://inch-ci.org/github/kyrylo/linenoise.svg?branch=master)](http://inch-ci.org/github/kyrylo/linenoise)
[![Downloads](https://img.shields.io/gem/dt/linenoise.svg?style=flat)](https://rubygems.org/gems/linenoise)

* [Documentation][documentation]

Introduction
------------

The Linenoise gem is a wrapper around the small self-contained alternative to
Readline and Libedit called [Linenoise](https://github.com/antirez/linenoise).

Features
--------

* History support
* Completion
* Hints (suggestions at the right of the prompt as you type)
* Single and multiline editing mode with the usual key bindings

Installation
------------

### Bundler

Add the Linenoise gem to your Gemfile:

```ruby
gem 'linenoise', '~> 1.0'
```

### Manual

Invoke the following command from your terminal:

```sh
gem install linenoise
```

Examples
--------

### Basic example

```ruby
require 'linenoise'

while buf = Linenoise.linenoise('> ')
  p buf
end
```

### Completion

```ruby
require 'linenoise'

LIST = %w[
  search download open help history quit url next clear prev past
].freeze

Linenoise.completion_proc = proc do |input|
  LIST.grep(/\A#{Regexp.escape(input)}/)
end

while line = Linenoise.linenoise('> ')
  p line
end
```

### Hints

```ruby
require 'linenoise'

Linenoise.hint_proc = proc do |input|
  case input
  when /git show/
    ' [<options>] [<object>...]'
  when /git log/
    ' [<options>] [<revision range>]'
  else
    ' --help'
  end
end

while line = Linenoise.linenoise('> ')
  p line
end
```

More examples and full API explanation is available on the
[documentation][documentation] page.

Development
-----------

### Running tests

```sh
bundle exec rake compile spec
```

### Launching development console

```
bundle exec rake compile console
```

Contact
-------

In case you have a problem, question or a bug report, feel free to:

* [file an issue](https://github.com/kyrylo/linenoise-rb/issues)
* send me an email (see https://github.com/kyrylo)
* [tweet at me](https://twitter.com/kyrylosilin)

License
-------

The project uses the MIT License. See LICENSE.md for details.

[documentation]: https://www.rubydoc.info/github/kyrylo/linenoise-rb/Linenoise

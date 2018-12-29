Linenoise
=========

[![CircleCI](https://circleci.com/gh/kyrylo/linenoise-rb.svg?style=svg)](https://circleci.com/gh/kyrylo/linenoise-rb)
[![Gem Version](https://badge.fury.io/rb/linenoise.svg)](http://badge.fury.io/rb/linenoise)
[![Documentation Status](http://inch-ci.org/github/kyrylo/linenoise.svg?branch=master)](http://inch-ci.org/github/kyrylo/linenoise)
[![Downloads](https://img.shields.io/gem/dt/linenoise.svg?style=flat)](https://rubygems.org/gems/linenoise)

* [Documentation][documentation]

Introduction
------------

The Linenoise gem is a wrapper around the small self-contained alternative to
Readline and Libedit called [Linenoise](https://github.com/antirez/linenoise).

Installation
------------

### Bundler

Add the Linenoise gem to your Gemfile:

```ruby
gem 'linenoise', '~> 1.0'
```

### Manual

Invoke the following command from your terminal:

```ruby
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

More examples and full API explanation is available on the
[documentation][documentation] page.

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

VASCO VACMAN Controller
=======================

[![Build status][build-status-badge]][build-status]
[![Code Climate][code-analysis-badge]][code-analysis]
[![Test Coverage][test-coverage-badge]][test-coverage]
[![Gem Version][gem-version-badge]][gem-version]
[![Docs analysis][docs-analysis-badge]][docs-analysis]

VACMAN Controller is VASCO's implementation of OTP physical and virtual
devices, revolving around the AAL2 library.

This gem contains a very thin wrapper around AAL2 and allows to parse DPX
files, generate OTPs and verify them.

Installation
------------

Get Vacman Controller library from VASCO. You should receive an
`aal2sdk-*.rpm` package. Install it, it will place its contents in
`/opt/vasco`. The Ruby extension looks for `/opt/vasco/VACMAN_Controller-*`

Add to your application Gemfile

    gem 'vacman_controller'

To run specs download the sources and execute

    rake

Usage
-----

The library provides access to the low-level functions of libaal2 through the
`VacmanController::LowLevel` module, that has only singleton methods and does
not keep any state.

The only shared state across all threads are the kernel params. Please ensure
that all threads coordinate when changing it.

The library provides also a `VacmanController::Token` abstraction, providing
token information and APIs that decode from and to Ruby objects when reading
and writing.

Ensure to persist the `token.to_h` value after performing any operation on a
token. The token hash contains the token state, that is altered by most APIs.

For extended usage examples, please have a look at the specs.

Contributing
------------

 1. Fork it
 2. Create your feature branch (`git checkout -b my-new-feature`)
 3. Commit your changes (`git commit -am 'Added some great feature'`)
 4. Push to the branch (`git push origin my-new-feature`)
 5. Create new Pull Request

[build-status]: https://travis-ci.org/ifad/vacman_controller
[build-status-badge]: https://travis-ci.org/ifad/vacman_controller.svg?branch=master
[code-analysis]: https://codeclimate.com/github/ifad/vacman_controller
[code-analysis-badge]: https://codeclimate.com/github/ifad/vacman_controller.svg
[docs-analysis]: https://inch-ci.org/github/ifad/vacman_controller
[docs-analysis-badge]: https://inch-ci.org/github/ifad/vacman_controller.svg?branch=master
[gem-version]: https://rubygems.org/gems/vacman_controller
[gem-version-badge]: https://badge.fury.io/rb/vacman_controller.svg
[test-coverage]: https://codeclimate.com/github/ifad/vacman_controller
[test-coverage-badge]: https://codeclimate.com/github/ifad/vacman_controller/badges/coverage.svg

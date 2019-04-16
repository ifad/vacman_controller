#!/usr/bin/env bash

set -ex

source /opt/rh/rh-ruby23/enable

command -v ruby
ruby -v

gem install bundler

command -v bundle
bundle -v

bundle install --path .bundle

# USER environment value is not set as a default in the container environment.
export USER=root

bundle exec rake

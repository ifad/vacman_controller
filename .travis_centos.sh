#!/usr/bin/env bash

set -eux

scl enable rh-ruby23 bash

gem install bundler

bundle install --path .bundle

# USER environment value is not set as a default in the container environment.
export USER=root

bundle exec rake

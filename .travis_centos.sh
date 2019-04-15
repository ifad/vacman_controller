#!/usr/bin/env bash

set -eux

source /opt/rh/rh-ruby23/enable

gem install bundler

bundle install --path .bundle

# USER environment value is not set as a default in the container environment.
export USER=root

bundle exec rake

#!/usr/bin/env bash

set -ex

export USER=root

export CC_TEST_REPORTER_URL=https://codeclimate.com/downloads/test-reporter/test-reporter-latest-linux-amd64

source /opt/rh/rh-ruby23/enable

command -v ruby
ruby -v

gem install bundler

command -v bundle
bundle -v

bundle install --path .bundle

echo "CC Test Reporter ID: $CC_TEST_REPORTER_ID"
curl -L $CC_TEST_REPORTER_URL -o cc-test-reporter
chmod +x cc-test-reporter

./cc-test-reporter -v

./cc-test-reporter -d before-build

bundle exec rake
build_status=$?

./cc-test-reporter -d after-build

exit $build_status

#!/usr/bin/env bash

# This script runs the full build and it is meant to run within a CentOS 7
# image as root (FIXME). It does not require docker, so if your build system
# already meets the requirements, you can execute this script directly.
#

# Break on errors
set -ex

export USER=root
export CC_TEST_REPORTER_URL=https://codeclimate.com/downloads/test-reporter/test-reporter-latest-linux-amd64

# Install VACMAN controller RPM
#
rpm -i $AAL2_RPM

# Use Ruby 2.3 from SCL
source /opt/rh/rh-ruby23/enable
command -v ruby
ruby -v

# Install bundler
gem install bundler

command -v bundle
bundle -v

# Install the bundle
bundle install --path .bundle

# Install CodeClimate test reporter
curl -L $CC_TEST_REPORTER_URL -o cc-test-reporter
chmod +x cc-test-reporter

./cc-test-reporter -v

./cc-test-reporter before-build

# Run the build
bundle exec rake
build_status=$?

echo "Build exited with $?"

./cc-test-reporter after-build

# Return build status to the caller
exit $build_status

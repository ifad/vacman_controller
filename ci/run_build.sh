#!/usr/bin/env bash

# This script runs the full build and it is meant to run within a CentOS 7
# image as root (FIXME). It does not require docker, so if your build system
# already meets the requirements, you can execute this script directly.
#

# Break on errors
set -ex

export USER=root

# Install CodeClimate test reporter
export CC_TEST_REPORTER_URL=https://codeclimate.com/downloads/test-reporter/test-reporter-latest-linux-amd64
curl -L $CC_TEST_REPORTER_URL -o cc-test-reporter
chmod +x cc-test-reporter

./cc-test-reporter -v

./cc-test-reporter before-build

# Run the build
bash ./ci/exec bundle exec rake
build_status=$?

echo "Build exited with $?"

./cc-test-reporter after-build --exit-code $build_status

# Return build status to the caller
exit $build_status

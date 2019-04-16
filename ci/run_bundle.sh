#!/usr/bin/env bash

#
# This script installs dependencies prepping for rake runs.
#

# Break on errors
set -ex

# Install VACMAN controller RPM
#
rpm -i $AAL2_RPM

#
# Use Ruby 2.3 from SCL
#
source /opt/rh/rh-ruby23/enable
command -v ruby
ruby -v

#
# Install bundler
#
gem install bundler
command -v bundle
bundle -v

#
# Bundle the app
#
bundle install --path .bundle

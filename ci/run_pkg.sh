#!/usr/bin/env bash

# This script packages and pushes the gem to RubyGems.org
# according to the current tag. Ensure to bind mount your
# .gem credentials into the container, if using one.
#

if [ ! -f $HOME/.gem/credentials ]; then
  echo "FATAL: ~/.gem/credentials not found"
  exit -1
fi

bash ./ci/exec bundle exec rake release

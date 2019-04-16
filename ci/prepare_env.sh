#!/usr/bin/env bash

# This script is run before the build is run and it is used to pass
# settings to the container via environment variables.
#
# Please define these in your environment if running the build locally.
#
# We are using Travis Environment Variables to have these saved there
# and not be displayed in the build log.
#

VARS="AAL2_RPM CC_TEST_REPORTER_ID"
ENV_FILE=ci/env

:> $ENV_FILE

for var in $VARS; do
  val=${!var}

  if [ -z "$val" ];then
    echo "FATAL: $var environment variable is not defined"
    exit -1
  fi

  echo $var=$val >> $ENV_FILE
done

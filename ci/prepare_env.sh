#!/usr/bin/env bash

if [ -z "$AAL2_RPM" ]; then
  echo "FATAL: AAL2_RPM environment variable is not defined"
  exit -1
fi

if [ -z "$CC_TEST_REPORTER_ID" ]; then
  echo "FATAL: CC_TEST_REPORTER_ID environment variable is not defined"
  exit -1
fi

ENV_FILE=ci/env

:> $ENV_FILE
echo AAL2_RPM=$AAL2_RPM >> $ENV_FILE
echo CC_TEST_REPORTER_ID=$CC_TEST_REPORTER_ID >> $ENV_FILE

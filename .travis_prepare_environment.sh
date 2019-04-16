#!/usr/bin/env bash

if [ -z "$AAL2_RPM" ]; then
  echo "FATAL: AAL2_RPM environment variable is not defined"
  exit -1
fi

if [ -z "$CC_TEST_REPORTER_ID" ]; then
  echo "FATAL: CC_TEST_REPORTER_ID environment variable is not defined"
  exit -1
fi

:> .travis_env
echo AAL2_RPM=$AAL2_RPM >> .travis_env
echo CC_TEST_REPORTER_ID=$CC_TEST_REPORTER_ID >> .travis_env

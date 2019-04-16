#/usr/bin/env bash
#
# This script builds the docker container, transfers private environment
# variables into the docker env file and runs the CMD specified in the
# Dockerfile.
#
set -ex

docker_build() {
  docker build --tag vacman_controller --file ci/Dockerfile .
}

docker_bash() {
  docker run --interactive --tty --env-file ci/env --workdir /build vacman_controller bash "$@"
}

stage=$1
if [ -z "$stage" ]; then
  stage=all
fi

if [ "$stage" = 'all' -o "$stage" = 'build' ]; then
  # Build and install
  docker_build

  # Prepare environment
  ./ci/prepare_env.sh
fi

if [ "$stage" = 'all' -o "$stage" = 'run' ]; then
  docker_bash ci/run_build.sh
fi

if [ "$stage" = 'shell' ]; then
  docker_bash
fi

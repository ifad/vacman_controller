#/usr/bin/env bash
#
# This script governs the actions we execute in the docker container.
#
# If invoked without arguments, it builds the docker container and runs
# the test suite within it.
#
# It can be run with:
#
#  all    -- build and run tests
#  build  -- build the container only
#  test   -- launches run_build.sh in the last built container
#  pkg    -- launches run_pkg.sh in the last built container
#  shell  -- launches a shell in the last built container
#

set -ex

docker_build() {
  docker build --tag vacman_controller --file ci/Dockerfile .
}

docker_bash() {
  docker run --interactive --tty --env-file ci/env --workdir /build $docker_opts vacman_controller bash "$@"
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

if [ "$stage" = 'all' -o "$stage" = 'test' ]; then
  docker_bash ci/run_build.sh
fi

if [ "$stage" = 'pkg' ]; then
  docker_opts="-v $HOME/.gem:/root/.gem"
  docker_bash ci/run_pkg.sh
fi

if [ "$stage" = 'shell' ]; then
  docker_bash ci/exec bash
fi

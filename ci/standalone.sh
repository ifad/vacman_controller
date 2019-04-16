# This script runs all that travis runs.
#
set -ex

export CC_TEST_REPORTER_ID=47866d6bec01d504297b03fb6b387103da34b1ac72a7bad784eaf52d85fc7bf8

docker build -t vacman_controller -f ci/Dockerfile .
sh -x ci/prepare_env.sh
docker run --interactive --tty --env-file ci/env --workdir /build vacman_controller

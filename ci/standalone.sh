# This script runs all that travis runs.
#
set -ex

docker build -t vacman_controller -f ci/Dockerfile .
bash ci/prepare_env.sh
docker run --interactive --tty --env-file ci/env --workdir /build vacman_controller

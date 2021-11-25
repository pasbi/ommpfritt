#!/usr/bin/env bash

usage="
This script builds ommpfritt with icons.
Please give the path to ommpfritt source as an argument.
"

if [ ! "$1" ]; then
    echo "$usage"
    exit 1
fi

set -e

source=$1
shift
config=${1:-Release}
shift || true

build=$(mktemp -d -t build-omm-XXXX)
install=$build/install
ncores=4

echo "Building omm "
echo "  source:  $source"
echo "  build:   $build"
echo "  config:  $config"
echo "  install: $install"
echo "  ncores:  $ncores"

cmake -S "$source" -B "$build" -DCMAKE_INSTALL_PREFIX="$install" -DCMAKE_BUILD_TYPE="$config" "$@"
cmake --build "$build" --target install --config "$config" -j$ncores
cmake --build "$build" --target icons -j$ncores
cmake -S "$source" -B "$build"
cmake --build "$build" --target install --config "$config" -j$ncores

echo "Finished building omm. Run $build/install/bin/ommpfritt"

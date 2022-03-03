#!/usr/bin/env bash

usage="Usage: $0 [CMAKE_ARGUMENTS ...]
This script builds ommpfritt with icons from source (root of this repository).
Builds and installs the application into '/tmp', see output for details.
Additional CMAKE_ARGUMENTS are passed verbatim to the cmake configuration call.
"
if [[ "$1" =~ (-h|--help) ]]; then
    echo "$usage"
    exit 0
fi
set -e
set -x

source=$(dirname $0)/..
config=Release

build=$(mktemp -d -t build-omm-XXXX)
install=$build/install

echo "Building omm "
echo "  source:  $source"
echo "  build:   $build"
echo "  config:  $config"
echo "  install: $install"

cmake -S "$source" -B "$build" -GNinja -DCMAKE_INSTALL_PREFIX="$install" -DCMAKE_BUILD_TYPE="$config" "$@"
cmake --build "$build" --target install --config "$config" -j$ncores
cmake --build "$build" --target icons -j$ncores
cmake -S "$source" -B "$build"
cmake --build "$build" --target install --config "$config" -j$ncores

echo "Finished building omm. Run $install/bin/ommpfritt"

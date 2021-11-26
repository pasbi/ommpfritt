#!/usr/bin/env bash

usage="Usage: $0 SOURCE CONFIG [CMAKE_ARGUMENTS ...]
This script builds ommpfritt with icons from SOURCE (root of this repository).
CONFIG can be 'Release' or 'Debug'.
Builds and installs the application into '/tmp', see output for details.
Additional CMAKE_ARGUMENTS are passed verbatim to the cmake configuration call.
"
if [[ "$1" =~ (-h|--help) ]]; then
    echo "$usage"
    exit 0
fi
if [ ! "$2" ]; then
    1>&2 echo "$usage"
    exit 1
fi

set -e

source=$1
config=$2

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

echo "Finished building omm. Run $install/bin/ommpfritt"

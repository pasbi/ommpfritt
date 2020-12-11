#!/usr/bin/env bash

if [ -z "$1" ]; then
  echo "Please provide the build directory."
  exit 1
fi

cd "$(dirname "$0")/.."
files="$(build-scripts/find-changed-files.sh "$2")"
echo $files

echo -e "\nRunning clang-format ..."
build-scripts/check.py --files $files --mode format -v

echo -e "\nRunning Clazy ..."
build-scripts/check.py --files $files --mode clazy --compile-commands "$1/compile_commands.json" -v

echo -e "\nRunning clang-tidy ..."
run-clang-tidy -p "$1" $files

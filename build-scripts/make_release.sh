#!/usr/bin/env bash

set -e

# Change to the root directory of the project
cd $(dirname $0)/..

if [ "$(git branch --show-current)" != "main" ]; then
  echo "You're not on main branch."
  exit 1
fi

if [ "$#" -ne 1 ]; then
  echo "Usage: ${0} <version>"
  echo "<version> has to be in the form of \"MAJOR.MINOR.PATCH\", for example \"0.1.2\""
  exit 1
fi

version=$1

# validate version
if ! echo "${version}" | grep --quiet "^[0-9]\+\.[0-9]\+\.[0-9]\+$" ; then
  echo "Invalid version ${version}"
  exit 1
fi

# https://stackoverflow.com/a/4024263
version_lte() {
  lesser_version=$(echo -e "$1\n$2" | sort -V | head -n1)
  [ "$1" = "$lesser_version" ]
}

version_lt() {
  [ "$1" = "$2" ] && return 1 || version_lte $1 $2
}

# make sure our git tree is up to date
git pull

# get the current git tag version.
git_version=$(git describe --tags | sed -E "s/^v([0-9]+\.[0-9]+\.[0-9]+).*/\1/")

# get the current version as provided in CMakeLists.txt
cmake_version=$(grep "^project(ommpfritt VERSION [0-9]\+\.[0-9]\+\.[0-9]\+" CMakeLists.txt | grep -o "[0-9]\+\.[0-9]\+\.[0-9]\+")

# check if the provided version is bigger than the current one. in cmake we allow it to be equal
if version_lte "${version}" "${git_version}"; then
  echo "error: ${version} is not bigger than currently tagged git version ${git_version}"
  exit 1
fi

if version_lt "${version}" "${cmake_version}"; then
  echo "error: ${version} is not bigger or equal than currently specified version in CMakeLists.txt ${cmake_version}"
  exit 1
fi

# update the version in cmake
sed -i -E "s/^(project\(ommpfritt VERSION )[0-9]+\.[0-9]+\.[0-9]+/\1${version}/" CMakeLists.txt

# commit and make an annotated tag.
git commit CMakeLists.txt -m "Bump version to ${version}"
git tag -a "v${version}" -m "Release version ${version}"

# push
echo "now you can run 'git push' and  'git push --tags'"

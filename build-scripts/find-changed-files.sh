#!/usr/bin/env bash

if [ "$1" = "all" ]; then
  files="$(find 'src' 'test')"
else
  files="$(git diff --name-only --diff-filter=d origin/master..HEAD)"
fi
files=$(grep -v 'src/external' <<< "$files")
files=$(grep -v 'test/external' <<< "$files")
files=$(grep -v 'test/unit' <<< "$files")
files=$(grep '\.\(cpp\|h\)$' <<< "$files")
echo "$files"

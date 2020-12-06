#!/usr/bin/env bash

files=$(git diff --name-only --diff-filter=d origin/master..HEAD)
files=$(grep -v 'src/external' <<< $files)
files=$(grep -v 'test/external' <<< $files)
files=$(grep -v 'test/unit' <<< $files)
files=$(grep '\.\(cpp\|h\)$' <<< $files)
echo $files

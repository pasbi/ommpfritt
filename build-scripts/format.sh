#!/usr/bin/env bash

cd $(dirname $0)/..

files=$(find src -path src/external -prune -false -o -type f \( -iname \*.cpp -o -iname \*.h \))

case "$1" in
"check")
  bad_files=0
  for f in $files; do
    d=$(diff --old-line-format="%5dn < %L" --new-line-format="%5dn > %L" --unchanged-line-format="" $f <(clang-format $f))
    if [ $? -ne 0 ]; then
      echo "ERROR: $f has wrong format!"
      echo -e "$d"
      bad_files=$((bad_files + 1))
    fi
  done
  echo "There were $bad_files bad files."
  if [ $bad_files -eq 0 ]; then
    exit 0
  else
    exit 1
  fi
;;
"apply")
  for f in $files; do
    clang-format -i $f
  done
;;
esac


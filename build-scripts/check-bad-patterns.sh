#!/usr/bin/env bash

src_dir="$(dirname "$0")/../src"
fail=0

function run-check {
  echo "#================================"
  echo "# running $1"
  echo "#================================"
  if ! "$1"; then
    fail=1
    echo "Fail."
  else
    echo "Success."
  fi
  echo ""
}

function check-size_t {
  if grep -nP '\b(?<!std::)size_t\b' --exclude-dir=external -R "$src_dir"; then
    echo 'Fail: Found one or more unqualified size_t.'
    echo 'Please replace it with std::size_t.'
    # Note that this script will also complain about variables named size_t,
    # type aliases named size_t, etc. or comments that contain the string
    # "size_t".
    # Its probably a good idea not chose another name for variables and
    # aliases, etc.
    # For comments, presumably the string "std::size_t" is more descriptive,
    # hence, consider to replace these occurences, too.'
    return 1
  else
    return 0
  fi
}

function check-system-includes {
  if grep -nP '^\s*#include <(?!(2geom|poppler|KF5)).*\.h>' \
     --exclude-dir=external -R "$src_dir"
  then
    echo 'Fail: do not include system libaries with .h suffix.'
    return 1
  else
    return 0
  fi
}

function check-local-includes {
  local_includes="$(grep -nP '^\s*#include ".*"' -R "$src_dir" \
                    --exclude-dir=external \
                    --exclude=registers.cpp)"
  local_includes="$(echo "$local_includes" | grep -vP '#include ".*\.h"')"
  local_includes="$(echo "$local_includes" | grep -vP '#include "external/.*\.hpp"')"
  if [ -z "$local_includes" ]; then
    return 0
  else
    echo 'Fail: do not include system libraries with angle brackets <>.'
    echo "$local_includes"
    return 1
  fi
}

function check-qdebug-includes {
  if grep -nP '^\s*#include .QDebug.' \
    --exclude-dir=external \
    --exclude="logging.h" \
    -R "$src_dir"
  then
    echo 'Fail: avoid including QDebug, use "logging.h" instead.'
    return 1
  else
    return 0
  fi
}

function check-qdebug-usage {
  if grep -niP 'qdebug|qinfo|qwarning|qcritical|qfatal' \
    --exclude-dir=external \
    --exclude="logging.h" \
    --exclude="logging.cpp" \
    -R "$src_dir"
  then
    echo 'Fail: avoid including QDebug, use "logging.h" instead.'
    return 1
  else
    return 0
  fi
}

run-check check-size_t
run-check check-system-includes
run-check check-local-includes
run-check check-qdebug-includes
run-check check-qdebug-usage

if [ "$fail" -eq 0 ]; then
  echo "No bad patterns detected."
  exit 0
else
  echo "At least one bad pattern detected."
  exit 1
fi

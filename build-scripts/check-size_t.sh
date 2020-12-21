#!/usr/bin/env bash

pattern='\b(?<!std::)size_t\b'
dir="$(dirname "$0")/../src"
if grep -P "$pattern" --exclude-dir=external -R "$dir"; then
  echo 'Fail: Found one or more unqualified `size_t`.'
  echo 'Please replace it with `std::size_t`.'
  echo 'Note that this script will also complain about variables named '
  echo '`size_t`, type aliases named `size_t`, etc. or comments that contain '
  echo 'the string "size_t".'
  echo 'Its probably a good idea not chose another name for variables and '
  echo 'aliases, etc.'
  echo 'For comments, presumably the string "std::size_t" is more descriptive,'
  echo 'hence, consider to replace these occurences, too.'
  exit 1
else
  echo 'Success: Found zero unqualified `size_t`.'
  exit 0
fi

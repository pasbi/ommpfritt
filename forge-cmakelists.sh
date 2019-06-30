#!/usr/bin/env bash

forge() {
  pushd $1 > /dev/null
  echo "" > CMakeLists.txt

  echo "file(GLOB SOURCES" >> CMakeLists.txt
  for cpp in *.cpp; do
    [ -e "$cpp" ] || continue
    echo "  ${cpp}" >> CMakeLists.txt
  done
  echo ")" >> CMakeLists.txt
  echo "" >> CMakeLists.txt

  echo "file(GLOB HEADERS" >> CMakeLists.txt
  for h in *.h; do
    [ -e "$h" ] || continue
    echo "  ${h}" >> CMakeLists.txt
  done
  echo ")" >> CMakeLists.txt
  echo "" >> CMakeLists.txt

  for d in *; do
    if [ -d "$d" ]; then
      echo "add_subdirectory($d)" >> CMakeLists.txt
      forge $d
    fi
  done
  echo -e "" >> CMakeLists.txt
  echo "target_sources(libommpfritt PRIVATE \${SOURCES} \${HEADERS})" >> CMakeLists.txt
  echo -e "" >> CMakeLists.txt
  popd > /dev/null
}

forge $(dirname $0)/src


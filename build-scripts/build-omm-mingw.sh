#!/usr/bin/env bash

# "Run this script inside MinGW64 shell"

# set -e
cd "$(dirname "$0")/.."

pacman --noconfirm -S \
    mingw-w64-x86_64-ninja \
    mingw-w64-x86_64-qt5 \
    mingw-w64-x86_64-poppler \
    mingw-w64-x86_64-python \
    mingw-w64-x86_64-pybind11 \
    mingw-w64-x86_64-kitemmodels-qt5 \
    mingw-w64-x86_64-openssl \
    mingw-w64-x86_64-libffi \
    mingw-w64-x86_64-zlib \
    mingw-w64-x86_64-python-pytest \
    mingw-w64-x86_64-dlfcn \
    mingw-w64-x86_64-nsis \
    make

QT_QM_PATH=/mingw64/share/qt5/translations/
# PYTHON_INSTALL_LOCATION="$HOME"

build_directory="build"
echo "CONFIGURE::"
cmake -G"Unix Makefiles" \
       -DCMAKE_BUILD_TYPE=Release \
       -DQT_QM_PATH="$QT_QM_PATH" \
       -S . \
       -B "$build_directory"
cat $build_directory/CMakeFiles/CMakeOutput.log
echo "BUILD::"
cmake --build "$build_directory" --target package

# -DCMAKE_CXX_FLAGS='-I/c/msys64/mingw64/include/QtCore/ -I/c/msys64/mingw64/include/QtGui/ -I/c/msys64/mingw64/include/QtWidgets/ -I/c/msys64/mingw64/include/python3.8/ -I/c/msys64/mingw64/include/QtSvg/'
# -DCMAKE_PREFIX_PATH="$PYTHON_INSTALL_LOCATION"


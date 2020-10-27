#!/usr/bin/env bash

# "Run this script inside MinGW64 shell"

# set -e
cd "$(dirname "$0")/.."

pacman --noconfirm -S \
    make \
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
    mingw-w64-x86_64-boost \
    mingw-w64-x86_64-double-conversion \
    mingw-w64-x86_64-gsl \
    mingw-w64-x86_64-cython \
    mingw-w64-x86_64-gtk3 \
    mingw-w64-x86_64-gtkmm3

git clone https://gitlab.com/inkscape/lib2geom
git checkout 37876ed4
echo "CONFIGURE LIB2GEOM:"
cmake -G"Unix Makefiles" \
  -B build-lib2geom \
  -S lib2geom \
  -DCMAKE_INSTALL_PREFIX=install-lib2geom \
  -D2GEOM_TESTING=OFF

echo "BUILD LIB2GEOM:"
cmake --build build-lib2geom --target install

QT_QM_PATH=/mingw64/share/qt5/translations/
# PYTHON_INSTALL_LOCATION="$HOME"

echo "Configure omm"
cmake -G"Unix Makefiles" \
       -DCMAKE_BUILD_TYPE=Release \
       -DQT_QM_PATH="$QT_QM_PATH" \
       -S . \
       -B build \
       -DCMAKE_PREFIX_PATH="install-lib2geom/lib/cmake"

echo "Build omm"
cmake --build "$build_directory" --target package

echo "> ls"
ls
echo "> ls build"
ls build
echo "> find"
find

# -DCMAKE_CXX_FLAGS='-I/c/msys64/mingw64/include/QtCore/ -I/c/msys64/mingw64/include/QtGui/ -I/c/msys64/mingw64/include/QtWidgets/ -I/c/msys64/mingw64/include/python3.8/ -I/c/msys64/mingw64/include/QtSvg/'
# -DCMAKE_PREFIX_PATH="$PYTHON_INSTALL_LOCATION"


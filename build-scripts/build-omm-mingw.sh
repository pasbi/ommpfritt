#!/usr/bin/env bash

# "Run this script inside MinGW64 shell"

set -e
cd "$(dirname "$0")/.."

echo "INSTALLED PACKAGES:"
pacman -Q
echo "END INSTALLED PACKAGES."

echo "Install lib2geom ..."
git clone https://gitlab.com/inkscape/lib2geom
pushd lib2geom
git checkout 37876ed4
popd
echo "configure lib2geom:"
cmake -GNinja \
  -B build-lib2geom \
  -S lib2geom \
  -DCMAKE_INSTALL_PREFIX=install-lib2geom \
  -D2GEOM_TESTING=OFF

echo "build lib2geom:"
cmake --build build-lib2geom --target install

QT_QM_PATH=/mingw64/share/qt5/translations/

echo "Configure omm"
cmake -GNinja \
       -DCMAKE_BUILD_TYPE=Release \
       -DQT_QM_PATH="$QT_QM_PATH" \
       -S . \
       -B build \
       -DCMAKE_PREFIX_PATH="install-lib2geom/lib/cmake" \
       -DCMAKE_INSTALL_PREFIX=install

echo "Build omm"
export PYTHONHOME=/mingw64/
cmake --build "build" --target package --parallel 4

# -DCMAKE_CXX_FLAGS='-I/c/msys64/mingw64/include/QtCore/ -I/c/msys64/mingw64/include/QtGui/ -I/c/msys64/mingw64/include/QtWidgets/ -I/c/msys64/mingw64/include/python3.8/ -I/c/msys64/mingw64/include/QtSvg/'
# -DCMAKE_PREFIX_PATH="$PYTHON_INSTALL_LOCATION"

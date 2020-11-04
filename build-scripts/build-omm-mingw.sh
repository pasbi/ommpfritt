#!/usr/bin/env bash

# "Run this script inside MinGW64 shell"

set -e
cd "$(dirname "$0")/.."

echo "Installing required packages ..."
pacman --noconfirm --needed -S \
    make \
    python \
    mingw-w64-x86_64-ninja \
    mingw-w64-x86_64-qt5 \
    mingw-w64-x86_64-poppler \
    mingw-w64-x86_64-python \
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

echo "INSTALLED PACKAGES:"
pacman -Q
echo "END INSTALLED PACKAGES."

if true; then
echo "Install lib2geom ..."
[ -d lib2geom ] || git clone https://gitlab.com/inkscape/lib2geom
pushd lib2geom
# we need more depth to check out that commit (next line)
git checkout 37876ed4
popd
echo "configure lib2geom:"
cmake -G"Unix Makefiles" \
  -B build-lib2geom \
  -S lib2geom \
  -DCMAKE_INSTALL_PREFIX=install-lib2geom \
  -D2GEOM_TESTING=OFF

echo "build lib2geom:"
cmake --build build-lib2geom --target install
fi

QT_QM_PATH=/mingw64/share/qt5/translations/
# PYTHON_INSTALL_LOCATION="$HOME"

if true; then
echo "Configure omm"
rm -rf build/libommpfritt_autogen build/qm build/qrc_resources_cli.cpp \
       build/ommpfritt-cli_autogen build/ommpfritt_unit_tests_autogen \
       build/ommpfritt_autogen build/qrc_resources.cpp
cmake -GNinja \
       -DCMAKE_BUILD_TYPE=Release \
       -DQT_QM_PATH="$QT_QM_PATH" \
       -S . \
       -B build \
       -DCMAKE_PREFIX_PATH="install-lib2geom/lib/cmake" \
       -DCMAKE_INSTALL_PREFIX=install
fi

echo "Build omm"
export PYTHONHOME=/mingw64/
cmake --build "build" --target package

# -DCMAKE_CXX_FLAGS='-I/c/msys64/mingw64/include/QtCore/ -I/c/msys64/mingw64/include/QtGui/ -I/c/msys64/mingw64/include/QtWidgets/ -I/c/msys64/mingw64/include/python3.8/ -I/c/msys64/mingw64/include/QtSvg/'
# -DCMAKE_PREFIX_PATH="$PYTHON_INSTALL_LOCATION"


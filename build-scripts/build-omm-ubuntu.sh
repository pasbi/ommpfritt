#!/usr/bin/env bash

set -e

cd $(dirname $0)/..

dist="$(lsb_release -cs)"
repo="$(basename -s .git `git config --get remote.origin.url`)"
echo "Build $repo on Ubuntu $dist".

echo "installing dependencies ..."
sudo apt-get clean
sudo apt update -y

# the mysql packages make a lot of trouble on xenial
#  https://askubuntu.com/questions/773287/how-to-repair-corrupt-package-installation-mysql/773441
# we don't need them, so removing is the quickest and cleanest way to avoid these troubles.
sudo apt remove mysql-client-5.7 mysql-server-5.7
sudo apt upgrade -y
sudo apt install -y ninja-build zlib1g-dev libssl-dev libffi-dev \
                    libgl-dev python3-dev
case "$dist" in
"xenial" | "bionic")
  sudo apt-add-repository -y ppa:ubuntu-toolchain-r/test
  sudo apt-add-repository -y ppa:beineri/opt-qt-5.12.3-"$dist"
  sudo apt update -y
  sudo apt install -y g++-8
  sudo apt install -y qt512tools qt512translations qt512svg \
                      qt512base qt512imageformats

  # pytest is required to build pybind11
  sudo apt install -y python3-pip
  sudo pip3 install pytest

  CXX_COMPILER=g++-8
  C_COMPILER=gcc-8
  QT_PREFIX="/opt/qt512"
  QT_QM_PATH="/opt/qt512/translations/"
  export PATH=/opt/qt512/bin:$PATH

  wget https://www.python.org/ftp/python/3.7.0/Python-3.7.0.tar.xz
  tar xf Python-3.7.0.tar.xz
  pushd Python-3.7.0
  ./configure --prefix=/usr
  sudo make altinstall -j2
  popd

  # pytest is required to build pybind11
  sudo python3.7 -m pip install pytest

  # Download cmake binaries
  wget https://github.com/Kitware/CMake/releases/download/v3.14.5/cmake-3.14.5-Linux-x86_64.tar.gz
  tar xf cmake-3.14.5-Linux-x86_64.tar.gz
  cmake=$(pwd)/cmake-3.14.5-Linux-x86_64/bin/cmake

  PYBIND11_VERSION="2.2.4"
  wget https://github.com/pybind/pybind11/archive/v${PYBIND11_VERSION}.tar.gz
  tar xf v${PYBIND11_VERSION}.tar.gz
  pybind11_DIR=pybind11-${PYBIND11_VERSION}

  mkdir -p $pybind11_DIR/build
  pushd $pybind11_DIR/build
  $cmake ..
  sudo make install
  popd

  ;;
"disco" | "eoan")
  sudo apt install -y g++ gcc pybind11-dev cmake
  sudo apt install -y qtbase5-dev qtchooser qt5-qmake \
                      qtbase5-dev-tools qt5-default \
                      libqt5svg5-dev qttools5-dev \
                      qttools5-dev
  CXX_COMPILER=g++
  C_COMPILER=gcc
  cmake=cmake
  QT_PREFIX=""
  QT_QM_PATH="/usr/share/qt5/translations/"
  ;;
esac

sudo apt install -y libpoppler-qt5-dev libkf5itemmodels-dev

if [ -d build ]; then
  rm -r build
fi
mkdir build

cd build
$cmake -GNinja \
       -DCMAKE_BUILD_TYPE=Release \
       -DCMAKE_CXX_COMPILER="$CXX_COMPILER" \
       -DCMAKE_C_COMPILER="$C_COMPILER" \
       -DQT_QM_PATH="$QT_QM_PATH" \
       -DCMAKE_PREFIX_PATH="$QT_PREFIX" \
       -DCMAKE_INSTALL_PREFIX=/usr \
       ..

ninja


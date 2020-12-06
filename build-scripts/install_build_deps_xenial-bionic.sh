#!/usr/bin/env bash

if ! [[ "$1" =~ ^(xenial|bionic)$ ]]; then
  echo "First argument must be 'xenial' or 'bionic' (was '$1')."
  exit 1
fi

sudo apt-get clean
sudo apt-get remove -y mysql-client-5.7 mysql-server-5.7
sudo apt-add-repository -y ppa:deadsnakes/ppa
sudo apt-add-repository -y ppa:ubuntu-toolchain-r/test
sudo apt-add-repository -y "ppa:beineri/opt-qt-5.12.3-$1"
sudo apt-get update -y -qq
sudo apt-get install -y \
  g++-9 qt512tools qt512translations qt512svg qt512base qt512imageformats \
  python3.7-dev ninja-build zlib1g-dev libssl-dev libffi-dev libgl-dev \
  libboost-all-dev libdouble-conversion-dev libgsl-dev libcairo2-dev \
  libpoppler-qt5-dev libkf5itemmodels-dev imagemagick
wget https://github.com/Kitware/CMake/releases/download/v3.14.5/cmake-3.14.5-Linux-x86_64.tar.gz
tar xf cmake-3.14.5-Linux-x86_64.tar.gz
export CXX_COMPILER=g++-9
export C_COMPILER=gcc-9
export QT_PREFIX="/opt/qt512"
export QT_QM_PATH="/opt/qt512/translations/"
export PATH="/opt/qt512/bin:$PATH"
export cmake="$(pwd)/cmake-3.14.5-Linux-x86_64/bin/cmake"

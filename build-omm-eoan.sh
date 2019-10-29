#!/usr/bin/env bash

set -e  # abort script on error

echo 'deb http://archive.ubuntu.com/ubuntu/ eoan main universe' | sudo tee -a /etc/apt/sources.list
sudo awk '!a[$0]++' /etc/apt/sources.list
sudo apt update && sudo apt upgrade -y
sudo apt install git ninja-build cmake vim -y

#env: global:

# before install
sudo apt-get update -qq
sudo apt-get -y upgrade
sudo apt-get install -y zlib1g-dev libssl-dev libffi-dev
sudo apt-get -y install libgl-dev python3-dev libpoppler-qt5-dev pybind11-dev
sudo apt-get -y install qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools
sudo apt-get -y install qt5-default libqt5svg5-dev qttools5-dev qttools5-dev-tools
sudo apt-get -y install libkf5itemmodels-dev

# script:
git clone https://github.com/pasbi/ommpfritt.git
cd ommpfritt  # not required in travis
mkdir build
cd build
cmake -G"Ninja" \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_COMPILER=g++ \
      -DQT_QM_PATH=/usr/share/qt5/translations/ \
      ..
ninja

# after success:
touch success


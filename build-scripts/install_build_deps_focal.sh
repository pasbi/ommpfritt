#!/usr/bin/env bash

sudo apt-get clean
sudo apt-get remove mysql-client-5.7 mysql-server-5.7
sudo apt-get update -y -qq
sudo apt-get install -y g++ gcc clang cmake qtbase5-dev qtchooser \
  qt5-qmake qtbase5-dev-tools qt5-default libqt5svg5-dev qttools5-dev \
  qttranslations5-l10n qttools5-dev ninja-build zlib1g-dev libssl-dev \
  libffi-dev libgl-dev python3-dev libboost-all-dev libdouble-conversion-dev \
  libgsl-dev libcairo2-dev libpoppler-qt5-dev libkf5itemmodels-dev imagemagick

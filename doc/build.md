# Building ommpfritt

## Overview

*omm* uses the [cmake](cmake.org) build system generator.
The sources contain

-    implementation (`*.cpp`), header (`*.h`) and interface files (`*.ui`).
-    configuration files (`*.cfg`)
-    translation files (`*.ts`)
-    resource files (`*.qrc`)
-    list files (`*.lst`)
-    cmake files (`CMakeLists.txt`)

Some implementation, header and resource files will be generated during the
build process.
That is an automated process and should not require any manual intervention.
The process is described in [contribute.md](contribute.md) and in
[`CMakeLists.txt`](../CMakeLists.txt).

## Build environment

*omm* is known to successfully compile and run on Arch Linux, Ubuntu Linux and 
Windows/MSYS2.
Neither building nor running on Mac has ever been tested, but it should work in theory!
We recommend using **g++8.x** or later. g++7.x or earlier will not work.
Building with a recent **clang** is possible (tested with 8.0.0).
A very recent Visual Studio might work, C++17 support is required.

### cmake
**cmake 3.14** or later is required.

### Dependencies

-   `Qt5.12` or later
-   `python3.7`
-   `poppler`
-   `opengl`
-   `pybind11`

## Examples

### Arch

1.   Install the dependencies using `pacman`.
They should be available in the required version.

```bash
pacman -S gcc cmake python3 poppler-qt5 qt5-base qt5-imageformats qt5-svg qt5-translations qt5-tools
```

2.   Install [`lib2geom-git`](https://aur.archlinux.org/packages/lib2geom-git/) from AUR.
3.   Clone and install [`pybind11`](https://github.com/pybind/pybind11).
`pacman` also comes with `pybind11`-packages, however `pybind11` is developing
quickly.
Maybe you have luck with the packages but I recommend installing it from
github.
4.   get omm: `git clone https://github.com/pasbi/ommpfritt`
5.   configure it:

```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DQT_QM_PATH=/usr/share/qt/translations \
      ..
```

6.   build it `make`, this may take a few minutes.
7.   start it: `./ommpfritt`

### Ubuntu 16.04 Xenial

[There is a build script](../build-scripts/build-omm-ubuntu.sh) which supports
Xenial.
Use it with care, it requires root and might damage your system.
You are responsible!

If your ubuntu is more recent, you might be able to skip some of the steps.
E.g., if you already have python3.7, Qt5.12, g++8, etc. installed, you don't
need to install it again! (see e.g. instructions below for 19.04 Disco)

1.   install g++-8 and Qt:
```bash
sudo apt-add-repository ppa:ubuntu-toolchain-r/test
sudo apt-add-repository ppa:beineri/opt-qt-5.12.3-xenial
sudo apt update
sudo apt install qt512tools qt512translations qt512svg qt512base qt512imageformats
sudo apt install libgl-dev python3-dev libpoppler-qt5-dev g++-8
```

2.  make the new Qt visible: `export PATH=/opt/qt512/bin/:$PATH`

3.  get recent [cmake binaries](cmake.org/download/) and unpack them to some
directory.

4.  get recent python:
```bash
wget https://www.python.org/ftp/python/3.7.0/Python-3.7.0.tar.xz
tar xvf Python-3.7.0.tar.xz
cd Python-3.7.0
./configure
sudo make altinstall
```

5.   clone and install [pybind11](github.com/pybind/pybind11)

6.   get [ommpfritt](github.com/pasbi/ommpfritt)

7.   configure it:
```bash
cd ommpfritt
mkdir build
cd build
/path/to/recent/cmake/bin/cmake -DCMAKE_BUILD_TYPE=Release \
                                -DCMAKE_CXX_COMPILER=g++-8 \
                                -DQT_QM_PATH=/opt/qt512/translations \
                                -DCMAKE_PREFIX_PATH=/opt/qt512 \
                                ..
```

8.   build it: `make`

9.   run it: `./ommpfritt`

### Ubuntu 19.04 Disco Dingo

[There is a build script](../build-scripts/build-omm-ubuntu.sh) which supports
Disco.
Use it with care, it requires root and might damage your system.
You are responsible!

#### Install dependencies

```bash
sudo apt install g++
sudo apt install cmake
sudo apt install pybind11-dev
sudo apt install qtbase5-dev libqt5svg5-dev qttools5-dev  # Qt 5
sudo apt install libpoppler-qt5-dev
sudo apt install libkf5itemmodels-dev
```

#### Configure, build and run

```bash
cd ommpfritt
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_CXX_COMPILER=g++-8 \
      -DQT_QM_PATH=/usr/share/qt5/translations/ \
      ..
make
./ommpfritt
```


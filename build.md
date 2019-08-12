# Building ommpfritt

## General Notes

omm is very recent software. You need recent build tools and updated 3rd party libraries to build:

### Compiler
We recommend using **g++8.x** or later. g++7.x or earlier will not work.
Building with a recent **clang** is possible (tested with 8.0.0).
MSVC is not recommended until it implements c++17 reasonably well.

### cmake
**cmake 3.14** and later works. cmake 3.5 is too old.

### Windows
We recommend using a MSYS/MinGW environment to build omm on windows. Make sure that you meet all the requirements listed above.

### Linux
No extra notes. It just works. See [Examples](#examples).

### Mac
Not tested, no problems expected.

### Dependencies

 - Qt5.12 or later
 - python3.7
 - poppler
 - opengl
 - pybind11

## Examples

### Arch

1. Install the dependencies using pacman. They should be available in the required version.

```
pacman -S gcc cmake python3 poppler-qt5 qt5-base qt5-imageformats qt5-svg qt5-translations qt5-tools
```

2. Clone and install https://github.com/pybind/pybind11. pacman also comes with `pybind11`-packages, however pybind11 is developing quickly. Maybe you have luck with the packages but I recommend installing it from github.
3. get omm: `git clone https://github.com/pasbi/ommpfritt`
4. configure it:

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DQT_QM_PATH/usr/share/qt/translations \
      ..
```

5. build it `make`, this may take a few minutes.
6. start it: `./ommpfritt`

### Ubuntu 16.04 Xenial

If your ubuntu is more recent, you might be able to skip some of the steps. E.g., if you already have python3.7, Qt5.12, g++8, etc. installed, you don't need to install it again! (see e.g. instructions below for 19.04 Disco)

1. install g++-8 and Qt:
```
sudo apt-add-repository ppa:ubuntu-toolchain-r/test
sudo apt-add-repository ppa:beineri/opt-qt-5.12.3-xenial
sudo apt update
sudo apt install qt512tools qt512translations qt512svg qt512base qt512imageformats
sudo apt install libgl-dev python3-dev libpoppler-qt5-dev g++-8
```

2. make the new Qt visible: `export PATH=/opt/qt512/bin/:$PATH`
3. get recent cmake binaries from https://cmake.org/download/ and unpack them to some directory.
4. get recent python:
```
wget https://www.python.org/ftp/python/3.7.0/Python-3.7.0.tar.xz
tar xvf Python-3.7.0.tar.xz
cd Python-3.7.0
./configure
sudo make altinstall
```
5. Clone and install https://github.com/pybind/pybind11.
6. get ommpfritt: `git clone https://github.com/pasbi/ommpfritt`
7. configure it:
```
cd ommpfritt
mkdir build
cd build
/path/to/recent/cmake/bin/cmake -DCMAKE_BUILD_TYPE=Release \
                                -DCMAKE_CXX_COMPILER=g++-8 \
                                -DQT_QM_PATH=/opt/qt512/translations \
                                -DCMAKE_PREFIX_PATH=/opt/qt512 \
                                ..
```
8. build it: `make`
9. run it: `./ommpfritt`

### Ubuntu 19.04 Disco Dingo

At the time of writing this release of ubuntu provides all dependencies for
ommpfritt at the right versions straight from the standard repositories.
As these installation instructions were not put together on a completely fresh
system, some necessary package install commands might be missing below (in that case
you could speculatively try `sudo apt install build-essential python3-dev` and/or follow up to the error messages cmake gives you, which are usually very helpful - note down the missing file/library and google e.g. `ubuntu disco package <the-missing-thing>` to find out which package to `sudo apt install <the-required-package>`).

#### Install dependencies

```bash
sudo apt install g++
sudo apt install cmake
sudo apt install pybind11-dev
sudo apt install qtbase5-dev libqt5svg5-dev qttools5-dev  # Qt 5
sudo apt install libpoppler-qt5-dev
```

#### Configure, build and run

```bash
cd ommpfritt
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_CXX_COMPILER=g++-8 \
      -DQT_QM_PATH=/usr/share/qt5/translations/
      ..
make
./ommpfritt
```

### Contributing to the build instructions

Pull requests are welcome (both for new and existing platforms)!

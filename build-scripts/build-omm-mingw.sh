echo "Run this script inside MinGW64 shell"

cd $(dirname $0)/..
repo="$(basename -s .git `git config --get remote.origin.url`)"

echo "Build $repo on MinGW64."

QT_QM_PATH=/mingw64/share/qt5/translations/
PYTHON_INSTALL_LOCATION="$HOME"

if [ ! -d build ]; then
  mkdir build
fi
cd build
cmake -G"Unix Makefiles" \
       -DCMAKE_BUILD_TYPE=Release \
       -DQT_QM_PATH="$QT_QM_PATH" \
       -DCMAKE_CXX_FLAGS='-I/c/msys64/mingw64/include/QtCore/ -I/c/msys64/mingw64/include/QtGui/ -I/c/msys64/mingw64/include/QtWidgets/ -I/c/msys64/mingw64/include/python3.8/ -I/c/msys64/mingw64/include/QtSvg/' \
       -DCMAKE_PREFIX_PATH="$PYTHON_INSTALL_LOCATION" \
       ..

make -j4
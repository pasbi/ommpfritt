#!/usr/bin/env bash

echo -e "\n\n\n"
pwd
ls
echo -e "\n\n\n"
find .
echo -e "\n\n\n"
ls ..
echo -e "\n\n\n"

mkdir -p ./appdir/usr/bin
cp ommpfritt ./appdir/usr/bin/ # FIXME

mkdir -p ./appdir/usr/share/applications
cp ommpfritt.desktop ./appdir/usr/share/applications/ # FIXME

mkdir -p ./appdir/usr/share/icons/hicolor/scalable/apps
touch ./appdir/usr/share/icons/hicolor/scalable/apps/ommpfritt.svg # FIXME

mkdir -p ./appdir/usr/lib
find /usr/lib/ -type d -name 'python3.7' -exec cp -r {} ./appdir/usr/lib/ \;

find appdir/usr/lib/python3.7
wget -c -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"

chmod a+x linuxdeployqt-continuous-x86_64.AppImage
wget -c "https://github.com/AppImage/AppImageKit/releases/download/continuous/AppRun-x86_64" -O ./appdir/AppRun
chmod +x appdir/AppRun
./linuxdeployqt-continuous-x86_64.AppImage appdir/usr/share/applications/*.desktop -appimage \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_ctypes.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_posixsubprocess.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_opcode.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_lsprof.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_bz2.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_dbm.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_testcapi.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/binascii.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/mmap.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_codecs_cn.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/select.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_testbuffer.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_sha512.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_sha3.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_json.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/pyexpat.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/unicodedata.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_bisect.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/audioop.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_asyncio.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_codecs_tw.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_testimportmultiple.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/resource.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_md5.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_codecs_jp.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_tkinter.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_random.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_sqlite3.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_uuid.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/termios.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/parser.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_sha256.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_ssl.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_datetime.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_curses.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/syslog.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_queue.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_gdbm.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_elementtree.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_struct.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_codecs_iso2022.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/fcntl.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_pickle.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/nis.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_sha1.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/xxlimited.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/zlib.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/readline.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_testmultiphase.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_hashlib.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_csv.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/spwd.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_crypt.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/ossaudiodev.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_blake2.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/array.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_heapq.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/cmath.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/math.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_ctypes_test.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_curses_panel.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_codecs_kr.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_multibytecodec.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/grp.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_xxtestfuzz.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_multiprocessing.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_socket.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_decimal.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_lzma.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_contextvars.*.so) \
  -executable=$(readlink -f appdir/usr/lib/python3.7/lib-dynload/_codecs_hk.*.so)

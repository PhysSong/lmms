#!/usr/bin/env bash
mkdir build
cd build

if [ $QT5 ]; then
	unset QTDIR QT_PLUGIN_PATH LD_LIBRARY_PATH
	source /opt/qt59/bin/qt59-env.sh
fi

cmake -DUSE_WERROR=ON -DCMAKE_INSTALL_PREFIX=../target $CMAKE_FLAGS ..

make -j4 install
make appimage
PACKAGE="$(ls lmms-*.AppImage)"
echo "Uploading $PACKAGE to transfer.sh..."
curl --upload-file "$PACKAGE" "https://transfer.sh/$PACKAGE" || true

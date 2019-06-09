#!/usr/bin/env bash
mkdir build
cd build

export CMAKE_OPTS="$CMAKE_FLAGS -DUSE_WERROR=ON"
../cmake/build_mingw32.sh

make -j4

make package
PACKAGE="$(ls lmms-*.exe)"
echo "Uploading $PACKAGE to transfer.sh..."
curl --upload-file "$PACKAGE" "https://transfer.sh/$PACKAGE" || true

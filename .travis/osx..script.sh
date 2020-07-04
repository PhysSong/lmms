#!/usr/bin/env bash
mkdir build
cd build

if [ $QT5 ]; then
        # Workaround; No FindQt5.cmake module exists
        export CMAKE_PREFIX_PATH="$(brew --prefix qt)"
fi

cmake -DCMAKE_INSTALL_PREFIX=../target/ $CMAKE_FLAGS -DUSE_WERROR=OFF ..

make -j4 install
make dmg
PACKAGE="$(ls lmms-*.dmg)"

echo "Uploading $PACKAGE to transfer.sh..."
curl -F "file=@$PACKAGE" "https://file.io" || true

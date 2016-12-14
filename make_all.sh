#!/bin/sh
echo "******* ${MAKE_DIR} START ["`date`"] *******"

build_root=$(pwd)

# DIR CHECK
if [ -d build ]; then
	rm -rf build
fi

mkdir build

cd build

echo "## Cmake Start ##"

cmake -DCMAKE_TOOLCHAIN_FILE=${build_root}/toolchain.cmake ../

echo "## Make Start ##"

make -j4

echo "******* ${MAKE_DIR} END  ["`date`"] *******"

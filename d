#!/bin/sh

set -e
if [ -n "$1" ]; then
    PARAL=$1
else
    PARAL=8
fi

rm -rf ./build
mkdir -p ./build/xel
mkdir -p ./build/engine

if [ ! -d logs ];then
mkdir logs
fi

cd build/xel
cmake -DCMAKE_BUILD_TYPE=Debug -Wno-dev ../../CppSample
make -j ${PARAL}
make test
make install
cd ../..

cd build/engine
cmake -DCMAKE_BUILD_TYPE=Debug -Wno-dev ../../Engine
make -j ${PARAL}
make test
make install
cd ../..

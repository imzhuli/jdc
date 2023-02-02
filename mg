#!/bin/sh
set -e
if [ -n "$1" ]; then
    PARAL=$1
else
    PARAL=8
fi

cd build/engine
rm -rf "./.local/"
make -j ${PARAL}
make test
make install
cd ../..

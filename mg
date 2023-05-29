#!/bin/sh
set -e

cd build/engine
rm -rf "./.local/"
make -j ${PARAL}
make test
make install
cd ../..

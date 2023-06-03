#!/bin/sh
set -e

cd build/xel
make
make test
make install
cd ../..

cd build/engine
rm -rf "./.local/"
make
make test
make install
cd ../..

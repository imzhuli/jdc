#!/bin/sh
set -e

cd build/engine
rm -rf "./.local/"
make
make test
make install
cd ../..

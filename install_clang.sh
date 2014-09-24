#!/bin/sh

wget -c 'http://llvm.org/releases/3.5.0/llvm-3.5.0.src.tar.xz'
wget -c 'http://llvm.org/releases/3.5.0/cfe-3.5.0.src.tar.xz'
tar xf 'llvm-3.5.0.src.tar.xz'
tar xf 'cfe-3.5.0.src.tar.xz'
mv 'cfe-3.5.0.src' 'llvm-3.5.0.src/tools/clang'
mkdir 'build' && cd build
cmake -DCMAKE_BUILD_TYPE=Release ../llvm-3.5.0.src/
make -j2
make install

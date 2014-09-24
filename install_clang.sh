#!/bin/sh

wget -c 'http://llvm.org/releases/3.4.2/cfe-3.4.2.src.tar.gz'
wget -c 'http://llvm.org/releases/3.4.2/llvm-3.4.2.src.tar.gz'
tar xf 'llvm-3.4.2.src.tar.xz'
tar xf 'cfe-3.4.2.src.tar.xz'
mv 'cfe-3.4.2.src' 'llvm-3.4.2.src/tools/clang'
mkdir 'build' && cd build
cmake -DCMAKE_BUILD_TYPE=Release ../llvm-3.4.2.src/
make -j2
make install

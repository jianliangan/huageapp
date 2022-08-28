#!/bin/bash
export NDK=/root/Android/Sdk/ndk/21.4.7075529
export HOST_TAG=linux-x86_64 # adjust to your building host
export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/$HOST_TAG

export CC=$TOOLCHAIN/bin/x86_64-linux-android21-clang
export CXX=$TOOLCHAIN/bin/x86_64-linux-android21-clang++

function build_x86_64
{
  ./configure \
  --prefix=./android/x86_64 \
  --enable-static \
  --enable-pic \
  --disable-opencl \
  --disable-cli \
  --host=x86_64-linux \
  --cross-prefix=$TOOLCHAIN/bin/x86_64-linux-android- \
  --sysroot=$TOOLCHAIN/sysroot \

  make clean
  make
  make install
}

build_x86_64
echo build_x86_64 finished

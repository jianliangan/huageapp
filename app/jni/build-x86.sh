#!/bin/bash
export NDK=/root/Android/Sdk/ndk/21.4.7075529
export HOST_TAG=linux-x86_64 # adjust to your building host
export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/$HOST_TAG

export CC=$TOOLCHAIN/bin/i686-linux-android18-clang
export CXX=$TOOLCHAIN/bin/i686-linux-android18-clang++

function build_x86
{
  ./configure \
  --prefix=./android/x86 \
  --enable-static \
  --enable-pic \
  --disable-opencl \
  --disable-cli \
  --host=i686-linux \
  --cross-prefix=$TOOLCHAIN/bin/i686-linux-android- \
  --sysroot=$TOOLCHAIN/sysroot \
  --extra-cflags=-mno-stackrealign \
  make clean
  make
  make install
}

build_x86
echo build_x86 finished

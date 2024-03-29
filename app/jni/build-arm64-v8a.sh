#!/bin/bash
export NDK=/root/Android/Sdk/ndk/21.4.7075529
export HOST_TAG=linux-x86_64 # adjust to your building host
export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/$HOST_TAG

export CC=$TOOLCHAIN/bin/aarch64-linux-android21-clang
export CXX=$TOOLCHAIN/bin/aarch64-linux-android21-clang++

function build_arm64-v8a
{
  ./configure \
  --prefix=./android/arm64-v8a \
  --enable-static \
  --enable-pic \
  --disable-opencl \
  --disable-cli \
  --host=aarch64-linux \
  --cross-prefix=$TOOLCHAIN/bin/aarch64-linux-android- \
  --sysroot=$TOOLCHAIN/sysroot \

  make clean
  make
  make install
}

build_arm64-v8a
echo build_arm64-v8a finished

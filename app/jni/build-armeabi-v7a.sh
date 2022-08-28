#!/bin/bash
#NDK=/root/Android/Sdk/ndk/21.4.7075529

#!/bin/bash
export NDK=/root/Android/Sdk/ndk/21.4.7075529 # your android ndk folder
export HOST_TAG=linux-x86_64 # adjust to your building host
export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/$HOST_TAG

export CC=$TOOLCHAIN/bin/armv7a-linux-androideabi18-clang # c compiler path
export CXX=$TOOLCHAIN/bin/armv7a-linux-androideabi18-clang++ # c++ compiler path

function build_armeabi-v7a
{
  ./configure \
  --prefix=./android/armeabi-v7a \
  --enable-static \
  --enable-pic \
  --disable-opencl \
  --disable-cli \
  --host=arm-linux \
  --cross-prefix=$TOOLCHAIN/bin/arm-linux-androideabi- \
  --sysroot=$TOOLCHAIN/sysroot \

  make clean
  make
  make install
}

build_armeabi-v7a
echo build_armeabi-v7a finished

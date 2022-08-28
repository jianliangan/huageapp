#!/usr/bin/env bash

chmod a+x build_android_*.sh

# Build arm v6 v7a
./build-armeabi-v7a.sh

# Build arm64 v8a
./build-arm64-v8a.sh

# Build mips
#./build_android_mips.sh

# Build mips64
#./build_android_mips64.sh

# Build x86
./build-x86.sh

# Build x86_64
./build-x86-64.sh

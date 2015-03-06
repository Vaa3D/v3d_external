#!/bin/bash
# 2010-08-07 by Hanchuan Peng
# a convenience script for building the system with cmake
# It will download and build CMake if it's not present,
# then launch the build.

# Run without arguments to see usage.

function download {
    if [[ ! -e $2 ]]; then
        echo "Downloading $1"
        curl -L $1 -o $2
    fi
}

shopt -s expand_aliases;
CMAKE_VERSION=3.1.3
CMAKE_ARGS=
CMAKE_BUILD="Release"

set -eu

KERNEL=(`uname -s | tr [A-Z] [a-z]`)
ARCH=(`uname -m | tr [A-Z] [a-z]`)
case $KERNEL in
    darwin)
        OS=macosx
        ;;
    mingw*)
        OS=windows
        KERNEL=windows
        if [[ $TARGET_CPU == "x64" ]]; then
            ARCH=x86_64
        fi
        ;;
    *)
        OS=$KERNEL
        ;;
esac
case $ARCH in
    arm*)
        ARCH=arm
        ;;
    i386|i486|i586|i686)
        ARCH=x86
        ;;
    amd64|x86-64)
        ARCH=x86_64
        ;;
esac
PLATFORM=$OS-$ARCH
echo "Detected platform \"$PLATFORM\""

while [[ $# > 0 ]]; do
    case "$1" in
        -platform)
            shift
            PLATFORM="$1"
            ;;
        -h5j)
            CMAKE_ARGS="-DUSE_FFMPEG:BOOL=ON -DUSE_X265:BOOL=ON -DUSE_HDF5:BOOL=ON"
            ;;
        -debug)
            CMAKE_BUILD="Debug"
            ;;
        install)
            OPERATION=install
            ;;
        clean)
            OPERATION=clean
            ;;
        clobber)
            OPERATION=clobber
            ;;
        *)
            PROJECTS+=("$1")
            ;;
    esac
    shift
done
echo "Targeting platform \"$PLATFORM\""

if [[ -z ${OPERATION:-} ]]; then
    echo "Usage: build.cmake [-platform <name>] [-h5j] [-debug] <install | clean | clobber>"
    echo "where possible platform names are: linux-x86, linux-x86_64, macosx-x86_64, windows-x86, windows-x86_64, etc."
    echo " -h5j - builds for the Janelia Farm HDF variant. Enables building of FFmpeg and HDF5"
    echo " -debug - Generates a debug build (default is release)"
    echo " clean - removes the current build for platform"
    echo " clobber - cleans, and removes the current cmake directories"
    exit 1
fi

case $OPERATION in
    install)
		if [[ ! -e cmake-$CMAKE_VERSION/bin/cmake ]]; then
			if [[ ! -e cmake-$CMAKE_VERSION ]]; then
				echo "Downloading cmake"
				download http://www.cmake.org/files/v3.1/cmake-$CMAKE_VERSION.tar.gz cmake-$CMAKE_VERSION.tar.gz
				tar xvzf cmake-$CMAKE_VERSION.tar.gz
			fi
			cd cmake-$CMAKE_VERSION
			./configure --prefix=.
			make
			make install
			cd ..
		fi
		if [[ ! -e build_$PLATFORM ]]; then
			mkdir build_$PLATFORM
		fi
		cd build_$PLATFORM
		../cmake-$CMAKE_VERSION/bin/cmake -DCMAKE_BUILD_TYPE:STRING=$CMAKE_BUILD $CMAKE_ARGS ..
		make
        ;;
    clean)
        echo "Cleaning build_$PLATFORM directories"
		if [[ -e build_$PLATFORM ]]; then
			rm -rf build_$PLATFORM
		fi
        ;;
    clobber)
        echo "Cleaning cmake directories"
		if [[ -e cmake-$CMAKE_VERSION ]]; then
			rm -rf cmake-$CMAKE_VERSION
		fi
		if [[ -e build ]]; then
			rm -rf build_$PLATFORM
		fi
        ;;
esac


#!/bin/bash

function download {
    if [[ ! -e $2 ]]; then
        echo "Downloading $1"
        curl -L $1 -o $2
    fi
}

shopt -s expand_aliases;
BUILD_HDF5=0
BOOST_MAJOR_VERSION=1_57
BOOST_VERSION=${BOOST_MAJOR_VERSION}_0
CMAKE_VERSION=2.8.12
CMAKE_ARGS=""
CMAKE_PLATFORM_ARGS=
CMAKE_BUILD="Release"
CMAKE_EXE=""
BUILD_DIR=`pwd`
ROOT_DIR=`pwd`

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
        ;;
    windows-x86_64)
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
            CMAKE_ARGS+="-DUSE_FFMPEG:BOOL=ON -DUSE_X265:BOOL=ON -DUSE_HDF5:BOOL=ON"
            BUILD_HDF5=1
            ;;
        -qt5)
            CMAKE_ARGS+=" -DFORCE_QT4:BOOL=OFF"
            ;;
        -16)
            CMAKE_ARGS+=" -DHIGH_BIT_DEPTH:BOOL=ON"
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
            BUILD_DIR="$1"
            ;;
    esac
    shift
done
echo "Targeting platform \"$PLATFORM\""
echo "Root directory \"$ROOT_DIR\""

if [[ -z ${OPERATION:-} ]]; then
    echo "Usage: build.cmake [-platform <name>] [-h5j] [-qt5] [-debug] <install | clean | clobber>"
    echo "where possible platform names are: linux-x86, linux-x86_64, macosx-x86_64, windows-x86, windows-x86_64, etc."
    echo " -h5j - builds for the Janelia Farm HDF variant. Enables building of FFmpeg, HDF5 and X265(HEVC)"
    echo " -qt5 - build with Qt5 (experimental)"
    echo " -debug - Generates a debug build (default is release)"
    echo " clean - removes the current build for platform"
    echo " clobber - cleans, and removes the current cmake directories"
    exit 1
fi

boost_prefix=$BUILD_DIR/build_$PLATFORM/v3d_main/common_lib
CMAKE_PLATFORM_ARGS="-DBOOST_ROOT:PATH=$boost_prefix "
if [ $PLATFORM = "windows-x86_64" ]; then
    CMAKE_PLATFORM_ARGS+="-DTIFF_INCLUDE_DIR:PATH=$ROOT_DIR/v3d_main/common_lib/include "
    CMAKE_PLATFORM_ARGS+="-DTIFF_LIBRARY:PATH=$ROOT_DIR/v3d_main/common_lib/winlib64/libtiff.lib "
    CMAKE_PLATFORM_ARGS+="-DFFTW_INCLUDE_DIR:PATH=$ROOT_DIR/v3d_main/common_lib/fftw-3.3.4-dll64 "
    CMAKE_PLATFORM_ARGS+="-DFFTW_LIBRARY:PATH=$ROOT_DIR/v3d_main/common_lib/fftw-3.3.4-dll64/libfftw3f-3.lib"
fi

: "${CMAKE_DIR:=""}"

case $OPERATION in
    install)
        # See if the CMAKE_DIR is set
        if [ ! "$CMAKE_DIR" = "" ]; then
            if [[ -e $CMAKE_DIR ]]; then
                CMAKE_EXE="$CMAKE_DIR/bin/cmake"
            fi
        fi

        # If CMAKE_EXE is not set, then either find or build cmake
        if [ "$CMAKE_EXE" = "" ]; then
            if hash cmake 2>/dev/null; then
                CMAKE_EXE="cmake"
            else
        		if [[ ! -e cmake-$CMAKE_VERSION/bin/cmake ]]; then
        			if [[ ! -e cmake-$CMAKE_VERSION ]]; then
        				echo "Downloading cmake"
        				download http://www.cmake.org/files/v3.1/cmake-$CMAKE_VERSION.tar.gz cmake-$CMAKE_VERSION.tar.gz
        				tar xvfz cmake-$CMAKE_VERSION.tar.gz
        			fi
        			cd cmake-$CMAKE_VERSION
        			./configure --prefix=.
        			make
        			make install
        			cd ..
        		fi
                CMAKE_EXE="../cmake-$CMAKE_VERSION/bin/cmake"
            fi
        fi

        echo "Using $CMAKE_EXE"

		if [[ ! -e $BUILD_DIR/build_$PLATFORM ]]; then
			mkdir -p $BUILD_DIR/build_$PLATFORM/v3d_main/common_lib
		fi

        echo $boost_prefix
        if [[ ! -e $boost_prefix/include ]]; then
            echo "Unpacking Boost"
            cd $boost_prefix
            if [ $PLATFORM = "windows-x86_64" ]; then
                if [[ ! -e boost_$BOOST_VERSION ]]; then
                    /c/Program\ Files/7-Zip/7z x -y $ROOT_DIR/v3d_main/common_lib/src_packages/boost_$BOOST_VERSION.tar.gz
                    /c/Program\ Files/7-Zip/7z x -y boost_$BOOST_VERSION.tar
                fi
                cd boost_$BOOST_VERSION
                cmd //c .\\bootstrap.bat
                cmd //c .\\b2.exe --toolset=msvc-11.0 address-model=64 --prefix=$boost_prefix install
            else
                tar xvf $ROOT_DIR/v3d_main/common_lib/src_packages/boost_$BOOST_VERSION.tar.gz
                cd boost_$BOOST_VERSION
                ./bootstrap.sh --prefix=$boost_prefix
                ./b2 install
            fi
            cd ../../../../
        fi

        if [ $PLATFORM = "windows-x86_64" ]; then
          if [[ ! -e $ROOT_DIR/v3d_main/common_lib/include/tiff.h ]]; then
              echo "Configuring TIFF headers"
              cd $ROOT_DIR/v3d_main/common_lib/build
              /c/Program\ Files/7-Zip/7z x -y ../src_packages/tiff-4.0.2.tar.gz
              cd tiff-4.0.2
              nmake Makefile.vc
              cp libtiff/tiff.h ../../include
              cp libtiff/tiffconf.h ../../include
              cp libtiff/tiffio.h ../../include
              cp libtiff/tiffio.hxx ../../include
              cp libtiff/tiffvers.h ../../include
              cp libtiff/libtiff.lib ../../winlib64
              cd ../../../..
          fi

            echo "Unpacking FFTW"
            CMAKE_EXE+=" -G \"Visual Studio 11 2012 Win64\""
            cd $ROOT_DIR/v3d_main/common_lib
            if [[ ! -e fftw-3.3.4-dll64.tgz ]]; then
                /c/Program\ Files/7-Zip/7z x -y fftw-3.3.4-dll64.tgz
            fi
            if [[ ! -e ffmpeg-2.5.2-win64 ]]; then
                /c/Program\ Files/7-Zip/7z x -y ffmpeg-2.5.2-win64.tgz
            fi
            cd ../../
        fi

        cd $BUILD_DIR/build_$PLATFORM
        echo $CMAKE_EXE -DCMAKE_BUILD_TYPE:STRING=$CMAKE_BUILD $CMAKE_ARGS $CMAKE_PLATFORM_ARGS $ROOT_DIR
		eval $CMAKE_EXE -DCMAKE_BUILD_TYPE:STRING=$CMAKE_BUILD $CMAKE_ARGS $CMAKE_PLATFORM_ARGS $ROOT_DIR

        if [ $PLATFORM = "windows-x86_64" ]; then
            if [ $BUILD_HDF5 = 1 ]; then
                echo "Building HDF5"
                devenv Vaa3D.sln -project HDF5 -build $CMAKE_BUILD -out hdf5.txt
            fi
            echo "Building Vaa3D"
            devenv Vaa3D.sln -build $CMAKE_BUILD -out all_build.txt
            echo "Installing"
            devenv Vaa3D.sln -project INSTALL -build $CMAKE_BUILD -out install.txt
            echo "Done."
        else
    		make
        fi
        ;;
    clean)
        echo "Cleaning build_$PLATFORM directories"
		if [[ -e $BUILD_DIR/build_$PLATFORM ]]; then
			rm -rf $BUILD_DIR/build_$PLATFORM
		fi
        ;;
    clobber)
        echo "Cleaning cmake directories"
		if [[ -e cmake-$CMAKE_VERSION ]]; then
			rm -rf cmake-$CMAKE_VERSION
		fi
		if [[ -e $BUILD_DIR/build_$PLATFORM  ]]; then
			rm -rf $BUILD_DIR/build_$PLATFORM
		fi
        ;;
esac


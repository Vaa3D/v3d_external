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
BUILD_HDF5=1
BOOST_MAJOR_VERSION=1_57
BOOST_VERSION=${BOOST_MAJOR_VERSION}_0
CMAKE_MAJOR_VERSION=3.9
CMAKE_MINOR_VERSION=4
CMAKE_VERSION=${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}
CMAKE_ARGS=""
CMAKE_PLATFORM_ARGS=
CMAKE_BUILD="Release"
CMAKE_EXE=""
BUILD_DIR=`pwd`
ROOT_DIR=`pwd`
DESIRED_QT_VERSION=4
NO_STOP_ON_ERROR="-i"

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

PLATFORM_ARCH=$OS-$ARCH
echo "Detected platform \"$PLATFORM_ARCH\""

while [[ $# > 0 ]]; do
    case "$1" in
        -platform)
            shift
            PLATFORM_ARCH="$1"
            ;;
        -h5j)
            CMAKE_ARGS+=" -DUSE_FFMPEG:BOOL=ON -DUSE_X265:BOOL=ON -DUSE_HDF5:BOOL=ON" 
            CMAKE_ARGS+=" -DJANELIA_BUILD:BOOL=ON " 
            BUILD_HDF5=1
            ;;
        -v)
            CMAKE_ARGS+=" -DCMAKE_VERBOSE_MAKEFILE:BOOL=TRUE "
            ;;
        -qt5)
            DESIRED_QT_VERSION=5
            CMAKE_ARGS+=" -DCMAKE_CXX_FLAGS=\"-std=c++11\" "
            ;;
        -dev)
            NO_STOP_ON_ERROR=""
            ;;
        -16)
            CMAKE_ARGS+=" -DHIGH_BIT_DEPTH:BOOL=ON "
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

CMAKE_ARGS+=" -DDESIRED_QT_VERSION=$DESIRED_QT_VERSION "

PLATFORM=$PLATFORM_ARCH
PLATFORM+="_"
PLATFORM+=$CMAKE_BUILD

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
if [ $PLATFORM_ARCH = "windows-x86_64" ]; then
    CMAKE_PLATFORM_ARGS+="-DTIFF_INCLUDE_DIR:PATH=$BUILD_DIR/build_$PLATFORM/v3d_main/common_lib/include "
    CMAKE_PLATFORM_ARGS+="-DTIFF_LIBRARY:PATH=$BUILD_DIR/build_$PLATFORM/v3d_main/common_lib/winlib64/libtiff.lib "
    CMAKE_PLATFORM_ARGS+="-DZLIB:FILEPATH=$BUILD_DIR/build_$PLATFORM/v3d_main/common_lib/winlib64/libzlib.lib "
    CMAKE_PLATFORM_ARGS+="-DFFTW_INCLUDE_DIR:PATH=$BUILD_DIR/build_$PLATFORM/v3d_main/common_lib/fftw-3.3.4-dll64 "
    CMAKE_PLATFORM_ARGS+="-DFFTW_LIBRARY:PATH=$BUILD_DIR/build_$PLATFORM/v3d_main/common_lib/fftw-3.3.4-dll64/libfftw3f-3.lib"
elif [ $PLATFORM_ARCH = "linux-x86_64" ]; then
    CMAKE_PLATFORM_ARGS+="-DCMAKE_EXE_LINKER_FLAGS:STRING=-Wl,-rpath,'\$ORIGIN/lib'"
fi

: "${CMAKE_DIR:=""}"

case $OPERATION in
    install)
        # See if the code is complete
        if [[ ! -e v3d_main/terafly/src/terarepo/src ]]; then
            echo "Missing the terafly repository. Did you do a git submodule command?"
            exit 1
        fi

        if [[ ! -e $BUILD_DIR/build_$PLATFORM ]]; then
            mkdir -p $BUILD_DIR/build_$PLATFORM/v3d_main/common_lib
            mkdir -p $BUILD_DIR/build_$PLATFORM/v3d_main/common_lib/build
            mkdir -p $BUILD_DIR/build_$PLATFORM/v3d_main/common_lib/include
            cp -r $ROOT_DIR/v3d_main/common_lib/winlib* $BUILD_DIR/build_$PLATFORM/v3d_main/common_lib
        fi

        # See if the CMAKE_DIR is set
        if [ ! "$CMAKE_DIR" = "" ]; then
            if [[ -e $CMAKE_DIR ]]; then
                CMAKE_EXE="$CMAKE_DIR/bin/cmake"
            fi
        fi

        # If CMAKE_EXE is not set, then either find or build cmake
        if [ "$CMAKE_EXE" = "" ]; then
            if hash cmake 2>/dev/null; then
                echo "cmake_minimum_required(VERSION 3.4)" > $BUILD_DIR/build_$PLATFORM/test_cmake_version
		set +e
                cmake -P $BUILD_DIR/build_$PLATFORM/test_cmake_version &> /dev/null
		status=$?
		set -e
                if [ $status = 0 ]; then
                    CMAKE_EXE="cmake"
                fi
            fi
        fi
        if [ "$CMAKE_EXE" = "" ]; then
    		if [[ ! -e $BUILD_DIR/cmake-$CMAKE_VERSION/bin/cmake ]]; then
                cd $BUILD_DIR
    			if [[ ! -e cmake-$CMAKE_VERSION ]]; then
    				echo "Downloading cmake"
    				download http://www.cmake.org/files/v$CMAKE_MAJOR_VERSION/cmake-$CMAKE_VERSION.tar.gz cmake-$CMAKE_VERSION.tar.gz
    				tar xvzf cmake-$CMAKE_VERSION.tar.gz
    			fi
    			cd cmake-$CMAKE_VERSION
    			./configure --prefix=. -- -DCMAKE_USE_OPENSSL=ON
    			make
    			make install
    			cd ..
                cd $ROOT_DIR
    		fi
            CMAKE_EXE="$BUILD_DIR/cmake-$CMAKE_VERSION/bin/cmake"
        fi

        echo "Using $CMAKE_EXE"

        echo $boost_prefix
        boost_include_prefix=$boost_prefix/include
        if [ $PLATFORM_ARCH = "windows-x86_64" ]; then
            boost_include_prefix=$boost_include_prefix/boost-$BOOST_MAJOR_VERSION
        else
            boost_include_prefix=$boost_include_prefix/boost
        fi

        if [[ ! -e $boost_include_prefix ]]; then
            echo "Unpacking Boost"
            cd $boost_prefix
            if [ $PLATFORM_ARCH = "windows-x86_64" ]; then
                if [[ ! -e boost_$BOOST_VERSION ]]; then
                    /c/Program\ Files/7-Zip/7z x -y $ROOT_DIR/v3d_main/common_lib/src_packages/boost_$BOOST_VERSION.tar.gz
                    /c/Program\ Files/7-Zip/7z x -y boost_$BOOST_VERSION.tar
                fi
                cd boost_$BOOST_VERSION
                cmd //c .\\bootstrap.bat 
                cmd //c .\\b2.exe --toolset=msvc-12.0 address-model=64 --prefix=$boost_prefix --without-python install
            else
                tar xzf $ROOT_DIR/v3d_main/common_lib/src_packages/boost_$BOOST_VERSION.tar.gz
                cd boost_$BOOST_VERSION
                ./bootstrap.sh --prefix=$boost_prefix -without-libraries=python
                ./b2 install
            fi
            cd ../../../../
        fi

        if [ $PLATFORM_ARCH = "windows-x86_64" ]; then
          if [[ ! -e $BUILD_DIR/build_$PLATFORM/v3d_main/common_lib/include/tiff.h ]]; then
              echo "Configuring TIFF headers"
              cd $BUILD_DIR/build_$PLATFORM/v3d_main/common_lib/build
              tar xzf $ROOT_DIR/v3d_main/common_lib//src_packages/tiff-4.0.2.tar.gz
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
            CMAKE_EXE+=" -G \"Visual Studio 12 2013 Win64\""
            cd $BUILD_DIR/build_$PLATFORM/v3d_main/common_lib
            if [[ ! -e fftw-3.3.4-dll64 ]]; then
                tar xzf $ROOT_DIR/v3d_main/common_lib/fftw-3.3.4-dll64.tgz
            fi
            if [[ ! -e ffmpeg-2.5.2-win64 ]]; then
                tar xzf $ROOT_DIR/v3d_main/common_lib/ffmpeg-2.5.2-win64.tgz
            fi
            cd ../../
        fi

        cd $BUILD_DIR/build_$PLATFORM
        echo $CMAKE_EXE -DCMAKE_BUILD_TYPE:STRING=$CMAKE_BUILD $CMAKE_ARGS $CMAKE_PLATFORM_ARGS $ROOT_DIR
		eval $CMAKE_EXE -DCMAKE_BUILD_TYPE:STRING=$CMAKE_BUILD $CMAKE_ARGS $CMAKE_PLATFORM_ARGS $ROOT_DIR

        if [ $PLATFORM_ARCH = "windows-x86_64" ]; then
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
    		make $NO_STOP_ON_ERROR
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


#!/bin/sh 
cd `dirname $0`
export LD_LIBRARY_PATH=`pwd`
./terastitcher-gui $*


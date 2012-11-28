@echo off
echo -----------------------------------------------------------------
echo This is a shell program to build the v3d program for win32(mingw)
echo Based on Mac version by Hanchuan Peng, this should be run
echo remotely from the Hudson build system.
echo.
echo 2012-11-27, Copied from build.bat by Les Foster 
echo 2008-09-17, by Zongcai Ruan, created

echo.
echo Usage: hudson_build.bat 
echo -----------------------------------------------------------------
echo on

set MAKEDIR=%CD%/hudson_build_cmake
set MINGW_DIR=c:/mingw
set CYGWIN_DIR=c:/cygwin/bin
set LOCAL_DIR=%CD%/common_lib

:: This prepares for commands like DEVENV /Build, which should take a .sln script as input.
::   Must eliminate double-quotes around space-bearing path-legs.
set OLDPATH=%PATH%
set PATH=%PATH:"=%
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" amd64

set PATH=%PATH%;%MINGW_DIR%/bin;%CYGWIN_DIR%;%LOCAL_DIR%/bin
set VPATH=%LOCAL_DIR%/include;%LOCAL_DIR%/lib_win32;

::cd jba/c++ 
::call make -f jba.makefile %*

::cd ../../
cd v3d
::call qmake v3d.pro

:: touch command for windows
copy/b v3d_version_info.cpp+,,

:: Adjust contents of the src packages directory.
::  TODO: should this be within make?  Should these tasks be obviated by check-ins to SVN?
set OLD_CD=%CD%
cd ../common_lib/src_packages/
tar -xf boost_1_46_0.tar.gz
curl http://download.osgeo.org/libtiff/tiff-4.0.1.zip > tiff-4.0.1.zip
jar -xf tiff-4.0.1.zip

:: Run C-Make to produce Vis-Studio 'solution' file.
::   Should set to base directory for whole build.
cd %OLD_CD%
cd ..\..
if NOT EXIST %MAKEDIR% mkdir %MAKEDIR%
cmake -G"Visual Studio 10 Win64" -H. -B%MAKEDIR%

cd %OLD_CD%
cd %MAKEDIR%
call DEVENV Vaa3D.sln /Build

:: Notify caller of failure, if the executable was not created.
cd %OLD_CD%
cd ../
if NOT EXIST %MAKEDIR%\v3d\Windows_MSVC10_64\vaa3d.exe  exit 1

:: Copy executable to all expected places.
copy %MAKEDIR%\v3d\Windows_MSVC10_64\vaa3d.exe v3d\release\ /y
copy v3d\release\vaa3d.exe %QTDIR%\bin\ /y
copy v3d\release\vaa3d.exe .\v3d\ /y
copy v3d\release\vaa3d.exe ..\v3d\ /y

cd ../
set PATH=%OLDPATH%
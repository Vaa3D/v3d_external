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

set MAKEDIR=%CD%\..\hudson_build_cmake
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

cd v3d

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

echo.
echo.
echo Attempting ALL_BUILD
echo call DEVENV Vaa3D.sln /Build Release /Project ALL_BUILD.vcxproj
call DEVENV Vaa3D.sln /Build Release /Project ALL_BUILD.vcxproj

echo.
echo.
echo Attempting PACKAGE project, to produce an NSIS Installer
echo call DEVENV Vaa3D.sln /Build Release /Project PACKAGE.vcxproj
call DEVENV Vaa3D.sln /Build Release /Project PACKAGE.vcxproj

cd %OLD_CD%
cd ../
 
set OUTPUT_BASE=%MAKEDIR%\v3d\Windows_MSVC10_64
set EXETGT=%OUTPUT_BASE%\vaa3d.exe

:: Copy files into a special zip-up area for Windows update.
set ZIPLOC=%MAKEDIR%\FlySuite_ZIP
mkdir %ZIPLOC%
mkdir %ZIPLOC%\bin
copy %EXETGT %ZIPLOC%\bin /y
xcopy /S %OUTPUT_BASE%\plugins %ZIPLOC%\bin\plugins\ /y
copy %MAKEDIR%\InstallVaa3D-*-Windows_MSVC10*.exe %ZIPLOC%

:: Notify caller of failure, if the required outputs were not created.
if NOT EXIST %ZIPLOC%\bin\Vaa3d.exe (
 echo ERROR: No Vaa3D Executable found
 exit 1
) 

if NOT EXIST %ZIPLOC%\bin\plugins\ (
 echo ERROR: No Vaa3D plugins found
 exit 2
)

if NOT EXIST %ZIPLOC%\InstallVaa3d*.exe (
 echo ERROR: No Installer found
 exit 3
)

:: Build the big zip file.  Check if that worked, also.
zip %MAKEDIR%\..\FlySuite_Windows_.zip %ZIPLOC%\*.*

set PATH=%OLDPATH%

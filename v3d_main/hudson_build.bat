@echo off
echo -----------------------------------------------------------------
echo This is a shell program to build the v3d program for win32(mingw)
echo Originally borrowed from Mac version by Hanchuan Peng, this
echo should be run remotely from the Hudson build system.
echo.
echo 2012-11-27, Copied from build.bat by Les Foster 

echo.
echo Usage: hudson_build.bat 
echo -----------------------------------------------------------------
echo on

set MAKEDIR=%CD%\..\hudson_build_cmake
set MINGW_DIR=c:/mingw
set CYGWIN_DIR=c:/cygwin/bin
set ZIP_7_DIR="c:/Program Files/7-zip/"
set LOCAL_DIR=%CD%/common_lib
::set BUILD_VERSION="%1"
echo Build version %BUILD_VERSION%

set LINUX_BUILD_LOC=\\dm11.janelia.priv\jacsData\FlySuiteStaging\FlySuite_linux_%BUILD_VERSION%\
if NOT EXIST %LINUX_BUILD_LOC% (
  echo %LINUX_BUILD_LOC% does not exist.  Therefore, the windows build cannot be completed.
  exit 10
)

:: This prepares for commands like DEVENV /Build, which should take a .sln script as input.
::   Must eliminate double-quotes around space-bearing path-legs.
set OLDPATH=%PATH%
set PATH=%PATH:"=%
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" amd64

set PATH=%PATH%;%MINGW_DIR%/bin;%CYGWIN_DIR%;%ZIP_7_DIR%;%LOCAL_DIR%/bin
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
cd %MAKEDIR%

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

:: Will collect all output files into a special area for Windows update.
set GATHER_LOC=%MAKEDIR%\FlySuite_GATHER
mkdir %GATHER_LOC%
mkdir %GATHER_LOC%\bin

copy %EXETGT% %GATHER_LOC%\bin /y
xcopy /S %OUTPUT_BASE%\plugins %GATHER_LOC%\bin\plugins\ /y
copy %MAKEDIR%\InstallVaa3D-*-Windows_MSVC10*.exe %GATHER_LOC%

:: Notify caller of failure, if the required outputs were not created.
if NOT EXIST %GATHER_LOC%\bin\vaa3d.exe (
  echo ERROR: No Vaa3D Executable found
  exit 1
) 

if NOT EXIST %GATHER_LOC%\bin\plugins\ (
  echo ERROR: No Vaa3D plugins produced
  exit 2
)

if NOT EXIST %GATHER_LOC%\InstallVaa3d*.exe (
  echo ERROR: No Installer produced
  exit 3
)

:: NOTE: most likely, the step of transferring from staging will produce this zip file.
:: Create initial zip file, with just binaries built here.
:: Build the big zip file.  Check if that worked, also.
::set ZIPFILE=%MAKEDIR%\..\FlySuite_Windows_%BUILD_VERSION%.zip
::set OLD_CD=%CD%
::cd %GATHER_LOC%

::echo 7z a -tzip %ZIPFILE% ALL-WILDCARD
::7z a -tzip %ZIPFILE% *
::cd %OLD_CD%

::if NOT EXIST %ZIPFILE% (
::  echo ERROR: No final zip file produced
::  exit 6
::)

:: Finally, copy the build over to the standard place.
set WIN_STAGING_LOC=\\dm11.janelia.priv\jacsData\FlySuiteStaging\FlySuite_windows_%BUILD_VERSION%\
if NOT EXIST %WIN_STAGING_LOC% (
  mkdir %WIN_STAGING_LOC%
)

:: The zip file sits parallel to the whole-group delivery.
::copy %ZIPFILE% %WIN_STAGING_LOC%\..
::if NOT ERRORLEVEL 0 (
::  echo Failed to copy %ZIPFILE% to %WIN_STAGING_LOC%\..
::  exit 7
::)

:: Push the plugins, etc., to the delivery share's windows subdirectory.
copy %GATHER_LOC%\bin\vaa3d.exe %WIN_STAGING_LOC%
if NOT ERRORLEVEL 0 (
  echo Failed to copy %GATHER_LOC%\bin\vaa3d.exe to %WIN_STAGING_LOC%
  exit 8
)

xcopy /S %GATHER_LOC%\bin\plugins %WIN_STAGING_LOC%\plugins\ /y
if NOT ERRORLEVEL 0 (
  echo Failed to xcopy %GATHER_LOC%\bin\plugins to %WIN_STAGING_LOC%
  exit 9
)

copy %GATHER_LOC%\Install*.exe %WIN_STAGING_LOC%
if NOT ERRORLEVEL 0 (
  echo Failed to copy %GATHER_LOC%\ executable installer to %WIN_STAGING_LOC%
  exit 10
)

:: Staging from upstream process should have placed generic Java output already.
if NOT EXIST %WIN_STAGING_LOC%\workstation_lib (
  echo Failed to find workstation_lib in %WIN_STAGING_LOC%
  echo Copying %LINUX_BUILD_LOC%\workstation_lib\ to %GATHER_LOC%\workstation_lib\
  xcopy /S %LINUX_BUILD_LOC%\workstation_lib %WIN_STAGING_LOC%\workstation_lib\ /y
  if NOT ERRORLEVEL 0 (
    echo Failed to copy workstation_lib from linux build.
	exit 12
  )
)

if NOT EXIST %WIN_STAGING_LOC%\workstation.jar (
  echo Failed to find workstation.jar in %WIN_STAGING_LOC%
  copy %LINUX_BUILD_LOC%\workstation.jar %WIN_STAGING_LOC%
  if NOT ERRORLEVEL 0 (
    echo Failed to copy workstation.jar from linux build.
	exit 11
  )
)

:: NOTE: most likely, the step of transferring from staging will produce this zip file.
::set OLD_CD=%CD%
::cd %WIN_STAGING_LOC%
:: Add extras borrowed from other places, to the zip file.
::echo 7z a -tzip %ZIPFILE% All batch files
::7z a -tzip %ZIPFILE% *.bat,workstation*
::cd %OLD_CD%

set PATH=%OLDPATH%

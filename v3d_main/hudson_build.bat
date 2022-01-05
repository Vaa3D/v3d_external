@echo off
echo -----------------------------------------------------------------
echo This is a shell program to build the v3d program for win32(mingw).
echo It should be run remotely from the Hudson build system.
echo.
echo 2012-11-27, Copied from build.bat by Les Foster 

echo.
echo Usage: hudson_build.bat 
echo -----------------------------------------------------------------
echo on

set MAKEDIR=%CD%\..\hudson_build_cmake
set MINGW_DIR=c:/mingw
set CYGWIN_DIR=c:/cygwin/bin
set CMAKE_DIR="c:/Program Files (x86)/CMake 2.8"
set ZIP_7_DIR="c:/Program Files/7-zip/"
set LOCAL_DIR=%CD%/common_lib
echo Build version %BUILD_VERSION%

:: This will signal whether to avoid carrying out this entire script.
if NOT "%BUILD_VAA3D_WINDOWS%"=="1" (
  echo Script bypassed by first parameter omitted or not set to 1.  If Hudson had been set to auto-clean, then only a raw-checkout will now exist in the workspace.
  exit 0
)

set LINUX_BUILD_LOC=\\dm11.janelia.priv\jacs\jacsShare\JaneliaWorkstationStaging\JaneliaWorkstation_linux_%BUILD_VERSION%\
if NOT EXIST %LINUX_BUILD_LOC% (
  echo %LINUX_BUILD_LOC% does not exist.  Therefore, the windows build cannot be completed.
  exit 10
)

:: This prepares for commands like DEVENV /Build, which should take a .sln script as input.
::   Must eliminate double-quotes around space-bearing path-legs.
set OLDPATH=%PATH%
set PATH=%PATH:"=%
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" amd64

set PATH=%CMAKE_DIR%/bin;%PATH%;%MINGW_DIR%/bin;%CYGWIN_DIR%;%ZIP_7_DIR%;%LOCAL_DIR%/bin
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
set GATHER_LOC=%MAKEDIR%\JaneliaWorkstation_GATHER
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

:: Copy the build over to the standard place.
set WIN_STAGING_LOC=\\dm11.janelia.priv\jacs\jacsShare\JaneliaWorkstationStaging\JaneliaWorkstation_windows_%BUILD_VERSION%\
if NOT EXIST %WIN_STAGING_LOC% (
  mkdir %WIN_STAGING_LOC%
)

:: Push the plugins, etc., to the delivery share's windows subdirectory.
copy %GATHER_LOC%\bin\vaa3d.exe %WIN_STAGING_LOC%
if NOT EXIST %WIN_STAGING_LOC%\vaa3d.exe (
  echo Failed to copy %GATHER_LOC%\bin\vaa3d.exe to %WIN_STAGING_LOC%
  exit 8
)

xcopy /S %GATHER_LOC%\bin\plugins %WIN_STAGING_LOC%\plugins\ /y
if NOT EXIST %WIN_STAGING_LOC%\plugins\ (
  echo Failed to xcopy %GATHER_LOC%\bin\plugins to %WIN_STAGING_LOC%
  exit 9
)

copy %GATHER_LOC%\Install*.exe %WIN_STAGING_LOC%
if NOT EXIST %WIN_STAGING_LOC%\Install*.exe (
  echo Failed to copy %GATHER_LOC%\ executable installer to %WIN_STAGING_LOC%
  exit 10
)

:: Staging from upstream process may have placed generic Java output already.  If not, report this.
if NOT EXIST %WIN_STAGING_LOC%\workstation_lib (
  echo No workstation_lib dir found in %WIN_STAGING_LOC%
  exit 11
)

if NOT EXIST %WIN_STAGING_LOC%\workstation.jar (
  echo No copy workstation.jar found in %WIN_STAGING_LOC%
  exit 12
)

if NOT EXIST %WIN_STAGING_LOC%\*.bat (
  echo No Win batch files found at %WIN_STAGING_LOC%
  exit 13
)

set PATH=%OLDPATH%

@echo off
echo -----------------------------------------------------------------
echo This is a shell program to build the v3d program for win32(mingw)
echo Based on Mac version by Hanchuan Peng
echo 2008-09-17, by Zongcai Ruan, created
:: 2008-09-20, by Zongcai Ruan, add PATH, VPATH
:: 2008-09-21, by Zongcai Ruan, add *_DIR
:: 2008-09-29, by Zongcai Ruan, del QT_DIR, add copy
:: 2009-01-13, by Zongcai Ruan, fixed for -B
echo.
echo Usage: build.bat 
echo Usage: build.bat clean 
echo Usage: build.bat -B
echo -----------------------------------------------------------------
::echo on


set MINGW_DIR=c:/mingw
:: set LOCAL_DIR=c:/msys/local
set LOCAL_DIR=%CD%/common_lib

set PATH=%PATH%;%MINGW_DIR%/bin;%LOCAL_DIR%/bin
set VPATH=%LOCAL_DIR%/include;%LOCAL_DIR%/lib_win32;

set QT_VER=5
set DEF_QT5=

if %QT_VER==4 (
::set QT_BIN=C:\Qt\mingw-qt-4.7.4\bin
::set QT_BIN=C:\Qt\mingw-qt-4.8.6\bin
)
if %QT_VER%==5 (
set QT_BIN=C:\Qt\5.12.7\mingw73_64\bin
set DEF_QT5="DEFINES += USE_Qt5" 
rem "QMAKE_CXXFLAGS += -std=c++0x"
)

cd v3d
qmake vaa3d.pro  %DEF_QT5% 
rem "QMAKE_CXXFLAGS += -std=c++0x"

:: touch command for windows
copy/b v3d_version_info.cpp+,,
:: MUST use make target of 'all/release/debug', otherwise qmake will enter 'Makefile:' dead loop
:: a%1 is a trick to avoid empty variable error
if a%1==a-B (
   call make clean
   call make release
) else (
   echo make release %*
   echo ==========================================
   call make release %*
)

if not exist ..\..\bin\ (
    mkdir ..\..\bin\ 
)
cd  ..\..\bin\

if %QT_VER%==4 (
copy %QT_BIN%\QtCore4.dll .\  /y
copy %QT_BIN%\QtGui4.dll   .\  /y
copy %QT_BIN%\QtOpenGL4.dll .\  /y
copy %QT_BIN%\QtXml4.dll .\  /y
copy %QT_BIN%\QtNetwork4.dll .\  /y
)
if %QT_VER%==5 (
copy %QT_BIN%\Qt5Core.dll .\  /y
copy %QT_BIN%\Qt5Gui.dll   .\  /y
copy %QT_BIN%\Qt5OpenGL.dll .\  /y
copy %QT_BIN%\Qt5Xml.dll .\  /y
copy %QT_BIN%\Qt5Network.dll .\  /y
)

copy ..\v3d_main\v3d\release\vaa3d.exe .\  /y

vaa3d.exe

cd ../

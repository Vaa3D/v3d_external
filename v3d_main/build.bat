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
echo on


set MINGW_DIR=c:/mingw
:: set LOCAL_DIR=c:/msys/local
set LOCAL_DIR=%CD%/common_lib

set PATH=%PATH%;%MINGW_DIR%/bin;%LOCAL_DIR%/bin
set VPATH=%LOCAL_DIR%/include;%LOCAL_DIR%/lib_win32;

cd jba/c++ 
call make -f jba.makefile %*
cd ../../

cd v3d
call qmake v3d.pro
:: touch command for windows
copy/b v3d_version_info.cpp+,,
:: MUST use make target of 'all/release/debug', otherwise qmake will enter 'Makefile:' dead loop
:: a%1 is a trick to avoid empty variable error
if a%1==a-B (
   call make clean
   call make release
) else (
   call make release %*
)
cd ../

copy v3d\release\v3d.exe %QTDIR%\bin\ /y
copy v3d\release\v3d.exe .\v3d\ /y
copy v3d\release\v3d.exe ..\v3d\ /y

cd ../

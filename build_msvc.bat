:: 2018-3-23 by Zongcai Ruan
:: a convenience script for building in VS???? x64 Win64 Command Prompt
::
  

cd v3d_main\v3d\
call ..\..\qmake vaa3d_msvc.pro %*

:: touch command for windows
copy/b v3d_version_info.cpp+,,
nmake -f Makefile.Release

if not exist ..\..\..\bin_msvc\ (
    mkdir ..\..\..\bin_msvc\ 
)
cd  ..\..\..\bin_msvc\

@echo off
if %QT_VER%==4 (
    echo copy Qt 4 dll
copy %QT_BIN%\QtCore4.dll .\  /y
copy %QT_BIN%\QtGui4.dll   .\  /y
copy %QT_BIN%\QtOpenGL4.dll .\  /y
copy %QT_BIN%\QtXml4.dll .\  /y
copy %QT_BIN%\QtNetwork4.dll .\  /y
)
if %QT_VER%==5 (
    echo copy Qt 5 dll
copy %QT_BIN%\Qt5Core.dll .\  /y
copy %QT_BIN%\Qt5Gui.dll   .\  /y
copy %QT_BIN%\Qt5OpenGL.dll .\  /y
copy %QT_BIN%\Qt5Xml.dll .\  /y
copy %QT_BIN%\Qt5Network.dll .\  /y
)
@echo on

copy ..\v3d_external\v3d_main\v3d\release\openvr_api.dll .\  /y
copy ..\v3d_external\v3d_main\v3d\release\SDL2.dll .\  /y
copy ..\v3d_external\v3d_main\v3d\release\vaa3d_msvc.exe .\  /y

vaa3d_msvc.exe

cd ..\v3d_external
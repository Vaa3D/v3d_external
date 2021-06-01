 set MINGW_DLL=%CD%\common_lib\mingw_dll
 set MINGW_DIR=C:\Program Files\mingw-w64\x86_64-8.1.0-win32-sjlj-rt_v6-rev0\mingw64
 :: set LOCAL_DIR=c:/msys/local
 set LOCAL_DIR=%CD%/common_lib

 set PATH=%PATH%;%MINGW_DIR%/bin;%LOCAL_DIR%/bin
 set VPATH=%LOCAL_DIR%/include;%LOCAL_DIR%/lib_win32

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



 copy %MINGW_DLL%\glew32.dll .\ /y
 copy %MINGW_DLL%\libtiff.dll .\ /y
 copy %MINGW_DLL%\openvr_api.dll .\ /y
 copy %MINGW_DLL%\SDL2.dll .\ /y
 copy %MINGW_DLL%\libteem.dll .\ /y
 copy %MINGW_DLL%\QtCore4.dll .\ /y
 copy %MINGW_DLL%\QtGui4.dll .\ /y
 copy %MINGW_DLL%\QtNetwork4.dll .\ /y
 copy %MINGW_DLL%\QtOpenGL4.dll .\ /y
 copy %MINGW_DLL%\QtXML4.dll .\ /y
 copy ..\v3d_main\v3d\release\vaa3d.exe .\  /y


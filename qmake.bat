:: a warpper for qmake to switch QT version 
::

set QT_VER=4
set DEF_QT5=

@echo off
if %QT_VER%==4 (
    echo Qt 4
::set QT_BIN=C:\Qt\vs2010x64-qt-4.7.4\bin
set QT_BIN=C:\Qt\vs2013x64-qt-4.8.6\bin
)
if %QT_VER%==5 (
    echo Qt 5
set QT_BIN=C:\Qt\5.12.7\msvc2017_64\bin
set QMAKE_MSC_VER=1910
set DEF_QT5="DEFINES += USE_Qt5" 
rem "QMAKE_CXXFLAGS += -std=c++0x"
)
@echo %QT_BIN%
@echo ============================================
@echo on

%QT_BIN%\qmake.exe %* %DEF_QT5%
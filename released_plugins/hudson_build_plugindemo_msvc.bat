:: batch build all released plugin projects using a VC Studio complier's nmake function
:: (window version) by Hanchuan Peng
:: 2010-05-20
:: 2012-11-28 Borrowed for Hudson by Les Foster
:: Best add release argument to make release\v3d.exe can recongnize plugin
:: revised from the original bat file

set PATH=%PATH%;

set OLDPATH=%PATH%
set PATH=%PATH:"=%
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" amd64

:: Keep the intermediat make info separated.
set BASEDIR=%CD%
set MAKEDIR=%CD%\hudson_build_cmake
if NOT EXIST %MAKEDIR% mkdir %MAKEDIR%

cd v3d_plugins
call :makepro %*
cd ..
goto :eof

:makepro
cmake -G"Visual Studio 10 Win64" -H%BASEDIR% -B%MAKEDIR%	
::for /D %%i in ( * ) do (
::  cd %%i
::  if exist *.pro (
::	cd %MAKEDIR%
::	dir *.sln
::	call DEVENV my.sln /Build
::
::  ) 
::  cd ..
::)
dir %MAKEDIR%\*.sln
cd %BASEDIR%
set PATH=%OLDPATH%

goto :eof

:: 2012 Les Foster
:: Borrowed from Hanshuan Peng's build.bat.
:: For use in the Hudson build tool.
::
:: examples: 
:: hudson_build.bat
  
cd v3d_main

call hudson_build.bat %1

:: build of plugin-demo.
::echo Building release plugins
::echo %CD%
::cd .\released_plugins
::echo %CD%

:: TEMP: comment out.  See if there are still things being built for the plugins.
::call hudson_build_plugindemo_msvc.bat
cd ..\
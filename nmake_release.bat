:: 2018-04-27 RZC
:: a convenience script for MSVC nmake
::

cd v3d_main\common_lib\winlib64
::copy /y libnewmat_VS2010.lib libnewmat.lib
copy /y libnewmat_VS2013.lib libnewmat.lib
cd ..\..\..
  
cd v3d_main\v3d
call ..\..\qmake -v
call ..\..\qmake  vaa3d_msvc.pro

copy/b v3d_version_info.cpp+,,
nmake -f Makefile.Release

cd release
dir vaa3d_msvc.exe



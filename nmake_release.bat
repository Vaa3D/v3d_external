:: 2018-04-27 RZC
:: a convenience script for MSVC nmake

  
cd v3d_main\v3d

call qmake vaa3d_msvc.pro

nmake -f Makefile.Release

cd release



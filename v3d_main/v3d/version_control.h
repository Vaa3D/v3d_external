#pragma once

/*********************************************************

This header file is for switching different code base.
Only enable line 16 and 17 if under VS2015-Qt5 environment.
Only enable line 20 if GLEW is installed.

Different windows OS versions are still being tested: Win7/Kits_81, Win10/Kits_10_14393
For VC++ or above, needs to install GLEW to make some gl functions available. However variables redefinition conflicts may occur between GLee_r.h and glew.h. 
Disable those repeated variables in GLee_r.h at this moment.

MK 11/23/2016
********************************************************/

//#define USE_Qt5_VS2015_Win7_81
//#define USE_Qt5_VS2015_Win10_10_14393


//#define USE_GLEW

#ifndef _VAA3D_VERSION_CTRL__
#define _VAA3D_VERSION_CTRL__

#include <QtGlobal> // MK, Feb, 2020 - OS checking preprocessor

#ifdef Q_OS_MAC // MK, Feb, 2020 - Qt5 hasn't been adapted by Vaa3D on Windows platform yet; need a OS check preprocessor here.
#define USE_Qt5 1
//added by PHC , 2020/1/31
#endif

#endif


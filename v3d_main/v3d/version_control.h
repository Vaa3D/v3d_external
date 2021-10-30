#pragma once

/*********************************************************

This header file is for switching different code base.
Only enable line 13 and 14 if under VS2015-Qt5 environment.
Only enable line 18 if GLEW is installed.

Different windows OS versions are still being tested: Win7/Kits_81, Win10/Kits_10_14393
For VC++ or above, needs to install GLEW to make some gl functions available. However variables redefinition conflicts may occur between GLee_r.h and glew.h. Disable those repeated variables in GLee_r.h at this moment.

MK 11/23/2016
********************************************************/

//#define USE_Qt5_VS2015_Win7_81
//#define USE_Qt5_VS2015_Win10_10_14393


//#define USE_GLEW
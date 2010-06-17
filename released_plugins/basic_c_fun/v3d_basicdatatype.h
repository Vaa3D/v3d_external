/*
v3d_basicdatatype.h: by Hanchuan Peng
2010-05-19
*/

#ifndef __V3D_BASICDATATYPE_H__
#define __V3D_BASICDATATYPE_H__

// be compatible with LP64(unix64) and LLP64(win64)
typedef unsigned char        uint8;
typedef unsigned short       uint16;
typedef unsigned int         uint32;
typedef unsigned long long   uint64;
typedef          char        sint8;
typedef          short       sint16;
typedef          int         sint32;
typedef          long long   sint64;
typedef          float       float32;
typedef          double      float64;

//2010-05-19: by Hanchuan Peng. add the MSVC specific version # (vc 2008 has a _MSC_VER=1500) and win64 macro. 
//Note that _WIN32 seems always defined for any windows application.
//For more info see page for example: http://msdn.microsoft.com/en-us/library/b0084kay%28VS.80%29.aspx

#if defined (_MSC_VER) && (_WIN64)

#define V3DLONG long long
#define strcasecmp strcmp

#else

#define V3DLONG long

#endif

enum ImagePixelType {V3D_UNKNOWN, V3D_UINT8, V3D_UINT16, V3D_FLOAT32};
enum TimePackType {TIME_PACK_NONE,TIME_PACK_Z,TIME_PACK_C}; 


#endif


#ifndef LOADV3DFFMPEG_H
#define LOADV3DFFMPEG_H

#ifdef V3D_ENABLE_LOAD_MOVIE

#include "../../v3d/v3d_core.h" // Image4DSimple

bool loadRaw2StackFFMpeg(const char* fileName, Image4DSimple* img);

#endif // V3D_ENABLE_LOAD_MOVIE

#endif // LOADV3DFFMPEG_H

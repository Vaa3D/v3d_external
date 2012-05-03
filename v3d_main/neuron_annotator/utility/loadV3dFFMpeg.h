#ifndef LOADV3DFFMPEG_H
#define LOADV3DFFMPEG_H

#ifdef USE_FFMPEG

#include "../../v3d/v3d_core.h" // Image4DSimple

bool loadRaw2StackFFMpeg(const char* fileName, Image4DSimple* img);

#endif // USE_FFMPEG

#endif // LOADV3DFFMPEG_H

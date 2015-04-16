#ifndef LOADV3DFFMPEG_H
#define LOADV3DFFMPEG_H

#ifdef USE_FFMPEG

#include "../../v3d/v3d_core.h" // Image4DSimple
extern "C" {
#include "libavcodec/avcodec.h"
}
#include <iostream>

bool loadStackFFMpeg( const char* fileName, Image4DSimple& image );
bool loadStackHDF5( const char* fileName, Image4DSimple& image );
bool loadStackFFMpegAsGray( const char* fileName, Image4DSimple& img );

bool loadStackFFMpeg( QUrl url, Image4DSimple& image );
bool loadIndexedStackFFMpeg( QByteArray* buffer, Image4DSimple& img, int channel, int num_channels,
                             long width, long height );
bool loadStackFFMpegAsGray( QUrl url, Image4DSimple& img );

bool saveStackFFMpeg( const char* fileName, const My4DImage& img, enum AVCodecID codec_id = AV_CODEC_ID_MPEG4 );
bool saveStackHDF5( const char* fileName, const My4DImage& img );

#endif // USE_FFMPEG

#endif // LOADV3DFFMPEG_H

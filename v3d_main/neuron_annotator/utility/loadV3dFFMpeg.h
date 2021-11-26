#ifndef LOADV3DFFMPEG_H
#define LOADV3DFFMPEG_H

#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <QBuffer>
#include <QUrl>

#ifdef USE_FFMPEG

#include "../../v3d/v3d_core.h" // Image4DSimple
extern "C" {
#include "libavcodec/avcodec.h"
}

typedef std::vector< std::pair< AVCodecID, std::string > > Codec_Mapping;

bool codec_lookup( std::string codec_name, AVCodecID* codec, std::string* defaults );
void generate_codec_mapping( Codec_Mapping& mapping, int num_channels );

bool loadStackFFMpeg( const char* fileName, Image4DSimple& image );
bool loadStackFFMpeg( QUrl url, Image4DSimple& image );

bool loadStackHDF5( const char* fileName, Image4DSimple& image );
bool loadStackHDF5( QUrl url, Image4DSimple& image );
bool loadStackHDF5( QBuffer& fileStream, Image4DSimple& image );

bool loadStackFFMpegAsGray( const char* fileName, Image4DSimple& img );

bool loadIndexedStackFFMpeg( QByteArray* buffer, Image4DSimple& img, int channel, int num_channels,
                             long width, long height );
bool loadStackFFMpegAsGray( QUrl url, Image4DSimple& img );

bool saveStackFFMpeg( const char* fileName, const My4DImage& img, AVCodecID codec_id = AV_CODEC_ID_MPEG4 );
bool saveStackHDF5( const char* fileName, const My4DImage& img, Codec_Mapping* mapping = NULL );

#endif // USE_FFMPEG

#endif // LOADV3DFFMPEG_H

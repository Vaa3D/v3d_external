
#ifdef USE_FFMPEG

#include "FFMpegVideo_utility.h"

extern "C" {
#include <libavformat/avio.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
}

#include <QIODevice>
#include <string>
#include <stdexcept>
#include <iostream>

// Custom read function so FFMPEG does not need to read from a local file by name.
// But rather from a stream derived from a URL or whatever.
extern "C" {

int readFunction(void* opaque, uint8_t* buf, int buf_size)
{
    QIODevice* stream = (QIODevice*)opaque;
    int numBytes = stream->read((char*)buf, buf_size);
    return numBytes;
}

// http://cdry.wordpress.com/2009/09/09/using-custom-io-callbacks-with-ffmpeg/
int64_t seekFunction(void* opaque, int64_t offset, int whence)
{
    QIODevice* stream = (QIODevice*)opaque;
    if (stream == NULL)
        return -1;
    else if (whence == AVSEEK_SIZE)
        return -1; // "size of my handle in bytes"
    else if (stream->isSequential())
        return -1; // cannot seek a sequential stream
        else if ( whence == SEEK_CUR ) // relative to start of file
        {
        if (! stream->seek(stream->pos() + offset) )
            return -1;
    }
        else if ( whence == SEEK_END ) // relative to end of file
        {
        assert(offset < 0);
        if (! stream->seek(stream->size() + offset) )
            return -1;
    }
        else if ( whence == SEEK_SET ) // relative to start of file
        {
        if (! stream->seek(offset) )
            return -1;
    }
        else
        {
        assert(false);
    }
    return stream->pos();
}

}

AVPacketWrapper::AVPacketWrapper()
{
}

/* virtual */
AVPacketWrapper::~AVPacketWrapper()
{
    free();
}

void AVPacketWrapper::free()
{
    av_packet_unref(&packet);
}





#endif // USE_FFMPEG

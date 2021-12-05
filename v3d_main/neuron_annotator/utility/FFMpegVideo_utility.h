
#ifndef FFMPEGVIDEO_UTILITY_H
#define FFMPEGVIDEO_UTILITY_H

#ifdef USE_FFMPEG

extern "C"
{
#include <libavcodec/avcodec.h>
}

// Avoid link error on some macs
#ifdef __APPLE__
extern "C" {
#include <stdlib.h>
#include <errno.h>
// #include "compiler/compiler.h"

/*
 * Darwin doesn't have posix_memalign(), provide a private
 * weak alternative
 */
    /*
int __weak posix_memalign(void **ptr, size_t align, size_t size)
{
       if (*ptr)
               return 0;

       return ENOMEM;
}
*/
}
#endif

// Custom read function so FFMPEG does not need to read from a local file by name.
// But rather from a stream derived from a URL or whatever.
extern "C" {

int readFunction(void* opaque, uint8_t* buf, int buf_size);

// http://cdry.wordpress.com/2009/09/09/using-custom-io-callbacks-with-ffmpeg/
int64_t seekFunction(void* opaque, int64_t offset, int whence);

}


/////////////////////////////
// AVPacketWrapper methods //
/////////////////////////////

class AVPacketWrapper
{
public:
    AVPacketWrapper();
    virtual ~AVPacketWrapper();
    void free();

    AVPacket packet;
};



#endif // USE_FFMPEG

#endif //FFMPEGVIDEO_UTILITY_H

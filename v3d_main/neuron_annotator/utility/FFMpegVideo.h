#ifndef FFMPEGVIDEO_H
#define FFMPEGVIDEO_H

#ifdef V3D_ENABLE_LOAD_MOVIE

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include <string>

// Translated to C++ by Christopher Bruns May 2012
// from ffmeg_adapt.c in whisk package by Nathan Clack, Mark Bolstadt, Michael Meeuwisse
class FFMpegVideo
{
public:
    enum Channel {
        RED = 0,
        GRAY = 0,
        GREEN = 1,
        BLUE = 2,
        ALPHA = 3
    };

    FFMpegVideo();
    FFMpegVideo(const std::string& fileName);
    virtual ~FFMpegVideo();
    void open(const std::string& fileName, PixelFormat formatParam = PIX_FMT_RGB24);
    bool readNextFrame(int target = 0);
    uint8_t getPixelIntensity(int x, int y, Channel c = GRAY) const;
    int getNumberOfFrames() const;
    int getWidth() const;
    int getHeight() const;
    int getNumberOfChannels() const;

protected:
    static bool b_is_one_time_inited;

    void initialize();
    static void maybeInitFFMpegLib();
    static void avtry(int result, const std::string& msg);

    AVFormatContext *pFormatCtx;
    AVCodecContext *pCtx;
    AVCodec *pCodec;
    AVFrame *pRaw;
    AVFrame *pFrameRGB;
    uint8_t *buffer,
            *blank;
    struct SwsContext *Sctx;
    int videoStream, width, height;
    PixelFormat format;
    int numBytes;
    int numFrames;
    int last;
    int sc; // number of color channels
};

#endif // V3D_ENABLE_LOAD_MOVIE

#endif // FFMPEGVIDEO_H

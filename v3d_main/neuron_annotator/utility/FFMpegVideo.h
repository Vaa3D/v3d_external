#ifndef FFMPEGVIDEO_H
#define FFMPEGVIDEO_H

/*
 * FFMpegVideo.h
 * May 2012 Christopher Bruns
 * The FFMpegVideo class is a C++ wrapper around the poorly documented
 * libavcodec movie API used by ffmpeg.  I made extensive use of Nathan
 * Clack's implemention in the whisk project.
 *
 * The FFMpegVideo.h and FFMpegVideo.cpp files depend only on the libavcodec
 * and allied sets of libraries.  To compartmentalize and reduce dependencies
 * I placed the Vaa3d specific use of this class into a separate set of
 * source files: loadV3dFFMpeg.h/cpp
 */

#ifdef USE_FFMPEG

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixfmt.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}

#include <string>
#include <stdexcept>
#include <iostream>

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
    void open(const std::string& fileName, enum PixelFormat formatParam = PIX_FMT_RGB24);
    uint8_t getPixelIntensity(int x, int y, Channel c = GRAY) const;
    bool fetchFrame(int targetFrameIndex = 0);
    int getNumberOfFrames() const;
    int getWidth() const;
    int getHeight() const;
    int getNumberOfChannels() const;

    void write(const std::string& fileName, int width, int height);
    static void maybeInitFFMpegLib();

protected:
    static bool b_is_one_time_inited;

    void initialize();
    static void avtry(int result, const std::string& msg);
    bool readNextFrame(int targetFrameIndex = 0);
    int seekToFrame(int targetFrameIndex = 0);

    AVFormatContext *container;
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
    int previousFrameIndex;
    int sc; // number of color channels
};


// TODO - finish refactoring based on
// http://svn.gnumonks.org/trunk/21c3-video/ffmpeg/ffmpeg-0.4.9-pre1/output_example.c
class FFMpegEncoder
{
public:
    FFMpegEncoder(const char * file_name, int width, int height);
    void write_frame();
    virtual ~FFMpegEncoder();

protected:
    AVFormatContext *container;
    AVCodecContext *pCtx;
    AVFrame *picture_yuv;
    AVFrame *picture_rgb;
    struct SwsContext *Sctx;
};


#endif // USE_FFMPEG

#endif // FFMPEGVIDEO_H

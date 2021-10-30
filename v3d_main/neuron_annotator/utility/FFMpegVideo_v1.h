#ifndef FFMPEGVIDEO_v1_H
#define FFMPEGVIDEO_v1_H

/*
 * FFMpegVideo_v1.h
 * May 2012 Christopher Bruns
 * The FFMpegVideo_v1 class is a C++ wrapper around the poorly documented
 * libavcodec movie API used by ffmpeg.  I made extensive use of Nathan
 * Clack's implemention in the whisk project.
 *
 * The FFMpegVideo_v1.h and FFMpegVideo_v1.cpp files depend only on the libavcodec
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

#include <QFile>
#include <QNetworkAccessManager>
#include <QMutex>
#include <QUrl>
#include <QBuffer>
#include <string>
#include <stdexcept>
#include <iostream>

// Translated to C++ by Christopher Bruns May 2012
// from ffmeg_adapt.c in whisk package by Nathan Clack, Mark Bolstadt, Michael Meeuwisse
class FFMpegVideo_v1
{
public:
    enum Channel {
        RED = 0,
        GRAY = 0,
        GREEN = 1,
        BLUE = 2,
        ALPHA = 3
    };

    // Some libavcodec calls are not reentrant
    static QMutex mutex;
    static void maybeInitFFMpegLib();

    FFMpegVideo_v1(AVPixelFormat pixelFormat=AV_PIX_FMT_RGB24);
    FFMpegVideo_v1(QUrl url, AVPixelFormat pixelFormat=AV_PIX_FMT_RGB24);
    FFMpegVideo_v1(QByteArray* buffer, AVPixelFormat pixelFormat=AV_PIX_FMT_RGB24);
    virtual ~FFMpegVideo_v1();
    bool open(QUrl url, enum AVPixelFormat formatParam = AV_PIX_FMT_RGB24);
    bool open(QIODevice& fileStream, QString& fileName, enum AVPixelFormat formatParam = AV_PIX_FMT_RGB24);
    uint8_t getPixelIntensity(int x, int y, Channel c = GRAY) const;
    bool fetchFrame(int targetFrameIndex = 0);
    int getNumberOfFrames() const;
    int getWidth() const;
    int getHeight() const;
    int getNumberOfChannels() const;
    bool readNextFrame(int targetFrameIndex = 0);
    bool readNextFrameWithPacket(int targetFrameIndex, AVPacket& packet, AVFrame* pYuv);
    int seekToFrame(int targetFrameIndex = 0);

    // make certain members public, for use by Fast3DTexture class
    AVFrame *pFrameRGB;
    AVFrame *pRaw;
    AVFormatContext *container;
    AVCodecContext *pCtx;
    int videoStream;
    int previousFrameIndex;
    bool isOpen;

protected:
    static bool b_is_one_time_inited;

    void initialize();
    bool open(QString& fileName, enum AVPixelFormat formatParam);
    bool openUsingInitializedContainer(enum AVPixelFormat formatParam);
    static bool avtry(int result, const std::string& msg);

    AVCodec *pCodec;
    uint8_t *buffer,
            *blank;
    struct SwsContext *Sctx;
    int width, height;
    AVPixelFormat format;
    size_t numBytes;
    int numFrames;
    int sc; // number of color channels

    // For loading from URL
    static const int ioBufferSize = 32768;
    unsigned char * ioBuffer;
    QNetworkAccessManager networkManager;
    AVIOContext* avioContext;
    QFile fileStream;
    QNetworkReply* reply;
    QBuffer fileBuffer;
    QByteArray byteArray;
};


#endif // USE_FFMPEG

#endif // FFMPEGVIDEO_v1_H

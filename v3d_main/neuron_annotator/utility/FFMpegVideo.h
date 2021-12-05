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

    // Some libavcodec calls are not reentrant
    static QMutex mutex;
    static void maybeInitFFMpegLib();

    FFMpegVideo();
    FFMpegVideo(QUrl url );
    FFMpegVideo(QByteArray* buffer );
    virtual ~FFMpegVideo();
    bool open(QUrl url);
    bool open(QIODevice& fileStream, QString& fileName);
    AVPixelFormat getPixelFormat() const { return format; }
    uint8_t getPixelIntensity(int x, int y, Channel c = GRAY) const;
    uint16_t getPixelIntensity16(int x, int y, Channel c = GRAY) const;
    bool fetchFrame(int targetFrameIndex = 0);
    int getBitDepth() const;
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
    bool open(QString& fileName);
    bool openUsingInitializedContainer();
    int open_codec_context( int *stream_idx, AVCodecContext **dec_ctx,
                            AVFormatContext *fmt_ctx, enum AVMediaType type );
    static bool avtry(int result, const std::string& msg);

    AVCodec *pCodec;
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


// TODO - finish refactoring based on
// http://svn.gnumonks.org/trunk/21c3-video/ffmpeg/ffmpeg-0.4.9-pre1/output_example.c
class FFMpegEncoder
{
public:
    typedef FFMpegVideo::Channel Channel;

    FFMpegEncoder(const char * file_name, int width, int height, int bit_depth,
        enum AVCodecID codec_id = AV_CODEC_ID_MPEG4, std::string options = "" );
    virtual ~FFMpegEncoder();
    void setPixelIntensity(int x, int y, int c, uint8_t value);
    void setPixelIntensity16(int x, int y, uint16_t value);
    void write_frame();
    void close();
    size_t buffer_size() { return _buffer_size; }
    uint8_t* buffer() { return _buffer; }
    void free_buffer() { if (_buffer_size > 0 ) { _buffer_size = 0; av_free( _buffer ); } }
    void encode( AVFrame* picture = NULL );
    int encoded_frames() { return _encoded_frames; }

protected:
    AVFormatContext *container;
    AVCodecContext *pCtx;
    AVStream *video_st;
    AVFrame *picture_yuv;
    AVFrame *picture_rgb;
    struct SwsContext *Sctx;
    bool use_buffer;
    size_t _buffer_size;
    uint8_t* _buffer;
    int _frame_count;
    int _encoded_frames;
    enum AVPixelFormat _raw_format;
};


#endif // USE_FFMPEG

#endif // FFMPEGVIDEO_H

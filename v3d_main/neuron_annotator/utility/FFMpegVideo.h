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

protected:
    static bool b_is_one_time_inited;

    void initialize();
    static void maybeInitFFMpegLib();
    static void avtry(int result, const std::string& msg);
    bool readNextFrame(int targetFrameIndex = 0);
    int seekToFrame(int targetFrameIndex = 0);

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
    int previousFrameIndex;
    int sc; // number of color channels
};


class FFMpegEncoder
{
public:
    FFMpegEncoder(std::ostream& output_stream, int width, int height)
        : ctx(NULL)
    {
        CodecID codec_id = CODEC_ID_MSMPEG4V2;
        int i, out_size, x, y, outbuf_size;
        AVFrame *picture;
        uint8_t *outbuf;
        int had_output=0;

        /* find the mpeg4 video encoder */
        codec = avcodec_find_encoder(codec_id);
        if (!codec) {
            fprintf(stderr, "codec not found\n");
            exit(1);
        }

        ctx = avcodec_alloc_context3(codec);
        picture= avcodec_alloc_frame();

        /* put sample parameters */
        ctx->bit_rate = 400000;
        /* resolution must be a multiple of two */
        ctx->width = width;
        ctx->height = height;
        /* frames per second */
        ctx->time_base= (AVRational){1,25};
        ctx->gop_size = 10; /* emit one intra frame every ten frames */
        ctx->max_b_frames=1;
        ctx->pix_fmt = PIX_FMT_YUV420P;

        if(codec_id == CODEC_ID_H264)
            av_opt_set(ctx->priv_data, "preset", "slow", 0);

        /* open it */
        if (avcodec_open2(ctx, codec, NULL) < 0) {
            fprintf(stderr, "could not open codec\n");
            exit(1);
        }

        if (! output_stream.good())
            throw std::runtime_error("Error using movie output stream");

        /* alloc image and output buffer */
        outbuf_size = 100000 + 12*ctx->width*ctx->height;
        outbuf = (uint8_t*)malloc(outbuf_size);

        /* the image can be allocated by any means and av_image_alloc() is
         * just the most convenient way if av_malloc() is to be used */
        av_image_alloc(picture->data, picture->linesize,
                       ctx->width, ctx->height, ctx->pix_fmt, 1);

        /* encode 1 second of video */
        for(i=0;i<25;i++) {
            fflush(stdout);
            /* prepare a dummy image */
            /* Y */
            for(y=0;y<ctx->height;y++) {
                for(x=0;x<ctx->width;x++) {
                    picture->data[0][y * picture->linesize[0] + x] = x + y + i * 3;
                }
            }

            /* Cb and Cr */
            for(y=0;y<ctx->height/2;y++) {
                for(x=0;x<ctx->width/2;x++) {
                    picture->data[1][y * picture->linesize[1] + x] = 128 + y + i * 2;
                    picture->data[2][y * picture->linesize[2] + x] = 64 + x + i * 5;
                }
            }

            /* encode the image */
            // TODO use non-deprecated avcodec_encode_video2(...)
            out_size = avcodec_encode_video(ctx, outbuf, outbuf_size, picture);
            had_output |= out_size;
            printf("encoding frame %3d (size=%5d)\n", i, out_size);
            output_stream.write((const char*)outbuf, out_size);
        }

        /* get the delayed frames */
        for(; out_size || !had_output; i++) {
            fflush(stdout);

            out_size = avcodec_encode_video(ctx, outbuf, outbuf_size, NULL);
            had_output |= out_size;
            printf("write frame %3d (size=%5d)\n", i, out_size);
            output_stream.write((const char*)outbuf, out_size);
        }

        /* add sequence end code to have a real mpeg file */
        outbuf[0] = 0x00;
        outbuf[1] = 0x00;
        outbuf[2] = 0x01;
        outbuf[3] = 0xb7;
        output_stream.write((const char*)outbuf, 4);
        free(outbuf);

        avcodec_close(ctx);
        av_free(ctx);
        av_free(picture->data[0]);
        av_free(picture);
        printf("\n");
    }

protected:
    AVCodec * codec;
    AVCodecContext * ctx;
};


#endif // USE_FFMPEG

#endif // FFMPEGVIDEO_H

#include "FFMpegVideo.h"

#ifdef USE_FFMPEG

extern "C"
{
#include <libswscale/swscale.h>
}

#include <stdexcept>
#include <iostream>

using namespace std;

// Translated to C++ by Christopher Bruns May 2012
// from ffmeg_adapt.c in whisk package by Nathan Clack, Mark Bolstadt, Michael Meeuwisse


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


AVPacketWrapper::AVPacketWrapper()
{
    packet.destruct = NULL;
}

/* virtual */
AVPacketWrapper::~AVPacketWrapper()
{
    free();
}

void AVPacketWrapper::free()
{
    av_free_packet(&packet);
}


/////////////////////////
// FFMpegVideo methods //
/////////////////////////

FFMpegVideo::FFMpegVideo() {initialize();}

FFMpegVideo::FFMpegVideo(const std::string& fileName) {
    initialize();
    open(fileName);
}

/* virtual */
FFMpegVideo::~FFMpegVideo()
{
    if (NULL != Sctx) {
        sws_freeContext(Sctx);
        Sctx = NULL;
    }
    if (NULL != pRaw) {
        av_free(pRaw);
        pRaw = NULL;
    }
    if (NULL != pFrameRGB) {
        av_free(pFrameRGB);
        pFrameRGB = NULL;
    }
    if (NULL != pCtx) {
        avcodec_close(pCtx);
        pCtx = NULL;
    }
    if (NULL != pFormatCtx) {
        avformat_close_input(&pFormatCtx);
        pFormatCtx = NULL;
    }
    if (NULL != buffer) {
        av_free(buffer);
        buffer = NULL;
    }
    if (NULL != blank) {
        av_free(blank);
        blank = NULL;
    }
    // Don't need to free pCodec?
}

void FFMpegVideo::open(const std::string& fileName, enum PixelFormat formatParam)
{
    format = formatParam;
    sc = getNumberOfChannels();

    // Open file, check usability
    avtry( avformat_open_input(&pFormatCtx, fileName.c_str(), NULL, NULL), fileName );
    avtry( avformat_find_stream_info(pFormatCtx, NULL), "Cannot find stream information." );
    avtry( videoStream=av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &pCodec, 0), "Cannot find a video stream." );
    pCtx=pFormatCtx->streams[videoStream]->codec;
    width  = pCtx->width;
    height = pCtx->height;
    avtry( avcodec_open2(pCtx, pCodec, NULL), "Cannot open video decoder." );

    /* Frame rate fix for some codecs */
    if( pCtx->time_base.num > 1000 && pCtx->time_base.den == 1 )
        pCtx->time_base.den = 1000;

    /* Compute the total number of frames in the file */
    /* duration is in microsecs */
    numFrames = (int)(( pFormatCtx->duration / (double)AV_TIME_BASE ) * pCtx->time_base.den );

    /* Get framebuffers */
    if (! (pRaw = avcodec_alloc_frame()) )
        throw std::runtime_error("");
    if (! (pFrameRGB = avcodec_alloc_frame()) )
        throw std::runtime_error("");

    /* Create data buffer */
    numBytes = avpicture_get_size( format, pCtx->width, pCtx->height ); // RGB24 format
    if (! (buffer   = (uint8_t*)av_malloc(numBytes + FF_INPUT_BUFFER_PADDING_SIZE)) ) // RGB24 format
        throw std::runtime_error("");
    if (! (blank    = (uint8_t*)av_mallocz(avpicture_get_size(pCtx->pix_fmt,width,height))) ) // native codec format
        throw std::runtime_error("");

    /* Init buffers */
    avpicture_fill( (AVPicture * ) pFrameRGB, buffer, format,
                    pCtx->width, pCtx->height );

    /* Init scale & convert */
    if (! (Sctx=sws_getContext(
            pCtx->width,
            pCtx->height,
            pCtx->pix_fmt,
            width,
            height,
            format,
            SWS_BICUBIC,NULL,NULL,NULL)) )
        throw std::runtime_error("");

    /* Give some info on stderr about the file & stream */
    //dump_format(pFormatCtx, 0, fname, 0);

    previousFrameIndex = -1;
}

// Open a new video file for writing
void FFMpegVideo::write(const std::string& fileName, int width, int height)
{


    enum PixelFormat format = PIX_FMT_RGB24;

    /* Create data buffer */
    numBytes = avpicture_get_size( format, pCtx->width, pCtx->height ); // RGB24 format
    if (! (buffer   = (uint8_t*)av_malloc(numBytes + FF_INPUT_BUFFER_PADDING_SIZE)) ) // RGB24 format
        throw std::runtime_error("");

    /* Init buffers */
    avpicture_fill( (AVPicture * ) pFrameRGB, buffer, format,
                    pCtx->width, pCtx->height );


}

bool FFMpegVideo::fetchFrame(int targetFrameIndex)
{
    if ((targetFrameIndex < 0) || (targetFrameIndex > numFrames))
        return false;
    if (targetFrameIndex == (previousFrameIndex + 1))
        if (! readNextFrame(targetFrameIndex))
            return false;
    else
        if (seekToFrame(targetFrameIndex) < 0)
            return false;
    previousFrameIndex = targetFrameIndex;
}

// \returns current frame on success, otherwise -1
int FFMpegVideo::seekToFrame(int targetFrameIndex)
{
    int64_t duration = pFormatCtx->streams[videoStream]->duration;
    int64_t ts = av_rescale(duration,targetFrameIndex,numFrames);
    int64_t tol = av_rescale(duration,1,2*numFrames);
    if ( (targetFrameIndex < 0) || (targetFrameIndex >= numFrames) ) {
        return -1;
    }
    int result = avformat_seek_file( pFormatCtx, //format context
                            videoStream,//stream id
                            0,               //min timestamp
                            ts,              //target timestamp
                            ts,              //max timestamp
                            0); //AVSEEK_FLAG_ANY),//flags
    if (result < 0)
        return -1;

    avcodec_flush_buffers(pCtx);
    if (! readNextFrame(targetFrameIndex))
        return -1;

    return targetFrameIndex;
}

bool FFMpegVideo::readNextFrame(int targetFrameIndex)
{
    AVPacket packet = {0};
    int finished = 0;
    do {
        finished = 0;
        av_free_packet(&packet);
        int result = av_read_frame( pFormatCtx, &packet ); // !!NOTE: see docs on packet.convergence_duration for proper seeking
        if (result < 0) {
            av_free_packet(&packet);
            return false;
        }
        if( packet.stream_index != videoStream ) /* Is it what we're trying to parse? */
        {
            av_free_packet(&packet);
            continue;
        }
        result = avcodec_decode_video2( pCtx, pRaw, &finished, &packet );
        if (result <= 0) {
            av_free_packet(&packet);
            return false;
        }
        if(pCtx->codec_id==CODEC_ID_RAWVIDEO && !finished)
        {
            avpicture_fill( (AVPicture * ) pRaw, blank, pCtx->pix_fmt,width, height ); // set to blank frame
            finished=1;
        }
#if 0 // very useful for debugging
        cout << "Packet - pts:" << (int)packet.pts;
        cout << " dts:" << (int)packet.dts;
        cout << " - flag: " << packet.flags;
        cout << " - finished: " << finished;
        cout << " - Frame pts:" << (int)pRaw->pts;
        cout << " " << (int)pRaw->best_effort_timestamp;
        cout << endl;
        /* printf("Packet - pts:%5d dts:%5d (%5d) - flag: %1d - finished: %3d - Frame pts:%5d %5d\n",
               (int)packet.pts,(int)packet.dts,
               packet.flags,finished,
               (int)pRaw->pts,(int)pRaw->best_effort_timestamp); */
#endif
        if(!finished)
            if (packet.pts == AV_NOPTS_VALUE)
                throw std::runtime_error("");
    } while ( (!finished) || (pRaw->best_effort_timestamp < targetFrameIndex));

    sws_scale(Sctx,              // sws context
              pRaw->data,        // src slice
              pRaw->linesize,    // src stride
              0,                      // src slice origin y
              pCtx->height,      // src slice height
              pFrameRGB->data,        // dst
              pFrameRGB->linesize );  // dst stride

    av_free_packet(&packet);

    previousFrameIndex = targetFrameIndex;
    return true;
}

uint8_t FFMpegVideo::getPixelIntensity(int x, int y, Channel c) const
{
    return *(pFrameRGB->data[0] + y * pFrameRGB->linesize[0] + x * sc + c);
}

int FFMpegVideo::getNumberOfFrames() const { return numFrames; }

int FFMpegVideo::getWidth() const { return width; }

int FFMpegVideo::getHeight() const { return height; }

int FFMpegVideo::getNumberOfChannels() const
{
    switch(format)
    {
    case PIX_FMT_RGB24:
        return 3;
        break;
    case PIX_FMT_GRAY8:
        return 1;
        break;
    default:
        return 0;
        break;
    }
    return 0;
}

void FFMpegVideo::initialize()
{
    Sctx = NULL;
    pRaw = NULL;
    pFrameRGB = NULL;
    pCtx = NULL;
    pFormatCtx = NULL;
    buffer = NULL;
    blank = NULL;
    pCodec = NULL;
    format = PIX_FMT_NONE;
    FFMpegVideo::maybeInitFFMpegLib();
}

void FFMpegVideo::maybeInitFFMpegLib()
{
    if (FFMpegVideo::b_is_one_time_inited)
        return;
    avcodec_register_all();
    av_register_all();
    avformat_network_init();
    FFMpegVideo::b_is_one_time_inited = true;
}

void FFMpegVideo::avtry(int result, const std::string& msg) {
    if ((result < 0) && (result != AVERROR_EOF)) {
        char buf[1024];
        av_strerror(result, buf, sizeof(buf));
        std::string message = std::string("FFMpeg Error: ") + msg + buf;
        throw std::runtime_error(message.c_str());
    }
}

bool FFMpegVideo::b_is_one_time_inited = false;



///////////////////////////
// FFMpegEncoder methods //
///////////////////////////


FFMpegEncoder::FFMpegEncoder(const char * file_name, int width, int height)
    : picture_raw(NULL)
    , picture_rgb(NULL)
    , pFormatCtx(NULL)
{
    FFMpegVideo::maybeInitFFMpegLib();

    AVOutputFormat * fmt = guess_format(NULL, file_name, NULL);
    if (!fmt)
        fmt = guess_format("mpeg", NULL, NULL);
    if (!fmt)
        throw std::runtime_error("Unable to deduce video format");
    pFormatCtx = av_alloc_format_context();
    if (NULL == pFormatCtx)
        throw std::runtime_error("Unable to allocate format context");
    pFormatCtx->oformat = fmt;

    AVStream * video_st = av_new_stream(pFormatCtx, 0);
    AVCodecContext * pCtx = &video_st->codec;
    pCtx->codec_id = fmt->video_codec;
    pCtx->codec_type = CODEC_TYPE_VIDEO;
    // resolution must be a multiple of two
    pCtx->bit_rate = 400000; // ?
    pCtx->width = width;
    pCtx->height = height;
    pCtx->time_base = (AVRational){1, 25};
    pCtx->gop_size = 10; // emit one intra frame every ten frames
    pCtx->max_b_frames = 0;
    pCtx->pix_fmt = PIX_FMT_YUV420P;

    if (av_set_parameters(pFormatCtx, NULL) < 0)
        throw std::runtime_error("Error setting format parameters");

    AVCodec * pCodec = avcodec_find_encoder(pCtx->codec_id);
    if (avcodec_open2(pCtx, pCodec, NULL) < 0)
        throw std::runtime_error("");

    /* open the output file, if needed */
    if (!(fmt->flags & AVFMT_NOFILE))
        if (url_fopen(&pFormatCtx->pb, file_name, URL_WRONLY) < 0)
             throw std::runtime_error("Error opening output video file");

    /* Get framebuffers */
    if (! (picture_raw = avcodec_alloc_frame()) ) // final frame format
        throw std::runtime_error("");
    if (! (picture_rgb = avcodec_alloc_frame()) ) // rgb version I can understand easily
        throw std::runtime_error("");

    av_write_header(pFormatCtx);

    if (! output_stream.good())
        throw std::runtime_error("Error using movie output stream");

    /* the image can be allocated by any means and av_image_alloc() is
         * just the most convenient way if av_malloc() is to be used */
    av_image_alloc(picture_raw->data, picture_raw->linesize,
                   pCtx->width, pCtx->height, pCtx->pix_fmt, 1);
    av_image_alloc(picture_rgb->data, picture_rgb->linesize,
                   pCtx->width, pCtx->height, PIX_FMT_RGB24, 1);

    /* Init scale & convert */
    if (! (Sctx=sws_getContext(
            width,
            height,
            PIX_FMT_RGB24,
            pCtx->width,
            pCtx->height,
            pCtx->pix_fmt,
            SWS_BICUBIC,NULL,NULL,NULL)) )
        throw std::runtime_error("");
}

void FFMpegEncoder::write_frame()
{
    // TODO - populate frame

    // convert from RGB24 to YUV
    sws_scale(Sctx,              // sws context
              picture_rgb->data,        // src slice
              picture_rgb->linesize,    // src stride
              0,                      // src slice origin y
              pCtx->height,      // src slice height
              picture_raw->data,        // dst
              picture_raw->linesize );  // dst stride

    /* encode the image */
    // use non-deprecated avcodec_encode_video2(...)
    AVPacket packet;
    av_init_packet(&packet);
    packet.data = NULL;
    packet.size = 0;

    int got_packet;
    int ret = avcodec_encode_video2(pCtx,
                                    &packet,
                                    picture_raw,
                                    &got_packet);
    if (ret < 0)
        throw std::runtime_error("Video encoding failed");
    if (got_packet)
    {
        output_stream.write((const char *)packet.data, packet.size);
        av_destruct_packet(&packet);
        std::cout << "encoding frame" << std::endl;
    }
}

void FFMpegEncoder::write_delayed_frames()
{
    AVPacket packet;
    while (true) {
        av_init_packet(&packet);
        packet.data = NULL;
        packet.size = 0;
        int got_packet;
        int ret = avcodec_encode_video2(pCtx, &packet, NULL, &got_packet);
        if ( (ret < 0) || (! got_packet) )
            break;
        std::cout << "got extra packet" << std::endl;
        output_stream.write((const char *)packet.data, packet.size);
        av_destruct_packet(&packet);
    }
}

/* virtual */
FFMpegEncoder::~FFMpegEncoder()
{
    /* add sequence end code to have a real mpeg file */
    uint8_t end_code[] = {0x00, 0x00, 0x01, 0xb7};
    output_stream.write((const char*)end_code, 4);
    output_stream.flush();

    avcodec_close(pCtx);
    av_free(pCtx);
    pCtx = NULL;
    av_free(picture_raw->data[0]);
    av_free(picture_raw);
    picture_raw = NULL;
    av_free(picture_rgb->data[0]);
    av_free(picture_rgb);
    picture_rgb = NULL;
}




#endif // USE_FFMPEG


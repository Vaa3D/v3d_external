#include "FFMpegVideo.h"
#include "FFMpegVideo_utility.h"

#ifdef USE_FFMPEG

extern "C"
{
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>

}

#include <boost/regex.hpp>

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QFileInfo>
#include <QMutexLocker>
#include <QDebug>
#include <stdexcept>
#include <iostream>
#include <cassert>

using namespace std;

// Translated to C++ by Christopher Bruns May 2012
// from ffmeg_adapt.c in whisk package by Nathan Clack, Mark Bolstadt, Michael Meeuwisse

QMutex FFMpegVideo::mutex;


/////////////////////////
// FFMpegVideo methods //
/////////////////////////

FFMpegVideo::FFMpegVideo()
    : isOpen(false)
{
    initialize();
}

FFMpegVideo::FFMpegVideo(QUrl url )
    : isOpen(false)
{
    QMutexLocker lock(&FFMpegVideo::mutex);
    initialize();
    isOpen = open(url);
}

FFMpegVideo::FFMpegVideo(QByteArray* buffer )
    : isOpen(false)
{
    QMutexLocker lock(&FFMpegVideo::mutex);
    initialize();
    fileBuffer.setBuffer(buffer);
    fileBuffer.open(QIODevice::ReadOnly);
    QString foo("foo");
    isOpen = open(fileBuffer, foo );
}

/* virtual */
FFMpegVideo::~FFMpegVideo()
{
    QMutexLocker lock(&FFMpegVideo::mutex);
    if ( NULL != Sctx )
    {
        sws_freeContext(Sctx);
        Sctx = NULL;
    }
    if ( NULL != pRaw )
    {
        av_free(pRaw);
        pRaw = NULL;
    }
    if ( NULL != pFrameRGB )
    {
        av_free(pFrameRGB);
        pFrameRGB = NULL;
    }
    if ( NULL != pCtx )
    {
        avcodec_close(pCtx);
        pCtx = NULL;
    }
    if ( NULL != container )
    {
        avformat_close_input(&container);
        container = NULL;
    }
    if ( reply != NULL )
    {
        reply->deleteLater();
        reply = NULL;
    }
    // Don't need to free pCodec?
}

bool FFMpegVideo::open(QUrl url)
{
    if (url.isEmpty())
        return false;

    // Is the movie source a local file?
    if (url.host() == "localhost")
        url.setHost("");
    QString fileName = url.toLocalFile();
    if ( (! fileName.isEmpty())
        && (QFileInfo(fileName).exists()) )
    {
        // Yes, the source is a local file
        fileStream.setFileName(fileName);
        // qDebug() << fileName;
        if (! fileStream.open(QIODevice::ReadOnly))
            return false;
        return open(fileStream, fileName );
    }

    // ...No, the source is not a local file
    if (url.host() == "")
        url.setHost("localhost");
    fileName = url.path();

    // http://stackoverflow.com/questions/9604633/reading-a-file-located-in-memory-with-libavformat
    // Load from URL
    QEventLoop loop; // for synchronous url fetch http://stackoverflow.com/questions/5486090/qnetworkreply-wait-for-finished
    QObject::connect(&networkManager, SIGNAL(finished(QNetworkReply*)),
            &loop, SLOT(quit()));
    QNetworkRequest request = QNetworkRequest(url);
    // qDebug() << "networkManager" << __FILE__ << __LINE__;
    reply = networkManager.get(request);
    loop.exec();
    if ( reply->error() != QNetworkReply::NoError )
    {
        // qDebug() << reply->error();
        reply->deleteLater();
        reply = NULL;
        return false;
    }
    QIODevice * stream = reply;
    // Mpeg needs seekable device, so create in-memory buffer if necessary
    if ( stream->isSequential() )
    {
        byteArray = stream->readAll();
        fileBuffer.setBuffer(&byteArray);
        fileBuffer.open(QIODevice::ReadOnly);
        if (! fileBuffer.seek(0))
            return false;
        stream = &fileBuffer;
        assert(! stream->isSequential());
    }
    bool result = open(*stream, fileName);
    return result;
}

bool FFMpegVideo::open(QIODevice& fileStream, QString& fileName)
{
    // http://stackoverflow.com/questions/9604633/reading-a-file-located-in-memory-with-libavformat
    // I think AVIOContext is the trick used to redivert the input stream
    ioBuffer = (unsigned char *)av_malloc(ioBufferSize + AV_INPUT_BUFFER_PADDING_SIZE); // can get av_free()ed by libav
    avioContext = avio_alloc_context(ioBuffer, ioBufferSize, 0, (void*)(&fileStream), &readFunction, NULL, &seekFunction);
    container = avformat_alloc_context();
    container->pb = avioContext;

    // Open file, check usability
    std::string fileNameStd = fileName.toStdString();
    if (!avtry( avformat_open_input(&container, fileNameStd.c_str(), NULL, NULL), fileNameStd ))
        return false;
    return openUsingInitializedContainer();
}

// file name based method for historical continuity
bool FFMpegVideo::open(QString& fileName)
{
    // Open file, check usability
    std::string fileNameStd = fileName.toStdString();
    if (!avtry( avformat_open_input(&container, fileNameStd.c_str(), NULL, NULL), fileNameStd ))
        return false;
    return openUsingInitializedContainer();
}


bool FFMpegVideo::openUsingInitializedContainer()
{
    if (!avtry( avformat_find_stream_info(container, NULL), "Cannot find stream information." ))
        return false;

    if (open_codec_context(&videoStream, &pCtx, container, AVMEDIA_TYPE_VIDEO) >= 0)
    {
       width = pCtx->width;
       height = pCtx->height;
       format = pCtx->pix_fmt;

       /* Frame rate fix for some codecs */
       if ( pCtx->time_base.num > 1000 && pCtx->time_base.den == 1 )
          pCtx->time_base.den = 1000;

       /* Compute the total number of frames in the file */
       /* duration is in microsecs */
       numFrames = (int)( container->streams[ videoStream ]->nb_frames );
       if ( numFrames == 0 )
          numFrames = (int)( ( container->duration / (double)AV_TIME_BASE ) *
                                 pCtx->time_base.den +
                             0.5 );

       /* Get framebuffers */
       if ( !( pRaw = av_frame_alloc() ) )
          throw std::runtime_error( "" );
       if ( !( pFrameRGB = av_frame_alloc() ) )
          throw std::runtime_error( "" );

       if ( av_image_alloc( pFrameRGB->data, pFrameRGB->linesize, width, height,
                            format, 1 ) < 0 ) 
          throw std::runtime_error( "" );
       /* Init scale & convert */
       if ( !( Sctx = sws_getContext( pCtx->width, pCtx->height, pCtx->pix_fmt, width,
                                      height, format,
                                      SWS_POINT, // fastest?
                                      NULL, NULL, NULL ) ) )
          throw std::runtime_error( "" );
    }

    /* Give some info on stderr about the file & stream */
    //dump_format(container, 0, fname, 0);

    previousFrameIndex = -1;
    return true;
}

bool FFMpegVideo::fetchFrame(int targetFrameIndex)
{
    if ((targetFrameIndex < 0) || (targetFrameIndex > numFrames))
        return false;
    if ( targetFrameIndex == ( previousFrameIndex + 1 ) )
    {
        if (! readNextFrame(targetFrameIndex))
            return false;
    }
    else if ( seekToFrame( targetFrameIndex ) < 0 )
            return false;
    previousFrameIndex = targetFrameIndex;
    return true;
}

// \returns current frame on success, otherwise -1
int FFMpegVideo::seekToFrame(int targetFrameIndex)
{
    int64_t duration = container->streams[videoStream]->duration;
    int64_t ts = av_rescale(duration,targetFrameIndex,numFrames);
    int64_t tol = av_rescale(duration,1,2*numFrames);
    if ( ( targetFrameIndex < 0 ) || ( targetFrameIndex >= numFrames ) )
    {
        return -1;
    }
    int result = avformat_seek_file( container, //format context
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
    av_init_packet(&packet);
    bool result = readNextFrameWithPacket(targetFrameIndex, packet, pRaw);
    av_packet_unref(&packet);
    return result;
}

// WARNING this method can raise an exception
bool FFMpegVideo::readNextFrameWithPacket(int targetFrameIndex, AVPacket& packet, AVFrame* pYuv)
{
    int finished = 0;
    do
    {
        finished = 0;
        av_free_packet(&packet);
        int result;
        if (!avtry(av_read_frame( container, &packet ), "Failed to read frame"))
            return false; // !!NOTE: see docs on packet.convergence_duration for proper seeking
        if( packet.stream_index != videoStream ) /* Is it what we're trying to parse? */
            continue;
        if (!avtry(avcodec_decode_video2( pCtx, pYuv, &finished, &packet ), "Failed to decode video"))
            return false;
        if ( !finished )
        {
            if (packet.size == 0) // packet.size==0 usually means EOF
                break;
        }
    }
    while ( ( !finished ) || ( pYuv->best_effort_timestamp < targetFrameIndex ) );

    av_free_packet(&packet);

    if ( finished && format != AV_PIX_FMT_NONE )
    {
        sws_scale(Sctx,              // sws context
                  pYuv->data,        // src slice
                  pYuv->linesize,    // src stride
                  0,                      // src slice origin y
                  pCtx->height,      // src slice height
                  pFrameRGB->data,        // dst
                  pFrameRGB->linesize );  // dst stride
    }

    previousFrameIndex = targetFrameIndex;
    return true;
}

uint8_t FFMpegVideo::getPixelIntensity(int x, int y, Channel c) const
{
    return *(pFrameRGB->data[0] + y * pFrameRGB->linesize[0] + x * sc + c);
}

uint16_t FFMpegVideo::getPixelIntensity16(int x, int y, Channel c) const
{
    return *((uint16_t *)(pFrameRGB->data[0]) + y * pFrameRGB->linesize[0]/2 + x);
}

int FFMpegVideo::getBitDepth() const { return (pCtx == 0 ? 0 : pCtx->pix_fmt == AV_PIX_FMT_GRAY12 ? 2 : 1); }
int FFMpegVideo::getNumberOfFrames() const { return numFrames; }

int FFMpegVideo::getWidth() const { return width; }

int FFMpegVideo::getHeight() const { return height; }

int FFMpegVideo::getNumberOfChannels() const
{
    switch(format)
    {
    case AV_PIX_FMT_BGRA:
        return 4;
        break;
    case AV_PIX_FMT_RGB24:
    case AV_PIX_FMT_YUV444P:
        return 3;
        break;
    case AV_PIX_FMT_GRAY16:
    case AV_PIX_FMT_GRAY12:
    case AV_PIX_FMT_GRAY8:
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
    container = NULL;
    pCodec = NULL;
    format = AV_PIX_FMT_NONE;
    reply = NULL;
    ioBuffer = NULL;
    avioContext = NULL;
    FFMpegVideo::maybeInitFFMpegLib();
}

void FFMpegVideo::maybeInitFFMpegLib()
{
    if (FFMpegVideo::b_is_one_time_inited)
        return;

    avcodec_register_all();
    avdevice_register_all();
    av_register_all();
    avformat_network_init();
    FFMpegVideo::b_is_one_time_inited = true;
}

bool FFMpegVideo::avtry( int result, const std::string& msg )
{
    if ( ( result < 0 ) && ( result != AVERROR_EOF ) )
    {
        char buf[1024];
        av_strerror(result, buf, sizeof(buf));
        std::string message = std::string("FFMpeg Error: ") + msg + buf;
        qDebug() << QString(message.c_str());
        return false;
    }
    return true;
}

int
FFMpegVideo::open_codec_context( int *stream_idx, AVCodecContext **dec_ctx,
                                 AVFormatContext *fmt_ctx, enum AVMediaType type )
{
   int ret, stream_index;
   AVStream *st;
   AVCodec *dec = NULL;
   AVDictionary *opts = NULL;
   ret = av_find_best_stream( fmt_ctx, type, -1, -1, NULL, 0 );
   if ( ret < 0 )
   {
      return ret;
   }
   else
   {
      stream_index = ret;
      st = fmt_ctx->streams[ stream_index ];
      /* find decoder for the stream */
      dec = avcodec_find_decoder( st->codecpar->codec_id );
      if ( !dec )
      {
         fprintf( stderr, "Failed to find %s codec\n", av_get_media_type_string( type ) );
         return AVERROR( EINVAL );
      }
      /* Allocate a codec context for the decoder */
      *dec_ctx = avcodec_alloc_context3( dec );
      if ( !*dec_ctx )
      {
         fprintf( stderr, "Failed to allocate the %s codec context\n",
                  av_get_media_type_string( type ) );
         return AVERROR( ENOMEM );
      }
      /* Copy codec parameters from input stream to output codec context */
      if ( ( ret = avcodec_parameters_to_context( *dec_ctx, st->codecpar ) ) < 0 )
      {
         fprintf( stderr, "Failed to copy %s codec parameters to decoder context\n",
                  av_get_media_type_string( type ) );
         return ret;
      }
      /* Init the decoders, with or without reference counting */
      av_dict_set( &opts, "refcounted_frames", "1", 0 );
      if ( ( ret = avcodec_open2( *dec_ctx, dec, &opts ) ) < 0 )
      {
         fprintf( stderr, "Failed to open %s codec\n", av_get_media_type_string( type ) );
         return ret;
      }
      *stream_idx = stream_index;
   }
   return 0;
}

bool FFMpegVideo::b_is_one_time_inited = false;



///////////////////////////
// FFMpegEncoder methods //
///////////////////////////


FFMpegEncoder::FFMpegEncoder( const char* file_name, int width, int height, int bit_depth,
                              enum AVCodecID codec_id, std::string options )
    : picture_yuv(NULL)
    , picture_rgb(NULL)
    , container(NULL)
    , use_buffer( false )
    , _buffer_size( 0 )
    , _buffer( NULL )
    , _frame_count( 0 )
    , _encoded_frames( 0 )
{

    if (0 != (width % 2))
        cerr << "WARNING: Video width is not a multiple of 2" << endl;
    if (0 != (height % 2))
        cerr << "WARNING: Video height is not a multiple of 2" << endl;

    FFMpegVideo::maybeInitFFMpegLib();

    AVCodec * codec = avcodec_find_encoder(codec_id);
    if (NULL == codec)
        throw std::runtime_error("Unable to find Mpeg4 codec");

    container = avformat_alloc_context();
    if (NULL == container)
        throw std::runtime_error("Unable to allocate format context");

    AVOutputFormat* fmt = NULL;
    if ( file_name == NULL )
        if ( codec_id == AV_CODEC_ID_FFV1 )
            fmt = av_guess_format( "mov", NULL, NULL );
        else
        fmt = av_guess_format("mp4", NULL, NULL);
    else
    {
        fmt = av_guess_format(NULL, file_name, NULL);
    }
    if (!fmt)
        fmt = av_guess_format("mpeg", NULL, NULL);
    if (!fmt)
        throw std::runtime_error("Unable to deduce video format");

    container->oformat = fmt;

    fmt->video_codec = codec_id;
    
    pCtx = avcodec_alloc_context3(codec);
    pCtx->width = width;
    pCtx->height = height;
    pCtx->bit_rate = width * height * 4; // ?
    pCtx->time_base = (AVRational){1, 25};
    pCtx->gop_size = 12; // emit one intra frame every twelve frames

    if (fmt->flags & AVFMT_GLOBALHEADER)
        pCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    AVDictionary* codec_options( 0 );
    _raw_format = AV_PIX_FMT_RGB24;
    switch ( pCtx->codec_id )
    {
        case AV_CODEC_ID_H264:
        {
           // http://stackoverflow.com/questions/3553003/encoding-h-264-with-libavcodec-x264
           pCtx->coder_type = 1;                  // coder = 1
           pCtx->flags |= AV_CODEC_FLAG_LOOP_FILTER; // flags=+loop
           pCtx->me_cmp |= 1;                     // cmp=+chroma, where CHROMA = 1
           // pCtx->partitions|=X264_PART_I8X8+X264_PART_I4X4+X264_PART_P8X8+X264_PART_B8X8;
           // // partitions=+parti8x8+parti4x4+partp8x8+partb8x8
           pCtx->me_subpel_quality = 7;      // subq=7
           pCtx->me_range = 16;              // me_range=16
           pCtx->gop_size = 250;             // g=250
           pCtx->keyint_min = 25;            // keyint_min=25
           pCtx->scenechange_threshold = 40; // sc_threshold=40
           pCtx->i_quant_factor = 0.71;      // i_qfactor=0.71
           pCtx->b_frame_strategy = 1;       // b_strategy=1
           pCtx->qcompress = 0.6;            // qcomp=0.6
           pCtx->qmin = 10;                  // qmin=10
           pCtx->qmax = 51;                  // qmax=51
           pCtx->max_qdiff = 4;              // qdiff=4
           pCtx->max_b_frames = 3;           // bf=3
           pCtx->refs = 3;                   // refs=3
           // pCtx->directpred = 1;  // directpred=1
           pCtx->trellis = 1; // trellis=1
                              // pCtx->flags2|=CODEC_FLAG2_BPYRAMID+CODEC_FLAG2_MIXED_REFS+CODEC_FLAG2_WPRED+CODEC_FLAG2_8X8DCT+CODEC_FLAG2_FASTPSKIP;
                              // // flags2=+bpyramid+mixed_refs+wpred+dct8x8+fastpskip
                              // pCtx->weighted_p_pred = 2; // wpredp=2
                              // libx264-main.ffpreset preset
                              // pCtx->flags2|=CODEC_FLAG2_8X8DCT;
                              // pCtx->flags2^=CODEC_FLAG2_8X8DCT;    // flags2=-dct8x8
           pCtx->pix_fmt = AV_PIX_FMT_YUV420P;
           break;
        }
        case AV_CODEC_ID_HEVC:
        {
           av_dict_set( &codec_options, "preset", "medium", 0 );
           av_dict_set( &codec_options, "x265-params", options.c_str(), 0 );
           pCtx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
           if ( bit_depth == 1 )
              pCtx->pix_fmt = AV_PIX_FMT_YUV444P;
           else
           {
              pCtx->pix_fmt = AV_PIX_FMT_GRAY12;
              _raw_format = AV_PIX_FMT_GRAY16;
           }
           break;
        }
        case AV_CODEC_ID_FFV1:
        {
           boost::regex re2( "\\:" );
           boost::sregex_token_iterator ii( options.begin(), options.end(), re2, -1 );
           boost::sregex_token_iterator it_end;
           while ( ii != it_end )
           {
              std::string param( *ii++ );
              std::string value( *ii++ );
              av_dict_set( &codec_options, param.c_str(), value.c_str(), 0 );
           }
           pCtx->pix_fmt = AV_PIX_FMT_YUV444P;
           break;
        }
        default:
           pCtx->pix_fmt = AV_PIX_FMT_YUV420P;
           break;
    }

    {
        QMutexLocker lock(&FFMpegVideo::mutex);
        if (avcodec_open2(pCtx, codec, &codec_options) < 0)
            throw std::runtime_error("Error opening codec");
    }

    /* open the output file */
    if (!(fmt->flags & AVFMT_NOFILE))
    {
        QMutexLocker lock(&FFMpegVideo::mutex);
        if ( file_name == NULL )
        {
            use_buffer = true;
            if ( avio_open_dyn_buf( &container->pb ) != 0 )
                 throw std::runtime_error("Error opening memory buffer for encoding");
        }
        else if ( avio_open( &container->pb, file_name, AVIO_FLAG_WRITE ) < 0 )
                 throw std::runtime_error("Error opening output video file");
    }

    video_st = avformat_new_stream(container, codec);
    if (!video_st) {
        fprintf(stderr, "Could not allocate stream\n");
        exit(1);
    }
    video_st->id = container->nb_streams-1;
    video_st->time_base = pCtx->time_base;

    if (avcodec_parameters_from_context(video_st->codecpar, pCtx) < 0)
        throw std::runtime_error("avcodec_parameters_from_context failed");

    /* Get framebuffers */
    if ( ! ( picture_yuv = av_frame_alloc() ) ) // final frame format
        throw std::runtime_error("");
    if ( ! ( picture_rgb = av_frame_alloc() ) ) // rgb version I can understand easily
        throw std::runtime_error("");
    /* the image can be allocated by any means and av_image_alloc() is
         * just the most convenient way if av_malloc() is to be used */
    if ( av_image_alloc(picture_yuv->data, picture_yuv->linesize,
                       pCtx->width, pCtx->height, pCtx->pix_fmt, 1) < 0 )
        throw std::runtime_error("Error allocating YUV frame buffer");
    if ( av_image_alloc(picture_rgb->data, picture_rgb->linesize,
                   pCtx->width, pCtx->height, _raw_format, 1) < 0 )
        throw std::runtime_error("Error allocating RGB frame buffer");

    // Fill in frame parameters
    picture_yuv->format = pCtx->pix_fmt;
    picture_yuv->width = pCtx->width;
    picture_yuv->height = pCtx->height;

    picture_rgb->format = _raw_format;
    picture_rgb->width = pCtx->width;
    picture_rgb->height = pCtx->height;

    /* Init scale & convert */
    if (! (Sctx=sws_getContext(
            width,
            height,
            _raw_format,
            pCtx->width,
            pCtx->height,
            pCtx->pix_fmt,
            SWS_BICUBIC,NULL,NULL,NULL)) )
        throw std::runtime_error( "Error in scaling" );

    avformat_write_header(container, &codec_options);
}

void FFMpegEncoder::setPixelIntensity(int x, int y, int c, uint8_t value)
{
    uint8_t * ptr = picture_rgb->data[0];
    ptr[y * picture_rgb->linesize[0] + x * 3 + c] = value;
}

void FFMpegEncoder::setPixelIntensity16(int x, int y, uint16_t value)
{
    uint16_t * ptr = (uint16_t *)(picture_rgb->data[0]);
    ptr[y * picture_rgb->linesize[0]/2 + x] = value;
}

void FFMpegEncoder::write_frame()
{
    // convert from RGB24 to YUV
    sws_scale(Sctx,              // sws context
              picture_rgb->data,        // src slice
              picture_rgb->linesize,    // src stride
              0,                      // src slice origin y
              pCtx->height,      // src slice height
              picture_yuv->data,        // dst
              picture_yuv->linesize );  // dst stride

    /* encode the image */
    // use non-deprecated avcodec_encode_video2(...)
    encode( picture_yuv );
}

void FFMpegEncoder::encode( AVFrame* picture )
{
    AVPacket* packet = av_packet_alloc();

    if ( picture )
    {
       picture->pts = _frame_count;
        _frame_count++;
    }

    int ret = 0;

    if (picture)
    {
        ret = avcodec_send_frame(pCtx, picture);
            if (ret < 0)
                throw std::runtime_error( "Can not send frame" );
    }
    else
        ret = avcodec_send_frame(pCtx, (AVFrame*)0);


    while ( ret >= 0 )
    {
        ret = avcodec_receive_packet(pCtx, packet);
        if (ret == AVERROR(EAGAIN) )
            return;
        else if (ret == AVERROR_EOF)
            return;
        else if (ret < 0)
            throw std::runtime_error( "Can not recieve packet" );

        if ( pCtx->codec_id == AV_CODEC_ID_HEVC )
        {
            if (packet->pts == AV_NOPTS_VALUE && (pCtx->codec->capabilities & AV_CODEC_CAP_DELAY) == 0) 
                packet->pts = _encoded_frames;
            
            packet->stream_index = video_st->index;
            packet->duration = 1;
            av_packet_rescale_ts(packet, pCtx->time_base, video_st->time_base);
        }
        
        _encoded_frames++;

        int result = av_interleaved_write_frame(container, packet);
        av_packet_unref(packet);
    }
    av_packet_free(&packet);
}

void FFMpegEncoder::close()
{
    int result = av_write_frame(container, NULL); // flush
    result = av_write_trailer(container);
    {
        QMutexLocker lock(&FFMpegVideo::mutex);
        if ( use_buffer )
            _buffer_size = avio_close_dyn_buf( container->pb, &_buffer );
        else
            avio_close(container->pb);
    }
}

/* virtual */
FFMpegEncoder::~FFMpegEncoder()
{
    if ( use_buffer )
        av_free( _buffer );

    for (int i = 0; i < container->nb_streams; ++i)
        av_freep(container->streams[i]);
    av_free(container);
    container = NULL;

    {
        QMutexLocker lock(&FFMpegVideo::mutex);
        avcodec_close(pCtx);
    }
    av_free(pCtx);
    pCtx = NULL;
    av_free(picture_yuv->data[0]);
    av_free(picture_yuv);
    picture_yuv = NULL;
    av_free(picture_rgb->data[0]);
    av_free(picture_rgb);
    picture_rgb = NULL;
}

#endif // USE_FFMPEG


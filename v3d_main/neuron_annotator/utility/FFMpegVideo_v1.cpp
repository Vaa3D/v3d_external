
#ifdef USE_FFMPEG

#include "FFMpegVideo_v1.h"
#include "FFMpegVideo_utility.h"

extern "C"
{
#include <libswscale/swscale.h>
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

QMutex FFMpegVideo_v1::mutex;

/////////////////////////
// FFMpegVideo methods //
/////////////////////////

FFMpegVideo_v1::FFMpegVideo_v1(AVPixelFormat pixelFormat)
    : isOpen(false)
{
    initialize();
    format = pixelFormat;
}

FFMpegVideo_v1::FFMpegVideo_v1(QUrl url, AVPixelFormat pixelFormat)
    : isOpen(false)
{
    QMutexLocker lock(&FFMpegVideo_v1::mutex);
    initialize();
    format = pixelFormat;
    isOpen = open(url, pixelFormat);
}

FFMpegVideo_v1::FFMpegVideo_v1(QByteArray* buffer, AVPixelFormat pixelFormat)
    : isOpen(false)
{
    QMutexLocker lock(&FFMpegVideo_v1::mutex);
    initialize();
    format = pixelFormat;
    fileBuffer.setBuffer(buffer);
    fileBuffer.open(QIODevice::ReadOnly);
    QString foo("foo");
    isOpen = open(fileBuffer, foo, pixelFormat);
}

/* virtual */
FFMpegVideo_v1::~FFMpegVideo_v1()
{
    QMutexLocker lock(&FFMpegVideo_v1::mutex);
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
    if ( NULL != buffer )
    {
        av_free(buffer);
        buffer = NULL;
    }
    if ( NULL != blank )
    {
        av_free(blank);
        blank = NULL;
    }
    /*
    if (NULL != avioContext) {
        av_free(avioContext);
        avioContext = NULL;
    }
    */
    if ( reply != NULL )
    {
        reply->deleteLater();
        reply = NULL;
    }
    // Don't need to free pCodec?
}

bool FFMpegVideo_v1::open(QUrl url, enum AVPixelFormat formatParam)
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
        // return open(fileName, formatParam); // for testing only

        // Yes, the source is a local file
        fileStream.setFileName(fileName);
        // qDebug() << fileName;
        if (! fileStream.open(QIODevice::ReadOnly))
            return false;
        return open(fileStream, fileName, formatParam);
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
    bool result = open(*stream, fileName, formatParam);
    return result;
}

bool FFMpegVideo_v1::open(QIODevice& fileStream, QString& fileName, enum AVPixelFormat formatParam)
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
    return openUsingInitializedContainer(formatParam);
}

// file name based method for historical continuity
bool FFMpegVideo_v1::open(QString& fileName, enum AVPixelFormat formatParam)
{
    // Open file, check usability
    std::string fileNameStd = fileName.toStdString();
    if (!avtry( avformat_open_input(&container, fileNameStd.c_str(), NULL, NULL), fileNameStd ))
        return false;
    return openUsingInitializedContainer(formatParam);
}


bool FFMpegVideo_v1::openUsingInitializedContainer(enum AVPixelFormat formatParam)
{
    format = formatParam;
    sc = getNumberOfChannels();

    if (!avtry( avformat_find_stream_info(container, NULL), "Cannot find stream information." ))
        return false;
    if (!avtry( videoStream=av_find_best_stream(container, AVMEDIA_TYPE_VIDEO, -1, -1, &pCodec, 0), "Cannot find a video stream." ))
        return false;
    pCtx=container->streams[videoStream]->codec;
    width  = pCtx->width;
    height = pCtx->height;
    if (!avtry( avcodec_open2(pCtx, pCodec, NULL), "Cannot open video decoder." ))
        return false;

    /* Frame rate fix for some codecs */
    if( pCtx->time_base.num > 1000 && pCtx->time_base.den == 1 )
        pCtx->time_base.den = 1000;

    /* Compute the total number of frames in the file */
    /* duration is in microsecs */
    numFrames = (int)(container->streams[videoStream]->nb_frames);
    if ( numFrames == 0 )
        numFrames = (int)(( container->duration / (double)AV_TIME_BASE ) * pCtx->time_base.den + 0.5);

    /* Get framebuffers */
    if (! (pRaw = av_frame_alloc()) )
        throw std::runtime_error("");
    if (! (pFrameRGB = av_frame_alloc()) )
        throw std::runtime_error("");

    /* Create data buffer */
    if ( format == AV_PIX_FMT_NONE )
    {
        numBytes = 0;
        buffer = NULL;
        blank = NULL;
        pFrameRGB = NULL;
        Sctx = NULL;
    }
    else
    {
        numBytes = avpicture_get_size( format, pCtx->width, pCtx->height ); // RGB24 format
        if (! (buffer   = (uint8_t*)av_malloc(numBytes + AV_INPUT_BUFFER_PADDING_SIZE)) ) // RGB24 format
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
                SWS_POINT, // fastest?
                NULL,NULL,NULL)) )
            throw std::runtime_error("");
    }

    /* Give some info on stderr about the file & stream */
    //dump_format(container, 0, fname, 0);

    previousFrameIndex = -1;
    return true;
}

bool FFMpegVideo_v1::fetchFrame(int targetFrameIndex)
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
int FFMpegVideo_v1::seekToFrame(int targetFrameIndex)
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

bool FFMpegVideo_v1::readNextFrame(int targetFrameIndex)
{
    AVPacket packet = {0};
    av_init_packet(&packet);
    bool result = readNextFrameWithPacket(targetFrameIndex, packet, pRaw);
    av_free_packet(&packet);
    return result;
}

// WARNING this method can raise an exception
bool FFMpegVideo_v1::readNextFrameWithPacket(int targetFrameIndex, AVPacket& packet, AVFrame* pYuv)
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
        // handle odd cases and debug
        if((pCtx->codec_id==AV_CODEC_ID_RAWVIDEO) && !finished)
        {
            avpicture_fill( (AVPicture * ) pYuv, blank, pCtx->pix_fmt,width, height ); // set to blank frame
            finished = 1;
        }
#if 0 // very useful for debugging
        cout << "Packet - pts:" << (int)packet.pts;
        cout << " dts:" << (int)packet.dts;
        cout << " - flag: " << packet.flags;
        cout << " - finished: " << finished;
        cout << " - Frame pts:" << (int)pYuv->pts;
        cout << " " << (int)pYuv->best_effort_timestamp;
        cout << endl;
        /* printf("Packet - pts:%5d dts:%5d (%5d) - flag: %1d - finished: %3d - Frame pts:%5d %5d\n",
               (int)packet.pts,(int)packet.dts,
               packet.flags,finished,
               (int)pYuv->pts,(int)pYuv->best_effort_timestamp); */
#endif
        if ( !finished )
        {
            if (packet.pts == AV_NOPTS_VALUE)
                throw std::runtime_error("");
            if (packet.size == 0) // packet.size==0 usually means EOF
                break;
        }
    }
    while ( ( !finished ) || ( pYuv->best_effort_timestamp < targetFrameIndex ) );

    av_free_packet(&packet);

    if ( format != AV_PIX_FMT_NONE )
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

uint8_t FFMpegVideo_v1::getPixelIntensity(int x, int y, Channel c) const
{
    return *(pFrameRGB->data[0] + y * pFrameRGB->linesize[0] + x * sc + c);
}

int FFMpegVideo_v1::getNumberOfFrames() const { return numFrames; }

int FFMpegVideo_v1::getWidth() const { return width; }

int FFMpegVideo_v1::getHeight() const { return height; }

int FFMpegVideo_v1::getNumberOfChannels() const
{
    switch(format)
    {
    case AV_PIX_FMT_BGRA:
        return 4;
        break;
    case AV_PIX_FMT_RGB24:
        return 3;
        break;
    case AV_PIX_FMT_GRAY8:
        return 1;
        break;
    default:
        return 0;
        break;
    }
    return 0;
}

void FFMpegVideo_v1::initialize()
{
    Sctx = NULL;
    pRaw = NULL;
    pFrameRGB = NULL;
    pCtx = NULL;
    container = NULL;
    buffer = NULL;
    blank = NULL;
    pCodec = NULL;
    format = AV_PIX_FMT_NONE;
    reply = NULL;
    ioBuffer = NULL;
    avioContext = NULL;
    FFMpegVideo_v1::maybeInitFFMpegLib();
}

void FFMpegVideo_v1::maybeInitFFMpegLib()
{
    if (FFMpegVideo_v1::b_is_one_time_inited)
        return;

    av_register_all();
    avcodec_register_all();
    avformat_network_init();
    FFMpegVideo_v1::b_is_one_time_inited = true;
}

bool FFMpegVideo_v1::avtry( int result, const std::string& msg )
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

bool FFMpegVideo_v1::b_is_one_time_inited = false;


#endif // USE_FFMPEG


#include "Fast3DTexture.h"

#ifdef USE_FFMPEG

#include "../utility/FFMpegVideo.h"
#include "../utility/url_tools.h"

extern "C"
{
#include <libswscale/swscale.h>
}
#include <QWriteLocker>
#include <QReadLocker>
#include <QCoreApplication>
#include <QFuture>
#include <QtConcurrentRun>
#include <QDebug>
#include <string>
#include <stdexcept>

// Multithreaded loading of an MPEG4 video into a texture that
// could be loaded into an opengl 3D texture


/////////////////////
// class MpegLoader //
/////////////////////

// Populates internal YUV pixel format image in one thread
MpegLoader::MpegLoader(int pixelFormat)
    : QObject(NULL)
    , thread(new QThread(this))
    , pixelFormat(pixelFormat)
    , frame_data(NULL)
{
    // Slot calls will be executed in a special thread
    thread->start();
    this->moveToThread(thread);
    FFMpegVideo::maybeInitFFMpegLib();
}

/* virtual */
MpegLoader::~MpegLoader()
{
    QWriteLocker locker(&lock);
    thread->quit();
    thread->wait(500);
    deleteData();
}

void MpegLoader::deleteData()
{
    for (int f = 0; f < frames.size(); ++f)
    {
        av_free(frames[f]);
        frames[f] = NULL;
    }
    frames.clear();
    if (NULL != frame_data) {
        av_free(frame_data);
        frame_data = NULL;
    }
}

/* slot */
bool MpegLoader::loadMpegFile(QUrl fileUrl)
{
    // qDebug() << "MpegLoader::loadMpegFile()" << fileUrl << __FILE__ << __LINE__;
    if (! exists(fileUrl)) {
        qDebug() << "No such file" << fileUrl << __FILE__ << __LINE__;
        emit mpegFileLoadFinished(false);
        return false;
    }
    bool bSucceeded = true; // start optimistic
    try {
        FFMpegVideo video(fileUrl, (PixelFormat)pixelFormat);
        {
            QWriteLocker locker(&lock);
            deleteData();
            width = video.getWidth();
            height = video.getHeight();
            depth = video.getNumberOfFrames();
            emit headerLoaded(width, height, depth);
            // How much memory does each stored frame occupy?
            frameBytes = avpicture_get_size(PIX_FMT_YUV420P, width, height);
            // Allocated frame memory for all frames in (fast?) YUV format
            size_t size = (size_t)depth * (size_t)frameBytes;
            // qDebug() << "size =" << size << __FILE__ << __LINE__;
            frame_data = (uint8_t*)av_malloc(size);
            // check for allocation failure
            if (NULL == frame_data) {
                throw std::runtime_error("Failed to allocate frame memory");
            }
            frames.assign(depth, NULL);
            for (int z = 0; z < depth; ++z)
            {
                // Initialize persistent frame storage
                frames[z] = av_frame_alloc();
                avpicture_fill((AVPicture*) frames[z], frame_data + z * frameBytes,
                               PIX_FMT_YUV420P, width, height);
                // Load frame from disk
                AVPacket packet = {0};
                av_init_packet(&packet);
                if (!video.readNextFrameWithPacket(z, packet, video.pRaw))
                {
                    bSucceeded = false;
                    break;
                }
                // Copy "pFrameRGB" (actually YUV) to frame buffer
                memcpy(frame_data + z * frameBytes, video.pFrameRGB->data[0], frameBytes);
                av_free_packet(&packet);

                emit frameDecoded(z);
            }
        }

        emit mpegFileLoadFinished(bSucceeded);
        return bSucceeded;
    } catch(...) {}
    // If we get this far, exception was raised, so signal failure
    emit mpegFileLoadFinished(false);
    return false;
}


///////////////////////
// class BlockScaler //
///////////////////////

BlockScaler::BlockScaler(QObject * parent)
    : QObject(parent)
    , firstFrame(0)
    , finalFrame(0)
    , mpegLoader(NULL)
    , Sctx(NULL)
    , pFrameBgra(NULL)
    , data(NULL)
    , buffer(NULL)
    , channel(CHANNEL_RGB)
{}

void BlockScaler::setup(int firstFrameParam, int finalFrameParam,
                        MpegLoader& mpegLoaderParam, uint8_t * dataParam,
                        Channel channelParam)
{
    timer.start();
    firstFrame = firstFrameParam;
    finalFrame = finalFrameParam;
    mpegLoader = &mpegLoaderParam;
    data = dataParam;
    channel = channelParam;
    height = mpegLoader->height;
    width = mpegLoader->width;
    // qDebug() << "Channel =" << (int)channel << __FILE__ << __LINE__;

    pFrameBgra = av_frame_alloc();
    sliceBytesOut = height * width * 4;
    PixelFormat pixelFormat = PIX_FMT_BGRA;
    /*
    if (channel != CHANNEL_RGB) {
        pixelFormat = PIX_FMT_GRAY8;
    }
     */
    size_t size = avpicture_get_size(pixelFormat, width, height)
                  + FF_INPUT_BUFFER_PADDING_SIZE;
    // qDebug() << "size =" << size << __FILE__ << __LINE__;
    buffer = (uint8_t*)av_malloc(size);
    avpicture_fill( (AVPicture * ) pFrameBgra, buffer, pixelFormat,
                    width, height );
    // Create scaling context for converting from YUV to RGB
    if (NULL != Sctx)
        sws_freeContext(Sctx);
    Sctx = sws_getContext(
            width,
            height,
            PIX_FMT_YUV420P,
            width,
            height,
            pixelFormat,
            SWS_POINT, // fastest? We are not scaling, so...
            NULL,NULL,NULL);
}

/* virtual */
BlockScaler::~BlockScaler()
{
    if (NULL != buffer)
    {
        av_free(buffer);
        buffer = NULL;
    }
    if (NULL != pFrameBgra)
    {
        av_free(pFrameBgra);
        pFrameBgra = NULL;
    }
    if (NULL != Sctx)
    {
        sws_freeContext(Sctx);
        Sctx = NULL;
    }
}

void BlockScaler::load()
{
    // qDebug() << "Channel =" << (int)channel << __FILE__ << __LINE__;
    const size_t linesize_out = 4 * width;
    const size_t channel_offset = (size_t) channel;
    for (int z = firstFrame; z <= finalFrame; ++z) {
        AVFrame * frame = mpegLoader->frames[z];
        uint8_t* slice_out = data + z * sliceBytesOut;
        sws_scale(Sctx,              // sws context
                  frame->data,        // src slice
                  frame->linesize,    // src stride
                  0,                      // src slice origin y
                  height,      // src slice height
                  pFrameBgra->data,
                  pFrameBgra->linesize);
        // Import three channels, RGB
        if (channel == CHANNEL_RGB) {
            // Copy the entire frame (alpha will be overwritten)
            // memcpy(slice_out, pFrameBgra->data[0], sliceBytesOut);
            // PIX_FMT_BGRA0 fails to zero alpha channel, so I will
            for (int y = 0; y < height; ++y)
            {
                // convert pointers to do four bytes at a time
                uint32_t* sl_in = (uint32_t*)(pFrameBgra->data[0] + y * pFrameBgra->linesize[0]);
                uint32_t* sl_out = (uint32_t*)(slice_out + y * linesize_out);
                for (int x = 0; x < width; ++x) {
                    // Overwrite BGR, without overwriting A
                    // (little endian only?)
                    sl_out[x] &= 0xff000000; // clear RGB
                    sl_out[x] |= (sl_in[x] & 0x00ffffff); // RGB-in + A-out
                }
            }
        }
        // Copy just one channel
        else {
            for (int y = 0; y < height; ++y)
            {
                // input scan line
                uint8_t* sl_in = pFrameBgra->data[0] + y * pFrameBgra->linesize[0];
                // output scan line
                uint8_t* sl_out = slice_out + y * linesize_out + channel_offset;
                for (int x = 0; x < width; ++x) {
                    sl_out[4*x] = sl_in[4*x]; // always copy red channel
                }
            }
        }
    }
    // qDebug() << "Converting frames" << firstFrame << "to" << finalFrame
    //         << "took" << timer.elapsed()/1000.0 << "seconds";
}


///////////////////////
// class Fast3DTexture //
///////////////////////

Fast3DTexture::Fast3DTexture()
    : texture_data(NULL)
    , width(0)
    , height(0)
    , depth(0)
    , mpegLoader(PIX_FMT_YUV420P) // Use internal format for fast first pass of rescaling
{
    FFMpegVideo::maybeInitFFMpegLib();
    connect(this, SIGNAL(loadRequested(QUrl)),
            &mpegLoader, SLOT(loadMpegFile(QUrl)));
    connect(&mpegLoader, SIGNAL(headerLoaded(int, int, int)),
            this, SLOT(onHeaderLoaded(int, int, int)));
    // Repeat header loaded signal
    connect(&mpegLoader, SIGNAL(headerLoaded(int,int,int)),
            this, SIGNAL(headerLoaded(int,int,int)));
    connect(&mpegLoader, SIGNAL(frameDecoded(int)),
            this, SLOT(gotFrame(int)));
    connect(&mpegLoader, SIGNAL(mpegFileLoadFinished(bool)),
            this, SLOT(onMpegFileLoadFinished(bool)));
    // Immediately and automatically upload texture to video card
    // This requires that the "home" thread of the Fast3DTexture object
    // be the same as the OpenGL thread
}

/* virtual */
Fast3DTexture::~Fast3DTexture()
{
    Writer(*this);
    deleteData();
}

void Fast3DTexture::loadFile(QUrl fileUrl, BlockScaler::Channel channel)
{
    timer.start();

    // qDebug() << "Fast3DTexture::loadFile()" << fileUrl << __FILE__ << __LINE__;
    // Read metadata, if any, for this file
    QFileInfo fi(fileUrl.path());
    QUrl dir = ::parent(fileUrl);
    // e.g. Reference2_100.metadata
    QUrl metadataFileUrl = appendPath(dir, fi.completeBaseName() + ".metadata");
    // qDebug() << metadataFileUrl;
    if (exists(metadataFileUrl)) {
        // Channel zero of the volume file might not be channel zero of our rgba texture
        int channel_offset = 0;
        switch(channel) {
            case BlockScaler::CHANNEL_RGB:   channel_offset = 0; break;
            case BlockScaler::CHANNEL_RED:   channel_offset = 0; break;
            case BlockScaler::CHANNEL_GREEN: channel_offset = 1; break;
            case BlockScaler::CHANNEL_BLUE:  channel_offset = 2; break;
            case BlockScaler::CHANNEL_ALPHA: channel_offset = 3; break;
            default: channel_offset = 0;
        }
        if (sampledVolumeMetadata.loadFromUrl(metadataFileUrl, channel_offset)) {
            // qDebug() << 3 << sampledVolumeMetadata.channelGamma[3]
            //         << __FILE__ << __LINE__;
            emit metadataChanged();
        }
    }

    currentLoadChannel = channel;
    // emit benchmarkTimerPrintRequested("Started loading movie file");
    emit loadRequested(fileUrl);
}

/* slot */
void Fast3DTexture::onHeaderLoaded(int x, int y, int z)
{
    scalers.clear();
    // Kill any outstanding scaling jobs
    for (int i = 0; i < blockScaleWatchers.size(); ++i) {
        blockScaleWatchers[i]->disconnect();
        QFuture<void> future = blockScaleWatchers[i]->future();
        future.cancel();
    }
    for (int i = 0; i < blockScaleWatchers.size(); ++i) {
        QFuture<void> future = blockScaleWatchers[i]->future();
        future.waitForFinished();
    }
    blockScaleWatchers.clear();

    // qDebug() << "Fast3DTexture::onHeaderLoaded" << x << y << z;
    // Only allocate memory if we need to, especially for loading single channels
    if ( (width != x)
        || (height != y)
        || (depth != z) )
    {
        deleteData();
        width = x;
        height = y;
        depth = z;
        size_t size = 4 * (size_t)width * (size_t)height * (size_t)depth;
        // qDebug() << "size =" << size << __FILE__ << __LINE__;
        texture_data = (uint8_t*) malloc(size);
        if (texture_data == NULL) {
            qDebug() << "Failed to allocate memory";
        }
        memset(texture_data, 0, size);
    }

    // Ready a set of scalers for the new data
    for (int i = 0; i < Fast3DTexture::numScalingThreads; ++i) {
        scalers << new BlockScaler(this);
        blockScaleWatchers << new QFutureWatcher<void>(this);
    }
    completedBlocks = 0;
}

/* slot */
void Fast3DTexture::gotFrame(int f)
{
    // Chunk format conversion tasks into threadable blocks of consecutive slices
    // First we ask: Is this the final frame of a threadable block?
    // "numSections" is the number of threads we will use to unpack frames
    double numSections = Fast3DTexture::numScalingThreads; // divide stack scaling into this many threads
    double boundaryInterval = (double(depth) - 0.9) / numSections; // 0.9 so final frame will be in section 6, not 7
    // "section" is an index between 1.0 and numSections
    double section = 1.0 + int( f / boundaryInterval );
    // "firstInSection" is the index of the first frame in this section
    int firstInSection = int((section - 1.0) * boundaryInterval + 0.9);
    int lastInSection = int(section * boundaryInterval);
    // qDebug() << f << "section" << section << firstInSection << lastInSection << boundaryInterval << __FILE__ << __LINE__;
    if (f == 0) {
        // emit benchmarkTimerPrintRequested("Started converting movie file");
    }
    if (f == lastInSection) {
        // qDebug() << "scaling frames" << firstInSection << "to" << lastInSection;
        int ix = int(section) - 1;
        // Notice support for loading just one channel
        scalers[ix]->setup(firstInSection, lastInSection, mpegLoader,
                           texture_data, currentLoadChannel);
        QFuture<void> future = QtConcurrent::run(scalers[ix], &BlockScaler::load);
        // Create a way to signal when the scaling is done
        blockScaleWatchers[ix]->setFuture(future);
        connect(blockScaleWatchers[ix], SIGNAL(finished()),
                this, SLOT(blockScaleFinished()));
        if (section == numSections) {
            // emit benchmarkTimerPrintRequested("Finished decoding movie file");
        }
    }
    // qDebug() << "Decoded frame" << f;
}

/* slot */
void Fast3DTexture::blockScaleFinished()
{
    ++completedBlocks;
    // qDebug() << completedBlocks << "scaling blocks completed of" << Fast3DTexture::numScalingThreads;
    if (completedBlocks >= Fast3DTexture::numScalingThreads) {
        completedBlocks = 0;
        // emit benchmarkTimerPrintRequested("Finished scaling movie file");
        // qDebug() << "Decoding and scaling took " << timer.elapsed()/1000.0 << "seconds";
        // send intermediate result to graphics card
        emit volumeUploadRequested(width, height, depth, texture_data);
        // send final result to other viewers
        if (volumeQueue.size() < 1)
            emit volumeLoadSequenceCompleted();
    }
}

/* slot */
void Fast3DTexture::onMpegFileLoadFinished(bool succeeded)
{
    if (! succeeded)
    {
        // Normal completion signal won't come, so move to next volume
        std::cerr << "Volume load failed!" << std::endl;
        loadNextVolume();
    }
}

void Fast3DTexture::loadNextVolume()
{
    while(true) {
        if (volumeQueue.empty())
            return;
        QueuedVolume v = volumeQueue.front();
        volumeQueue.pop_front();
        if (exists(v.fileUrl)) {
            // qDebug() << "Loading volume" << v.fileUrl << __FILE__ << __LINE__;
            loadFile(v.fileUrl, v.channel);
            break;
        }
    }
}

void Fast3DTexture::deleteData()
{
    if (NULL != texture_data) {
        free(texture_data);
        texture_data = NULL;
    }
    width = height = depth = 0;
}

#endif // USE_FFMPEG


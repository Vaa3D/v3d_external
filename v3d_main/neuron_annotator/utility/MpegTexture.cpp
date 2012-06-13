#include "MpegTexture.h"

#ifdef USE_FFMPEG

#include "FFMpegVideo.h"
#include <QWriteLocker>
#include <QReadLocker>
#include <QCoreApplication>
#include <QFuture>
#include <QtConcurrentRun>
#include <QDebug>

// Multithreaded loading of an MPEG4 video into a texture that
// could be loaded into an opengl 3D texture


/////////////////////
// class MpegLoader //
/////////////////////

// Populates internal YUV pixel format image in one thread
MpegLoader::MpegLoader(PixelFormat pixelFormat)
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
bool MpegLoader::loadMpegFile(QString fileName)
{
    qDebug() << "MpegLoader::loadMpegFile()" << fileName << __FILE__ << __LINE__;
    QTime timer;
    timer.start();
    bool bSucceeded = true; // start optimistic
    try {
        FFMpegVideo video(fileName.toStdString().c_str(), pixelFormat);
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
            qDebug() << "size =" << size << __FILE__ << __LINE__;
            frame_data = (uint8_t*)av_malloc(size);
            frames.assign(depth, NULL);
            for (int z = 0; z < depth; ++z)
            {
                // Initialize persistent frame storage
                frames[z] = avcodec_alloc_frame();
                avpicture_fill((AVPicture*) frames[z], frame_data + z * frameBytes,
                               PIX_FMT_YUV420P, width, height);
                // Load frame from disk
                AVPacket packet = {0};
                av_init_packet(&packet);
                video.readNextFrameWithPacket(z, packet, video.pRaw);

                // Copy "pFrameRGB" (actually YUV) to frame buffer
                memcpy(frame_data + z * frameBytes, video.pFrameRGB->data[0], frameBytes);
                av_free_packet(&packet);

                emit frameDecoded(z);
            }
        }
        if (bSucceeded)
            qDebug() << "Number of frames found =" << depth;
        qDebug() << "File decode took" << timer.elapsed()/1000.0 << "seconds";
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
{}

void BlockScaler::setup(int firstFrameParam, int finalFrameParam, MpegLoader& mpegLoaderParam, uint8_t * dataParam)
{
    timer.start();
    firstFrame = firstFrameParam;
    finalFrame = finalFrameParam;
    mpegLoader = &mpegLoaderParam;
    data = dataParam;
    height = mpegLoader->height;

    int width = mpegLoader->width;
    pFrameBgra = avcodec_alloc_frame();
    size_t size = avpicture_get_size(PIX_FMT_BGRA, width, height)
                  + FF_INPUT_BUFFER_PADDING_SIZE;
    qDebug() << "size =" << size << __FILE__ << __LINE__;
    buffer = (uint8_t*)av_malloc(size);
    avpicture_fill( (AVPicture * ) pFrameBgra, buffer, PIX_FMT_BGRA,
                    width, height );
    sliceBytesOut = height * width * 4;
    if (NULL != Sctx)
        sws_freeContext(Sctx);
    Sctx = sws_getContext(
            width,
            height,
            PIX_FMT_YUV420P,
            width,
            height,
            PIX_FMT_BGRA,
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
    for (int z = firstFrame; z <= finalFrame; ++z) {
        AVFrame * frame = mpegLoader->frames[z];
        sws_scale(Sctx,              // sws context
                  frame->data,        // src slice
                  frame->linesize,    // src stride
                  0,                      // src slice origin y
                  height,      // src slice height
                  pFrameBgra->data,
                  pFrameBgra->linesize);
        memcpy(data + z * sliceBytesOut, pFrameBgra->data[0], sliceBytesOut);
    }
    qDebug() << "Converting frames" << firstFrame << "to" << finalFrame
            << "took" << timer.elapsed()/1000.0 << "seconds";
}


///////////////////////
// class MpegTexture //
///////////////////////

MpegTexture::MpegTexture(GLenum textureUnit, QObject * parent)
    : QObject(parent)
    , texture_data(NULL)
    , width(0)
    , height(0)
    , depth(0)
    , textureUnit(textureUnit)
    , textureId(0)
    , mpegLoader(PIX_FMT_YUV420P) // TODO internal format for speed testing
{
    FFMpegVideo::maybeInitFFMpegLib();
    connect(this, SIGNAL(loadRequested(QString)),
            &mpegLoader, SLOT(loadMpegFile(QString)));
    connect(&mpegLoader, SIGNAL(headerLoaded(int, int, int)),
            this, SLOT(onHeaderLoaded(int, int, int)));
    connect(&mpegLoader, SIGNAL(frameDecoded(int)),
            this, SLOT(gotFrame(int)));
    // Immediately and automatically upload texture to video card
    // This requires that the "home" thread of the MpegTexture object
    // be the same as the OpenGL thread
    connect(this, SIGNAL(loadFinished(bool)),
            this, SLOT(uploadToVideoCard()));
}

/* virtual */
MpegTexture::~MpegTexture()
{
    deleteData();
    if (0 != textureId)
        glDeleteTextures(1, &textureId);
}

void MpegTexture::loadFile(QString fileName)
{
    qDebug() << "MpegTexture::loadFile()" << fileName << __FILE__ << __LINE__;
    timer.start();
    emit loadRequested(fileName);
}

/* slot */
void MpegTexture::onHeaderLoaded(int x, int y, int z)
{
    // qDebug() << "MpegTexture::onHeaderLoaded" << x << y << z;
    width = x;
    height = y;
    depth = z;
    deleteData();
    size_t size = 4 * (size_t)width * (size_t)height * (size_t)depth;
    qDebug() << "size =" << size << __FILE__ << __LINE__;
    texture_data = (uint8_t*) malloc(size);
    if (texture_data == NULL) {
        qDebug() << "Failed to allocate memory";
    }
    scalers.clear();
    blockScaleWatchers.clear();
    for (int i = 0; i < MpegTexture::numScalingThreads; ++i) {
        scalers << new BlockScaler(this);
        blockScaleWatchers << new QFutureWatcher<void>(this);
    }
    completedBlocks = 0;
}

/* slot */
void MpegTexture::gotFrame(int f)
{
    // Chunk format conversion tasks into threadable blocks of consecutive slices
    // First we ask: Is this the final frame of a threadable block?
    // "numSections" is the number of threads we will use to unpack frames
    double numSections = MpegTexture::numScalingThreads; // divide stack scaling into this many threads
    // "section" is an index between 1.0 and numSections
    double section = 1.0 + int( f * numSections / depth );
    // "firstInSection" is the index of the first frame in this section
    int firstInSection = int((section - 1.0) * depth / numSections);
    int lastInSection = int(section * depth / numSections - 1.0);
    if (f == lastInSection) {
        // qDebug() << "scaling frames" << firstInSection << "to" << lastInSection;
        int ix = int(section) - 1;
        scalers[ix]->setup(firstInSection, lastInSection, mpegLoader, texture_data);
        QFuture<void> future = QtConcurrent::run(scalers[ix], &BlockScaler::load);
        // Create a way to signal when the scaling is done
        blockScaleWatchers[ix]->setFuture(future);
        connect(blockScaleWatchers[ix], SIGNAL(finished()),
                this, SLOT(blockScaleFinished()));
    }
    // qDebug() << "Decoded frame" << f;
}

/* slot */
void MpegTexture::blockScaleFinished()
{
    ++completedBlocks;
    qDebug() << completedBlocks << "scaling blocks completed";
    if (completedBlocks >= MpegTexture::numScalingThreads) {
        completedBlocks = 0;
        qDebug() << "Total load time =" << timer.elapsed()/1000.0 << "seconds";
        emit loadFinished(true);
    }
}

/* slot */
bool MpegTexture::uploadToVideoCard()
{
    QTime stopwatch;
    stopwatch.start();
    qDebug() << "MpegTexture::uploadToVideoCard()";
    {
        // check for previous errors
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            qDebug() << "OpenGL error" << err << __FILE__ << __LINE__;
            return false;
        }
    }
    if (NULL == texture_data)
        return false;
    // Upload volume image as an OpenGL 3D texture
    glPushAttrib(GL_TEXTURE_BIT); // remember previous OpenGL state
    if (0 == textureId)
        glGenTextures(1, &textureId); // allocate a handle for this texture
    glEnable(GL_TEXTURE_3D); // we are using a 3D texture
    glActiveTextureARB(textureUnit); // multitexturing index, because there are other textures
    glBindTexture(GL_TEXTURE_3D, textureId); // make this the current texture
    // Black/zero beyond edge of texture
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    // Interpolate between texture values
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    {
        // check for new errors
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            qDebug() << "OpenGL error" << err << __FILE__ << __LINE__;
            return false;
        }
    }
    qDebug() << width << height << depth << (long)texture_data;
    // Load the data onto video card
    glTexImage3D(GL_TEXTURE_3D,
        0, ///< mipmap level; zero means base level
        GL_RGBA8, ///< texture format, in bytes per pixel
        width,
        height,
        depth,
        0, // border
        GL_BGRA, // image format
        GL_UNSIGNED_INT_8_8_8_8_REV, // image type
        (GLvoid*)texture_data);
    glPopAttrib(); // restore OpenGL state
    {
        // check for new errors
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            qDebug() << "OpenGL error" << err << __FILE__ << __LINE__;
            return false;
        }
    }
    qDebug() << "Uploading 3D volume texture took"
             << stopwatch.elapsed()
             << "milliseconds";
    emit textureUploaded(textureId);
    return true;
}

void MpegTexture::deleteData()
{
    if (NULL != texture_data) {
        free(texture_data);
        texture_data = NULL;
    }
}

#endif // USE_FFMPEG


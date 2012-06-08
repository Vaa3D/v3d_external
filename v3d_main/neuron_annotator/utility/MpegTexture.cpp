#include "MpegTexture.h"
#include "FFMpegVideo.h"
#include <QTime>
#include <QWriteLocker>
#include <QReadLocker>
#include <QCoreApplication>

// Multithreaded loading of an MPEG4 video into a texture that
// could be loaded into an opengl 3D texture


/////////////////////
// class YuvLoader //
/////////////////////

// Populates internal YUV pixel format image in one thread
YuvLoader::YuvLoader()
    : QObject(NULL)
    , thread(new QThread(this))
    , frames(NULL)
{
    // Slot calls will be executed in a special thread
    thread->start();
    this->moveToThread(thread);
}

/* virtual */
YuvLoader::~YuvLoader()
{
    QWriteLocker locker(&lock);
    thread->quit();
    thread->wait(500);
    deleteFrames();
}

/* slot */
bool YuvLoader::loadMpegFile(QString fileName)
{
    QTime timer;
    timer.start();
    bool bSucceeded = true; // start optimistic
    try {
        FFMpegVideo video(fileName.toStdString().c_str());
        {
            QWriteLocker locker(&lock);
            sx = video.getWidth();
            sy = video.getHeight();
            sz = video.getNumberOfFrames();
            frameBytes = avpicture_get_size(
                    PIX_FMT_YUV420P, sx, sy);
            deleteFrames();
            frames = new uint8_t[sz * frameBytes];
            for (int z = 0; z < sz; ++z)
            {
                // TODO - video.fetchFrame(z) sometimes returns false, and I'm ignoring that.
                video.fetchFrame(z);
                if (bSucceeded)
                    memcpy(frames + z * frameBytes, video.pRaw->data, frameBytes);
                emit frameDecoded(z);
            }
        }
        if (bSucceeded)
            qDebug() << "Number of frames found =" << sz;
        qDebug() << "File decode took" << timer.elapsed()/1000.0 << "seconds";
        emit mpegFileLoadFinished(bSucceeded);
        return bSucceeded;
    } catch(...) {}
    // If we get this far, exception was raised, so signal failure
    emit mpegFileLoadFinished(false);
    return false;
}

void YuvLoader::deleteFrames()
{
    if (NULL != frames) {
        delete [] frames;
        frames = NULL;
    }
}


/////////////////////////
// class BgraConverter //
/////////////////////////

BgraConverter::BgraConverter()
    : QObject(NULL)
    , thread(new QThread(this))
    , frames(NULL)
    , sx(0)
    , sy(0)
    , sz(0)
{
    // Slot calls will be executed in a special thread
    thread->start();
    this->moveToThread(thread);
    connect(this, SIGNAL(loadRequested(QString)),
            &yuvLoader, SLOT(loadMpegFile(QString)));
    connect(&yuvLoader, SIGNAL(mpegFileLoadFinished(bool)),
            this, SLOT(onLoadFinished(bool)));
}

/* virtual */
BgraConverter::~BgraConverter()
{
    QWriteLocker locker(&lock);
    thread->quit();
    thread->wait(500);
    deleteFrames();
}

void BgraConverter::deleteFrames()
{
    if (NULL != frames) {
        delete [] frames;
        frames = NULL;
    }
}

/* slot */
bool BgraConverter::convertFrame(int z)
{
    {
        QReadLocker yuvLocker(&yuvLoader.lock);
        QWriteLocker locker(&lock);
        // If needed, allocate frame buffer
        if ( (NULL == frames)
            || (sx != yuvLoader.sx)
            || (sy != yuvLoader.sy)
            || (sz != yuvLoader.sz) )
            {
            deleteFrames();
            sx = yuvLoader.sx;
            sy = yuvLoader.sy;
            sz = yuvLoader.sz;
            frameBytes = avpicture_get_size(
                    PIX_FMT_BGRA, sx, sy);
            frames = new uint8_t[sz * frameBytes];
        }
        // TODO sws scale
    }
    emit frameConverted(z);
}

/* slot */
void BgraConverter::onLoadFinished(bool bSucceeded)
{
    // TODO
    if (bSucceeded)
        // Convert pending frames
        QCoreApplication::processEvents();
    emit convertFinished(bSucceeded);
}


///////////////////////
// class MpegTexture //
///////////////////////

MpegTexture::MpegTexture(GLenum textureUnit)
    : QObject(NULL)
    , texture_data(NULL)
    , width(0)
    , height(0)
    , depth(0)
    , textureUnit(textureUnit)
    , textureId(0)
{
    connect(this, SIGNAL(loadRequested(QString)),
            &bgraConverter, SIGNAL(loadRequested(QString)));
    connect(&bgraConverter, SIGNAL(frameConverted(int)),
            this, SLOT(gotFrame(int)));
    connect(&bgraConverter, SIGNAL(convertFinished(bool)),
            this, SIGNAL(loadFinished(bool)));
}

/* virtual */
MpegTexture::~MpegTexture()
{
    deleteData();
    if (0 != textureId)
        glDeleteTextures(1, &textureId);
}

/* slot */
void MpegTexture::gotFrame(int f)
{
    // qDebug() << "Decoded frame" << f;
}

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
        (GLvoid*)texture_data); ///< NULL means initialize but don't populate
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
    return true;
}

void MpegTexture::deleteData()
{
    if (NULL != texture_data) {
        delete [] texture_data;
        texture_data = NULL;
    }
}


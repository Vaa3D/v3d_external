#ifndef MPEGTEXTURE_H
#define MPEGTEXTURE_H

// MpegTexture class is designed to help load an MPEG4 movie file
// directly into a 3D OpenGL texture.

#ifdef USE_FFMPEG

#include "FFMpegVideo.h"
extern "C"
{
#include <libswscale/swscale.h>
}

#include <QObject>
#include <QThread>
#include <QString>
#include <QTime>
#include <QReadWriteLock>
#include <QFutureWatcher>
#include <QList>

#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <vector>

// Read an mpeg4 file and populates a YUV texture volume, in its own thread.
class MpegLoader : public QObject
{
    Q_OBJECT

public:
    MpegLoader(PixelFormat pixelFormat);
    virtual ~MpegLoader();
    void deleteData();

signals:
    void headerLoaded(int, int, int); // fires as soon as the movie dimensions are known
    void frameDecoded(int); // fires once per frame decoded
    void mpegFileLoadFinished(bool bSucceeded);

public slots:
    bool loadMpegFile(QString fileName);

public:
    QReadWriteLock lock;
    size_t width, height, depth;
    QThread * thread;
    PixelFormat pixelFormat;
    size_t frameBytes;
    uint8_t * frame_data;
    std::vector<AVFrame*> frames;
};


// BlockScaler converts one consecutive substack from YUV format to BGRA
class BlockScaler : public QObject
{
    Q_OBJECT

public:
    BlockScaler(QObject * parent = NULL);
    virtual ~BlockScaler();
    void setup(int firstFrameParam, int finalFrameParam,
               MpegLoader& mpegLoaderParam, uint8_t * dataParam);
    void load();

    QTime timer;
    int firstFrame, finalFrame;
    size_t height, sliceBytesOut;
    MpegLoader* mpegLoader;
    SwsContext* Sctx;
    AVFrame* pFrameBgra;
    uint8_t * data;
    uint8_t * buffer;
};

class MpegTexture : public QObject
{
    Q_OBJECT

public:
    static const int numScalingThreads = 6;

    MpegTexture(GLenum textureUnit = GL_TEXTURE0_ARB, QObject * parent = NULL);
    virtual ~MpegTexture();
    void loadFile(QString fileName);

signals:
    void loadRequested(QString fileName);
    void loadFinished(bool bSucceeded);
    void textureUploaded(int textureId);

public slots:
    void onHeaderLoaded(int, int, int);
    void gotFrame(int);
    // uploadToVideoCard() MUST be called from OpenGL thread!
    bool uploadToVideoCard();

protected slots:
    void blockScaleFinished();

protected:
    void deleteData();

    QTime timer; // for performance testing
    QList<QFutureWatcher<void>* > blockScaleWatchers;
    int completedBlocks;
    GLsizei width, height, depth;
    GLuint textureId;
    GLenum textureUnit;
    uint8_t * texture_data;
    MpegLoader mpegLoader;
    QList<BlockScaler*> scalers;
};

#endif // USE_FFMPEG

#endif // MPEGTEXTURE_H

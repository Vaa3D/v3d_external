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
#include <QGLWidget>
#include <QDir>

#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <vector>
#include <deque>

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
    enum Channel {
        CHANNEL_RED = 2, // 2 not 0 because pixels are in bgra order?
        CHANNEL_GREEN = 1,
        CHANNEL_BLUE = 0,
        CHANNEL_ALPHA = 3,
        CHANNEL_RGB = 37
    };

    BlockScaler(QObject * parent = NULL);
    virtual ~BlockScaler();
    void setup(int firstFrameParam, int finalFrameParam,
               MpegLoader& mpegLoaderParam, uint8_t * dataParam,
               Channel channelParam);
    void load();

    QTime timer;
    int firstFrame, finalFrame;
    size_t width, height, sliceBytesOut;
    MpegLoader* mpegLoader;
    SwsContext* Sctx;
    AVFrame* pFrameBgra;
    uint8_t * data;
    uint8_t * buffer;
    Channel channel;
};

struct QueuedVolume
{
    QueuedVolume(QString fn, BlockScaler::Channel c)
        : fileName(fn), channel(c) {}
    QString fileName;
    BlockScaler::Channel channel;
};

class MpegTexture : public QObject
{
    Q_OBJECT

public:
    static const int numScalingThreads = 6;

    MpegTexture(GLenum textureUnit = GL_TEXTURE0_ARB,
                QGLWidget * glWidget = NULL,
                QObject * parent = NULL);
    virtual ~MpegTexture();
    void loadFile(QString fileName, BlockScaler::Channel channel=BlockScaler::CHANNEL_RGB);

    void queueVolume(QString fileName, BlockScaler::Channel channel) {
        volumeQueue.push_back(QueuedVolume(fileName, channel));
    }

signals:
    void loadRequested(QString fileName);
    void loadFinished(bool bSucceeded);
    void textureUploaded(int textureId);

public slots:
    void onHeaderLoaded(int, int, int);
    void gotFrame(int);
    // uploadToVideoCard() MUST be called from OpenGL thread!
    bool uploadToVideoCard();
    void loadNextVolume() {
        if (volumeQueue.empty()) return;
        QueuedVolume v = volumeQueue.front();
        volumeQueue.pop_front();
        loadFile(v.fileName, v.channel);
    }

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
    uint8_t * texture_data; // z*y*x*BGRA, ready for glTexImage3D()
    MpegLoader mpegLoader;
    QList<BlockScaler*> scalers;
    BlockScaler::Channel currentLoadChannel;
    QGLWidget* glWidget;
    std::deque<QueuedVolume> volumeQueue;
};

#endif // USE_FFMPEG

#endif // MPEGTEXTURE_H

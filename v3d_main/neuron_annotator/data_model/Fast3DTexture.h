#ifndef MPEGTEXTURE_H
#define MPEGTEXTURE_H

// Fast3DTexture class is designed to help load an MPEG4 movie file
// directly into a 3D OpenGL texture.

#ifdef USE_FFMPEG

#include "NaLockableData.h"
#include "Dimension.h"
#include "SampledVolumeMetadata.h"

class AVFrame;
class SwsContext;

#include <QObject>
#include <QThread>
#include <QString>
#include <QTime>
#include <QElapsedTimer>
#include <QReadWriteLock>
#include <QFutureWatcher>
#include <QList>
#include <QDir>

#include <vector>
#include <deque>
#include <string>
#include <stdint.h>

// Read an mpeg4 file and populates a YUV texture volume, in its own thread.
class MpegLoader : public QObject
{
    Q_OBJECT

public:
    MpegLoader(int pixelFormat);
    virtual ~MpegLoader();
    void deleteData();

signals:
    void headerLoaded(int, int, int); // fires as soon as the movie dimensions are known
    void frameDecoded(int); // fires once per frame decoded
    void mpegFileLoadFinished(bool bSucceeded);

public slots:
    bool loadMpegFile(QUrl url);

public:
    QReadWriteLock lock;
    size_t width, height, depth;
    QThread * thread;
    int pixelFormat;
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

    QElapsedTimer timer;
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
    QueuedVolume(QUrl url, BlockScaler::Channel c)
        : fileUrl(url), channel(c) {}
    QUrl fileUrl;
    BlockScaler::Channel channel;
};


class Fast3DTexture : public NaLockableData
{
    Q_OBJECT

public:
    static const int numScalingThreads = 6;

    Fast3DTexture();
    virtual ~Fast3DTexture();
    void loadFile(QUrl url, BlockScaler::Channel channel=BlockScaler::CHANNEL_RGB);


signals:
    void loadRequested(QUrl url);
    void volumeUploadRequested(int w, int h, int d, void* texture_data);
    void headerLoaded(int, int, int);
    void volumeLoadSequenceCompleted();
    void benchmarkTimerResetRequested();
    void benchmarkTimerPrintRequested(QString);
    void metadataChanged();

public slots:
    void onHeaderLoaded(int, int, int);
    void gotFrame(int);
    void loadNextVolume();
    void queueVolume(QUrl url, int channel) {
        volumeQueue.push_back(QueuedVolume(url, (BlockScaler::Channel)channel));
    }
    void onMpegFileLoadFinished(bool succeeded);

protected slots:
    void blockScaleFinished();

protected:
    void deleteData();

public:
    size_t width, height, depth;
    uint8_t * texture_data; // z*y*x*BGRA, ready for glTexImage3D()

protected:
    QElapsedTimer timer; // for performance testing
    QList<QFutureWatcher<void>* > blockScaleWatchers;
    int completedBlocks;
    MpegLoader mpegLoader;
    QList<BlockScaler*> scalers;
    BlockScaler::Channel currentLoadChannel;
    std::deque<QueuedVolume> volumeQueue;
    SampledVolumeMetadata sampledVolumeMetadata;

public:

    class Reader; friend class Reader;
    class Reader : public BaseReadLocker
    {
    public:
        Reader(const Fast3DTexture& texture)
            : BaseReadLocker(texture)
            , m_data(&texture)
        {}
        size_t width() const {return m_data->width;}
        size_t height() const {return m_data->height;}
        size_t depth() const {return m_data->depth;}
        const uint8_t* data() const {return m_data->texture_data;}
        const SampledVolumeMetadata& metadata() const {return m_data->sampledVolumeMetadata;}

    private:
        const Fast3DTexture * m_data;
    };


    class Writer; friend class Writer;
    class Writer : public BaseWriteLocker
    {
    public:
        Writer(Fast3DTexture& texture)
            : BaseWriteLocker(texture)
            , m_data(&texture)
        {}

    private:
        Fast3DTexture * m_data;
    };
};

#endif // USE_FFMPEG

#endif // MPEGTEXTURE_H

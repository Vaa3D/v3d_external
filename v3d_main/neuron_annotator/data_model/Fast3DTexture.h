#ifndef MPEGTEXTURE_H
#define MPEGTEXTURE_H

// Fast3DTexture class is designed to help load an MPEG4 movie file
// directly into a 3D OpenGL texture.

#ifdef USE_FFMPEG

#include "NaLockableData.h"
#include "Dimension.h"

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
#include <sstream>
#include <fstream>
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
    bool loadMpegFile(QString fileName);

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
    QueuedVolume(QString fn, BlockScaler::Channel c)
        : fileName(fn), channel(c) {}
    QString fileName;
    BlockScaler::Channel channel;
};


// Data structure to store relationship between sampled volume and original image
class SampledVolumeMetaData
{
public:
    SampledVolumeMetaData()
        : channelGamma((size_t)4, 1.0)
        , channelHdrMinima((size_t)4, (uint32_t)0)
        , channelHdrMaxima((size_t)4, (uint32_t)4095)
    {}

    bool loadFromFile(QString fileName)
    {
        std::ifstream in(fileName.toStdString().c_str());
        if (! in.good())
            return false;
        char lineBuffer[1024];
        bool bChanged = false;
        while (in.getline(lineBuffer, 1024))
        {
            std::string line(lineBuffer);
            std::istringstream lineStream(line);
            std::string recordType;
            lineStream >> recordType;
            if (recordType.length() < 1)
                continue;
            if (recordType == "CHANNEL_RESCALE") {
                // CHANNEL_RESCALE         0       9       4276
                int channel;
                uint32_t min, max;
                lineStream >> channel;
                lineStream >> min;
                lineStream >> max;
                if (min != channelHdrMinima[channel]) {
                    channelHdrMinima[channel] = min;
                    bChanged = true;
                }
                if (max != channelHdrMaxima[channel]) {
                    channelHdrMaxima[channel] = max;
                    bChanged = true;
                }
            }
            else if (recordType == "CHANNEL_GAMMA") {
                // CHANNEL_GAMMA   0       0.46
                int channel;
                float gamma;
                lineStream >> channel;
                lineStream >> gamma;
                if (gamma != channelGamma[channel]) {
                    channelGamma[channel] = gamma;
                    bChanged = true;
                }
            }
            else if (recordType == "PARENT_DIMS") {
                // PARENT_DIMS     512     512     445
                size_t x, y, z;
                lineStream >> x >> y >> z;
                if (x*y*z < 1)
                    continue;
                jfrc::Dimension d(x, y, z);
                if (d != originalImageSize) {
                    originalImageSize = d;
                    bChanged = true;
                }
            }
            else if (recordType == "RESAMPLED_DIMS") {
                // RESAMPLED_DIMS  392     392     336
                size_t x, y, z;
                lineStream >> x >> y >> z;
                if (x*y*z < 1)
                    continue;
                jfrc::Dimension d(x, y, z);
                if (d != paddedImageSize) {
                    paddedImageSize = d;
                    bChanged = true;
                }
            }
            else if (recordType == "USED_DIMS") {
                // USED_DIMS  386     386     336
                size_t x, y, z;
                lineStream >> x >> y >> z;
                if (x*y*z < 1)
                    continue;
                jfrc::Dimension d(x, y, z);
                if (d != usedImageSize) {
                    usedImageSize = d;
                    bChanged = true;
                }
            }
        }
        return bChanged;
    }

    // Image metadata from .sizes/.colors file
    jfrc::Dimension originalImageSize;
    jfrc::Dimension paddedImageSize;
    jfrc::Dimension usedImageSize;
    std::vector<float> channelGamma;
    std::vector<uint32_t> channelHdrMinima;
    std::vector<uint32_t> channelHdrMaxima;

};


class Fast3DTexture : public NaLockableData
{
    Q_OBJECT

public:
    static const int numScalingThreads = 6;

    Fast3DTexture();
    virtual ~Fast3DTexture();
    void loadFile(QString fileName, BlockScaler::Channel channel=BlockScaler::CHANNEL_RGB);

    void queueVolume(QString fileName, BlockScaler::Channel channel) {
        volumeQueue.push_back(QueuedVolume(fileName, channel));
    }

signals:
    void loadRequested(QString fileName);
    void volumeUploadRequested(int w, int h, int d, void* texture_data);
    void headerLoaded(int, int, int);
    void volumeLoadSequenceCompleted();
    void benchmarkTimerResetRequested();
    void benchmarkTimerPrintRequested(QString);
    void metaDataChanged();

public slots:
    void onHeaderLoaded(int, int, int);
    void gotFrame(int);
    void loadNextVolume();

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
    SampledVolumeMetaData sampledVolumeMetaData;

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
        const SampledVolumeMetaData& metadata() const {return m_data->sampledVolumeMetaData;}

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

#ifndef MPEGTEXTURE_H
#define MPEGTEXTURE_H

#include "FFMpegVideo.h"
#include <QObject>
#include <QThread>
#include <QString>
#include <QTime>
#include <QDebug>
#include <QReadWriteLock>

#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif


// Read an mpeg4 file and populates a YUV texture volume, in its own thread.
class YuvLoader : public QObject
{
    Q_OBJECT

public:
    YuvLoader();
    virtual ~YuvLoader();

signals:
    void mpegFileLoadFinished(bool bSucceeded);
    void frameDecoded(int);

public slots:
    bool loadMpegFile(QString fileName);

public:
    QReadWriteLock lock;
    size_t sx, sy, sz;
    size_t frameBytes;

protected:
    void deleteFrames();

    QThread * thread;
    uint8_t * frames;
};


// Reads a YUV volume and writes a BGRA volume
class BgraConverter : public QObject
{
    Q_OBJECT

public:
    BgraConverter();
    virtual ~BgraConverter();

signals:
    void loadRequested(QString fileName);
    void convertFinished(bool);
    void frameConverted(int);

public slots:
    bool convertFrame(int);
    void onLoadFinished(bool);

public:
    QReadWriteLock lock;

protected:
    void deleteFrames();

    size_t sx, sy, sz;
    size_t frameBytes;
    YuvLoader yuvLoader;
    QThread * thread;
    uint8_t * frames;
};


class MpegTexture : public QObject
{
    Q_OBJECT

public:
    MpegTexture(GLenum textureUnit = GL_TEXTURE0_ARB);
    virtual ~MpegTexture();
    void loadFile(QString fileName) {
        emit loadRequested(fileName);
    }

    // uploadToVideoCard() MUST be called from OpenGL thread!
    bool uploadToVideoCard();

signals:
    void loadRequested(QString fileName);
    void loadFinished(bool bSucceeded);

public slots:
    void gotFrame(int);

protected:
    void deleteData();

    GLsizei width, height, depth;
    GLuint textureId;
    GLenum textureUnit;
    char * texture_data;
    BgraConverter bgraConverter;
};

#endif // MPEGTEXTURE_H

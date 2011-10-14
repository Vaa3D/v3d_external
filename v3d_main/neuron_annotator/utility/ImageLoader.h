#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include <QString>
#include <QtCore>
#include <QDir>
#include "../../v3d/v3d_core.h"

using namespace std;


class ImageLoader : public QObject
{
    Q_OBJECT

public:
    ImageLoader();
    ~ImageLoader();

    static string getCommandLineDescription() {
        return "image-loader";
    }

    static string getUsage() {
        string usage;
        usage.append("  Image Loader Utility                                                    \n");
        usage.append("                                                                          \n");
        usage.append("   -load <filepath>                                                       \n");
        return usage;
    }

    bool execute();
    bool validateFile();
    My4DImage* loadImage(QString filepath);

    int saveStack2RawPBD(const char * filename, unsigned char* data, const V3DLONG * sz, int datatype);
    int loadRaw2StackPBD(char * filename, My4DImage * & image);

    int processArgs(vector<char*> *argList);
    QString getFilePrefix(QString filepath);

    V3DLONG decompressPBD(unsigned char * sourceData, unsigned char * targetData, V3DLONG sourceLength);

signals:
    void updateCompressionBuffer(unsigned char * updatedCompressionBuffer);


private:
    QString inputFilepath;
    My4DImage * image;
    unsigned char * compressedData;
    FILE * fid;
    char * keyread;

    V3DLONG compressPBD(unsigned char * imgRe, unsigned char * preBuffer, V3DLONG bufferLength, V3DLONG spaceLeft);
    int exitWithError(QString errorMessage);

};

class ImageLoaderDecompressor : public QThread
{
    Q_OBJECT

public:
    explicit ImageLoaderDecompressor(
            ImageLoader * imageLoader,
            unsigned char * compressionBuffer,
            unsigned char * decompressionBuffer,
            V3DLONG maxDecompressionSize);

    bool hasError();
    V3DLONG getDecompressionSize();
    bool isProcessing();

public slots:
    void updateCompressionBuffer(unsigned char * updatedCompressionBuffer);

private:
    ImageLoader * imageLoader;
    unsigned char * compressionBuffer;
    unsigned char * decompressionBuffer;
    V3DLONG maxDecompressionSize;
    bool decompressionError;
    bool processing;

    unsigned char * compressionPosition;
    unsigned char * decompressionPosition;

};

#endif // IMAGELOADER_H

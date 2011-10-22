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

    static const int MODE_UNDEFINED;
    static const int MODE_LOAD_TEST;
    static const int MODE_CONVERT;

    static string getCommandLineDescription() {
        return "image-loader";
    }

    static string getUsage() {
        string usage;
        usage.append("  Image Loader Utility                                                    \n");
        usage.append("                                                                          \n");
        usage.append("   -loadtest <filepath>                                                   \n");
        usage.append("   -convert  <source file>    <target file>                               \n");
        return usage;
    }

    bool execute();
    bool validateFile();
    My4DImage* loadImage(QString filepath);
    void loadImage(My4DImage * stackp, QString filepath);

    int saveStack2RawPBD(const char * filename, unsigned char* data, const V3DLONG * sz);
    int loadRaw2StackPBD(char * filename, My4DImage * & image, bool useThreading);

    int processArgs(vector<char*> *argList);
    QString getFilePrefix(QString filepath);

    V3DLONG decompressPBD(unsigned char * sourceData, unsigned char * targetData, V3DLONG sourceLength);

//signals:
//    void updateCompressionBuffer(unsigned char * updatedCompressionBuffer);


private:
    int mode;
    QString inputFilepath;
    QString targetFilepath;
    My4DImage * image;
    FILE * fid;
    char * keyread;

    V3DLONG compressPBD(unsigned char * compressionBuffer, unsigned char * sourceBuffer, V3DLONG sourceBufferLength, V3DLONG spaceLeft);
    int exitWithError(QString errorMessage);
    void updateCompressionBuffer(unsigned char * updatedCompressionBuffer);
    unsigned char * convertType2Type1(const V3DLONG * sz, My4DImage *image);

    unsigned char * compressionBuffer;
    unsigned char * decompressionBuffer;
    V3DLONG maxDecompressionSize;
    unsigned char * compressionPosition;
    unsigned char * decompressionPosition;
    unsigned char decompressionPrior;

    QFuture<void> * decompressionThread;

};

#endif // IMAGELOADER_H

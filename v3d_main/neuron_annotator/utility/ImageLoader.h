#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include <QString>
#include <QtCore>
#include <QDir>
#include "../../v3d/v3d_core.h"

using namespace std;


class ImageLoader : public QObject, public QRunnable
{

Q_OBJECT

public:
    ImageLoader();
    ~ImageLoader();

    static const int MODE_UNDEFINED;
    static const int MODE_LOAD_TEST;
    static const int MODE_CONVERT;
    static const int MODE_CONVERT8;
    static const int MODE_MIP;
    static const int MODE_MAP_CHANNELS;

    static const unsigned char oooooool;
    static const unsigned char oooooolo;
    static const unsigned char ooooooll;
    static const unsigned char oooooloo;
    static const unsigned char ooooolol;
    static const unsigned char ooooollo;
    static const unsigned char ooooolll;

    static string getCommandLineDescription() {
        return "image-loader";
    }

    static bool hasPbdExtension(QString filename) {
        if (filename.endsWith(".pbd") || filename.endsWith(".v3dpbd") || filename.endsWith(".vaa3dpbd")) {
            return true;
        }
        return false;
    }

    static string getUsage() {
        string usage;
        usage.append("  Image Loader Utility                                                                                  \n");
        usage.append("                                                                                                        \n");
        usage.append("   -loadtest <filepath>                                                                                 \n");
        usage.append("   -convert  <source file>    <target file>                                                             \n");
        usage.append("   -convert8 <source file>    <target file>                                                             \n");
        usage.append("   -mip <stack input filepath>  <2D mip tif output filepath> [-flipy]                                   \n");
        usage.append("   -mapchannels <sourcestack> <targetstack> <csv map string, eg, \"0,1,2,0\" maps s0 to t1 and s2 to t0>\n");
        return usage;
    }

    bool execute();
    bool validateFile();
    My4DImage* loadImage(QString filepath);
    bool loadImage(Image4DSimple * stackp, QString filepath);
    bool saveImage(My4DImage * stackp, QString filepath);
    bool saveImage(My4DImage *stackp, QString filepath, bool saveTo8bit);

    int saveStack2RawPBD(const char * filename, ImagePixelType dataType, unsigned char* data, const V3DLONG * sz);
    int loadRaw2StackPBD(char * filename, Image4DSimple * & image, bool useThreading);

    int processArgs(vector<char*> *argList);
    QString getFilePrefix(QString filepath);

    V3DLONG decompressPBD8(unsigned char * sourceData, unsigned char * targetData, V3DLONG sourceLength);
    V3DLONG decompressPBD16(unsigned char * sourceData, unsigned char * targetData, V3DLONG sourceLength);
    void create2DMIPFromStack(My4DImage * image, QString mipFilepath);
    My4DImage* create2DMIPFromStack(My4DImage * image);

    bool mapChannels();

    virtual void run();
    // Set numeric index to help track which file is being loaded, for use in progress computation.
    ImageLoader& setProgressIndex(int index) {progressIndex = index; return *this;}

    unsigned char * convertType2Type1(My4DImage *image);
    void convertType2Type1InPlace(My4DImage *image);

signals:
    void progressValueChanged(int progress, int progressIndex); // 0-100
    void progressComplete(int progressIndex);
    void progressAborted(int progressIndex);
    void progressMessageChanged(QString message);

private:
    int mode;
    QString inputFilepath;
    QString targetFilepath;
    QString mapChannelString;
    My4DImage * image;
    FILE * fid;
    char * keyread;
    bool flipy;
    int loadDatatype;

    V3DLONG compressPBD8(unsigned char * compressionBuffer, unsigned char * sourceBuffer, V3DLONG sourceBufferLength, V3DLONG spaceLeft);
    V3DLONG compressPBD16(unsigned char * compressionBuffer, unsigned char * sourceBuffer, V3DLONG sourceBufferLength, V3DLONG spaceLeft);
    int exitWithError(QString errorMessage);
    void updateCompressionBuffer8(unsigned char * updatedCompressionBuffer);
    void updateCompressionBuffer16(unsigned char * updatedCompressionBuffer);

    V3DLONG totalReadBytes;
    V3DLONG maxDecompressionSize;
    unsigned char * compressionBuffer;
    unsigned char * decompressionBuffer;
    unsigned char * compressionPosition;
    unsigned char * decompressionPosition;
    int decompressionPrior;

    int progressIndex;
};

#endif // IMAGELOADER_H

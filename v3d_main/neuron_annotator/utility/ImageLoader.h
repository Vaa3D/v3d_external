#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include "ImageLoaderBasic.h"
#include "../../v3d/v3d_core.h"
#include <QIODevice>
#include <QString>
#include <QtCore>
#include <QDir>

using namespace std;

class ImageLoader : public QObject, public QRunnable, ImageLoaderBasic
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
    static const int MODE_CONVERT3;

    static string getCommandLineDescription() {
        return "image-loader";
    }

    static string getUsage() {
        string usage;
        usage.append("  Image Loader Utility                                                                                  \n");
        usage.append("                                                                                                        \n");
        usage.append("   -loadtest <filepath>                                                                                 \n");
        usage.append("   -convert  <source file>    <target file>                                                             \n");
        usage.append("   -convert8 <source file>    <target file>                                                             \n");
	usage.append("   -convert3 <source file>    <target file>                                                             \n");
        usage.append("   -mip <stack input filepath>  <2D mip tif output filepath> [-flipy]                                   \n");
        usage.append("   -mapchannels <sourcestack> <targetstack> <csv map string, eg, \"0,1,2,0\" maps s0 to t1 and s2 to t0>\n");
        return usage;
    }

    bool execute();
    bool validateFile();
    bool validateFile(QString filename);

    // URL versions, as oppose to file name versions
    My4DImage* loadImage(const char* filepath);
    My4DImage* loadImage(QUrl url);
    virtual bool loadImage(Image4DSimple * stackp, const char* filepath);
    bool loadImage(Image4DSimple * stackp, QUrl url);
    virtual int loadRaw2StackPBD(const char * filename, Image4DSimple * image, bool useThreading);
    int loadRaw2StackPBDFromUrl(QUrl url, Image4DSimple * image, bool useThreading);
    int loadRaw2StackPBDFromStream(QIODevice& fileStream, V3DLONG fileSize, Image4DSimple * image, bool useThreading);

    bool saveImage(My4DImage *stackp, const char* filepath, bool saveTo8bit=false);
    bool saveImage(My4DImage * stackp, const QString& filepath, bool saveTo8bit=false) {
        return saveImage(stackp, filepath.toStdString().c_str(), saveTo8bit);
    }

    int processArgs(vector<char*> *argList);
    QString getFilePrefix(const char* filepath);

    void create2DMIPFromStack(My4DImage * image, QString mipFilepath);
    My4DImage* create2DMIPFromStack(My4DImage * image);

    bool mapChannels();

    virtual void run();
    // Set numeric index to help track which file is being loaded, for use in progress computation.
    ImageLoader& setProgressIndex(int index) {progressIndex = index; return *this;}

    unsigned char * convertType2Type1(My4DImage *image);
    void convertType2Type1InPlace(My4DImage *image);

public slots:
    void cancel() {
        if (bIsCanceled) return;
        bIsCanceled = true;
        emit canceled();
    }

signals:
    void progressValueChanged(int progress, int progressIndex); // 0-100
    void progressComplete(int progressIndex);
    void progressAborted(int progressIndex);
    void progressMessageChanged(QString message);
    void canceled();

protected:
    virtual int exitWithError(QString errorMessage) {
        return exitWithError(errorMessage.toStdString());
    }
    virtual int exitWithError(std::string errorMessage) {
        return exitWithError(errorMessage.c_str());
    }
    virtual int exitWithError(const char* errorMessage) {
        int berror = ImageLoaderBasic::exitWithError(errorMessage);
        emit progressAborted(progressIndex);
        return berror;
    }

    bool saveImageByMode(My4DImage *stackp, const char* filepath, int saveMode);

private:
    int mode;
    QString inputFilepath;
    QString targetFilepath;
    QString mapChannelString;
    My4DImage * image;
    bool flipy;
    int progressIndex;

};

#endif // IMAGELOADER_H

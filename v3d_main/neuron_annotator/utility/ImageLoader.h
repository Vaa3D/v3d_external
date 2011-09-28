#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include <QString>
#include <QtCore>
#include <QDir>
#include "../../v3d/v3d_core.h"

using namespace std;

class ImageLoader
{
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
        usage.append("   -load <filepath1> <filepath2> ...                                      \n");
        return usage;
    }

    bool execute();
    bool validateFiles();
    My4DImage* loadImage(QString filepath);
    int saveStack2RawRE(const char * filename, unsigned char**** data, const V3DLONG * sz, int datatype);
    int loadRaw2StackRE(char * filename, My4DImage * & image);

    int processArgs(vector<char*> *argList);
    QString getFilePrefix(QString filepath);

private:
    QList<QString> inputFileList;
    QList<My4DImage*> imageList;

    V3DLONG compressCubeBuffer(unsigned char * imgRe, unsigned char * cubeBuffer, V3DLONG bufferLength, V3DLONG spaceLeft);


};

#endif // IMAGELOADER_H

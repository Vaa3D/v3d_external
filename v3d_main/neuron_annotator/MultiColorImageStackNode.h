#ifndef MULTICOLORIMAGESTACKNODE_H
#define MULTICOLORIMAGESTACKNODE_H

#include <QString>
#include <QFile>
#include "../v3d/v3d_core.h"
#include <iostream>
#include <fstream>
#include <limits>

class MultiColorImageStackNode
{
public:

    const static char * IMAGE_STACK_BASE_FILENAME;
    const static char * IMAGE_MASK_BASE_FILENAME;
    const static char * IMAGE_REFERENCE_BASE_FILENAME;

    MultiColorImageStackNode(QUrl imageDirParam);

    QUrl getPathToOriginalImageStackFile() { return pathToOriginalImageStackFile; }

    QUrl getPathToMulticolorLabelMaskFile() { return pathToMulticolorLabelMaskFile; }

    QUrl getPathToMulticolorLabelMaskIndexFile() { return pathToMulticolorLabelMaskIndexFile; }

    QUrl getPathToReferenceStackFile() { return pathToReferenceStackFile; }

    QUrl getPathToLsmFilePathsFile();

    QList<QUrl> getPathsToLsmMetadataFiles();

    QUrl getImageDir() {return imageDir;}

    void setPathToOriginalImageStackFile(QUrl pathToOriginalImageStackFileParam) {
        pathToOriginalImageStackFile=pathToOriginalImageStackFileParam;
    }
    void setPathToMulticolorLabelMaskFile(QUrl pathToMulticolorLabelMaskFileParam) {
        pathToMulticolorLabelMaskFile=pathToMulticolorLabelMaskFileParam;
    }

    void setPathToMulticolorLabelMasIndexFile(QUrl pathToMulticolorLabelMaskIndexFileParam) {
        pathToMulticolorLabelMaskIndexFile=pathToMulticolorLabelMaskIndexFileParam;
    }

    void setPathToReferenceStackFile(QUrl pathToReferenceStackFileParam) {
        pathToReferenceStackFile=pathToReferenceStackFileParam;
    }

    static bool writeMy4DImageToMaskFile(My4DImage* my4DImageAsMask, QString maskFilePath) {
        QString msgPrefix="MultiColorImageStackNode::writeMy4DToMaskFile()";
        ofstream out ( maskFilePath.toStdString().c_str(), ios::out | ios::binary );

        unsigned char **** rawMaskData = (unsigned char ****) my4DImageAsMask->getData();
        int maskSizeX = my4DImageAsMask->getXDim();
        int maskSizeY = my4DImageAsMask->getYDim();
        int maskSizeZ = my4DImageAsMask->getZDim();
        long longLength = maskSizeX * maskSizeY * maskSizeZ * 1 * 1; // one slice, 8-bit
        if (longLength >= INT_MAX) {
            cerr << "Length of my4DImageAsMask is greater than INT_MAX=" << INT_MAX << ". Implementation needs to be modified. " << endl;
            return false;
        }
        int intLength = maskSizeX * maskSizeY * maskSizeZ * 1 * 1; // one slice, 8-bit
        cout << "Preparing to output raw data size=" << intLength << endl;
        int progressVal=0;
        int writeUnit=100000;
        unsigned char * buffer = new unsigned char[writeUnit];
        int counter=0;
        int x,y,z;
        for (z=0;z<maskSizeZ;z++) {
            for (y=0;y<maskSizeY;y++) {
                for (x=0;x<maskSizeX;x++) {
                    if (counter==writeUnit) {
                        out.write((const char *)buffer, counter);
                        if (out.bad()) {
                            cerr << msgPrefix.toStdString() << " : error writing data - aborting" << endl;
                            delete [] buffer;
                            return false;
                        }
                        progressVal+=counter;
                        counter=0;
                    }
                    buffer[counter++]=rawMaskData[0][z][y][x];
                }
            }
        }
        out.write((const char *)buffer, counter);
        progressVal+=counter;
        cout << "Wrote " << progressVal << " bytes - done." << endl;
        out.flush();
        out.close();
        delete [] buffer;

        return true;
    }

    QStringList getLsmFilePathList();

private:
    long objectId;
    QUrl pathToOriginalImageStackFile;
    QUrl pathToMulticolorLabelMaskFile;
    QUrl pathToMulticolorLabelMaskIndexFile;
    QUrl pathToReferenceStackFile;
    QUrl imageDir;
};

#endif // MULTICOLORIMAGESTACKNODE_H

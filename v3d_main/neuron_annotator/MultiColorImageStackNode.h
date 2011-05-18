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

    const static char * IMAGE_STACK_FILENAME;
    const static char * IMAGE_MASK_FILENAME;
    const static char * IMAGE_MASK_INDEX_FILENAME;

    MultiColorImageStackNode();

    QString getPathToOriginalImageStackFile() { return pathToOriginalImageStackFile; }

    QString getPathToMulticolorLabelMaskFile() { return pathToMulticolorLabelMaskFile; }

    QString getPathToMulticolorLabelMaskIndexFile() { return pathToMulticolorLabelMaskIndexFile; }

    void setPathToOriginalImageStackFile(QString pathToOriginalImageStackFileParam) {
        pathToOriginalImageStackFile=pathToOriginalImageStackFileParam;
    }
    void setPathToMulticolorLabelMaskFile(QString pathToMulticolorLabelMaskFileParam) {
        pathToMulticolorLabelMaskFile=pathToMulticolorLabelMaskFileParam;
    }

    void setPathToMulticolorLabelMasIndexFile(QString pathToMulticolorLabelMaskIndexFileParam) {
        pathToMulticolorLabelMaskIndexFile=pathToMulticolorLabelMaskIndexFileParam;
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
        int progress=0;
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
                        progress+=counter;
                        counter=0;
                    }
                    buffer[counter++]=rawMaskData[0][z][y][x];
                }
            }
        }
        out.write((const char *)buffer, counter);
        progress+=counter;
        cout << "Wrote " << progress << " bytes - done." << endl;
        out.flush();
        out.close();
        delete [] buffer;

        return true;
    }

    // This method assumes that the given my4DImageAsMask already has the correct dimensions
    static bool readMaskFileToMy4DImage(My4DImage* my4DImageAsMask, QString maskFilePath) {
        QString msgPrefix = "MultiColorImageStackNode::readMaskFileToMy4DImage()";
        ifstream maskFile(maskFilePath.toAscii().data(), ios::in|ios::binary);
        const int BUFFER_SIZE=100000;
        char * buffer = new char [BUFFER_SIZE];
        unsigned char * ubuffer;
        int bufferPosition=BUFFER_SIZE; // to trigger initial read
        int maskSizeX=my4DImageAsMask->getXDim();
        int maskSizeY=my4DImageAsMask->getYDim();
        int maskSizeZ=my4DImageAsMask->getZDim();
        unsigned char **** maskData=(unsigned char ****)my4DImageAsMask->getData();
        long bytesExpected = maskSizeX*maskSizeY*maskSizeZ;
        long bytesRead=0;
        int x,y,z;
        for (z=0;z<maskSizeZ;z++) {
            for (y=0;y<maskSizeY;y++) {
                for (x=0;x<maskSizeX;x++) {
                    if (bufferPosition==BUFFER_SIZE) {
                        maskFile.read(buffer, BUFFER_SIZE);
                        ubuffer = (unsigned char *)buffer;
                        bytesRead+=maskFile.gcount();
                        if (maskFile.eof()) {
                            if (bytesRead < bytesExpected) {
                                maskFile.close();
                                cerr << msgPrefix.toStdString() << " : error , expected " << bytesExpected << " bytes but only read " <<
                                        bytesRead << " from file=" << maskFilePath.toStdString();
                                return false;
                            }
                        }
                        bufferPosition=0;
                    }
                    maskData[0][z][y][x]=ubuffer[bufferPosition++];
                }
            }
        }
        cout << msgPrefix.toStdString() << " : read " << bytesRead << " bytes from mask file=" << maskFilePath.toStdString() << endl;
        delete [] buffer;
        return true;
    }

private:
    long objectId;
    QString pathToOriginalImageStackFile;
    QString pathToMulticolorLabelMaskFile;
    QString pathToMulticolorLabelMaskIndexFile;
};

#endif // MULTICOLORIMAGESTACKNODE_H

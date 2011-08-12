#include <QString>
#include <QtCore>
#include <QDir>
#include "../../v3d/v3d_core.h"
#include "StitchedFileUtility.h"

using namespace std;

StitchedFileUtility::StitchedFileUtility()
{
    inputStitchedFilepath="";
    outputTifFilepath="";
}

int StitchedFileUtility::processArgs(vector<char*> *argList) {
    for (int i=0;i<argList->size();i++) {
        QString arg=(*argList)[i];
        if (arg=="-i") {
            i++;
            inputStitchedFilepath= (*argList)[i];
        } else if (arg=="-t") {
            i++;
            outputTifFilepath= (*argList)[i];
        }
    }
    if (inputStitchedFilepath.length()<1 || outputTifFilepath.size()<1) {
        return 1;
    }
    return 0;
}

bool StitchedFileUtility::execute() {

    // Validate input
    QFileInfo stitchedFileInfo(inputStitchedFilepath);
    if (stitchedFileInfo.exists()) {
        qDebug() << "Using stitched input file = " << inputStitchedFilepath;
    } else {
        qDebug() <<"Error: could not find stitched input file = " << inputStitchedFilepath;
        return false;
    }

    // We will load the raw file into a My4DImage and inspect the contents
    My4DImage stitchedImage;
    stitchedImage.loadImage(inputStitchedFilepath.toAscii().data());
    if (stitchedImage.isEmpty()) {
        qDebug() << "Error: stitched image is empty after loading";
        return false;
    }

    qDebug() << "Stitched input dimensions: x=" << stitchedImage.getXDim() << " y=" << stitchedImage.getYDim() << " z=" << stitchedImage.getZDim() << " c=" << stitchedImage.getCDim();

    if (stitchedImage.getCDim()<4) {
        qDebug() << "Expected 4 channels but only found c=" << stitchedImage.getCDim();
        return false;
    }

    // Attempt to save as tif file
    qDebug() << "Saving stitched image to file = " << outputTifFilepath;
    stitchedImage.saveFile(outputTifFilepath);

    return true;
}


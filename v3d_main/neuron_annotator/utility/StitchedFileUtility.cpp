#include <QString>
#include <QtCore>
#include <QDir>
#include "../../v3d/v3d_core.h"
#include "StitchedFileUtility.h"
#include "../basic_c_fun/imageio_mylib.h"

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
    stitchedImage.loadImage(inputStitchedFilepath.toUtf8().data());
    if (stitchedImage.isEmpty()) {
        qDebug() << "Error: stitched image is empty after loading";
        return false;
    }

    qDebug() << "Stitched input dimensions: x=" << stitchedImage.getXDim() << " y=" << stitchedImage.getYDim() << " z=" << stitchedImage.getZDim() << " c=" << stitchedImage.getCDim();

    if (stitchedImage.getCDim()<4) {
        qDebug() << "Expected 4 channels but only found c=" << stitchedImage.getCDim();
        return false;
    }

    // We need to reconstruct a new file with the signal information
    My4DImage signalImage;
    signalImage.loadImage(stitchedImage.getXDim(), stitchedImage.getYDim(), stitchedImage.getZDim(), 3 /* number of channels */, 1 /* bytes per channel */);
    Image4DProxy<My4DImage> signalProxy(&signalImage);
    Image4DProxy<My4DImage> stitchedProxy(&stitchedImage);

    qDebug() << "Populating new image with signal data";
    for (int z=0;z<stitchedImage.getZDim();z++) {
        for (int y=0;y<stitchedImage.getYDim();y++) {
            for (int x=0;x<stitchedImage.getXDim();x++) {
                int r=((*stitchedProxy.at_uint16(x,y,z,0))*256)/4096;
                int g=((*stitchedProxy.at_uint16(x,y,z,1))*256)/4096;
                int b=((*stitchedProxy.at_uint16(x,y,z,3))*256)/4096;

                if (r>255) r=255;
                if (g>255) g=255;
                if (b>255) b=255;

                signalProxy.put8bit_fit_at(x,y,z,0, r);
                signalProxy.put8bit_fit_at(x,y,z,1, g);
                signalProxy.put8bit_fit_at(x,y,z,2, b);
            }
        }
    }

    qDebug() << "Saving signal image to file=" << outputTifFilepath;
    //signalImage.saveFile(outputTifFilepath);

    V3DLONG mysz[4];
    mysz[0] = signalImage.getXDim();
    mysz[1] = signalImage.getYDim();
    mysz[2] = signalImage.getZDim();
    mysz[3] = signalImage.getCDim();

	// This version of mylib does not work with Windows.
#if defined(_WIN32) || defined(_WIN64)
	saveStack2Tif(outputTifFilepath.toUtf8().data(), (const unsigned char*)signalImage.getData(), mysz, signalImage.getDatatype());
#else
    saveStack2TifMylib(outputTifFilepath.toUtf8().data(), (const unsigned char*)signalImage.getData(), mysz, signalImage.getDatatype());
#endif

    signalImage.cleanExistData();
    return true;
}


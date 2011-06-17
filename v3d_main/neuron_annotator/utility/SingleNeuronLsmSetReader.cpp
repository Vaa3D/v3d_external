#include <QString>
#include <QtCore>
#include <QDir>
#include "../../v3d/v3d_core.h"
#include "SingleNeuronLsmSetReader.h"

using namespace std;

SingleNeuronLsmSetReader::SingleNeuronLsmSetReader()
{
    outputSignalTifFilepath="";
    outputReferenceTifFilepath="";
}

int SingleNeuronLsmSetReader::processArgs(vector<char*> *argList) {
    for (int i=0;i<argList->size();i++) {
        QString arg=(*argList)[i];
        if (arg=="-files") {
            bool done=false;
            do {
                QString possibleFile=(*argList)[++i];
                if (!possibleFile.startsWith("-")) {
                    inputLsmFileList.append(possibleFile);
                } else {
                    done=true;
                    i--; // rewind
                }
            } while(!done);
        } else if (arg=="-so") {
            i++;
            outputSignalTifFilepath = (*argList)[i];
        } else if (arg=="-ro") {
            i++;
            outputReferenceTifFilepath = (*argList)[i];
        }
    }
    if (inputLsmFileList.length()<1 || outputSignalTifFilepath.size()<1 || outputReferenceTifFilepath.size()<1) {
        return 1;
    }
    return 0;
}

bool SingleNeuronLsmSetReader::execute() {

    // Validate input
    if (inputLsmFileList.size()>2) {
        qDebug() << "SingleNeuronLsmSetReader currently supports only 2 lsm files per set";
        return false;
    }
    qDebug() << "Input lsm files:";
    for (int i=0;i<inputLsmFileList.size();i++) {
        QString filepath=inputLsmFileList.at(i);
        qDebug() << "Filepath=" << filepath;
        QFileInfo fileInfo(filepath);
        if (fileInfo.exists()) {
            qDebug() << " verified this file exists with size=" << fileInfo.size();
        } else {
            qDebug() << " file does not exist";
            return false;
        }
    }

    // We expect one file to contain a reference channel and 2 signal channels, and the other file
    // to contain a reference channel and 1 signal channel.

    My4DImage lsmImage1;
    lsmImage1.loadImage(inputLsmFileList.at(0).toAscii().data());
    if (lsmImage1.isEmpty()) {
        qDebug() << " error: image " << inputLsmFileList.at(0) << " loaded to empty state " << endl;
        return false;
    }
    qDebug() << "Lsm file 1 dimensions: x=" << lsmImage1.getXDim() << " y=" << lsmImage1.getYDim() << " z=" << lsmImage1.getZDim() << " c=" << lsmImage1.getCDim();

    My4DImage lsmImage2;
    lsmImage2.loadImage(inputLsmFileList.at(1).toAscii().data());
    if (lsmImage2.isEmpty()) {
        qDebug() << " error: image " << inputLsmFileList.at(1) << " loaded to empty state " << endl;
        return false;
    }
    qDebug() << "Lsm file 2 dimensions: x=" << lsmImage2.getXDim() << " y=" << lsmImage2.getYDim() << " z=" << lsmImage2.getZDim() << " c=" << lsmImage2.getCDim();

    bool twoChannelCheck=false;
    bool threeChannelCheck=false;

    if (lsmImage1.getCDim()==2 || lsmImage2.getCDim()==2) {
        twoChannelCheck=true;
    }

    if (lsmImage1.getCDim()==3 || lsmImage2.getCDim()==3) {
        threeChannelCheck=true;
    }

    if (!(twoChannelCheck && threeChannelCheck)) {
        qDebug() << "One lsm file must have 2 channels and the other 3 channels";
        return false;
    }

    My4DImage* twoChannelLsm;
    My4DImage* threeChannelLsm;

    if (lsmImage1.getCDim()==2) {
        twoChannelLsm=&lsmImage1;
        threeChannelLsm=&lsmImage2;
    } else {
        twoChannelLsm=&lsmImage2;
        threeChannelLsm=&lsmImage1;
    }

    // Check dimensions
    if (!(twoChannelLsm->getXDim()==threeChannelLsm->getXDim() &&
          twoChannelLsm->getYDim()==threeChannelLsm->getYDim() &&
          twoChannelLsm->getZDim()==threeChannelLsm->getZDim())) {
        qDebug() << "X, Y, and Z dimensions of the lsm files must match";
        return false;
    }

    int twoChannelReferenceIndex=findReferenceChannel(twoChannelLsm);
    int threeChannelReferenceIndex=findReferenceChannel(threeChannelLsm);

    int twoChannelSignal1=0;
    int threeChannelSignal1=0;
    int threeChannelSignal2=0;

    if (twoChannelReferenceIndex==0) {
        twoChannelSignal1=1;
    } else {
        twoChannelSignal1=0;
    }

    if (threeChannelReferenceIndex==0) {
        threeChannelSignal1=1;
        threeChannelSignal2=2;
    } else if (threeChannelReferenceIndex==1) {
        threeChannelSignal1=0;
        threeChannelSignal2=2;
    } else if (threeChannelReferenceIndex==2) {
        threeChannelSignal1=0;
        threeChannelSignal2=1;
    }

    // We need to reconstruct a new file with the signal information
    My4DImage signalImage;
    signalImage.loadImage(twoChannelLsm->getXDim(), twoChannelLsm->getYDim(), twoChannelLsm->getZDim(), 3 /* number of channels */, 1 /* bytes per channel */);
    Image4DProxy<My4DImage> signalProxy(&signalImage);
    Image4DProxy<My4DImage> twoChannelProxy(twoChannelLsm);
    Image4DProxy<My4DImage> threeChannelProxy(threeChannelLsm);

    qDebug() << "Populating new image with signal data";
    for (int z=0;z<twoChannelLsm->getZDim();z++) {
        for (int y=0;y<twoChannelLsm->getYDim();y++) {
            for (int x=0;x<twoChannelLsm->getXDim();x++) {
                signalProxy.put8bit_fit_at(x,y,z,0,threeChannelProxy.value8bit_at( x, y, z, threeChannelSignal1));
                signalProxy.put8bit_fit_at(x,y,z,1,threeChannelProxy.value8bit_at( x, y, z, threeChannelSignal2));
                signalProxy.put8bit_fit_at(x,y,z,2,twoChannelProxy.value8bit_at(   x, y, z, twoChannelSignal1));
            }
        }
    }

    qDebug() << "Saving signal image to file=" << outputSignalTifFilepath;
    signalImage.saveFile(outputSignalTifFilepath);
    signalImage.cleanExistData();

    My4DImage referenceImage;
    referenceImage.loadImage(threeChannelLsm->getXDim(), threeChannelLsm->getYDim(), threeChannelLsm->getZDim(), 1 /* number of channels */, 1 /* bytes per channel */);
    Image4DProxy<My4DImage> referenceProxy(&referenceImage);

    qDebug() << "Populating new image with reference data";
    for (int z=0;z<threeChannelLsm->getZDim();z++) {
        for (int y=0;y<threeChannelLsm->getYDim();y++) {
            for (int x=0;x<threeChannelLsm->getXDim();x++) {
                referenceProxy.put8bit_fit_at(x,y,z,0,threeChannelProxy.value8bit_at( x, y, z, threeChannelReferenceIndex));
            }
        }
    }

    qDebug() << "Saving reference image to file=" << outputReferenceTifFilepath;
    referenceImage.saveFile(outputReferenceTifFilepath);
    referenceImage.cleanExistData();

    twoChannelLsm->cleanExistData();
    threeChannelLsm->cleanExistData();

    return true;
}

// This function simply picks the channel with the highest aggregate
// intensity as the reference channel.
int SingleNeuronLsmSetReader::findReferenceChannel(My4DImage* image) {
    QList<long> intensityCount;
    Image4DProxy<My4DImage> imageProxy(image);
    for (int c=0;c<image->getCDim();c++) {
        long count=0;
        for (int z=0;z<image->getZDim();z++) {
            for (int y=0;y<image->getYDim();y++) {
                for (int x=0;x<image->getXDim();x++) {
                    count+=imageProxy.value8bit_at(x,y,z,c);
                }
            }
        }
        intensityCount.append(count);
    }
    int maxIndex=0;
    long maxValue=0;
    for (int c=0;c<intensityCount.size();c++) {
        long intensityTotal=intensityCount.at(c);
        qDebug() << "channel " << c << " intensityCount=" << intensityTotal;
        if (intensityTotal>maxValue) {
            maxIndex=c;
            maxValue=intensityTotal;
        }
    }
    qDebug() << " returning reference channel = " << maxIndex;
    return maxIndex;
}



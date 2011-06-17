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


    return true;
}



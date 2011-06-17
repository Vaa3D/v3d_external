#include <QString>
#include <QtCore>
#include <QDir>
#include <QRegExp>
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
    qDebug() << "Input lsm files:";
    for (int i=0;i<inputLsmFileList.size();i++) {
        QString filepath=inputLsmFileList.at(i);
        qDebug() << "Filepath=" << filepath;
        QFileInfo fileInfo(filepath);
        if (fileInfo.exists()) {
            qDebug() << " verified this file exists with size=" << fileInfo.size();
        } else {
            qDebug() << " file does not exist";
        }
    }
    return true;
}



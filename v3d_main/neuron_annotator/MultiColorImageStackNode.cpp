#include "MultiColorImageStackNode.h"

const char * MultiColorImageStackNode::IMAGE_STACK_FILENAME = "ConsolidatedSignal.tif";
const char * MultiColorImageStackNode::IMAGE_MASK_FILENAME = "ConsolidatedLabel.tif";
const char * MultiColorImageStackNode::IMAGE_REFERENCE_FILENAME = "Reference.tif";

MultiColorImageStackNode::MultiColorImageStackNode(QDir imageDirParam)
{
    imageDir=imageDirParam;
}

QStringList MultiColorImageStackNode::getPathsToLsmMetadataFiles() {
    QStringList metadataPathList;
    QFileInfoList fileInfoList=imageDir.entryInfoList();
    for (int i=0;i<fileInfoList.size();i++) {
        QFileInfo fileInfo=fileInfoList.at(i);
        if (fileInfo.fileName().endsWith(".metadata")) {
            metadataPathList.append(fileInfo.absoluteFilePath());
        }
    }
    return metadataPathList;
}

QString MultiColorImageStackNode::getPathToLsmFilePathsFile() {
    QString dirPath=imageDir.absolutePath();
    dirPath.append("/lsmFilePaths.txt");
    return dirPath;
}

QStringList MultiColorImageStackNode::getLsmFilePathList() {
    QStringList lsmFilePathList;
    QString filePath=getPathToLsmFilePathsFile();
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        cerr << "Could not open file=" << filePath.toStdString() << " to read\n";
        return lsmFilePathList;
    }
    while(!file.atEnd()) {
        lsmFilePathList.append(file.readLine());
    }
    file.close();
    return lsmFilePathList;
}




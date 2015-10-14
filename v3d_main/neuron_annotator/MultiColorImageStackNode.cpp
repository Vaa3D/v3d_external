#include "MultiColorImageStackNode.h"
#include "utility/url_tools.h"

const char * MultiColorImageStackNode::IMAGE_STACK_BASE_FILENAME = "ConsolidatedSignal";
const char * MultiColorImageStackNode::IMAGE_MASK_BASE_FILENAME = "ConsolidatedLabel";
const char * MultiColorImageStackNode::IMAGE_REFERENCE_BASE_FILENAME = "Reference";

MultiColorImageStackNode::MultiColorImageStackNode(QUrl imageDirParam)
{
    imageDir=imageDirParam;
}

QList<QUrl> MultiColorImageStackNode::getPathsToLsmMetadataFiles()
{
    QList<QUrl> metadataPathList;
    QString dirString = imageDir.toLocalFile();
    if (dirString.isEmpty()) {
        // Only local files can be loaded this way.
        // Use Entity OpticalResolution instead.
        // TODO - handle non-local directories that way.
        return metadataPathList;
    }
    // Load file list from local file system
    QDir dir(dirString);
    QFileInfoList fileInfoList=dir.entryInfoList();
    for (int i=0;i<fileInfoList.size();i++) {
        QFileInfo fileInfo=fileInfoList.at(i);
        if (fileInfo.fileName().endsWith(".metadata")) {
            metadataPathList.append(
                    QUrl::fromLocalFile(fileInfo.absoluteFilePath()));
        }
    }
    return metadataPathList;
}

QUrl MultiColorImageStackNode::getPathToLsmFilePathsFile() {
    return appendPath(imageDir, "lsmFilePaths.txt");
}

QStringList MultiColorImageStackNode::getLsmFilePathList() {
    QStringList lsmFilePathList;
    QUrl fileUrl=getPathToLsmFilePathsFile();
    UrlStream stream(fileUrl);
    if (stream.io() == NULL) {
        qDebug() << "Could not open file=" << fileUrl << " to read";
        return lsmFilePathList;
    }
    while(!stream.io()->atEnd()) {
        lsmFilePathList.append(stream.io()->readLine());
    }
    return lsmFilePathList;
}




#include "NeuronAnnotatorResultNode.h"
#include <QDir>
#include <QFile>
#include <QString>

NeuronAnnotatorResultNode::NeuronAnnotatorResultNode(long objectId)
{
    this->objectId=objectId;
}

bool NeuronAnnotatorResultNode::ensureDirectoryExists() {
    QString path = getDirectoryPath();
    QDir dir(path);
    return dir.mkpath(path);
}

QDir NeuronAnnotatorResultNode::getUserNeuronAnnotatorHomeDir() {
    QDir homeDir = QDir::home();
    QString neuronAnnotatorHomeDirPath = homeDir.absolutePath();
    neuronAnnotatorHomeDirPath.append("/");
    neuronAnnotatorHomeDirPath.append("NeuronAnnotator");
    QDir neuronAnnotatorHomeDir(neuronAnnotatorHomeDirPath);
    return neuronAnnotatorHomeDir;
}

QString NeuronAnnotatorResultNode::getDirectoryPath() {
    if (directoryPath.isNull() || directoryPath.isEmpty()) {
        QDir homeDir = getUserNeuronAnnotatorHomeDir();
        directoryPath = homeDir.absolutePath();
        directoryPath.append("/");
        directoryPath.append("NeuronAnnotatorResultNode");
        directoryPath.append("/");
        QString objectIdString = QString("%1").arg(objectId);
        directoryPath.append(objectIdString);
    }
    return directoryPath;
}





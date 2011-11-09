#ifndef NEURONANNOTATORRESULTNODE_H
#define NEURONANNOTATORRESULTNODE_H

#include <QString>
#include <QDir>

class NeuronAnnotatorResultNode
{
public:
    NeuronAnnotatorResultNode(long objectId);
    long getObjectId() { return objectId; }
    bool ensureDirectoryExists();
    QString getDirectoryPath();
    QDir getUserNeuronAnnotatorHomeDir();


private:
    long objectId;
    QString directoryPath;
};

#endif // NEURONANNOTATORRESULTNODE_H

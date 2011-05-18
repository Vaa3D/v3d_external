#ifndef NEURONMASKENTRY_H
#define NEURONMASKENTRY_H

#include <QString>

class NeuronMaskEntry
{
public:
    NeuronMaskEntry();
    QString toString();
    bool populateFromString(QString fromString);

    int getLabelIndex() { return labelIndex; }
    int getSegmentIndex() { return segmentIndex; }
    int getNeuronIndex() { return neuronIndex; }
    QString getFilePath() { return filePath; }
    bool operator< (const NeuronMaskEntry &other) const;

private:
    int labelIndex;
    int segmentIndex;
    int neuronIndex;
    QString filePath;
};

#endif // NEURONMASKENTRY_H

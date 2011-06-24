#include <QRegExp>
#include "NeuronMaskEntry.h"
#include <iostream>

using namespace std;

NeuronMaskEntry::NeuronMaskEntry()
{
}

NeuronMaskEntry::NeuronMaskEntry(int label, int segment, int neuron) {
    this->labelIndex=label;
    this->segmentIndex=segment;
    this->neuronIndex=neuron;
}

bool NeuronMaskEntry::populateFromString(QString fromString) {
    QRegExp regexp("(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\S.+\\S)");
    int matchPosition = regexp.indexIn(fromString);
    if (matchPosition<0) {
        cerr << "NeuronMaskEntry::populateFromString() : warning: could not construct NeuronMaskEntry from string=" << fromString.toStdString() << endl;
        return false;
    }
    labelIndex=regexp.cap(1).toInt();
    segmentIndex=regexp.cap(2).toInt();
    neuronIndex=regexp.cap(3).toInt();
    filePath=regexp.cap(4);
    return true;
}

QString NeuronMaskEntry::toString() {
    QString toString = QString("%1 %2 %3 %4").arg(labelIndex).arg(segmentIndex).arg(neuronIndex).arg(filePath);
    return toString;
}

bool NeuronMaskEntry::operator< (const NeuronMaskEntry &other) const {
    if (labelIndex < other.labelIndex) {
        return true;
    } else if (labelIndex == other.labelIndex) {
        if (segmentIndex < other.segmentIndex) {
            return true;
        } else if (segmentIndex == other.segmentIndex) {
            if (neuronIndex < other.neuronIndex) {
                return true;
            }
        }
    }
    return false;
}


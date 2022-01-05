#ifndef SINGLENEURONLSMSETREADER_H
#define SINGLENEURONLSMSETREADER_H

#include <QString>
#include <QtCore>
#include <QDir>
#include "../../v3d/v3d_core.h"

//using namespace std;

class SingleNeuronLsmSetReader
{
public:
    SingleNeuronLsmSetReader();

    static string getCommandLineDescription() {
        return "single-neuron-lsm-set-reader";
    }

    static string getUsage() {
        string usage;
        usage.append("  Single-Neuron Data Lsm Set Reader                                       \n");
        usage.append("                                                                          \n");
        usage.append("   -files f1 [f2] [f3] ...                                                \n");
        usage.append("   -so  <signal channels tif output file>                                 \n");
        usage.append("   -ro  <reference channel tif output file>                               \n");
        return usage;
    }

    bool execute();

    int processArgs(vector<char*> *argList);


private:
    QList<QString> inputLsmFileList;
    QString outputSignalTifFilepath;
    QString outputReferenceTifFilepath;
    int findReferenceChannel(My4DImage* image);

};

#endif // SINGLENEURONLSMSETREADER_H

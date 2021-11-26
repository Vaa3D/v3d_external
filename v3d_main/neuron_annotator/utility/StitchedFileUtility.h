#ifndef STITCHEDFILEUTILITY_H
#define STITCHEDFILEUTILITY_H

#include <QString>
#include <QtCore>
#include <QDir>
#include "../../v3d/v3d_core.h"

using namespace std;

class StitchedFileUtility
{
public:
    StitchedFileUtility();

    static string getCommandLineDescription() {
        return "stitched-file-utility";
    }

    static string getUsage() {
        string usage;
        usage.append("  Stitched File Utility                                                       \n");
        usage.append("                                                                              \n");
        usage.append("   -i  <stitched file in .raw format with 3 signal channels plus reference>   \n");
        usage.append("                                                                              \n");
        usage.append(" The -t option is used to convert the raw file to a tif to match the         \n");
        usage.append("  input requirements for the sampsep neuron separation program                \n");
        usage.append("                                                                              \n");
        usage.append("   [ -t <tif filename> ]                                                      \n");
        return usage;
    }

    bool execute();

    int processArgs(vector<char*> *argList);


private:
    QString inputStitchedFilepath;
    QString outputTifFilepath;

};

#endif // STITCHEDFILEUTILITY_H

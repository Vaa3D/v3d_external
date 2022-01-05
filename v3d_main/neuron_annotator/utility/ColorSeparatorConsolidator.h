#ifndef COLORSEPARATORCONSOLIDATOR_H
#define COLORSEPARATORCONSOLIDATOR_H

#include <QString>
#include <QtCore>
#include <QDir>
//#include <QRegExp>
#include "../../v3d/v3d_core.h"

//using namespace std;

class ColorSeparatorConsolidator
{
public:
    ColorSeparatorConsolidator();
    bool execute();
    int processArgs(vector<char*> *argList);

    static string getCommandLineDescription() {
        return "consolidate-color-separator-output";
    }

    static QString maskFileRegExpString;

    static string getUsage() {
        string usage;
        usage.append("  ColorSeparatorConsolidator                                              \n");
        usage.append("                                                                          \n");
        usage.append("    This tool takes the output of the NeuronSeparator pipeline and creates\n");
        usage.append("    a consolidated mask file which is used as input for the NeuronAnnotator.\n");
        usage.append("                                                                          \n");
        usage.append("   -i  <separator results directory>                                      \n");
        usage.append("   -o  <output mask file>                                                 \n");
        usage.append("\n");
        return usage;
    }

private:
    QString separatorDirectory;
    QString outputFile;
    My4DImage* consolidatedMask;
    QStringList maskFileList;
    //QRegExp maskFileRegExp;
    int maskSizeX;
    int maskSizeY;
    int maskSizeZ;

    bool populateMaskFileList();
    bool writeMetadataFile();
    bool generateMask();
    bool writeMask();
    void cleanup();
    bool sortMaskFileList();

};

#endif // COLORSEPARATORCONSOLIDATOR_H

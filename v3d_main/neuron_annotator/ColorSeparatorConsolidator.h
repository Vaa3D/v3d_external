#ifndef COLORSEPARATORCONSOLIDATOR_H
#define COLORSEPARATORCONSOLIDATOR_H

#include <QString>
#include <QtCore>
#include <QDir>
#include <QRegExp>
#include "../v3d/v3d_core.h"

using namespace std;

class ColorSeparatorConsolidator
{
public:
    ColorSeparatorConsolidator(QString separatorDirectory, QString outputFile);
    bool execute();

    static string getCommandLineDescription() {
        return "consolidate-color-separator-output";
    }

    static QString maskFileRegExpString;

private:
    QString separatorDirectory;
    QString outputFile;
    My4DImage* consolidatedMask;
    QStringList maskFileList;
    QRegExp maskFileRegExp;
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

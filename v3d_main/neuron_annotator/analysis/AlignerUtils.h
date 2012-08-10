#ifndef ALIGNERUTILS_H
#define ALIGNERUTILS_H

#include <QString>
#include <QtCore>
#include <QDir>
#include "../../v3d/v3d_core.h"

using namespace std;

class AlignerUtils
{
public:

    static const int MODE_UNDEFINED;
    static const int MODE_SUBVOLUME;

    AlignerUtils();

    ~AlignerUtils();

    static string getCommandLineDescription() {
        return "aligner-utils";
    }

    static string getUsage() {
        string usage;
        usage.append("  Aligner Utilities                                                                                     \n");
        usage.append("                                                                                                        \n");
        usage.append("    Tools for working with the brain aligner.                                                           \n");
        usage.append("                                                                                                        \n");
        usage.append("     -sourceStack <stack from which to create subvolume>                                                \n");
        usage.append("     -subVolume \"x0 x1 y0 y1 z0 z1\"                                                                   \n");
        usage.append("     -outputStack <output stack>                                                                        \n");
        usage.append("                                                                                                        \n");
        return usage;
    }

    bool execute();
    int processArgs(vector<char*> *argList);

private:

    int mode;

    QString sourceStackFilepath;
    QString subVolumeString;
    QString outputStackFilepath;

    bool createSubVolume();

};

#endif // ALIGNERUTILS_H

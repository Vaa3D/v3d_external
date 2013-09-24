#ifndef ALIGNERUTILS_H
#define ALIGNERUTILS_H

#include <QString>
#include <QtCore>
#include <QDir>
#include "../../v3d/v3d_core.h"
#include "../../v3d/histogramsimple.h"

using namespace std;

class AlignerUtils
{
public:

    static const int MODE_UNDEFINED;
    static const int MODE_REFERENCE_RANK;

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
	usage.append("       Mode reference_rank ranks each channel in a list, with the first channel as suggested reference  \n");
	usage.append("        and each channel thereafter ranked from most intensity to least.                                \n");
	usage.append("                                                                                                        \n");
	usage.append("     -mode <reference_rank>                                                                             \n");
        usage.append("                                                                                                        \n");
	usage.append("    For mode reference_rank:                                                                            \n");
	usage.append("                                                                                                        \n");
        usage.append("     -sourceStack <stack from which to create subvolume>                                                \n");
        usage.append("                                                                                                        \n");
        return usage;
    }

    bool execute();
    int processArgs(vector<char*> *argList);

private:

    int mode;
    QString sourceStackFilepath;
    HistogramSimple histogram;

    bool referenceRank();

};

#endif // ALIGNERUTILS_H

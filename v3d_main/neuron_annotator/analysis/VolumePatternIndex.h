#ifndef VOLUMEPATTERNINDEX_H
#define VOLUMEPATTERNINDEX_H

#include <QString>
#include <QtCore>
#include <QDir>
#include "../../v3d/v3d_core.h"

using namespace std;

class VolumePatternIndex
{
public:

    static const int MODE_UNDEFINED;
    static const int MODE_INDEX;
    static const int MODE_SEARCH;

    VolumePatternIndex();

    ~VolumePatternIndex();

    static string getCommandLineDescription() {
        return "volume-pattern-index";
    }

    static string getUsage() {
        string usage;
        usage.append("  Volume Pattern Index Tool                                                                             \n");
        usage.append("                                                                                                        \n");
        usage.append("    Tool for indexing and searching volumes for similar patterns                                        \n");
        usage.append("                                                                                                        \n");
        usage.append("     -mode [index|search]                                                                               \n");
        usage.append("                                                                                                        \n");
        usage.append("    For mode index:                                                                                     \n");
        usage.append("                                                                                                        \n");
        usage.append("     -inputList <list of stacks to index>                                                               \n");
        usage.append("     -outputIndex <index file>                                                                          \n");
        usage.append("     [ -subVolume \"x0 x1 y0 y1 z0 z1\" ]                                                               \n");
        usage.append("                                                                                                        \n");
        usage.append("    For mode search:                                                                                    \n");
        usage.append("                                                                                                        \n");
        usage.append("     -query <image volume to use as query>                                                              \n");
        usage.append("     [ -maxHits <maximum number of hits> ] : default=100                                                \n");
        usage.append("     [ -fast ] : search only using index, faster but less accurate                                      \n");
        usage.append("                                                                                                        \n");
        return usage;
    }

    bool execute();
    int processArgs(vector<char*> *argList);

private:

    int mode;

    QString modeString;
    QString inputFileList;
    QString outputIndexFile;
    QString subVolumeString;

    QString queryImageFile;
    int maxHits;
    bool fastSearch;

    bool createSubVolume();
    bool createIndex();
    bool doSearch();

};

#endif // VOLUMEPATTERNINDEX_H

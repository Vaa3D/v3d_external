#ifndef SCREENPATTERNANNOTATOR_H
#define SCREENPATTERNANNOTATOR_H

#include <QString>
#include <QtCore>
#include <QDir>
#include "../../v3d/v3d_core.h"

using namespace std;


class ScreenPatternAnnotator
{
public:
    ScreenPatternAnnotator();

    ~ScreenPatternAnnotator();

    static string getCommandLineDescription() {
        return "screen-pattern-annotator";
    }

    static string getUsage() {
        string usage;
        usage.append("  Screen Pattern Analysis Tool                                                                          \n");
        usage.append("                                                                                                        \n");
        usage.append("    This tool takes as input the 3D stack containing a Fly-Brain expression pattern. It assumes the     \n");
        usage.append("    stack is already aligned.                                                                           \n");
        usage.append("                                                                                                        \n");
        usage.append("   -input <filepath for stack>                                                                          \n");
        usage.append("   -pattern_channel  <0-based channel index in stack for pattern channel>                               \n");
        usage.append("   -prefix <prefix for output files>                                                                    \n");
        usage.append("   -dir <output directory>                                                                              \n");
        return usage;
    }

    bool execute();

    int processArgs(vector<char*> *argList);



};

#endif // SCREENPATTERNANNOTATOR_H

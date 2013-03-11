#ifndef NEURONFRAGMENTEDITOR_H
#define NEURONFRAGMENTEDITOR_H

#include <QString>
#include <QtCore>
#include <QDir>
#include "../../v3d/v3d_core.h"

using namespace std;

class NeuronFragmentEditor
{
public:

    NeuronFragmentEditor();

    ~NeuronFragmentEditor();

    static string getCommandLineDescription() {
        return "neuron-fragment-editor";
    }

    static string getUsage() {
        string usage;
        usage.append("  Neuron Fragment Editor                                                                                \n");
        usage.append("                                                                                                        \n");
        usage.append("    This tool creates composites of neuron fragments.                                                   \n");
        usage.append("                                                                                                        \n");
        usage.append("   -sourceImage <original image file>                                                                   \n");
        usage.append("   -labelIndex <consolidated signal label index file>                                                   \n");
        usage.append("   -fragments <comma-separated list of fragments, e.g., '13,18,24'>                                     \n");
        usage.append("   -outputMip <path for output max intensity projection>                                                \n");
        usage.append("   -outputStack <path for output stack>                                                                 \n");
        usage.append("                                                                                                        \n");
        return usage;
    }

    bool execute();

    int processArgs(vector<char*> *argList);

    My4DImage * createMIPFromImage(My4DImage * image);
    bool createFragmentComposite();

private:

    QString sourceImageFilepath;
    QString inputLabelIndexFilepath;
    QString fragmentListString;
    QString outputMipFilepath;
    QString outputStackFilepath;

    QList<int> fragmentList;

};

#endif // NEURONFRAGMENTEDITOR_H

#ifndef NEURONFRAGMENTEDITOR_H
#define NEURONFRAGMENTEDITOR_H

#include <QString>
#include <QtCore>
#include <QDir>
#include "../../v3d/v3d_core.h"

using namespace std;

class MaskRay
{
public:
    long skipCount;
    QList<long> startList;
    QList<long> endList;
};

class NeuronFragmentEditor
{
public:
    static const int MODE_UNDEFINED;
    static const int MODE_COMBINE;
    static const int MODE_REVERSE_LABEL;

    NeuronFragmentEditor();

    ~NeuronFragmentEditor();

    static string getCommandLineDescription() {
        return "neuron-fragment-editor";
    }

    static string getUsage() {
        string usage;
        usage.append("  Neuron Fragment Editor                                                                                \n");
        usage.append("                                                                                                        \n");
        usage.append("   [ -mode <either 'combine' or 'reverse-label', default=combine> ]                                     \n");
        usage.append("   -sourceImage <original image file>                                                                   \n");
        usage.append("   -labelIndex <consolidated signal label index file>                                                   \n");
        usage.append("                                                                                                        \n");
        usage.append("  For mode=combine, create composites of neuron fragments:                                              \n");
        usage.append("                                                                                                        \n");
        usage.append("   -fragments <comma-separated list of fragments, e.g., '13,18,24'>                                     \n");
        usage.append("   -outputMip <path for output max intensity projection>                                                \n");
        usage.append("   -outputStack <path for output stack>                                                                 \n");
        usage.append("                                                                                                        \n");
        usage.append("  For mode=reverse-label, split label file to constituent mask/intensity subfiles:                      \n");
        usage.append("                                                                                                        \n");
        usage.append("   -outputDir <output directory>                                                                        \n");
        usage.append("   [ -outputPrefix <prefix for each output file> ]                                                      \n");
        usage.append("                                                                                                        \n");
        return usage;
    }

    bool execute();
    int processArgs(vector<char*> *argList);
    bool createFragmentComposite();
    bool reverseLabel();
    bool loadSourceAndLabelImages();

private:
    int mode;
    QString sourceImageFilepath;
    QString inputLabelIndexFilepath;
    My4DImage* sourceImage;
    My4DImage* labelImage;
    long xdim;
    long ydim;
    long zdim;
    long cdim;
    v3d_uint8* label8;
    v3d_uint16* label16;

    // mode=combine
    QString fragmentListString;
    QString outputMipFilepath;
    QString outputStackFilepath;
    QList<int> fragmentList;

    // mode=reverse-label
    QString outputDirPath;
    QString outputPrefix;

    void writeMaskList(FILE* fid, QList<MaskRay*>& list);
    void axisTracer(int direction, int label, QList<MaskRay*> * rayList, long& pairCount, long& voxelCount,
                    long& x0, long& x1, long& y0, long& y1, long& z0, long& z1, void* data=0L, long assumedVoxels=0L);

};

#endif // NEURONFRAGMENTEDITOR_H

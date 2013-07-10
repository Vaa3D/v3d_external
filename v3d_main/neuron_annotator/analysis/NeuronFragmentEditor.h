#ifndef NEURONFRAGMENTEDITOR_H
#define NEURONFRAGMENTEDITOR_H

#include <QString>
#include <QtCore>
#include <QDir>
#include "../../v3d/v3d_core.h"
#include "MaskChan.h"

using namespace std;

class NeuronFragmentEditor
{
public:
    static const int MODE_UNDEFINED;
    static const int MODE_COMBINE;
    static const int MODE_COMBINE_MASK;
    static const int MODE_REVERSE_LABEL;
    static const int MODE_MIPS;

    NeuronFragmentEditor();

    ~NeuronFragmentEditor();

    static string getCommandLineDescription() {
        return "neuron-fragment-editor";
    }

    static string getUsage() {
        string usage;
        usage.append("  Neuron Fragment Editor                                                                                \n");
        usage.append("                                                                                                        \n");
        usage.append("  Mode summary                                                                                          \n");
        usage.append("                                                                                                        \n");
        usage.append("   combine: given signal/label images, combine fragments into a new 3D stack and mip                    \n");
        usage.append("   combine-mask: given a list of mask files, combine into new 3D stack and mip                          \n");
        usage.append("   reverse-label: given signal/label images, produce a set of mask/channel files                        \n");
        usage.append("   mips: generate all mips for signal/label stacks                                                      \n");
        usage.append("                                                                                                        \n");
        usage.append("   [ -mode < combine | combine-mask | reverse-label | mips , default=combine> ]                         \n");
        usage.append("                                                                                                        \n");
        usage.append("  For modes combine | reverse-label | mips:                                                             \n");
        usage.append("   -sourceImage <original image file>                                                                   \n");
        usage.append("   -labelIndex <consolidated signal label index file>                                                   \n");
        usage.append("                                                                                                        \n");
        usage.append("  For modes combine | combine-mask:                                                                     \n");
        usage.append("                                                                                                        \n");
        usage.append("   -outputMip <path for output max intensity projection>                                                \n");
        usage.append("   -outputStack <path for output stack>                                                                 \n");
        usage.append("                                                                                                        \n");
        usage.append("  For mode=combine, create composites from signal/label files:                                          \n");
        usage.append("   -fragments <comma-separated list of fragments, e.g., '13,18,24'>                                     \n");
        usage.append("                                                                                                        \n");
        usage.append("  For mode=combine-mask, create composites from mask/chan files:                                        \n");
        usage.append("                                                                                                        \n");
        usage.append("   -maskFiles <list of fullpaths for mask files to be included, will look for corres chan files>        \n");
        usage.append("                                                                                                        \n");
        usage.append("  For mode=reverse-label | mips                                                                         \n");
        usage.append("                                                                                                        \n");
        usage.append("   -outputDir <output directory>                                                                        \n");
        usage.append("   -outputPrefix <prefix for each output file>                                                          \n");
        usage.append("                                                                                                        \n");
        usage.append("  For all modes:                                                                                        \n");
        usage.append("                                                                                                        \n");
        usage.append("   -maxThreadCount <max threads>                                                                        \n");
        return usage;
    }

    bool execute();
    int processArgs(vector<char*> *argList);
    bool createFragmentComposite();
    bool createImagesFromFragmentList(QList<int> fragmentList, QString stackFilename, QString mipFilename);
    bool createMaskComposite();
    bool reverseLabel();
    bool loadSourceAndLabelImages();
    bool createMips();

private:
    int mode;
    QString sourceImageFilepath;
    QString inputLabelIndexFilepath;
    My4DImage* sourceImage;
    My4DImage* labelImage;
    MaskChan maskChan;
    long xdim;
    long ydim;
    long zdim;
    long cdim;
    v3d_uint8* label8;
    v3d_uint16* label16;
    long* labelIndex;
    QReadWriteLock mutex;
    int maxThreadCount;

    // mode=combine | combine-mask
    QString outputMipFilepath;
    QString outputStackFilepath;

    // mode=combine
    QString fragmentListString;
    QList<int> fragmentList;

    // mode=combine-mask
    QStringList maskFilePaths;

    // mode=reverse-label
    QString outputDirPath;
    QString outputPrefix;

    QString createFullPathFromLabel(int label, QString extension);

};

#endif // NEURONFRAGMENTEDITOR_H

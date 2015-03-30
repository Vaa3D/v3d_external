#include <iostream>
#include <QtCore>
#include <QDir>
#include <QDir>
#include <QRegExp>
#include <QFile>
#include <QDataStream>
#include <QTextStream>
#include <QtAlgorithms>
#include "ColorSeparatorConsolidator.h"
#include "../MultiColorImageStackNode.h"
#include "NeuronMaskEntry.h"
#include "FileTreeSearcher.h"
#include "../../v3d/v3d_core.h"
#include <limits>
#include <fstream>

using namespace std;

QString ColorSeparatorConsolidator::maskFileRegExpString("(\\S+)\\.seg\\_(\\d+)\\.n(\\d+)\\.tif");

ColorSeparatorConsolidator::ColorSeparatorConsolidator()
{
    this->separatorDirectory="";
    this->outputFile="";
    maskFileRegExp.setPattern(ColorSeparatorConsolidator::maskFileRegExpString);
}

bool ColorSeparatorConsolidator::execute() {

    if (!populateMaskFileList()) {
        cleanup();
        return false;
    }
    if (!writeMetadataFile()) {
        cleanup();
        return false;
    }
    if (!generateMask()) {
        cleanup();
        return false;
    }
    if (!writeMask()) {
        cleanup();
        return false;
    }

    return true;
}

void ColorSeparatorConsolidator::cleanup() {
    if (consolidatedMask!=0) {
        delete consolidatedMask;
    }
}

bool ColorSeparatorConsolidator::populateMaskFileList() {
    FileTreeSearcher fileTreeSearcher(maskFileRegExp);
    QDir separatorDir(separatorDirectory);
    if (!separatorDir.exists()) {
        cerr << "ColorSeparatorConsolidator::populateMaskFileList() directory=" << separatorDir.absolutePath().toStdString() << " does not exist" << endl;
        return false;
    }
    maskFileList.append(fileTreeSearcher.findFilesInDirectory(separatorDirectory));
    // We don't want the Max-Projections - just the mask tifs
    for (int i=0; i<maskFileList.size();++i) {
        if (maskFileList.at(i).contains("MIP", Qt::CaseInsensitive)) {
            maskFileList.removeAt(i--);
        }
    }
    if (!sortMaskFileList()) {
        return false;
    }
    return true;
}

bool ColorSeparatorConsolidator::sortMaskFileList() {
    QList<NeuronMaskEntry> entryList;
    for (int i=0;i<maskFileList.size();++i) {
        QString maskFile = maskFileList.at(i);
        int position = maskFileRegExp.indexIn(maskFile);
        if (position < 0) {
            cerr << "ColorSeparatorConsolidator::sortMaskFileList() Unexpectedly could not parse segment and neuron from file=" << maskFile.toStdString() << endl;
            return false;
        }
        QString segmentIndex = maskFileRegExp.cap(2);
        QString neuronIndex = maskFileRegExp.cap(3);
        int bitIndex = 0; // We hard-code this to zero for each entry to sort on the segment and neuron fields
        // Validate against NeuronMaskEntry
        QString maskString = QString("%1 %2 %3 %4").arg(bitIndex).arg(segmentIndex).arg(neuronIndex).arg(maskFile);
        NeuronMaskEntry entry;
        if (!entry.populateFromString(maskString)) {
            cerr << "ColorSeparatorConsolidator::sortMaskFileList() : error , could not create NeuronMaskEntry from string=" << maskString.toStdString() << endl;
            return false;
        }
        entryList.append(entry);
    }
    qSort(entryList.begin(),entryList.end());
    // Transfer
    maskFileList.clear();
    for (int i=0;i<entryList.size();++i) {
        NeuronMaskEntry entry=entryList.at(i);
        maskFileList.append(entry.getFilePath());
    }
    return true;
}

bool ColorSeparatorConsolidator::writeMetadataFile() {
    // We assume the maskFileList has already been sorted
    QString labelIndexFilename(outputFile);
    labelIndexFilename.append(".labelindex");
    QFile file(labelIndexFilename);
    if (!file.open(QIODevice::WriteOnly)) {
        cerr << "Could not open file=" << labelIndexFilename.toStdString() << " to write\n";
        return false;
    }
    QTextStream out(&file);
    QString header("# Label index for color-separated neuron data\n");
    header.append( "# Format: <index> <segment-index> <neuron-index> <source mask tif file>\n");
    out << header;
    for (int i=0; i<maskFileList.size();++i) {
        QString maskFile = maskFileList.at(i);
        int position = maskFileRegExp.indexIn(maskFile);
        if (position < 0) {
            cerr << "ColorSeparatorConsolidator::writeMetadataFile() Unexpectedly could not parse segment and neuron from file=" << maskFile.toStdString() << endl;
            return false;
        }
        QString segmentIndex = maskFileRegExp.cap(2);
        QString neuronIndex = maskFileRegExp.cap(3);
        int bitIndex = i+1; // since 0 connotes no mask
        // Validate against NeuronMaskEntry
        QString maskString = QString("%1 %2 %3 %4").arg(bitIndex).arg(segmentIndex).arg(neuronIndex).arg(maskFile);
        NeuronMaskEntry entry;
        if (!entry.populateFromString(maskString)) {
            cerr << "ColorSeparatorConsolidator::writeMetadataFile() : error , could not create NeuronMaskEntry from string=" << maskString.toStdString() << endl;
            return false;
        }
        out << entry.toString() << endl;
    }
    file.close();
    return true;
}

bool ColorSeparatorConsolidator::generateMask() {
    // Load first mask and use these as the dimensions, throwing an error if these values change
    std::string msgPrefix = "ColorSeparatorConsolidator::generateMask()";
    My4DImage maskImage;
    if (maskFileList.size()>255) {
        cerr << msgPrefix << " error: maskFileList is size=" << maskFileList.size() << " and must be less than 256 for 8-bit mask" << endl;
        return false;
    }
    unsigned char **** consolidatedMaskData;
    for (int i=0;i<maskFileList.size();++i) {
        QString maskFilename = maskFileList.at(i);
        cout << msgPrefix << " loading " << maskFilename.toStdString() << endl;
        // The following loadImage() step clears any previous data
        maskImage.loadImage(maskFilename.toUtf8().data());
        if (maskImage.isEmpty()) {
            cerr << msgPrefix << " error: image loaded to empty state " << endl;
            return false;
        }
        if (i==0) {
            // Initialize
            cout << msgPrefix << " using dimensions X=" << maskImage.getXDim() << " Y=" << maskImage.getYDim() << " Z=" << maskImage.getZDim() << endl;
            consolidatedMask = new My4DImage();
            consolidatedMask->loadImage(maskImage.getXDim(), maskImage.getYDim(), maskImage.getZDim(), 1 /* single 4D slice */, 1 /* 8-bit */);
            maskSizeX=maskImage.getXDim();
            maskSizeY=maskImage.getYDim();
            maskSizeZ=maskImage.getZDim();
            consolidatedMaskData = (unsigned char ****)consolidatedMask->getData();
        }
        // Check dimensional consistency
        if (maskImage.getXDim()!=maskSizeX ||
            maskImage.getYDim()!=maskSizeY ||
            maskImage.getZDim()!=maskSizeZ) {
            cerr << msgPrefix << " dimensions do not match for file=" << maskFilename.toStdString() << " X=" << maskImage.getXDim() <<
                    " Y=" << maskImage.getYDim() << " Z=" << maskImage.getZDim() << endl;
            return false;
        }
        cout << msgPrefix << " starting mask assignment..." << endl;
        unsigned char maskValue = i+1; // assume C++ does a friendly common-sense assignment here
        int x,y,z;
        for (z=0;z<maskSizeZ;z++) {
            for (y=0;y<maskSizeY;y++) {
                for (x=0;x<maskSizeX;x++) {
                    if (maskImage.at(x,y,z,0)!=0.0) {
                        consolidatedMaskData[0][z][y][x]=maskValue;
                    }
                }
            }
        }
        cout << msgPrefix << " done." << endl;
    }
    return true;
}

bool ColorSeparatorConsolidator::writeMask() {
    QString msgPrefix = "ColorSeparatorConsolidator::writeMask()";
    cout << msgPrefix.toStdString() << " start\n";
    if (!MultiColorImageStackNode::writeMy4DImageToMaskFile(consolidatedMask, outputFile)) {
        cerr << msgPrefix.toStdString() << " : error writing to mask file" << endl;
        return false;
    }
    return true;
}

int ColorSeparatorConsolidator::processArgs(vector<char*> *argList) {
    for (int i=0;i<argList->size();i++) {
        QString arg=(*argList)[i];
        if (arg=="-i") {
            i++;
            separatorDirectory = (*argList)[i];
        } else if (arg=="-o") {
            i++;
            outputFile = (*argList)[i];
        }
    }
    if (separatorDirectory=="" || outputFile=="") {
        return 1;
    }
    return 0;
}


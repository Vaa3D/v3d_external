#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include "CommandManager.h"
#include "../neuron_annotator/utility/ColorSeparatorConsolidator.h"
#include "../cell_counter/CellCounter3D.h"
#include "../neuron_annotator/utility/SingleNeuronLsmSetReader.h"
#include "../neuron_annotator/utility/StitchedFileUtility.h"
#include "../neuron_annotator/utility/ImageLoader.h"
#include "../neuron_annotator/analysis/ScreenPatternAnnotator.h"
#include "../neuron_annotator/analysis/NeuronFragmentEditor.h"
#include "../neuron_annotator/analysis/AlignerUtils.h"
#include "../neuron_annotator/analysis/VolumePatternIndex.h"
#include "../neuron_annotator/analysis/VolumeIndex.h"

using namespace std;

CommandManager::CommandManager(vector<char*> *argList)
{
    this->argList=argList;
}

bool CommandManager::execute() {
    if ((*argList).size()==0) {
        // Nothing to do
        cout << getUsageString();
        return false;
    }
    string firstArg=(*argList)[0];
    if (firstArg==ColorSeparatorConsolidator::getCommandLineDescription()) {
        ColorSeparatorConsolidator csc;
        int status=csc.processArgs(argList);
        if (status!=0) {
            cout << getUsageString();
            return false;
        }
        return csc.execute();
    } else if (firstArg==CellCounter3D::getCommandLineDescription()) {
        CellCounter3D cellCounter;
        int status=cellCounter.processArgs(argList);
        if (status==CellCounter3D::ARG_STATUS_USAGE) {
            cout << getUsageString();
            return false;
        } else if (status==CellCounter3D::ARG_STATUS_HELP) {
            cout << CellCounter3D::getHelp();
            return true;
        } else if (status==CellCounter3D::ARG_STATUS_OK) {
            cellCounter.loadInputFile();
            cellCounter.findCells();
            cellCounter.markImage();
            cellCounter.writeOutputImageFile();
            cellCounter.writeOutputReportFile();
            return true;
        }
    } else if (firstArg==SingleNeuronLsmSetReader::getCommandLineDescription()) {
        SingleNeuronLsmSetReader lsmSetReader;
        int status=lsmSetReader.processArgs(argList);
        if (status!=0) {
            cout << getUsageString();
            return false;
        }
        return lsmSetReader.execute();
    } else if (firstArg==StitchedFileUtility::getCommandLineDescription()) {
        StitchedFileUtility stitchedFileUtility;
        int status=stitchedFileUtility.processArgs(argList);
        if (status!=0) {
            cout << getUsageString();
            return false;
        }
        return stitchedFileUtility.execute();
    } else if (firstArg==ImageLoader::getCommandLineDescription()) {
        ImageLoader imageLoader;
        int status=imageLoader.processArgs(argList);
        if (status!=0) {
            cout << getUsageString();
            return false;
        }
        return imageLoader.execute();
    } else if (firstArg==ScreenPatternAnnotator::getCommandLineDescription()) {
        ScreenPatternAnnotator screenPatternAnnotator;
        int status=screenPatternAnnotator.processArgs(argList);
        if (status!=0) {
            cout << getUsageString();
            return false;
        }
        return screenPatternAnnotator.execute();
    } else if (firstArg==NeuronFragmentEditor::getCommandLineDescription()) {
        NeuronFragmentEditor neuronFragmentEditor;
        int status=neuronFragmentEditor.processArgs(argList);
        if (status!=0) {
            cout << getUsageString();
            return false;
        }
        return neuronFragmentEditor.execute();
    } else if (firstArg==AlignerUtils::getCommandLineDescription()) {
        AlignerUtils alignerUtils;
        int status=alignerUtils.processArgs(argList);
        if (status!=0) {
            cout << getUsageString();
            return false;
        }
        return alignerUtils.execute();
    } else if (firstArg==VolumePatternIndex::getCommandLineDescription()) {
        VolumePatternIndex volumePatternIndex;
        int status=volumePatternIndex.processArgs(argList);
        if (status!=0) {
            cout << getUsageString();
            return false;
        }
        return volumePatternIndex.execute();
    } else if (firstArg==VolumeIndex::getCommandLineDescription()) {
        VolumeIndex volumeIndex;
        int status=volumeIndex.processArgs(argList);
        if (status!=0) {
            cout << getUsageString();
            return false;
        }
        return volumeIndex.execute();
    } else {
        cout << getUsageString();
        return false;
    }
    return true;
}

string CommandManager::getUsageString() {
    string usage="The command interface permits v3d to run in production mode, without a viewer or human interface.\n";
    usage.append("To use the command interface, use '-cmd' followed by a command and appropriate options:\n");
    usage.append("\n");
    usage.append("-cmd\n");
    usage.append("\n");
    usage.append(ColorSeparatorConsolidator::getCommandLineDescription());
    usage.append("\n");
    usage.append(ColorSeparatorConsolidator::getUsage());
    usage.append("\n");
    usage.append(CellCounter3D::getCommandLineDescription());
    usage.append("\n");
    usage.append(CellCounter3D::getUsage());
    usage.append("\n");
    usage.append(SingleNeuronLsmSetReader::getCommandLineDescription());
    usage.append("\n");
    usage.append(SingleNeuronLsmSetReader::getUsage());
    usage.append("\n");
    usage.append(StitchedFileUtility::getCommandLineDescription());
    usage.append("\n");
    usage.append(StitchedFileUtility::getUsage());
    usage.append("\n");
    usage.append(ImageLoader::getCommandLineDescription());
    usage.append("\n");
    usage.append(ImageLoader::getUsage());
    usage.append("\n");
    usage.append(ScreenPatternAnnotator::getCommandLineDescription());
    usage.append("\n");
    usage.append(ScreenPatternAnnotator::getUsage());
    usage.append("\n");
    usage.append(NeuronFragmentEditor::getCommandLineDescription());
    usage.append("\n");
    usage.append(NeuronFragmentEditor::getUsage());
    usage.append("\n");
    usage.append(AlignerUtils::getCommandLineDescription());
    usage.append("\n");
    usage.append(AlignerUtils::getUsage());
    usage.append("\n");
    usage.append(VolumePatternIndex::getCommandLineDescription());
    usage.append("\n");
    usage.append(VolumePatternIndex::getUsage());
    usage.append("\n");
    usage.append(VolumeIndex::getCommandLineDescription());
    usage.append("\n");
    usage.append(VolumeIndex::getUsage());
    usage.append("\n");
    return usage;
}

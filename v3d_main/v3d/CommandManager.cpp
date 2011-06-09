#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include "CommandManager.h"
#include "../neuron_annotator/ColorSeparatorConsolidator.h"
#include "../cell_counter/CellCounter3D.h"

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
        if ((*argList).size()<3) {
            cout << getUsageString();
            return false;
        }
        QString separatorDirectory=(*argList)[1];
        QString outputFile=(*argList)[2];
        ColorSeparatorConsolidator csc(separatorDirectory, outputFile);
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
    } else {
        cout << getUsageString();
        return false;
    }
}

string CommandManager::getUsageString() {
    string usage="The command interface permits v3d to run in production mode, without a viewer or human interface.\n";
    usage.append("To use the command interface, use '-cmd' followed by a command and appropriate options:\n");
    usage.append("\n");
    usage.append("-cmd\n");
    usage.append("\n");
    usage.append(ColorSeparatorConsolidator::getCommandLineDescription());
    usage.append("  <separator output directory>  <output mask file>\n");
    usage.append("\n");
    usage.append(CellCounter3D::getCommandLineDescription());
    usage.append("\n");
    usage.append(CellCounter3D::getUsage());
    usage.append("\n");
    return usage;
}

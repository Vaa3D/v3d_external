// test_load_chk4.cpp - Test loading and saving checkpoint files from 
// Gene Myers' neuron separation tool.

#include "../data_model/NeuronSeparationResult.h"
#include <fstream>
#include <string>
#include <stdexcept>
#include <sstream>

using namespace std;
using namespace jfrc;

int main(int argc, const char* argv[])
{
    if (argc < 2) {
        ostringstream msg;
        msg << "program " << argv[0] << " needs one argument";
        throw runtime_error(msg.str().c_str());
    }
    ifstream is(argv[1]);
    if (! is) {
        ostringstream msg("Problem opening file '");
        msg << argv[1] << "'";
        throw runtime_error(msg.str().c_str());
    }
    NeuronSeparationResult neuronSeparationResult;
    neuronSeparationResult.loadChk4File(is);

    ofstream os("test.chk4");
    neuronSeparationResult.saveChk4File(os);

    return 0;
}


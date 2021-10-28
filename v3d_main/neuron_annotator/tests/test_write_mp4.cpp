#include "../utility/loadV3dFFMpeg.h"
#include "../../v3d/v3d_application.h"
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

// Kludge to avoid having to ALSO link in v3d main.cpp.
V3dApplication* V3dApplication::theApp = NULL;

int main(int argc, char **argv)
{
    if (argc < 3) {
        cerr << "Usage: " << argv[0];
        cerr << " " << "<input_volume>";
        cerr << " " << "<output_mp4>";
        cerr << endl;
    }

    string input_file_name(argv[1]);
    string output_file_name(argv[2]);
    My4DImage img;
    loadStackFFMpeg(input_file_name.c_str(), img);
    saveStackFFMpeg(output_file_name.c_str(), img);

    return 0;
}




#include "../utility/loadV3dFFMpeg.h"
#include "../../v3d/v3d_application.h"

using namespace std;

// Kludge to avoid having to ALSO link in v3d main.cpp.
V3dApplication* V3dApplication::theApp = NULL;


int main(int argc, char **argv)
{
    if (argc < 2) {
        cerr << "Video file name not specified" << endl;
        return 1;
    }

    My4DImage *img = new My4DImage();
#ifdef USE_FFMPEG
    loadRaw2StackFFMpeg(argv[1], img);
#endif

    return 0;
}


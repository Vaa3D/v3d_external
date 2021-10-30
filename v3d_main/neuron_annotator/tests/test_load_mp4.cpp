
#ifdef __APPLE__
extern "C" {
    extern int posix_memalign();
}
#endif

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

    My4DImage img;
#ifdef USE_FFMPEG
    loadStackFFMpeg(argv[1], img);
#endif

    return 0;
}


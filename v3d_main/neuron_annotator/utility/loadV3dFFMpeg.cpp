#include "loadV3dFFMpeg.h"
#include "FFMpegVideo.h"
#include <iostream>

#ifdef V3D_ENABLE_LOAD_MOVIE

using namespace std;

bool loadRaw2StackFFMpeg(const char* fileName, Image4DSimple* img)
{
    try {
        FFMpegVideo video(fileName);
        int sx = video.getWidth();
        int sy = video.getHeight();
        int sz = video.getNumberOfFrames();
        int sc = video.getNumberOfChannels();
        cout << "Number of frames = " << sz << endl;

        img->createBlankImage(sx, sy, sz, sc, 1); // 1 byte = 8 bits per value
        Image4DProxy<Image4DSimple> proxy(img);

        int frameCount = 0;
        while (video.readNextFrame())
        {
            int z = frameCount;
            frameCount++;
            for(int c = 0; c < sc; ++c) {
                for (int y = 0; y < sy; ++y) {
                    for (int x = 0; x < sx; ++x) {
                        proxy.put_at(x, y, z, c,
                                     video.getPixelIntensity(x, y, (FFMpegVideo::Channel)c)
                                     );
                    }
                }
            }
        }
        cout << "Number of frames found = " << frameCount << endl;

        return true;

    } catch(...) {}

    return false;
}

#endif // V3D_ENABLE_LOAD_MOVIE



#include "loadV3dFFMpeg.h"
#include "FFMpegVideo.h"
#include <iostream>

#ifdef USE_FFMPEG

using namespace std;

bool saveStackFFMpeg(std::ostream& os, const Image4DSimple& img)
{
    try {
        Image4DProxy<Image4DSimple> proxy(const_cast<Image4DSimple*>(&img));
        FFMpegEncoder encoder(os, proxy.sx, proxy.sy);
        for (int z = 0; z < proxy.sz; ++z) {
            for (int y = 0; y < proxy.sy; ++y) {
                for (int x = 0; x < proxy.sx; ++x)
                {
                    double red   = proxy.value_at(x, y, z, 0);
                    double green = proxy.value_at(x, y, z, 1);
                    double blue  = proxy.value_at(x, y, z, 2);
                    // TODO write value
                }
            }
            encoder.write_frame();
        }
        encoder.write_delayed_frames();
        return true;
    } catch (...) {}

    return false;
}

bool loadStackFFMpeg(const char* fileName, Image4DSimple& img)
{
    try {
        FFMpegVideo video(fileName);
        int sx = video.getWidth();
        int sy = video.getHeight();
        int sz = video.getNumberOfFrames();
        int sc = video.getNumberOfChannels();
        cout << "Number of frames = " << sz << endl;

        img.createBlankImage(sx, sy, sz, sc, 1); // 1 byte = 8 bits per value
        Image4DProxy<Image4DSimple> proxy(&img);

        int frameCount = 0;
        for (int z = 0; z < sz; ++z)
        {
            video.fetchFrame(z);
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

#endif // USE_FFMPEG



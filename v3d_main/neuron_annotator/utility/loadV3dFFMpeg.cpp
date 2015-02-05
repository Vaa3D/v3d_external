#include "loadV3dFFMpeg.h"
#include "FFMpegVideo.h"
#include <iostream>

#ifdef USE_FFMPEG

using namespace std;

bool saveStackFFMpeg(const char * file_name, const My4DImage& img, enum AVCodecID codec_id)
{
    try {
        Image4DProxy<My4DImage> proxy(const_cast<My4DImage*>(&img));
        double default_irange = 1.0; // assumes data range is 0-255.0
        if (proxy.su > 1) {
            default_irange = 1.0 / 16.0; // 0-4096, like our microscope images
        }
        std::vector<double> imin(proxy.sc, 0.0);
        std::vector<double> irange2(proxy.sc, default_irange);
        // rescale if converting from 16 bit to 8 bit
        if (proxy.su > 1) {
            if (img.p_vmin && img.p_vmax)
                proxy.set_minmax(img.p_vmin, img.p_vmax);
            if (proxy.has_minmax()) {
                for (int c = 0; c < proxy.sc; ++c) {
                    imin[c] = proxy.vmin[c];
                    irange2[c] = 255.0 / (proxy.vmax[c] - proxy.vmin[c]);
                }
            }
        }
        FFMpegEncoder encoder(file_name, proxy.sx, proxy.sy, codec_id);
        for (int z = 0; z < proxy.sz; ++z) {
            for (int y = 0; y < proxy.sy; ++y) {
                for (int x = 0; x < proxy.sx; ++x) {
                    for (int c = 0; c < 3; ++c) {
                        int ic = c;
                        if (c >= proxy.sc) ic = 0; // single channel volume to gray RGB movie
                        double val = proxy.value_at(x, y, z, ic);
                        val = (val - imin[ic]) * irange2[ic]; // rescale to range 0-255
                        encoder.setPixelIntensity(x, y, c, (int)val);
                    }
                }
            }
            encoder.write_frame();
        }
        return true;
    } catch (...) {}

    return false;
}

bool loadStackFFMpeg(const char* fileName, Image4DSimple& img)
{
    return loadStackFFMpeg(QUrl::fromLocalFile(fileName), img);
}

bool loadStackFFMpeg(QUrl url, Image4DSimple& img)
{
    try {
        FFMpegVideo video(url);
        if (! video.isOpen)
            return false;
        int sx = video.getWidth();
        int sy = video.getHeight();
        int sz = video.getNumberOfFrames();
        int sc = video.getNumberOfChannels();
        // cout << "Number of frames = " << sz << endl;

        img.createBlankImage(sx, sy, sz, sc, 1); // 1 byte = 8 bits per value
        Image4DProxy<Image4DSimple> proxy(&img);

        int frameCount = 0;
        for (int z = 0; z < sz; ++z)
        {
            video.fetchFrame(z);
            // int z = frameCount;
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

bool loadStackFFMpegAsGray(const char* fileName, Image4DSimple& img)
{
    return loadStackFFMpegAsGray(QUrl::fromLocalFile(fileName), img);
}

bool loadStackFFMpegAsGray(QUrl url, Image4DSimple& img)
{
    try {
        FFMpegVideo video(url);
        int sx = video.getWidth();
        int sy = video.getHeight();
        int sz = video.getNumberOfFrames();
        int sc = video.getNumberOfChannels();
        // cout << "Number of frames = " << sz << endl;

        img.createBlankImage(sx, sy, sz, 1, 1); // 1 byte = 8 bits per value
        Image4DProxy<Image4DSimple> proxy(&img);

        int frameCount = 0;
        for (int z = 0; z < sz; ++z)
        {
            video.fetchFrame(z);
            // int z = frameCount;
            frameCount++;
            for (int y = 0; y < sy; ++y) {
                for (int x = 0; x < sx; ++x) {
                    // Use average of R,G,B as gray value
                    int val = 0;
                    for(int c = 0; c < sc; ++c) {
                        val += video.getPixelIntensity(x, y, (FFMpegVideo::Channel)c);
                    }
                    val /= sc; // average of rgb
                    proxy.put_at(x, y, z, 0, val);
                }
            }
        }
        // cout << "Number of frames found = " << frameCount << endl;

        return true;

    } catch(...) {}

    return false;
}

#endif // USE_FFMPEG



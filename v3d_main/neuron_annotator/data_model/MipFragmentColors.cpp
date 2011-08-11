#include "MipFragmentColors.h"
#include <cassert>

/* explicit */
MipFragmentColors::MipFragmentColors(const MipFragmentData& mipFragmentDataParam,
                  const DataColorModel& colorModelParam)
    : mipFragmentData(mipFragmentDataParam)
    , dataColorModel(colorModelParam)
{
    connect(&mipFragmentData, SIGNAL(dataChanged()),
            this, SLOT(update()));
    connect(&dataColorModel, SIGNAL(dataChanged()),
            this, SLOT(update()));
}

/* slot */
void MipFragmentColors::update()
{
    QTime stopwatch;
    stopwatch.start();

    MipFragmentData::Reader mipReader(mipFragmentData); // readlock one of two
    if (! mipReader.hasReadLock()) return;
    if (mipReader.getNumberOfDataChannels() == 0) return; // what's the use?
    DataColorModel::Reader colorReader(dataColorModel); // readlock two of two
    if (! colorReader.hasReadLock()) return;
    if (! (colorReader.getNumberOfDataChannels() == mipReader.getNumberOfDataChannels()) )
    {
        qDebug() << "Error: color model does not match data model";
        return;
    }

    const Image4DProxy<My4DImage> mipProxy = mipReader.getMipProxy();
    const Image4DProxy<My4DImage> intensityProxy = mipReader.getIntensityProxy();
    int refIndex = intensityProxy.sz - 1; // relative to fragment indices

    Writer writer(*this); // acquire write lock

    int sx = mipProxy.sx; // image width
    int sy = mipProxy.sy; // image height
    int sf = intensityProxy.sz; // number of mips
    // Do we need to change the size of our images?
    if (fragmentMips.size() == 0)
        writer.allocateImages(sx, sy, sf);
    else if (fragmentMips.size() != sf)
        writer.allocateImages(sx, sy, sf);
    else if (fragmentMips[0]->width() != sx)
        writer.allocateImages(sx, sy, sf);
    else if (fragmentMips[0]->height() != sy)
        writer.allocateImages(sx, sy, sf);

    if (! mipReader.refreshLock()) return;
    if (! colorReader.refreshLock()) return;

    double *intensities = new double [mipProxy.sc + 1];
    int refChannel = mipProxy.sc;
    intensities[refChannel] = 0.0; // turn off reference channel for fragment colors
    // Background and neuron/fragment images
    for (int f = 0; f < mipProxy.sz; ++f) {
        QImage * img = fragmentMips[f];
        for (int y = 0; y < mipProxy.sy; ++y) {
            for (int x = 0; x < mipProxy.sx; ++x) {
                for (int c = 0; c < mipProxy.sc; ++c) {
                    intensities[c] = mipProxy.value_at(x, y, f, c);
                }
                // blend intensities using color model
                QRgb color = colorReader.blend(intensities);
                img->setPixel(x, y, color);
            }
        }
        if (! mipReader.refreshLock()) return;
        if (! colorReader.refreshLock()) return;
    }
    // Reference image
    QImage * img = fragmentMips[refIndex];
    for (int c = 0; c < refChannel; ++c) intensities[c] = 0.0; // clear non-reference intensities
    for (int y = 0; y < intensityProxy.sy; ++y) {
        for (int x = 0; x < intensityProxy.sx; ++x) {
            intensities[refChannel] = intensityProxy.value_at(x, y, refIndex, 0);
            // blend intensities using color model
            QRgb color = colorReader.blend(intensities);
            img->setPixel(x, y, color);
        }
    }

    writer.unlock();
    mipReader.unlock();
    colorReader.unlock();

    qDebug() << "Colorizing fragment MIPs took " << stopwatch.elapsed() / 1000.0 << " seconds";
    size_t data_size = 4 * sx * sy * sf;
    qDebug() << "Colorized fragment MIPs use " << data_size / 1e6 << " MB of RAM";

	if (intensities) {delete []intensities; intensities=0;}
    emit dataChanged();
}


///////////////////////////////////////
// MipFragmentColors::Writer methods //
///////////////////////////////////////

void MipFragmentColors::Writer::allocateImages(int x, int y, int nFrags)
{
    // qDebug() << "Allocating mip images";
    while (! mipFragmentColors.fragmentMips.isEmpty())
        delete mipFragmentColors.fragmentMips.takeFirst();
    for (int f = 0; f < nFrags; ++f) {
        mipFragmentColors.fragmentMips << new QImage(x, y, QImage::Format_ARGB32);
    }
}


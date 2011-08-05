#include "GalleryMipImages.h"
#include "../gui/GalleryButton.h"

/* explicit */
GalleryMipImages::GalleryMipImages(const MipFragmentColors& mipFragmentColorsParam,
                                   QObject *parentParam /* = 0 */)
    : NaLockableData(parentParam)
    , mipFragmentColors(mipFragmentColorsParam)
{
    connect(&mipFragmentColorsParam, SIGNAL(dataChanged()),
            this, SLOT(update()));
}

void GalleryMipImages::update()
{
    QTime stopwatch;
    stopwatch.start();

    MipFragmentColors::Reader mipReader(mipFragmentColors); // acquire read lock
    if (! mipReader.hasReadLock()) return;
    Writer mipWriter(*this); // acquire write lock

    int height = GalleryButton::ThumbnailPixelHeight;
    int nFrags = mipReader.getNumImages() - 2;
    if (nFrags < 0) return; // not enough upstream data
    if (overlayMipList.size() != 2) // nothing allocated yet
        mipWriter.allocateImages(nFrags);
    else if (neuronMipList.size() != nFrags) // number of fragments has changed
        mipWriter.allocateImages(nFrags);
    // background
    *(overlayMipList[0]) = mipReader.getImage(0)->scaledToHeight(height);
    // reference
    *(overlayMipList[1]) = mipReader.getImage(nFrags + 1)->scaledToHeight(height);
    // fragments
    for (int f = 1; f <= nFrags; ++f)
        *(neuronMipList)[f - 1] = mipReader.getImage(f)->scaledToHeight(height);

    // nerd report
    qDebug() << "Rescaling gallery mips took " << stopwatch.elapsed() / 1000.0 << " seconds.";

    mipReader.unlock(); // unlock before emit
    mipWriter.unlock();
    emit dataChanged();
}





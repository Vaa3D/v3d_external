#include "GalleryMipImages.h"
#include "../gui/GalleryButton.h"


//////////////////////////////
// GalleryMipImages methods //
//////////////////////////////

/* explicit */
GalleryMipImages::GalleryMipImages(const MipFragmentColors& mipFragmentColorsParam,
                                   QObject *parentParam /* = 0 */)
    : NaLockableData(parentParam)
    , mipFragmentColors(mipFragmentColorsParam)
{
    connect(&mipFragmentColorsParam, SIGNAL(dataChanged()),
            this, SLOT(update()));
}

GalleryMipImages::~GalleryMipImages()
{
    Writer writer(*this); // wait for read clients to return locks
    writer.deleteImages();
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
    *(overlayMipList[AnnotationSession::BACKGROUND_MIP_INDEX]) = mipReader.getImage(0)->scaledToHeight(height, Qt::SmoothTransformation);
    // reference
    *(overlayMipList[AnnotationSession::REFERENCE_MIP_INDEX]) = mipReader.getImage(nFrags + 1)->scaledToHeight(height, Qt::SmoothTransformation);
    // fragments
    for (int f = 1; f <= nFrags; ++f)
        *(neuronMipList)[f - 1] = mipReader.getImage(f)->scaledToHeight(height);

    // nerd report
    qDebug() << "Rescaling gallery mips took " << stopwatch.elapsed() / 1000.0 << " seconds.";

    mipReader.unlock(); // unlock before emit
    mipWriter.unlock();
    emit dataChanged();
}


//////////////////////////////////////
// GalleryMipImages::Writer methods //
//////////////////////////////////////

void GalleryMipImages::Writer::deleteImages()
{
    while (! galleryMipImages.neuronMipList.isEmpty())
        delete galleryMipImages.neuronMipList.takeFirst();
    while (! galleryMipImages.overlayMipList.isEmpty())
        delete galleryMipImages.overlayMipList.takeFirst();
}

void GalleryMipImages::Writer::allocateImages(int nFrags)
{
    qDebug() << "Allocating scaled mip images";
    deleteImages();
    // Fragments/neurons
    for (int f = 0; f < nFrags; ++f)
        galleryMipImages.neuronMipList << new QImage(1, 1, QImage::Format_ARGB32);
    // Overlays/background/reference
    for (int f = 0; f < 2; ++f) {
        galleryMipImages.overlayMipList << new QImage(1, 1, QImage::Format_ARGB32);
    }
}








#ifndef GALLERYMIPIMAGES_H
#define GALLERYMIPIMAGES_H

#include "NaLockableData.h"
#include "MipFragmentColors.h"

// GalleryMipImages creates scaled versions of individual fragment QImages,
// so the gui thread does not need to perform the scaling.  All the gui
// thread needs to do is call QPixmap::fromImage() for each GalleryButton.
class GalleryMipImages : public NaLockableData
{
    Q_OBJECT
public:
    explicit GalleryMipImages(const MipFragmentColors& mipFragmentColorsParam,
                              QObject *parent = 0);

public slots:
    void update();

protected:
    // input
    const MipFragmentColors& mipFragmentColors; // TODO - maybe feed directly from MipFragmentData, or even a scaled MipFragmentData
    // output
    // Use same structure as was found in AnnotationSession, for ease of refactoring.
    QList<QImage*> neuronMipList;
    QList<QImage*> overlayMipList;


    class Reader : public BaseReadLocker
    {
    public:
        Reader(const GalleryMipImages& galleryMipImagesParam)
            : BaseReadLocker(galleryMipImagesParam.getLock())
            , galleryMipImages(galleryMipImagesParam)
        {}

        const QImage* getNeuronMip(int index) {return galleryMipImages.neuronMipList[index];}
        const QImage* getOverlayMip(int index) {return galleryMipImages.overlayMipList[index];}

    protected:
        const GalleryMipImages& galleryMipImages;
    };


    class Writer : public QWriteLocker
    {
    public:

        Writer(GalleryMipImages& galleryMipImagesParam)
            : QWriteLocker(galleryMipImagesParam.getLock())
            , galleryMipImages(galleryMipImagesParam)
        {}

        void allocateImages(int nFrags) {
            qDebug() << "Allocating scaled mip images";
            // Fragments/neurons
            while (! galleryMipImages.neuronMipList.isEmpty())
                delete galleryMipImages.neuronMipList.takeFirst();
            for (int f = 0; f < nFrags; ++f)
                galleryMipImages.neuronMipList << new QImage(1, 1, QImage::Format_ARGB32);
            // Overlays/background/reference
            while (! galleryMipImages.overlayMipList.isEmpty())
                delete galleryMipImages.overlayMipList.takeFirst();
            for (int f = 0; f < 2; ++f) {
                galleryMipImages.overlayMipList << new QImage(1, 1, QImage::Format_ARGB32);
            }
        }

    protected:
        GalleryMipImages& galleryMipImages;
    };
};

#endif // GALLERYMIPIMAGES_H

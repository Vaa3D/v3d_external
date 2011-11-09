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
    explicit GalleryMipImages(const MipFragmentColors& mipFragmentColorsParam);
    ~GalleryMipImages();

public slots:
    void update();

protected:
    // input
    const MipFragmentColors& mipFragmentColors; // TODO - maybe feed directly from MipFragmentData, or even a scaled MipFragmentData
    // output
    // Use same structure as was found in AnnotationSession, for ease of refactoring.
    QList<QImage*> neuronMipList;
    QList<QImage*> overlayMipList;


public:

    class Reader : public BaseReadLocker
    {
    public:
        Reader(const GalleryMipImages& galleryMipImagesParam)
            : BaseReadLocker(galleryMipImagesParam)
            , galleryMipImages(galleryMipImagesParam)
        {}

        const QImage* getNeuronMip(int index) const {return galleryMipImages.neuronMipList[index];}
        const QImage* getOverlayMip(int index) const {return galleryMipImages.overlayMipList[index];}
        size_t getNumberOfNeurons() const {return galleryMipImages.neuronMipList.size();}
        size_t getNumberOfOverlays() const {return galleryMipImages.overlayMipList.size();}

    protected:
        const GalleryMipImages& galleryMipImages;
    };


    class Writer : public BaseWriteLocker
    {
    public:
        Writer(GalleryMipImages& galleryMipImagesParam)
            : BaseWriteLocker(galleryMipImagesParam)
            , galleryMipImages(galleryMipImagesParam)
        {}

        QImage* getNeuronMip(int index) {return galleryMipImages.neuronMipList[index];}
        QImage* getOverlayMip(int index) {return galleryMipImages.overlayMipList[index];}
        size_t getNumberOfNeurons() const {return galleryMipImages.neuronMipList.size();}
        size_t getNumberOfOverlays() const {return galleryMipImages.overlayMipList.size();}

        void allocateImages(int nFrags);
        void deleteImages();

    protected:
        GalleryMipImages& galleryMipImages;
    };
};

#endif // GALLERYMIPIMAGES_H

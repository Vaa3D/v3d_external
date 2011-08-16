#ifndef PRIVATEVOLUMEMODELDATA_H
#define PRIVATEVOLUMEMODELDATA_H

#include <QSharedData>
#include <QDir>
#include "../v3d/v3d_core.h"

class PrivateVolumeModelData : public QSharedData
{
public:
    PrivateVolumeModelData();
    PrivateVolumeModelData(const PrivateVolumeModelData& other);
    virtual ~PrivateVolumeModelData();

    // read-only methods
    const Image4DProxy<My4DImage>& getNeuronMaskProxy() const;
    const Image4DProxy<My4DImage>& getOriginalImageProxy() const;
    const Image4DProxy<My4DImage>& getReferenceImageProxy() const;
    ImagePixelType getOriginalDatatype() const;
    int getNumberOfNeurons() const;

    // modifying methods
    void loadFromDirectory(QDir directory);
    void clearImageData();
    Image4DProxy<My4DImage>& updNeuronMaskProxy();
    Image4DProxy<My4DImage>& updOriginalImageProxy();
    Image4DProxy<My4DImage>& updReferenceImageProxy();

private:
    static My4DImage* copyMy4DImage(My4DImage* dest, const My4DImage* src);

    My4DImage* originalImageStack;
    My4DImage* neuronMaskStack;
    My4DImage* referenceStack;
    My4DImage* emptyImage;
    Image4DProxy<My4DImage> originalImageProxy;
    Image4DProxy<My4DImage> neuronMaskProxy;
    Image4DProxy<My4DImage> referenceImageProxy;
};

#endif // PRIVATEVOLUMEMODELDATA_H

#include "PrivateVolumeModelData.h"
#include <cassert>

PrivateVolumeModelData::PrivateVolumeModelData()
    : originalImageStack(NULL)
    , neuronMaskStack(NULL)
    , referenceStack(NULL)
    , emptyImage(new My4DImage())
    , originalImageProxy(emptyImage)
    , neuronMaskProxy(emptyImage)
    , referenceImageProxy(emptyImage)
{}

/* static */
My4DImage* PrivateVolumeModelData::copyMy4DImage(My4DImage* dest, const My4DImage* src)
{
    My4DImage* result = dest;
    // Don't reallocate unless the data size is wrong
    if (   (result == NULL)
        || (result->getXDim() != src->getXDim())
        || (result->getYDim() != src->getYDim())
        || (result->getZDim() != src->getZDim())
        || (result->getCDim() != src->getCDim())
        || (result->getDatatype() != src->getDatatype())
        || (result->getTotalBytes() != src->getTotalBytes()) )
    {
        // Reallocate memory
        if (result != NULL)
            delete result;
        result = new My4DImage();
        result->loadImage(
                src->getXDim(),
                src->getYDim(),
                src->getZDim(),
                src->getCDim(),
                src->getDatatype());
    }
    // Copy volume data
    assert(result->getTotalBytes() == src->getTotalBytes());
    memcpy(result->getData(),
           const_cast<My4DImage*>(src)->getData(),
           src->getTotalBytes());
    // copy min/max values
    if (src->p_vmin) {
        if (! result->p_vmin) {
            result->p_vmin = new double(src->getCDim());
            result->p_vmax = new double(src->getCDim());
        }
        for (int c = 0; c < src->getCDim(); ++c) {
            result->p_vmin[c] = src->p_vmin[c];
            result->p_vmax[c] = src->p_vmax[c];
        }
    }
    return result;
}

PrivateVolumeModelData::PrivateVolumeModelData(const PrivateVolumeModelData& other)
    : originalImageStack(NULL)
    , neuronMaskStack(NULL)
    , referenceStack(NULL)
    , emptyImage(new My4DImage())
    , originalImageProxy(emptyImage)
    , neuronMaskProxy(emptyImage)
    , referenceImageProxy(emptyImage)
{
    qDebug() << "copying PrivateVolumeModelData";
    // Drat! My4DImage class has no copy constructor built in.
    originalImageStack = copyMy4DImage(originalImageStack, other.originalImageStack);
    originalImageProxy = Image4DProxy<My4DImage>(originalImageStack);
    originalImageProxy.set_minmax(originalImageStack->p_vmin, originalImageStack->p_vmax);

    neuronMaskStack = copyMy4DImage(neuronMaskStack, other.neuronMaskStack);
    neuronMaskProxy = Image4DProxy<My4DImage>(neuronMaskStack);
    neuronMaskProxy.set_minmax(neuronMaskStack->p_vmin, neuronMaskStack->p_vmax);

    referenceStack = copyMy4DImage(referenceStack, other.referenceStack);
    referenceImageProxy = Image4DProxy<My4DImage>(referenceStack);
    referenceImageProxy.set_minmax(referenceStack->p_vmin, referenceStack->p_vmax);
}

/* virtual */
PrivateVolumeModelData::~PrivateVolumeModelData()
{
    clearImageData();
}

// read-only methods
const Image4DProxy<My4DImage>& PrivateVolumeModelData::getNeuronMaskProxy() const
{
    return neuronMaskProxy;
}

const Image4DProxy<My4DImage>& PrivateVolumeModelData::getOriginalImageProxy() const
{
    return originalImageProxy;
}

const Image4DProxy<My4DImage>& PrivateVolumeModelData::getReferenceImageProxy() const
{
    return referenceImageProxy;
}

ImagePixelType PrivateVolumeModelData::getOriginalDatatype() const
{
    return originalImageStack->getDatatype();
}

int PrivateVolumeModelData::getNumberOfNeurons() const
{
    return neuronMaskStack->getChannalMaxIntensity(0);
}

// modifying methods
void PrivateVolumeModelData::loadFromDirectory(QDir directory)
{
    qDebug() << "ERROR: Don't know how to load from directory yet";
    // TODO
}

void PrivateVolumeModelData::clearImageData()
{
    if (originalImageStack != NULL) {
        delete originalImageStack;
        originalImageStack = NULL;
    }
    if (neuronMaskStack != NULL) {
        delete neuronMaskStack;
        neuronMaskStack = NULL;
    }
    if (referenceStack != NULL) {
        delete referenceStack;
        referenceStack = NULL;
    }
    if (emptyImage != NULL) {
        delete emptyImage;
        emptyImage = NULL;
    }
}

Image4DProxy<My4DImage>& PrivateVolumeModelData::updNeuronMaskProxy()
{
    return neuronMaskProxy;
}

Image4DProxy<My4DImage>& PrivateVolumeModelData::updOriginalImageProxy()
{
    return originalImageProxy;
}

Image4DProxy<My4DImage>& PrivateVolumeModelData::updReferenceImageProxy()
{
    return referenceImageProxy;
}



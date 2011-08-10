#include "MipMergedData.h"
#include <cassert>

///////////////////////////
// MipMergeLayer methods //
///////////////////////////

void MipMergeLayer::setVisiblity(bool bIsVisibleParam)
{}

void MipMergeLayer::update() // emits dataChanged() if data changes.
{
    if (updateWithoutSignal())
        emit dataChanged();
}

bool MipMergeLayer::copyChildState(MipMergeLayer * child)
{
    bool answer = false; // no change yet
    if (child->getIndex() != getIndex()) {
        index = child->getIndex();
        answer = true;
    }
    if (child->getProxyIndex() != getProxyIndex()) {
        proxyIndex = child->getProxyIndex();
        answer = true;
    }
    if (child->isVisible() != isVisible()) {
        setVisibility(child->isVisible());
        answer = true;
    }
    return answer;
}

bool MipMergeLayer::updateWithoutSignal() // for manual full updates
{
    bool answer = false; // no change yet
    // Leaf nodes cannot be updated
    if ( (child1 == NULL) && (child2 == NULL) )
        return answer; // not updated; this is a leaf
    else if (child2 == NULL)
    {
        // Just one child?  No extra computation needed.
        if (copyChildState(child1))
            answer = true;
    }
    else {
        // Two child nodes to combine
        if (! (child1->isVisible() || child2->isVisible()) ) {
            // Both child nodes invisible? So is this combined one.  Only compute isVisible flag.
            if (isVisible()) {
                bIsVisible = false;
                answer = true; // changed
            }
        }
        else if (child1->isVisible() && child2->isVisible()) {
            // Both are visible, so combine them.  This is the slow part
            int destinationSlice = proxyIndex = index; // Destination slice.  This node is a primary data source.
            int sourceSlice1 = child1->getProxyIndex(); // First source slice.
            int sourceSlice2 = child2->getProxyIndex(); // Second source slice.

            Image4DProxy<My4DImage>& layerProxy =  mipMergedData.layerZProxy;
            Image4DProxy<My4DImage>& intensityProxy = mipMergedData.layerIntensityProxy;
            Image4DProxy<My4DImage>& dataProxy = mipMergedData.layerDataProxy;

            int sourceSlice;
            int intensity;
            for (int y = 0; y < layerProxy.sy; ++y)
                for (int x = 0; x < layerProxy.sx; ++x)
                {
                    // Choose maximum intensity between two child voxels...
                    double i1 = intensityProxy.value_at(x, y, sourceSlice1, 0);
                    double i2 = intensityProxy.value_at(x, y, sourceSlice2, 0);
                    if (i1 >= i2) {
                        sourceSlice = sourceSlice1;
                        intensity = i1;
                    }
                    else {
                        sourceSlice = sourceSlice2;
                        intensity = i2;
                    }
                    // ...and place that maximum intensity value into current node.
                    layerProxy.put_at(x, y, destinationSlice, 0,
                                      layerProxy.value_at(x, y, sourceSlice, 0));
                    intensityProxy.put_at(x, y, destinationSlice, 0, intensity);
                    for (int c = 0; c < dataProxy.sc; ++c)
                        dataProxy.put_at(x, y, destinationSlice, c,
                                         dataProxy.value_at(x, y, sourceSlice, c));
                }
            // qDebug() << "Merging mip layers" << destinationSlice << sourceSlice1 << sourceSlice2;
            bIsVisible = true;
            answer = true;
            // TODO
        }
        else {
            // Only one visible child.  Copy its state into this node.
            MipMergeLayer * child = child1;
            if (child2->isVisible()) child = child2;
            copyChildState(child);
        }
    }
    return answer;
}


///////////////////////////
// MipMergedData methods //
///////////////////////////

/* explicit */
MipMergedData::MipMergedData(
                const NaVolumeData& volumeDataParam,
                const MipFragmentData& mipFragmentDataParam,
                const DataColorModel& dataColorModelParam,
                const NeuronSelectionModel& neuronSelectionModelParam)
        : volumeData(volumeDataParam)
        , mipFragmentData(mipFragmentDataParam)
        , dataColorModel(dataColorModelParam)
        , neuronSelectionModel(neuronSelectionModelParam)
        , mergedImage(NULL)
        , layerZValues(NULL)
        , layerIntensities(NULL)
        , layerData(NULL)
        , layerZProxy(new My4DImage()) // TODO - leaks a bit of memory
        , layerIntensityProxy(new My4DImage()) // TODO - leaks a bit of memory
        , layerDataProxy(new My4DImage())
{
    connect(&mipFragmentData, SIGNAL(dataChanged()),
            this, SLOT(update()));
    connect(&dataColorModel, SIGNAL(dataChanged()),
            this, SLOT(updateColors()));
    connect(&neuronSelectionModel, SIGNAL(neuronVisibilityChanged(int,bool)),
            this, SLOT(toggleNeuronVisibility(int,bool)));
    connect(&neuronSelectionModel, SIGNAL(overlayVisibilityChanged(int,bool)),
            this, SLOT(toggleOverlayVisibility(int,bool)));
    connect(&neuronSelectionModel, SIGNAL(multipleVisibilityChanged()),
            this, SLOT(updateNeuronVisibility()));
}

/* virtual */
MipMergedData::~MipMergedData()
{
    Writer writer(*this);
    writer.clearData();
}

/* virtual */
void MipMergedData::update()
{

    QTime stopwatch;
    stopwatch.start();
    {
        // copy fragment z values from MipFragmentData
        MipFragmentData::Reader mipFragmentReader(mipFragmentData);
        if (! mipFragmentReader.hasReadLock()) return;
        const Image4DProxy<My4DImage> fragmentZProxy = mipFragmentReader.getZBufferProxy();
        const Image4DProxy<My4DImage> fragmentIntensityProxy = mipFragmentReader.getIntensityProxy();
        const Image4DProxy<My4DImage> fragmentDataProxy = mipFragmentReader.getMipProxy();

        Writer mipMergedWriter(*this);
        mipMergedWriter.clearData();

        layerZValues = new My4DImage();
        layerZValues->loadImage(
                fragmentZProxy.sx,
                fragmentZProxy.sy,
                // binary tree takes up to 2 times (nFrags + background) + reference
                2 * fragmentZProxy.sz,
                1, // only one channel, containing z values
                V3D_UINT16 );
        // clear each byte to xFF, should result in -1?
        memset(layerZValues->getRawData(), 255, layerZValues->getTotalBytes());
        layerZProxy = Image4DProxy<My4DImage>(layerZValues);

        layerIntensities = new My4DImage();
        layerIntensities->loadImage(
                fragmentZProxy.sx,
                fragmentZProxy.sy,
                // binary tree takes up to 2 times (nFrags + background) + reference
                2 * fragmentZProxy.sz,
                1, // only one channel, containing z values
                V3D_UINT16 );
        memset(layerIntensities->getRawData(), 0, layerIntensities->getTotalBytes());
        layerIntensityProxy = Image4DProxy<My4DImage>(layerIntensities);

        layerData = new My4DImage();
        layerData->loadImage(
                fragmentZProxy.sx,
                fragmentZProxy.sy,
                // binary tree takes up to 2 times (nFrags + background) + reference
                2 * fragmentZProxy.sz,
                fragmentDataProxy.sc,
                mipFragmentReader.getOriginalDatatype());
        memset(layerData->getRawData(), 0, layerData->getTotalBytes());
        layerDataProxy = Image4DProxy<My4DImage>(layerData);

        // Leaf level of binary tree contains individual fragment mips (includes background but not reference)
        QList<MipMergeLayer*> levelLayers;
        int levelSize = fragmentZProxy.sz - 1; // -1 => all but reference
        for (int z = 0; z < levelSize - 1; ++z) {
            for (int y = 0; y < fragmentZProxy.sy; ++y)
                for (int x = 0; x < fragmentZProxy.sx; ++x)
                {
                    layerZProxy.put_at(x, y, z, 0, fragmentZProxy.value_at(x, y, z, 0));
                    layerIntensityProxy.put_at(x, y, z, 0, fragmentIntensityProxy.value_at(x, y, z, 0));
                    for (int c = 0; c < fragmentDataProxy.sc; ++c)
                        layerDataProxy.put_at(x, y, z, c,
                                              fragmentDataProxy.value_at(x, y, z, c));
                }
            MipMergeLayer * newLayer = new MipMergeLayer(*this, z, true, NULL, NULL);
            layers << newLayer; // store permanently
            levelLayers << newLayer; // store for next level build up
        }
        // Build up rest of binary tree, rooted at final merged fragment mip
        int nextSliceIndex = levelSize;
        while (levelLayers.size() > 1) {
            QList<MipMergeLayer*> previousLevelLayers = levelLayers;
            levelLayers.clear();
            while (! previousLevelLayers.isEmpty())
            {
                // binary tree joins two child nodes from previous (leafier) level
                MipMergeLayer * child1 = previousLevelLayers.takeAt(0);
                int zIndex = child1->getIndex(); // in case there is no child2, the new layer shares an slice with its child.
                MipMergeLayer * child2 = NULL;
                if (! previousLevelLayers.isEmpty())
                {
                    child2 = previousLevelLayers.takeAt(0);
                    zIndex = nextSliceIndex;
                    assert(zIndex < layerZProxy.sz);
                    nextSliceIndex++;
                }
                MipMergeLayer * newLayer = new MipMergeLayer(*this, zIndex, true, child1, child2);
                layers << newLayer;
                levelLayers << newLayer;
            }
        }
        mergedIndex = nextSliceIndex - 1;
        recomputeLayerTree();
    } // release locks
    qDebug() << "Setting up MipMergedData structure took " << stopwatch.elapsed() / 1000.0 << " seconds";

    updateNeuronVisibility();
}

void MipMergedData::toggleNeuronVisibility(int index, bool status) // update a single neuron, on neuronSelectionModel.neuronMaskUpdated, O(log nfrags)
{}

void MipMergedData::toggleOverlayVisibility(int index, bool status)
{}

void MipMergedData::updateNeuronVisibility() // remerge all neurons O(nfrags), on neuronSelectionModel.visibilityChanged? or dirty partial update.
{
    {
        Writer writer(*this);
        recomputeLayerTree();
    } // release locks

    updateColors();
}

void MipMergedData::updateColors() // on dataColorModel.dataChanged, or mergedImage change
{
    Writer writer(*this);
    if (computeMergedImage()) {
        writer.unlock();
        emit dataChanged();
    }
}

bool MipMergedData::recomputeLayerTree()
{
    QTime stopwatch;
    stopwatch.start();
    // updating is ascending order should be OK
    for (int i = 0; i < layers.size(); ++i)
    {
        layers[i]->updateWithoutSignal();
    }
    qDebug() << "Computing full mip layer tree took" << stopwatch.elapsed() / 1000.0 << "seconds";
    return true;
}

bool MipMergedData::computeMergedImage()
{
    // Must have 16 bit data before we can compute color data.
    if (!layerZValues) return false;

    QTime stopwatch;
    stopwatch.start();

    // Color model is needed
    DataColorModel::Reader colorReader(dataColorModel);
    if (! colorReader.hasReadLock()) return false;

    if (mergedImage) {
        delete mergedImage;
        mergedImage = NULL;
    }

    int sx = layerZProxy.sx;
    int sy = layerZProxy.sy;
    int sc = layerDataProxy.sc;
    double channelIntensities[sc + 1]; // nFrags plus reference
    channelIntensities[sc] = 0; // punt reference for the moment
    mergedImage = new QImage(sx, sy, QImage::Format_RGB32);
    mergedImage->fill(qRgb(0, 0, 0));
    for (int y = 0; y < sy; ++y)
        for (int x = 0; x < sx; ++x)
        {
            for (int c = 0; c < sc; ++c)
                channelIntensities[c] = layerDataProxy.value_at(x, y, mergedIndex, c);
            QRgb color = colorReader.blend(channelIntensities);
            mergedImage->setPixel(x, y, color);
        }
    qDebug() << "Colorizing merged mip took" << stopwatch.elapsed() / 1000.0 << "seconds";
    return true;
}

///////////////////////////////////
// MipMergedData::Writer methods //
///////////////////////////////////

void MipMergedData::Writer::clearData()
{
    while (! mipMergedData.layers.isEmpty())
        delete mipMergedData.layers.takeAt(0);

    if (mipMergedData.layerZValues != NULL)
    {
        delete mipMergedData.layerZValues;
        mipMergedData.layerZValues = NULL;
    }

    if (mipMergedData.layerIntensities != NULL)
    {
        delete mipMergedData.layerIntensities;
        mipMergedData.layerIntensities = NULL;
    }

    if (mipMergedData.layerData != NULL)
    {
        delete mipMergedData.layerData;
        mipMergedData.layerData = NULL;
    }

    if (mipMergedData.mergedImage) {
        delete mipMergedData.mergedImage;
        mipMergedData.mergedImage = NULL;
    }
}


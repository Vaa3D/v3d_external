#include "MipMergedData.h"
#include "../DataFlowModel.h"
#include <cassert>
#include <vector>

///////////////////////////
// MipMergeLayer methods //
///////////////////////////

/* explicit */
MipMergeLayer::MipMergeLayer(MipMergedData& mipMergedDataParam,
                       int indexParam, bool isVisibleParam,
                       MipMergeLayer * child1Param, MipMergeLayer * child2Param)
    : mipMergedData(mipMergedDataParam)
    , index(indexParam), bIsVisible(isVisibleParam)
    , proxyIndex(indexParam)
    , child1(child1Param), child2(child2Param)
{
    if (child1) {
        // assert(child1->index < index); // it's OK for child1 to have the same index if this is an edge node (one child)
        connect(child1, SIGNAL(dataChanged()), this, SLOT(update()));
    }
    if (child2) {
        assert(child1);
        assert(child1->index < index);
        assert(child2->index < index);
        assert(child1->index != child2->index);
        connect(child2, SIGNAL(dataChanged()), this, SLOT(update()));
    }
}

void MipMergeLayer::setVisibility(bool bIsVisibleParam)
{
    bIsVisible = bIsVisibleParam;
}

void MipMergeLayer::update() // emits dataChanged()
{
    // qDebug() << "Updating mip layer" << index << this;
    updateWithoutSignal();
    emit dataChanged();
}

bool MipMergeLayer::copyChildState(MipMergeLayer * child)
{
    bool bChanged = false; // no change yet
    if (child->getProxyIndex() != getProxyIndex()) {
        proxyIndex = child->getProxyIndex();
        bChanged = true;
    }
    if (child->isVisible() != isVisible()) {
        setVisibility(child->isVisible());
        bChanged = true;
    }
    return bChanged;
}

bool MipMergeLayer::updateWithoutSignal() // for manual full updates
{
    // qDebug() << "MipMergeLayer::updateWithoutSignal()" << __FILE__ << __LINE__;
    bool bChanged = false; // no change yet
    // Leaf nodes cannot be updated (or are always updated?)
    if ( (child1 == NULL) && (child2 == NULL) ) {
        // qDebug() << "Leaf node, no update";
        // return bChanged; // not updated; this is a leaf; but let's defer to end of method for consistency.
    }
    else if (child2 == NULL)
    {
        // Just one child?  No extra computation needed.
        // qDebug() << "Edge node, simple copy";
        if (copyChildState(child1))
            bChanged = true;
    }
    else {
        // Two child nodes to combine
        if (! (child1->isVisible() || child2->isVisible()) ) {
            // Both child nodes invisible? So is this combined one.  Only compute isVisible flag.
            if (isVisible()) {
                bIsVisible = false;
                bChanged = true; // changed
            }
            // qDebug() << "Invisible node";
        }
        else if (child1->isVisible() && child2->isVisible()) {
            // Both are visible, so combine them.  This is the slow part
            proxyIndex = index; // Destination slice.  This node is a primary data source.
            int destinationSlice = getProxyIndex();
            int sourceSlice1 = child1->getProxyIndex(); // First source slice.
            int sourceSlice2 = child2->getProxyIndex(); // Second source slice.

            assert(destinationSlice > sourceSlice1);
            assert(destinationSlice > sourceSlice2);
            assert(sourceSlice1 != sourceSlice2);

            Image4DProxy<My4DImage>& layerProxy =  mipMergedData.layerZProxy;
            Image4DProxy<My4DImage>& intensityProxy = mipMergedData.layerIntensityProxy;
            Image4DProxy<My4DImage>& dataProxy = mipMergedData.layerDataProxy;
            Image4DProxy<My4DImage>& neuronProxy = mipMergedData.layerNeuronProxy;

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
                    neuronProxy.put_at(x, y, destinationSlice, 0,
                                      neuronProxy.value_at(x, y, sourceSlice, 0));
                    intensityProxy.put_at(x, y, destinationSlice, 0, intensity);
                    for (int c = 0; c < dataProxy.sc; ++c)
                        dataProxy.put_at(x, y, destinationSlice, c,
                                         dataProxy.value_at(x, y, sourceSlice, c));
                }
            // qDebug() << "Merging mip layers" << destinationSlice << sourceSlice1 << sourceSlice2;
            bIsVisible = true;
            bChanged = true;
            // TODO
        }
        else {
            // Only one visible child.  Copy its state into this node.
            // qDebug() << "One visible child, simple copy";
            MipMergeLayer * child = child1;
            if (child2->isVisible()) child = child2;
            if (copyChildState(child))
                bChanged = true;
        }
    }
    return bChanged;
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
        , layerNeurons(NULL)
        , emptyMy4DImage(new My4DImage())
        , layerZProxy(emptyMy4DImage)
        , layerIntensityProxy(emptyMy4DImage)
        , layerDataProxy(emptyMy4DImage)
        , layerNeuronProxy(emptyMy4DImage)
        , bShowReferenceChannel(false)
{
    connect(&mipFragmentData, SIGNAL(dataChanged()),
            this, SLOT(update()));
    connect(&dataColorModel, SIGNAL(dataChanged()),
            this, SLOT(colorizeImage()));
    // qDebug() << "connecting neuronVisibilityChanged signal" << &neuronSelectionModel << this;
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
    invalidate();
    Writer writer(*this);
    writer.clearData();
}

/* virtual */
void MipMergedData::update()
{
    if (! representsActualData()) return;

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

        if (! representsActualData()) return;
        if (layerZValues != NULL)
            delete layerZValues;
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

        if (! representsActualData()) return;
        if (layerIntensities != NULL)
            delete layerIntensities;
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

        if (! representsActualData()) return;
        if (layerData != NULL)
            delete layerData;
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

        if (! representsActualData()) return;
        if (layerNeurons != NULL)
            delete layerNeurons;
        layerNeurons = new My4DImage();
        layerNeurons->loadImage(
                fragmentZProxy.sx,
                fragmentZProxy.sy,
                // binary tree takes up to 2 times (nFrags + background) + reference
                2 * fragmentZProxy.sz,
                1,
                V3D_UINT16);
        memset(layerNeurons->getRawData(), 0, layerNeurons->getTotalBytes());
        layerNeuronProxy = Image4DProxy<My4DImage>(layerNeurons);
        if (! representsActualData()) return;

        // Leaf level of binary tree contains individual fragment mips (includes background but not reference)
        QList<MipMergeLayer*> levelLayers;
        int levelSize = fragmentDataProxy.sz; // all but reference
        for (int z = 0; z < levelSize; ++z) {
            for (int y = 0; y < fragmentZProxy.sy; ++y)
                for (int x = 0; x < fragmentZProxy.sx; ++x)
                {
                    layerZProxy.put_at(x, y, z, 0, fragmentZProxy.value_at(x, y, z, 0));
                    double intensity = fragmentIntensityProxy.value_at(x, y, z, 0);
                    layerIntensityProxy.put_at(x, y, z, 0, intensity);
                    if (intensity == 0)
                        layerNeuronProxy.put_at(x, y, z, 0, 0); // actually background
                    else
                        layerNeuronProxy.put_at(x, y, z, 0, z); // At leaf level neuron-index == slice-number
                    for (int c = 0; c < fragmentDataProxy.sc; ++c)
                        layerDataProxy.put_at(x, y, z, c,
                                              fragmentDataProxy.value_at(x, y, z, c));
                }
            if (! representsActualData()) return; // short circuit on reload
            MipMergeLayer * newLayer = new MipMergeLayer(*this, z, true, NULL, NULL);
            // qDebug() << z << newLayer;
            layers << newLayer; // store permanently
            levelLayers << newLayer; // store for next level build up
        }
        // Build up rest of binary tree, rooted at final merged fragment mip
        int nextSliceIndex = levelSize;
        while (levelLayers.size() > 1) {
            if (! representsActualData()) return; // short circuit on reload
            QList<MipMergeLayer*> previousLevelLayers = levelLayers;
            levelLayers.clear();
            while (! previousLevelLayers.isEmpty())
            {
                // binary tree joins two child nodes from previous (leafier) level
                MipMergeLayer * child1 = previousLevelLayers.takeAt(0);
                int zIndex = child1->getIndex(); // in case there is no child2, the new layer shares a slice with its child.
                MipMergeLayer * child2 = NULL;
                if (! previousLevelLayers.isEmpty())
                {
                    child2 = previousLevelLayers.takeAt(0);
                    zIndex = nextSliceIndex;
                    assert(zIndex < layerZProxy.sz);
                    nextSliceIndex++;
                }
                MipMergeLayer * newLayer = new MipMergeLayer(*this, zIndex, true, child1, child2);
                // qDebug() << zIndex << newLayer;
                layers << newLayer;
                levelLayers << newLayer;
            }
        }
        // qDebug() << "mergedIndex = " << mergedIndex << layers.last();
        // Note that "layers" does not necessarily have the same number of elements as
        // layerZValues has slices.  "mergedIndex" is a slice number.
        connect(layers.last(), SIGNAL(dataChanged()),
                this, SLOT(colorizeImage()));
        recomputeLayerTree();
    } // release locks

    size_t data_size = 0;
    data_size += layerZValues->getTotalBytes();
    data_size += layerIntensities->getTotalBytes();
    data_size += layerData->getTotalBytes();
    data_size += layerNeurons->getTotalBytes();

    updateNeuronVisibility();
    // qDebug() << "Setting up MipMergedData structure took " << stopwatch.elapsed() / 1000.0 << " seconds";
    // qDebug() << "MipMergedData consumes" << data_size / 1000.0 << "MB for" << layerNeuronProxy.sz / 2 << "neurons";
}

void MipMergedData::toggleNeuronVisibility(int index, bool status) // update a single neuron, on neuronSelectionModel.neuronMaskUpdated, O(log nfrags)
{
    QTime stopwatch;
    stopwatch.start();
    // qDebug() << "MipMergedData::toggleNeuronVisibility" << index << status << __FILE__ << __LINE__;
    int ix = index + 1; // layers has background layer at zero
    if (ix < 0) return;
    if (ix > layers.size()) return;
    if (status == layers[ix]->isVisible()) return; // no change
    {
        Writer writer(*this);
        layers[ix]->setVisibility(status);
    }
    {
        Writer writer(*this);
        layers[ix]->update(); // after lock, because it might emit.  It probably propagates directly in this thread.
    }
    // qDebug() << "MipMergedData::toggleNeuronVisibility() took" << stopwatch.elapsed() << "milliseconds";
}

void MipMergedData::toggleOverlayVisibility(int index, bool status)
{
    if (DataFlowModel::BACKGROUND_MIP_INDEX == index) {
        if (layers.size() < 1) return;
        // background is in layer zero
        if (status == layers[0]->isVisible()) return;
        {
            Writer writer(*this);
            layers[0]->setVisibility(status);
        }
        layers[0]->update();
    }
    else if (DataFlowModel::REFERENCE_MIP_INDEX == index) {
        // reference is handled specially
        if (status == bShowReferenceChannel) return;
        {
            Writer writer(*this);
            bShowReferenceChannel = status;
        }
        colorizeImage();
    }
}

void MipMergedData::updateNeuronVisibility() // remerge all neurons O(nfrags), on neuronSelectionModel.visibilityChanged? or dirty partial update.
{
    // qDebug() << "MipMergedData::updateNeuronVisibility()" << __FILE__ << __LINE__;
    {
        // Even if we cannot get the current visibilities, it might be worth updating the tree anyway
        NeuronSelectionModel::Reader selectionReader(neuronSelectionModel);
        if (selectionReader.hasReadLock()) {
            const QList<bool>& visibilityList = selectionReader.getMaskStatusList();
            for (int i = 0; i < visibilityList.size(); ++i) {
                if ((i+1) >= layers.size()) break;
                layers[i + 1]->setVisibility(visibilityList[i]);
            }
            const QList<bool>& overlayList = selectionReader.getOverlayStatusList();
            if ( (overlayList.size() >= 2) && (layers.size() > 0) ) {
                layers[0]->setVisibility(overlayList[DataFlowModel::BACKGROUND_MIP_INDEX]);
                bShowReferenceChannel = overlayList[DataFlowModel::REFERENCE_MIP_INDEX];
            }
            // qDebug() << "MipMergedData::updateNeuronVisibility()" << __FILE__ << __LINE__;
        }
    } // release read lock
    {
        Writer writer(*this);
        recomputeLayerTree();
        // qDebug() << "MipMergedData::updateNeuronVisibility()" << __FILE__ << __LINE__;
    } // release locks

    colorizeImage();
}

void MipMergedData::colorizeImage() // on dataColorModel.dataChanged, or mergedImage change
{
    // qDebug() << "MipMergedData::colorizeImage()" << __FILE__ << __LINE__;
    bool success = false;
    {
        Writer writer(*this);
        success = computeMergedImage();
    }
    if (success)
        emit dataChanged();
}

bool MipMergedData::recomputeLayerTree()
{
    QTime stopwatch;
    stopwatch.start();
    // updating is ascending order should be OK
    for (int i = 0; i < layers.size(); ++i)
    {
        if (! representsActualData())
            return false; // shortcut bomb out during data reload
        layers[i]->updateWithoutSignal();
        // qDebug() << "MipMergedData::recomputeLayerTree()"
        //        << i << layers[i]->isVisible() << layers[i]->getIndex() << layers[i]->getProxyIndex()
        //        << __FILE__ << __LINE__;
    }
    // qDebug() << "Computing full mip layer tree took" << stopwatch.elapsed() / 1000.0 << "seconds";
    return true;
}

bool MipMergedData::computeMergedImage()
{
    // qDebug() << "MipMergedData::computeMergedImage()" << __FILE__ << __LINE__;
    // Must have 16 bit data before we can compute color data.
    if (!layerZValues) return false;

    QTime stopwatch;
    stopwatch.start();

    // Color model is needed
    DataColorModel::Reader colorReader(dataColorModel);
    if (dataColorModel.readerIsStale(colorReader)) return false;

    MipFragmentData::Reader fragmentReader(mipFragmentData);
    if (! fragmentReader.hasReadLock()) return false;
    const Image4DProxy<My4DImage>& intensityProxy = fragmentReader.getIntensityProxy();

    int sx = layerZProxy.sx;
    int sy = layerZProxy.sy;
    int sc = layerDataProxy.sc;

    if (mergedImage && (mergedImage->size() != QSize(sx, sy)))
    {
        delete mergedImage;
        mergedImage = NULL;
    }
    if (!mergedImage)
        mergedImage = new QImage(sx, sy, QImage::Format_RGB32);

    int refIndex = intensityProxy.sz - 1;
    std::vector<double> channelIntensities(sc + 1, 0.0); // nFrags plus reference
    channelIntensities[sc] = 0; // punt reference for the moment
    mergedImage->fill(qRgb(0, 0, 0));
    bool bShowNeurons = layers.last()->isVisible(); // all neurons plus background might be hidden
    int proxyIndex = layers.last()->getProxyIndex(); // the layer with the actual image data
    // qDebug() << "proxyIndex =" << proxyIndex;
    for (int y = 0; y < sy; ++y)
        for (int x = 0; x < sx; ++x)
        {
            channelIntensities.assign(sc + 1, 0.0);
            if (bShowReferenceChannel)
                channelIntensities[sc] = intensityProxy.value_at(x, y, refIndex, 0);
            if (bShowNeurons) {
                for (int c = 0; c < sc; ++c)
                    channelIntensities[c] = layerDataProxy.value_at(x, y, proxyIndex, c);
            }
            QRgb color = colorReader.blend(&channelIntensities[0]);
            mergedImage->setPixel(x, y, color);
        }
    // qDebug() << "Colorizing merged mip took" << stopwatch.elapsed() / 1000.0 << "seconds"; // 22ms for 512x512
    // qDebug() << "MipMergedData::computeMergedImage()" << __FILE__ << __LINE__;

    return true;
}


///////////////////////////////////
// MipMergedData::Reader methods //
///////////////////////////////////

int MipMergedData::Reader::getMergedImageLayerIndex()
{
    if (mipMergedData.layers.size() < 1)
        return -1;
    return mipMergedData.layers.last()->getProxyIndex();
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

    if (mipMergedData.layerNeurons != NULL)
    {
        delete mipMergedData.layerNeurons;
        mipMergedData.layerNeurons = NULL;
    }

    if (mipMergedData.mergedImage) {
        delete mipMergedData.mergedImage;
        mipMergedData.mergedImage = NULL;
    }

    if (mipMergedData.emptyMy4DImage) {
        delete mipMergedData.emptyMy4DImage;
        mipMergedData.emptyMy4DImage = NULL;
    }
}


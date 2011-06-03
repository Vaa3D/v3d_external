#include "MipData.h"
#include <cassert>

/////////////////////
// MipData methods //
/////////////////////

MipData::MipData(const My4DImage* img, const My4DImage* maskImg, QObject * parent)
    : QObject(parent)
    , numNeurons(0)
    , neuronLayers(NULL)
{
    loadMy4DImage(img, maskImg);
}

MipData::~MipData()
{
    if (neuronLayers) {
        delete [] neuronLayers;
        neuronLayers = NULL;
    }
}

typedef std::vector<double> NeuronChannelIntegrator;
typedef std::vector<NeuronChannelIntegrator> NeuronChannelIntegratorList;

bool MipData::loadMy4DImage(const My4DImage* img, const My4DImage* maskImg)
{
    // Validate data in this thread
    if (!img) return false;

    benchmark.start();

    dataMin = 1e9;
    dataMax = -1e9;

    data.assign(img->getXDim(), MipData::Column(img->getYDim(), MipData::Pixel(img->getCDim()))); // 50 ms

    // qDebug() << "size = " << data.size();
    // qDebug() << "nColumns = " << nColumns();
    // MipData& t = *this;
    volume4DImage = img;
    My4DImage * mutable_img = const_cast<My4DImage*>(img);
    Image4DProxy<My4DImage> imgProxy(mutable_img);

    numNeurons = 0;

    NeuronChannelIntegratorList neuronColors;
    // First loop "quickly" updates intensity, without updating neuron masks
    for (int x = 0; x < nColumns(); ++x) {
        for (int y = 0; y < nRows(); ++y) {
            for (int z = 0; z < img->getZDim(); ++z)
            {
                int neuronIndex = -1;
                if (maskImg) {
                    neuronIndex = maskImg->at(x, y, z);
                    if (neuronIndex >= numNeurons) {
                        numNeurons = neuronIndex + 1;
                        neuronColors.resize(numNeurons, NeuronChannelIntegrator(nChannels(), 0.0));
                    }
                }
                float intensity = 0.0;
                for (int c = 0; c < nChannels(); ++c) {
                    float val = (float)imgProxy.value_at(x,y,z,c);
                    // Update minimum and maximum values
                    if (val > dataMax) dataMax = val;
                    if (val < dataMin) dataMin = val;
                    // Update current voxel intensity
                    intensity += val;
                    if (neuronIndex >= 0)
                        neuronColors[neuronIndex][c] += val;
                }
                assert(intensity >= 0.0);
                // Maximum intensity projection - regardless of neuron masks
                if (intensity > data[x][y].intensity) {
                    for (int c = 0; c < nChannels(); ++c)
                        data[x][y][c] = (float)imgProxy.value_at(x,y,z,c);
                    data[x][y].z = z; // remember z-value of max intensity pixel
                    data[x][y].intensity = intensity;
                    if (maskImg) {
                        data[x][y].neuronIndex = (int) maskImg->at(x, y, z);
                    }
                }
            }
        }
        if (! (x % 10))
            emit processedXColumn(x + 1);
        // qDebug() << "processed column " << x + 1;
    }
    qDebug() << "Computing MIP took " << benchmark.restart() << " milliseconds";
    for (int n = 0; n < neuronColors.size(); ++n)
    {
        NeuronChannelIntegrator& neuronColor = neuronColors[n];
        // find maximum
        double maxCount = -1e9;
        for (int c = 0; c < neuronColor.size(); ++c) {
            double channelCount = neuronColor[c];
            if (channelCount > maxCount)
                maxCount = channelCount;
        }
        // scale by maximum
        for (int c = 0; c < neuronColor.size(); ++c) {
            neuronColor[c] /= maxCount;
        }
    }
    qDebug() << "Computing neuron colors took " << benchmark.restart() << " milliseconds";
    // TODO - actually use the color information

    emit intensitiesUpdated();

    // Populate individual neuron mip layers
    if (maskImg && (numNeurons > 0))
    {
        if (neuronLayers) delete [] neuronLayers;
        neuronLayers = new MipLayer*[numNeurons];
        for (int i = 0; i < numNeurons; ++i)
            neuronLayers[i] = new MipLayer(QSize(nColumns(), nRows()), this);

        qDebug() << "processing MIP masks";
        for (int x = 0; x < nColumns(); ++x) {
            for (int y = 0; y < nRows(); ++y) {
                for (int z = 0; z < img->getZDim(); ++z)
                {
                    int neuronMaskId = maskImg->at(x,y,z);
                    if (neuronMaskId < 0) continue;
                    float intensity = 0.0;
                    for (int c = 0; c < nChannels(); ++c)
                        intensity += (float)imgProxy.value_at(x,y,z,c);
                    assert(intensity >= 0.0);
                    MipLayer::Pixel& currentPixel = neuronLayers[neuronMaskId]->getPixel(x, y);
                    if (   (currentPixel.neuronIndex != neuronMaskId) // no data for this neuron so far
                        || (intensity > currentPixel.intensity) ) // brightest intensity seen so far
                    {
                        currentPixel.neuronIndex = neuronMaskId;
                        currentPixel.zCoordinate = z;
                        currentPixel.intensity = intensity;
                    }
                }
            }
        }
        qDebug() << "finished creating MIP masks; took " << benchmark.restart() << " milliseconds";
        // TODO create binary tree of mip layers leading to combined image
        std::vector<MipLayer*> layers;
        for (int n = 0; n < numNeurons; ++n) {
            layers.push_back(neuronLayers[n]);
        }
        while (layers.size() > 1) {
            std::vector<MipLayer*> nextLevel;
            while (layers.size() > 0) {
                MipLayer* node1 = layers.back(); layers.pop_back();
                MipLayer* node2 = NULL;
                if (layers.size() > 0) {
                    node2 = layers.back();
                    layers.pop_back();
                }
                nextLevel.push_back(new MipLayer(node1, node2, this));
            }
            layers = nextLevel;
            qDebug() << "layers size = " << layers.size();
        }
        assert(layers.size() == 1);
        combinedMipLayer = layers.back();
        connect(combinedMipLayer, SIGNAL(layerChanged()),
                this, SLOT(onCombinedMipLayerUpdated()));
        qDebug() << "Creating MIP layer binary tree took " << benchmark.restart() << " milliseconds";
    }

    return true;
}

int MipData::nColumns() const {return data.size();}
int MipData::nRows() const {
    if (nColumns() < 1) return 0;
    return data[0].size();
}
int MipData::nChannels() const {
    if (nRows() < 1) return 0;
    return data[0][0].size();
}


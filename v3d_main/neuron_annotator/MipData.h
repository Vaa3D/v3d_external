#ifndef NA_MIP_DATA_H
#define NA_MIP_DATA_H

#include <QObject>
#include <vector>
#include "../v3d/v3d_core.h"
#include "MipLayer.h"

// MipData is a generic container for float representation of original MIP data.
// MipDisplayImage can contain a binary tree of MipDisplayImages,
// for efficient update when toggling different components (e.g. neurons).
class MipData : public QObject
{
    Q_OBJECT

public:

    class Pixel : public std::vector<float>
    {
    public:
        Pixel(size_t nChannels = 1)
            : std::vector<float>(nChannels, 0.0f)
            , z(-1), intensity(0.0f), neuronIndex(-1)
        {}

        int z;
        float intensity;
        int neuronIndex;
    };

    typedef std::vector<MipData::Pixel> Column;

    explicit MipData(QObject * parentObject = NULL)
        : QObject(parentObject)
        , neuronLayers(NULL)
    {}
    // Initialize from either a 3D data set...
    explicit MipData(const My4DImage* img, const My4DImage* maskImg = NULL, QObject * parent = NULL);
    // ...or a pair of intermediate images in a binary tree
    // explicit MipData(const MipData* child1, const MipData* child2, QObject * parent = NULL);
    virtual ~MipData();

    const MipData::Column& operator[](int index) const {
        return data[index];
    }
    MipData::Column& operator[](int index) {
        return data[index];
    }

    bool loadMy4DImage(const My4DImage* img, const My4DImage* maskImg = NULL);

    int nColumns() const;
    int nRows() const;
    int nChannels() const;

    float dataMin; // actual minimum value
    float dataMax; // actual maximum value
    int numNeurons;

signals:
    void processedXColumn(int);
    void intensitiesUpdated();
    void neuronMasksUpdated();

public slots:
    void updateData() {}
    void toggleNeuronDisplay(int neuronIndex, bool checked)
    {
        // qDebug() << "MipData toggleNeuronDisplay";
        // qDebug() << "MipData neuron " << neuronIndex << " toggled to " << checked;
        if (! neuronLayers) return;
        // qDebug() << "neuronLayers exists";
        // fix off by 1 error after recent change to meaning of neuron index.
        ++neuronIndex;
        if (neuronIndex < 0) return;
        // qDebug() << "neuronIndex >= 0";
        if (! neuronLayers[neuronIndex]) return;
        // qDebug() << "neuronLayers[" << neuronIndex << "] exists";
        MipLayer& neuronMip = *(neuronLayers[neuronIndex]);
        if (neuronMip.isEnabled() == checked) return;
        // qDebug() << "neuronMip state has changed";
        neuronMip.enableWithSignal(checked);
        // qDebug() << "MipData toggleNeuronDisplay2";
    }

    void onCombinedMipLayerUpdated()
    {
        // qDebug() << "combined MIP layer updated";
        if (! combinedMipLayer) return;
        if (! volume4DImage) return;
        const int dimY = combinedMipLayer->size().height();
        const int dimX = combinedMipLayer->size().width();
        My4DImage * mutable_img = const_cast<My4DImage*>(volume4DImage);
        Image4DProxy<My4DImage> imgProxy(mutable_img);
        for (int y = 0; y < dimY; ++y) {
            for (int x = 0; x < dimX; ++x) {
                const MipLayer::Pixel& layerPixel = combinedMipLayer->getPixel(x, y);
                MipData::Pixel& dataPixel = data[x][y];
                dataPixel.intensity = layerPixel.intensity;
                dataPixel.z = layerPixel.zCoordinate;
                dataPixel.neuronIndex = layerPixel.neuronIndex;
                int z = dataPixel.z;
                for (int c = 0; c < (int)dataPixel.size(); ++c) {
                    if (z >= 0)
                        dataPixel[c] = (float)imgProxy.value_at(x,y,z,c);
                    else
                        dataPixel[c] = 0;
                }
            }
        }
        emit intensitiesUpdated();
    }

private:
    std::vector<MipData::Column> data; // Image pixels
    // bool bToggledOn; // Whether this MIP image is on or off (within binary tree, say)
    MipLayer** neuronLayers;
    MipLayer* combinedMipLayer;
    QTime benchmark; // for performance testing
    const My4DImage * volume4DImage;
};

#endif // NA_MIP_DATA_H

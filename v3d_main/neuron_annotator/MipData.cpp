#include "MipData.h"
#include <cassert>

/////////////////////
// MipData methods //
/////////////////////

MipData::MipData(const My4DImage* img, const My4DImage* maskImg, QObject * parent)
    : QObject(parent)
    , numNeurons(0)
{
    loadMy4DImage(img, maskImg);
}

typedef std::vector<double> NeuronChannelIntegrator;
typedef std::vector<NeuronChannelIntegrator> NeuronChannelIntegratorList;

bool MipData::loadMy4DImage(const My4DImage* img, const My4DImage* maskImg)
{
    // Validate data in this thread
    if (!img) return false;

    dataMin = 1e9;
    dataMax = -1e9;
    std::map<int, MipData*> neuronImages;

    data.assign(img->getXDim(), MipColumn(img->getYDim(), MipPixel(img->getCDim()))); // 50 ms

    // qDebug() << "size = " << data.size();
    // qDebug() << "nColumns = " << nColumns();
    // MipData& t = *this;
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
    // TODO - actually use the color information

    emit intensitiesUpdated();

    if (maskImg && (numNeurons > 0))
    {
        qDebug() << "processing MIP masks";
        for (int x = 0; x < nColumns(); ++x) {
            for (int y = 0; y < nRows(); ++y) {
                for (int z = 0; z < img->getZDim(); ++z) {
                    if (false) {
                        int neuronMaskId = maskImg->at(x,y,z);
                        if (neuronImages.find(neuronMaskId) == neuronImages.end()) {
                            // TODO create another neuron mask
                            qDebug() << "creating neuron image #" << neuronMaskId;
                            neuronImages[neuronMaskId] = new MipData(NULL, NULL, this); // TODO - use img
                        }
                        MipData * neuronImage = neuronImages[neuronMaskId];
                        // TODO - react to neuron information
                    }
                }
            }
        }
    }
    qDebug() << "finished processing MIP masks";
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

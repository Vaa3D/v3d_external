#include "PrivateVolumeTexture.h"
#include "DataColorModel.h"
#include <QColor>
#include <cassert>


namespace jfrc {

// LabelSampler class for efficiently sampling a label field.
// This is meant to be fast, so all inline.
// with precomputed data offsets
template<typename T, class InputValueType, class OutputValueType>
// template<class InputValueType, class OutputValueType>
struct BaseSampler
{
    typedef size_t IndexType;
    static const int nDims = 3;

    BaseSampler(const Image4DProxy<My4DImage>& input, Base3DTexture<OutputValueType>& output)
    {
        // cache volume dimensions
        dims_in.assign(4, 0);
        dims_in[0] = input.sx;
        dims_in[1] = input.sy;
        dims_in[2] = input.sz;
        dims_in[3] = input.sc;

        dims_out.assign(nDims, 0);
        dims_out[0] = output.getWidth();
        dims_out[1] = output.getHeight();
        dims_out[2] = output.getDepth();
        Dimension used_size_out = output.getUsedSize();

        // lesser offsets will be set during recursion
        data_in = (const InputValueType*)input.data_p; // offset[2] = start of data block
        data_out = (OutputValueType*)output.getData();

        // precompute mapping between coordinates - nearest match
        coords_in_from_out.assign(nDims, std::vector<IndexType>());
        coords_out_from_in.assign(nDims, std::vector<IndexType>());
        for (int d = 0; d < nDims; ++d) {
            // Use "used" size for mapping, not padded size
            double ratio = 1.0;
            if (dims_in[d] > 0)
                ratio = double(used_size_out[d])/double(dims_in[d]);
            if (dims_in[d] > 0) {
                coords_out_from_in[d].assign(dims_in[d], 0);
                for (int x1 = 0; x1 < dims_in[d]; ++x1) {
                    int x2 = int((x1 + 0.5) * ratio);
                    assert(x2 >= 0);
                    assert(x2 < dims_out[d]);
                    coords_out_from_in[d][x1] = x2;
                }
            }
            if (dims_out[d] > 0) {
                coords_in_from_out[d].assign(dims_out[d], 0);
                for (int x2 = 0; x2 < dims_out[d]; ++x2) {
                    int x1 = 0;
                    if (ratio > 0)
                        x1 = int((x2 + 0.5) / ratio);
                    assert(x1 >= 0);
                    if (x1 < dims_in[d])
                        coords_in_from_out[d][x2] = x1;
                    else
                        coords_in_from_out[d][x2] = dims_in[d] - 1;
                }
            }
        }
    }

    inline void sample_over_input()
    {
        const IndexType sx = dims_in[0];
        const IndexType sy = dims_in[1];
        const IndexType sz = dims_in[2];
        const IndexType sc = dims_in[3];
        const size_t vol_in_stride = sx * sy * sz;
        const size_t slice_in_stride = sx * sy;
        const size_t row_in_stride = sx;
        const size_t slice_out_stride = dims_out[0] * dims_out[1];
        const size_t row_out_stride = dims_out[0];

        for (IndexType c = 0; c < sc; ++c) {
            const InputValueType* color_in = data_in + c * vol_in_stride;
            // precompute funny bgra mapping 0xAARRGGBB
            IndexType c2 = 2 - c;
            for (IndexType z = 0; z < sz; ++z) {
                const IndexType z_out = coords_out_from_in[2][z];
                const InputValueType* slice_in = color_in + z * slice_in_stride;
                OutputValueType* slice_out = data_out + z_out * slice_out_stride;
                for (IndexType y = 0; y < sy; ++y) {
                    const IndexType y_out = coords_out_from_in[1][y];
                    const InputValueType* row_in = slice_in + y * row_in_stride;
                    OutputValueType* row_out = slice_out + y_out * row_out_stride;
                    for (IndexType x = 0; x < sx; ++x)
                    {
                        const InputValueType in_val = row_in[x];
                        // for speed, short circuit when input is zero
                        if (0 == in_val)
                            continue;
                        const IndexType x_out = coords_out_from_in[0][x];
                        static_cast<T*>(this)->sample(in_val, row_out[x_out], c2);
                    }
                }
            }
        }
    }

    inline void sample_over_output()
    {
        const IndexType sx = dims_out[0];
        const IndexType sy = dims_out[1];
        const IndexType sz = dims_out[2];
        const IndexType sc = dims_in[3];
        const size_t slice_out_stride = sx * sy;
        const size_t row_out_stride = sx;
        const size_t vol_in_stride = dims_in[0] * dims_in[1] * dims_in[2];
        const size_t slice_in_stride = dims_in[0] * dims_in[1];
        const size_t row_in_stride = dims_in[0];
        for (IndexType c = 0; c < sc; ++c) {
            IndexType c2 = 2 - c;
            // qDebug() << "Color channel" << c << c2;
            const InputValueType* color_in = data_in + c * vol_in_stride;
            for (IndexType z = 0; z < sz; ++z) {
                const IndexType z_in = coords_in_from_out[2][z];
                OutputValueType* slice_out = data_out + z * slice_out_stride;
                const InputValueType* slice_in = color_in + z_in * slice_in_stride;
                for (IndexType y = 0; y < sy; ++y) {
                    const IndexType y_in = coords_in_from_out[1][y];
                    OutputValueType* row_out = slice_out + y * row_out_stride;
                    const InputValueType* row_in = slice_in + y_in * row_in_stride;
                    for (IndexType x = 0; x < sx; ++x)
                    {
                        const IndexType x_in = coords_in_from_out[0][x];
                        const InputValueType& in_val = row_in[x_in];
                        // for speed, short circuit when input is zero
                        if (0 == in_val)
                            continue;
                        OutputValueType& out_val = row_out[x];
                        static_cast<T*>(this)->sample(in_val, out_val, c2, 1 /* TODO */);
                    }
                }
            }
        }
    }

    std::vector<IndexType> dims_in; // number of voxels in each direction x,y,z,c
    const InputValueType* data_in;

    // For each dimension, precomputed mapping between coordinates
    std::vector<std::vector<IndexType> > coords_in_from_out;
    std::vector<std::vector<IndexType> > coords_out_from_in;

    std::vector<IndexType> dims_out; // number of voxels in each direction x,y,z,c
    OutputValueType* data_out;
};


template<class InputValueType, class OutputValueType>
struct LabelSampler : public BaseSampler<LabelSampler<InputValueType, OutputValueType>, InputValueType, OutputValueType>
{
private:
    typedef BaseSampler<LabelSampler<InputValueType, OutputValueType>, InputValueType, OutputValueType> super;

public:
    typedef uint16_t IndexType;

    LabelSampler(const Image4DProxy<My4DImage>& input, Base3DTexture<OutputValueType>& output)
        : super(input, output)
    {
        this->sample_over_input();
    }

    inline void sample(const InputValueType& in_val, OutputValueType& out_val, IndexType c)
    {
        // Take smallest non-zero value
        if ( (0 == out_val) || (out_val > in_val) )
            out_val = in_val;
    }
};

template<class InputValueType, class OutputValueType>
struct SignalSampler : public BaseSampler<SignalSampler<InputValueType, OutputValueType>, InputValueType, OutputValueType>
{
private:
    typedef BaseSampler<SignalSampler<InputValueType, OutputValueType>, InputValueType, OutputValueType> super;

public:
    typedef uint16_t IndexType;

    SignalSampler(const Image4DProxy<My4DImage>& input, Base3DTexture<OutputValueType>& output)
        : super(input, output)
        , truncate_bits(0)
    {
        if (sizeof(InputValueType) > 1)
            truncate_bits = 4; // convert 12-bit to 8-bit
        this->sample_over_input(); // combine multiple samples
    }

    inline void sample(const InputValueType& in_val, OutputValueType& out_val, IndexType c)
    {
        uint8_t& out_byte = ((uint8_t*)&out_val)[c];
        uint8_t in_byte = in_val >> truncate_bits;
        if (in_byte > out_byte)
            out_byte = in_byte; // maximum
    }

    int truncate_bits;
};


template<class InputValueType, class OutputValueType>
struct ReferenceSampler : public BaseSampler<ReferenceSampler<InputValueType, OutputValueType>, InputValueType, OutputValueType>
{
private:
    typedef BaseSampler<ReferenceSampler<InputValueType, OutputValueType>, InputValueType, OutputValueType> super;

public:
    typedef uint16_t IndexType;

    ReferenceSampler(const Image4DProxy<My4DImage>& input, Base3DTexture<OutputValueType>& output)
        : super(input, output)
        , truncate_bits(0)
    {
        if (sizeof(InputValueType) > 1)
            truncate_bits = 4; // convert 12-bit to 8-bit
        this->sample_over_input(); // combine multiple samples
    }

    inline void sample(const InputValueType& in_val, OutputValueType& out_val, IndexType c)
    {
        uint8_t& out_byte = ((uint8_t*)&out_val)[3];
        uint8_t in_byte = in_val >> truncate_bits;
        if (in_byte > out_byte)
            out_byte = in_byte; // maximum
    }

    int truncate_bits;
};


/////////////////////////////
// NeuronVisibilityTexture //
/////////////////////////////

/* explicit */
NeuronVisibilityTexture::NeuronVisibilityTexture(int maxNeurons)
    : visibilities(maxNeurons, 0x000000ff) // visible, neither selected nor highlighted
    , neuronSelectionModel(NULL)
{}

/// Connect this texture to the NeuronAnnotator unified data model
void NeuronVisibilityTexture::setNeuronSelectionModel(const NeuronSelectionModel& m)
{
    if (neuronSelectionModel == &m) return; // no change
    neuronSelectionModel = &m;
    update();
}

bool NeuronVisibilityTexture::update()
{
    if(! neuronSelectionModel) return false;
    bool bChanged = false; // For efficiency, keep track of whether the data actually change.
    {
        NeuronSelectionModel::Reader selectionReader(*neuronSelectionModel);
        if (! selectionReader.hasReadLock())
            return false;
        // Check each neuron fragment
        const QList<bool>& visList = selectionReader.getMaskStatusList();
        const QList<bool>& selList = selectionReader.getNeuronSelectList();
        assert(visList.size() == selList.size());
        for(int i = 0; i < visList.size(); ++i)
        {
            int ix = i + 1; // position zero is for background signal
            if (ix >= visibilities.size()) break;
            // Visiblity
            unsigned int val;
            if (visList[i]) { // visible
                val = visibilities[ix] | 0x000000ff; // 0xAABBGGRR, turn on red/visibility
            }
            else { // not visible
                val = visibilities[ix] & 0xffffff00; // 0xAABBGGRR, turn off red/visibility
            }
            if (val != visibilities[ix]) {
                visibilities[ix] = val;
                bChanged = true;
            }
            // Selection
            if (selList[i]) { // selected
                val = visibilities[ix] | 0x0000ff00; // 0xAABBGGRR, turn on green/selection
            }
            else { // not selected
                val = visibilities[ix] & 0xffff00ff; // 0xAABBGGRR, turn off green/selection
            }
            if (val != visibilities[ix]) {
                visibilities[ix] = val;
                bChanged = true;
            }
        }
        // Background signal is found in overlay list, not neuron fragment list.
        const QList<bool>& overlayList = selectionReader.getOverlayStatusList();
        if ( (overlayList.size() > DataFlowModel::BACKGROUND_MIP_INDEX)
            && (visibilities.size() > 0) )
        {
            int val;
            if (overlayList[DataFlowModel::BACKGROUND_MIP_INDEX])
                val = 0x000000ff; // visible, never selected nor highlighted
            else
                val = 0x00000000; // not visible
            if (val != visibilities[0]) {
                visibilities[0] = val;
                bChanged = true;
            }
        }
    } // release read lock
    return bChanged;
}


/////////////////////
// ColorMapTexture //
/////////////////////

ColorMapTexture::ColorMapTexture()
    : dataColorModel(NULL)
{
    colors.fill(0, 4 * 256);
    for (int c = 0; c < 4; ++c) {
        uint32_t color_mask = 0xff << (8 * c); // 0,1,2 => red,green,blue
        if (3 == c)
            color_mask = 0x00aaaaaa; // gray for channel 4
        for (int i = 0; i < 256; ++i) {
            // 0xAABBGGRR
            uint32_t alpha_mask = i << 24; // 0xAA000000
            colors[c*256 + i] = alpha_mask | color_mask;
        }
    }
}

bool ColorMapTexture::update()
{
    if (NULL == dataColorModel)
        return false;
    const DataColorModel::Reader colorReader(*dataColorModel);
    if (dataColorModel->readerIsStale(colorReader))
        return false;
    for (int rgb = 0; rgb < 4; ++rgb) // loop red, then green, then blue, then reference
    {
        // qDebug() << "color" << rgb;
        QRgb channelColor = colorReader.getChannelColor(rgb);
        for (int i_in = 0; i_in < 256; ++i_in)
        {
            // R/G/B color channel value is sum of data channel values
            qreal i_out_f = colorReader.getChannelScaledIntensity(rgb, i_in / 255.0) * 255.0;
            // qDebug() << rgb << i_in << i_out_f << colorReader.getChannelScaledIntensity(rgb, i_in / 255.0)
            //     << colorReader.getChannelVisibility(rgb);
            if (i_out_f < 0.0f) i_out_f = 0.0f;
            if (i_out_f > 255.0) i_out_f = 255.0;
            uint32_t i_out = (uint32_t) (i_out_f + 0.49999);
            // Intensities are set in the alpha channel only
            // (i.e. not R, G, or B)
            uint32_t color = 0;
            color |= (  qRed(channelColor) <<  0);
            color |= (qGreen(channelColor) <<  8);
            color |= ( qBlue(channelColor) << 16);
            color |= ( i_out << 24 ); // alpha
            colors[rgb*256 + i_in] = color;
        }
    }
    return true;
}

void ColorMapTexture::setDataColorModel(const DataColorModel& cm)
{
    dataColorModel = &cm;
}


//////////////////////////
// PrivateVolumeTexture //
//////////////////////////

PrivateVolumeTexture::PrivateVolumeTexture()
    : memoryLimit(5e8) // 500 MB for volume texture
    , memoryAlignment(8)
    , subsampleScale(1.0)
    , bUse3DSignalTexture(true)
{
}

/* virtual */
PrivateVolumeTexture::~PrivateVolumeTexture()
{
    qDebug() << "Deleting PrivateVolumeTexture";
}

/* explicit */
PrivateVolumeTexture::PrivateVolumeTexture(const PrivateVolumeTexture& rhs)
    : memoryLimit(rhs.memoryLimit)
    , memoryAlignment(rhs.memoryAlignment)
    , subsampleScale(rhs.subsampleScale)
    , originalImageSize(rhs.originalImageSize)
    , usedTextureSize(rhs.usedTextureSize)
    , paddedTextureSize(rhs.paddedTextureSize)
    , neuronLabelTexture(rhs.neuronLabelTexture)
    , neuronVisibilityTexture(rhs.neuronVisibilityTexture)
    , neuronSignalTexture(rhs.neuronSignalTexture)
    , bUse3DSignalTexture(rhs.bUse3DSignalTexture)
{
    qDebug() << "PrivateVolumeTexture is being copied";
}

void PrivateVolumeTexture::setNeuronSelectionModel(const NeuronSelectionModel& neuronSelectionModel)
{
    neuronVisibilityTexture.setNeuronSelectionModel(neuronSelectionModel);
}

void PrivateVolumeTexture::setDataColorModel(const DataColorModel& dataColorModel)
{
    colorMapTexture.setDataColorModel(dataColorModel);
}

void PrivateVolumeTexture::initializeSizes(const NaVolumeData::Reader& volumeReader)
{
    const Image4DProxy<My4DImage>& imageProxy = volumeReader.getOriginalImageProxy();
    Dimension inputSize(imageProxy.sx, imageProxy.sy, imageProxy.sz);
    // if (inputSize != originalImageSize) // new/changed volume size // label size only might have changed
    {
        originalImageSize = inputSize;
        subsampleScale = inputSize.computeLinearSubsampleScale(memoryLimit/3);
        usedTextureSize = inputSize.sampledSize(memoryLimit/3);
        paddedTextureSize = usedTextureSize.padToMultipleOf(memoryAlignment);
        neuronLabelTexture.allocateSize(paddedTextureSize, usedTextureSize);
        neuronSignalTexture.allocateSize(paddedTextureSize, usedTextureSize);
    }
}

bool PrivateVolumeTexture::subsampleLabelField(const NaVolumeData::Reader& volumeReader)
{
    QTime time2;
    time2.start();
    const Image4DProxy<My4DImage>& labelProxy = volumeReader.getNeuronMaskProxy();
    size_t size = labelProxy.sx * labelProxy.sy * labelProxy.sz;
    if (size < 1) return false;
    qDebug() << "starting label resample";
    // Label field file can be either 8 or 16 bits.
    // Our sammpled version must be 16 bits
    if (1 == labelProxy.su) { // 8 bit input
        LabelSampler<uint8_t, uint16_t> sampler(labelProxy, neuronLabelTexture);
    }
    else { // 16 bit input
        LabelSampler<uint16_t, uint16_t> sampler(labelProxy, neuronLabelTexture);
    }
    qDebug() << "label resample took" << time2.elapsed() << "milliseconds";
    return true;
}

bool PrivateVolumeTexture::subsampleColorField(const NaVolumeData::Reader& volumeReader)
{
    QTime time2;
    time2.start();
    const Image4DProxy<My4DImage>& imageProxy = volumeReader.getOriginalImageProxy();
    qDebug() << "starting signal resample";
    // Label field file can be either 8 or 16 bits.
    // Our sammpled version must be 16 bits
    if (1 == imageProxy.su) { // 8 bit input
        SignalSampler<uint8_t, uint32_t> sampler(imageProxy, neuronSignalTexture);
    }
    else { // 16 bit input
        SignalSampler<uint16_t, uint32_t> sampler(imageProxy, neuronSignalTexture);
    }
    qDebug() << "color resample took" << time2.elapsed() << "milliseconds";
    return true;
}

bool PrivateVolumeTexture::subsampleReferenceField(const NaVolumeData::Reader& volumeReader)
{
    QTime time2;
    time2.start();
    const Image4DProxy<My4DImage>& refProxy = volumeReader.getReferenceImageProxy();
    qDebug() << "starting reference resample";
    // Label field file can be either 8 or 16 bits.
    // Our sammpled version must be 16 bits
    if (1 == refProxy.su) { // 8 bit input
        ReferenceSampler<uint8_t, uint32_t> sampler(refProxy, neuronSignalTexture);
    }
    else { // 16 bit input
        ReferenceSampler<uint16_t, uint32_t> sampler(refProxy, neuronSignalTexture);
    }
    qDebug() << "reference resample took" << time2.elapsed() << "milliseconds";
    return true;
}

// Create host texture memory for data volume
// TODO fill 3D texture
bool PrivateVolumeTexture::populateVolume(const NaVolumeData::Reader& volumeReader, int zBegin, int zEnd)
{
    // qDebug() << "Populating volume textures for 3D renderer..." << __FILE__ << __LINE__;
    QTime stopwatch;
    stopwatch.start();
    const Image4DProxy<My4DImage>& imageProxy = volumeReader.getOriginalImageProxy();
    const Image4DProxy<My4DImage>& referenceProxy = volumeReader.getReferenceImageProxy();
    const Image4DProxy<My4DImage>& labelProxy = volumeReader.getNeuronMaskProxy();
    initializeSizes(volumeReader);
    // Scale RGBA channel colors to actual data range of input
    // (Final coloring will be handled in the shader, so don't use colorReader.blend())
    // Precompute coefficients for scaling.
    int minData[4] = {0, 0, 0, 0}; // minimum data value of channel
    double rangeData[4] = {1.0, 1.0, 1.0, 1.0}; // 255.0 divided by data range of channel
    size_t refIx = imageProxy.sc; // index of reference channel
    assert(imageProxy.sc <= 3); // that's what I'm assuming for now...
    for (int c = 0; c < imageProxy.sc; ++c)
    {
        minData[c] = imageProxy.vmin[c];
        rangeData[c] = 255.0 / (imageProxy.vmax[c] - imageProxy.vmin[c]);
        // elide use of colorReader
        // minData[c] = colorReader.getChannelDataMin(c);
        // rangeData[c] = 255.0 / (colorReader.getChannelDataMax(c) - colorReader.getChannelDataMin(c));
    }
    if (volumeReader.hasReferenceImage()) {
        minData[refIx] = referenceProxy.vmin[0];
        rangeData[refIx] = 255.0 / (referenceProxy.vmax[0] - referenceProxy.vmin[0]);
    }

    // Use stupid box filter for now.  Once that's working, use Lanczos for better sampling.
    // TODO
    double xScale = (double)originalImageSize.x() / (double)usedTextureSize.x();
    double yScale = (double)originalImageSize.y() / (double)usedTextureSize.y();
    double zScale = (double)originalImageSize.z() / (double)usedTextureSize.z();
    // qDebug() << "x, y, z Scale =" << xScale << yScale << zScale << __FILE__ << __LINE__;
    std::vector<double> channelIntensities(imageProxy.sc + 1, 0.0); // For colorReader::blend() interface; +1 for reference channel
    if (zEnd < 0) // -1 means actual final z
        zEnd = (int)usedTextureSize.z();
    if (zEnd > usedTextureSize.z())
        zEnd = (int)usedTextureSize.z();
    int channelCount = imageProxy.sc;
    if (volumeReader.hasReferenceImage())
        channelCount += 1;
    for(int z = zBegin; z < zEnd; ++z)
    {
        // qDebug() << z << __FILE__ << __LINE__;
        int z0 = (int)(z * zScale + 0.49);
        int z1 = (int)((z + 1) * zScale + 0.49);
        for(int y = 0; y < usedTextureSize.y(); ++y)
        {
            int y0 = (int)(y * yScale + 0.49);
            int y1 = (int)((y + 1) * yScale + 0.49);
            for(int x = 0; x < usedTextureSize.x(); ++x)
            {
                int x0 = (int)(x * xScale + 0.49);
                int x1 = (int)((x + 1) * xScale + 0.49);
                float weight = 0.0;
                // Choose exactly one neuron index for this voxel.  Default to zero (background),
                // but accept any non-background value in its place.
                int neuronIndex = 0;
                // Average over multiple voxels in input image
                channelIntensities.assign(channelCount, 0.0);
                for(int sx = x0; sx < x1; ++sx)
                    for(int sy = y0; sy < y1; ++sy)
                        for(int sz = z0; sz < z1; ++sz)
                        {
                            for (int c = 0; c < imageProxy.sc; ++c)
                                channelIntensities[c] += imageProxy.value_at(sx, sy, sz, c);
                            if (volumeReader.hasReferenceImage())
                                channelIntensities[refIx] += referenceProxy.value_at(sx, sy, sz, 0);
                            if (volumeReader.hasNeuronMask()) {
                                if (neuronIndex == 0) // take first non-zero value
                                    neuronIndex = (int)labelProxy.value_at(sx, sy, sz, 0);
                            }
                            weight += 1.0;
                        }
                for (int c = 0; c < channelCount; ++c) // Normalize
                    if (weight > 0)
                        channelIntensities[c]  = ((channelIntensities[c] / weight) - minData[c]) * rangeData[c];
                // Swap red and blue from RGBA to BGRA, for Windows texture efficiency
                // Create unsigned int with #AARRGGBB pattern
                BGRA color = 0;
                color |= (((int)channelIntensities[2]));       // #000000BB blue  : channel 3
                color |= (((int)channelIntensities[1]) << 8);  // #0000GGBB green : channel 2
                color |= (((int)channelIntensities[0]) << 16); // #00RRGGBB red   : channel 1
                color |= (((int)channelIntensities[3]) << 24); // #AARRGGBB white : reference
                neuronSignalTexture.setValueAt(x, y, z, color);
            }
        }
    }
    // qDebug() << "Sampling 3D volume for 3D viewer took" << stopwatch.elapsed() / 1000.0 << "seconds";
    return true;
}

struct BlockCopier
{
    void setup(uint8_t* destParam, const uint8_t* srcParam, size_t nbytesParam)
    {
        src = srcParam;
        dest = destParam;
        nbytes = nbytesParam;
    }

    void copy()
    {
        memcpy(dest, src, nbytes);
    }

    const uint8_t* src;
    uint8_t* dest;
    size_t nbytes;
};

bool PrivateVolumeTexture::loadFast3DTexture(int sx, int sy, int sz, const uint8_t* data) // from fast texture
{
    qDebug() << "PrivateVolumeTexture::loadFast3DTexture" << sx << sy << sz << __FILE__ << __LINE__;
    Dimension size(sx, sy, sz); // TODO - what about used size?
    neuronSignalTexture.allocateSize(size, size);
    size_t texture_bytes = sx * sy * sz * 4;
    // TODO - copy this in parallel
    bool doParallel = false; // It's not faster in parallel
    const uint8_t* src = data;
    uint8_t* dest = (uint8_t*)neuronSignalTexture.getData();
    if (doParallel) {
        const int nThreads = 6;
        QList<QFuture<void> > futures;
        std::vector<BlockCopier> copiers(nThreads);
        size_t nBytes = texture_bytes/nThreads;
        for (int t = 0; t < nThreads; ++t)
        {
            // final block might be a different size
            size_t actualNBytes = nBytes;
            if (t == nThreads-1)
                actualNBytes = texture_bytes - nBytes*t;
            copiers[t].setup(dest+t*nBytes, src+t*nBytes, actualNBytes);
            futures << QtConcurrent::run(&copiers[t], &BlockCopier::copy);
        }
        // wait here until everyone is done
        for (int t = 0; t < nThreads; ++t)
            futures[t].waitForFinished();
    }
    else {
        memcpy(dest, src, texture_bytes);
    }
    return true;
}

bool PrivateVolumeTexture::updateNeuronVisibilityTexture()
{
    return neuronVisibilityTexture.update();
}

bool PrivateVolumeTexture::updateColorMapTexture()
{
    return colorMapTexture.update();
}

} // namespace jfrc


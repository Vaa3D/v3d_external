#include "PrivateVolumeTexture.h"
#include "DataColorModel.h"
#include "../utility/ImageLoader.h"
#include "../utility/FooDebug.h"
#include "../utility/url_tools.h"
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
        Dimension padded_size_out = output.getPaddedSize();

        // lesser offsets will be set during recursion
        data_in = (const InputValueType*)input.data_p; // offset[2] = start of data block
        data_out = (OutputValueType*)output.getData();

        // precompute mapping between coordinates - nearest match
        coords_in_from_out.assign(nDims, std::vector<IndexType>());
        coords_out_from_in.assign(nDims, std::vector<IndexType>());
        for (int d = 0; d < nDims; ++d) {
            double ratio = 1.0;
            if (dims_in[d] > 0) {
                // Use "used" size for mapping, not padded size
                ratio = double(used_size_out[d])/double(dims_in[d]);
                // ratio = double(padded_size_out[d])/double(dims_in[d]);
            }
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

        for (IndexType c = 0; c < sc; ++c) { // color channel
            const InputValueType* color_in = data_in + c * vol_in_stride;
            // precompute funny bgra mapping 0xAARRGGBB
            IndexType c2 = 2 - c;
            for (IndexType z = 0; z < sz; ++z) { // input z depth dimension
                const IndexType z_out = coords_out_from_in[2][z];
                const InputValueType* slice_in = color_in + z * slice_in_stride;
                OutputValueType* slice_out = data_out + z_out * slice_out_stride;
                for (IndexType y = 0; y < sy; ++y) { // input y height dimension
                    const IndexType y_out = coords_out_from_in[1][y];
                    const InputValueType* row_in = slice_in + y * row_in_stride;
                    OutputValueType* row_out = slice_out + y_out * row_out_stride;
                    for (IndexType x = 0; x < sx; ++x) // input x width dimension
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


////////////////////////////////
// NeuronLabelTexture methods //
////////////////////////////////

bool NeuronLabelTexture::loadFromPbdFile(QUrl fileUrl)
{
    ImageLoader loader;
    My4DImage stack;
    if (! loader.loadImage(&stack, fileUrl))
        return false;
    assert(stack.getCDim() == 1);
    // assert(stack.getUnitBytes() == 2);
    Dimension size(stack.getXDim(), stack.getYDim(), stack.getZDim());
    allocateSize(size, size);
    if (stack.getUnitBytes() == 2) { // direct copy possible
        size_t numBytes = stack.getTotalBytes();
        uint16_t* dest = &data[0];
        const uint16_t* src = (const uint16_t*)stack.getRawData();
        memcpy(dest, src, numBytes);
    }
    else {
        assert(stack.getUnitBytes() == 1);
        uint16_t* dest_channel = &data[0];
        const uint8_t* src_channel = (const uint8_t*)stack.getRawData();
        size_t row_voxels = size.x();
        size_t slice_voxels = row_voxels * size.y();
        for (int z = 0; z < size.z(); ++z) {
            uint16_t* dest_slice = dest_channel + z * slice_voxels;
            const uint8_t* src_slice = src_channel + z * slice_voxels;
            for (int y = 0; y < size.y(); ++y) {
                uint16_t* dest_row = dest_slice + y * row_voxels;
                const uint8_t* src_row = src_slice + y * row_voxels;
                for (int x = 0; x < size.x(); ++x) {
                    dest_row[x] = src_row[x];
                }
            }
        }
    }
    return true;
}


/////////////////////////
// NeuronSignalTexture //
/////////////////////////

bool NeuronSignalTexture::loadReferenceFromRawFile(QUrl fileUrl)
{
    ImageLoader loader;
    My4DImage stack;
    if (! loader.loadImage(&stack, fileUrl))
        return false;
    Dimension paddedSize(stack.getXDim(), stack.getYDim(), stack.getZDim());
    // Load metadata, if metadata file is present
    QFileInfo fi(fileUrl.path());
    QUrl dir = parent(fileUrl);
    // e.g. Reference2_100.metadata
    QUrl metadataFileUrl = appendPath(dir, fi.completeBaseName() + ".metadata");
    if (exists(metadataFileUrl)) {
        SampledVolumeMetadata svm;
        if (svm.loadFromUrl(metadataFileUrl, 0)) {
            assert(svm.paddedImageSize == paddedSize);
            usedSize = svm.usedImageSize;
        }
    }
    allocateSize(paddedSize, usedSize);
    // We should only be loading from an 8-bit file.
    assert(stack.getUnitBytes() == 1);
    size_t sc = stack.getCDim();
    assert(sc == 1);
    size_t sz = stack.getZDim();
    size_t sy = stack.getYDim();
    size_t sx = stack.getXDim();
    size_t src_row_bytes = sx * stack.getUnitBytes();
    size_t src_slice_bytes = sy * src_row_bytes;
    size_t dest_row_bytes = 4 * sx;
    size_t dest_slice_bytes = sy * dest_row_bytes;
    const uint8_t* src_channel = stack.getRawData();
    uint8_t* dest_channel = (uint8_t*)&data[0] + 3; // 3 = offset to alpha channel byte
    for (int z = 0; z < sz; ++z) {
        const uint8_t* src_slice = src_channel + z * src_slice_bytes;
        uint8_t* dest_slice = dest_channel + z * dest_slice_bytes ;
        for (int y = 0; y < sy; ++y) {
            const uint8_t* src_row = src_slice + y * src_row_bytes;
            uint8_t* dest_row = dest_slice + y * dest_row_bytes;
            for (int x = 0; x < sx; ++x) {
                dest_row[x*4] = src_row[x];
            }
        }
    }
    return true;
}

bool NeuronSignalTexture::loadSignalFromRawFile(QUrl fileUrl)
{
    ImageLoader loader;
    My4DImage stack;
    if (! loader.loadImage(&stack, fileUrl))
        return false;
    Dimension paddedSize(stack.getXDim(), stack.getYDim(), stack.getZDim());
    // Load metadata, if metadata file is present
    QFileInfo fi(fileUrl.path());
    QUrl dir = parent(fileUrl);
    // e.g. Reference2_100.metadata
    QUrl metadataFileUrl = appendPath(dir, fi.completeBaseName() + ".metadata");
    if (exists(metadataFileUrl)) {
        SampledVolumeMetadata svm;
        if (svm.loadFromUrl(metadataFileUrl, 0)) {
            assert(svm.paddedImageSize == paddedSize);
            usedSize = svm.usedImageSize;
        }
    }
    allocateSize(paddedSize, usedSize);
    // We should only be loading from an 8-bit file.
    assert(stack.getUnitBytes() == 1);
    size_t sc = stack.getCDim();
    size_t sz = stack.getZDim();
    size_t sy = stack.getYDim();
    size_t sx = stack.getXDim();
    size_t src_row_bytes = sx * stack.getUnitBytes();
    size_t src_slice_bytes = sy * src_row_bytes;
    size_t src_channel_bytes = sz * src_slice_bytes;
    size_t dest_row_bytes = 4 * sx;
    size_t dest_slice_bytes = sy * dest_row_bytes;
    for (int c = 0; c < sc; ++c) {
        const uint8_t* src_channel = stack.getRawData() + c * src_channel_bytes;
        int c2 = c;
        if (c < 3) c2 = 2 - c;
        uint8_t* dest_channel = (uint8_t*)&data[0] + c2;
        for (int z = 0; z < sz; ++z) {
            const uint8_t* src_slice = src_channel + z * src_slice_bytes;
            uint8_t* dest_slice = dest_channel + z * dest_slice_bytes ;
            for (int y = 0; y < sy; ++y) {
                const uint8_t* src_row = src_slice + y * src_row_bytes;
                uint8_t* dest_row = dest_slice + y * dest_row_bytes;
                for (int x = 0; x < sx; ++x) {
                    dest_row[x*4] = src_row[x];
                }
            }
        }
    }
    // fooDebug() << this << paddedSize.x() << __FILE__ << __LINE__;
    return true;
}



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
    // qDebug() << "initializing dataColorModel to" << dataColorModel;
    colors.assign(4 * 256, 0);
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

ColorMapTexture::ColorMapTexture( ColorMapTexture const& rhs ):
    dataColorModel( rhs.dataColorModel ), colors( rhs.colors )
    {}


bool ColorMapTexture::update()
{
    if (NULL == dataColorModel) {
        // qDebug() << "data color model is NULL" << __FILE__ << __LINE__;
        return false;
    }
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
    // qDebug() << "setting dataColorModel to" << dataColorModel;
}


//////////////////////////
// PrivateVolumeTexture //
//////////////////////////

PrivateVolumeTexture::PrivateVolumeTexture()
    : subsampleScale(1.0)
    , memoryAlignment(8)
    , bUse3DSignalTexture(true)
{
}

/* virtual */
PrivateVolumeTexture::~PrivateVolumeTexture()
{
    // qDebug() << "Deleting PrivateVolumeTexture";
}

/* explicit */
PrivateVolumeTexture::PrivateVolumeTexture(const PrivateVolumeTexture& rhs)
    : memoryAlignment(rhs.memoryAlignment)
    , subsampleScale(rhs.subsampleScale)
    // , originalImageSize(rhs.originalImageSize)
    // , usedTextureSize(rhs.usedTextureSize)
    // , paddedTextureSize(rhs.paddedTextureSize)
    , neuronLabelTexture(rhs.neuronLabelTexture)
    , neuronVisibilityTexture(rhs.neuronVisibilityTexture)
    , neuronSignalTexture(rhs.neuronSignalTexture)
    , colorMapTexture(rhs.colorMapTexture)
    , bUse3DSignalTexture(rhs.bUse3DSignalTexture)
    , metadata(rhs.metadata)
{
    // qDebug() << "PrivateVolumeTexture is being copied";
}

bool PrivateVolumeTexture::loadSignalRawFile(QUrl fileUrl)
{
    bool bSucceeded = neuronSignalTexture.loadSignalFromRawFile(fileUrl);
    if (bSucceeded) {
        metadata.paddedImageSize = neuronSignalTexture.getPaddedSize();
        metadata.usedImageSize = neuronSignalTexture.getUsedSize();
    }
    // fooDebug() << this << metadata.paddedImageSize.x() << __FILE__ << __LINE__;
    return bSucceeded;
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
        metadata.originalImageSize = inputSize;
        size_t memoryLimit = 3.5e8; // 350 MB
        QSettings settings(QSettings::UserScope, "HHMI", "Vaa3D");
        QVariant val = settings.value("NaMaxVideoMegabytes");
        if (val.isValid())
            memoryLimit = 1e6 * val.toInt();
        subsampleScale = inputSize.computeLinearSubsampleScale(memoryLimit);
        metadata.usedImageSize = inputSize.sampledSize(memoryLimit);
        metadata.paddedImageSize = metadata.usedImageSize.padToMultipleOf(memoryAlignment);
        neuronLabelTexture.allocateSize(getPaddedTextureSize(), getUsedTextureSize());
        neuronSignalTexture.allocateSize(getPaddedTextureSize(), getUsedTextureSize());
    }
}

bool PrivateVolumeTexture::subsampleLabelField(const NaVolumeData::Reader& volumeReader)
{
    QTime time2;
    time2.start();
    const Image4DProxy<My4DImage>& labelProxy = volumeReader.getNeuronMaskProxy();
    size_t size = labelProxy.sx * labelProxy.sy * labelProxy.sz;
    if (size < 1) return false;
    // qDebug() << "starting label resample";
    // Label field file can be either 8 or 16 bits.
    // Our sammpled version must be 16 bits
    if (1 == labelProxy.su) { // 8 bit input
        LabelSampler<uint8_t, uint16_t> sampler(labelProxy, neuronLabelTexture);
    }
    else { // 16 bit input
        LabelSampler<uint16_t, uint16_t> sampler(labelProxy, neuronLabelTexture);
    }
    // qDebug() << "label resample took" << time2.elapsed() << "milliseconds";
    return true;
}

bool PrivateVolumeTexture::subsampleColorField(const NaVolumeData::Reader& volumeReader)
{
    QTime time2;
    time2.start();
    const Image4DProxy<My4DImage>& imageProxy = volumeReader.getOriginalImageProxy();
    // qDebug() << "starting signal resample";
    // Label field file can be either 8 or 16 bits.
    // Our sammpled version must be 16 bits
    if (1 == imageProxy.su) { // 8 bit input
        SignalSampler<uint8_t, uint32_t> sampler(imageProxy, neuronSignalTexture);
    }
    else { // 16 bit input
        SignalSampler<uint16_t, uint32_t> sampler(imageProxy, neuronSignalTexture);
    }
    // qDebug() << "color resample took" << time2.elapsed() << "milliseconds";
    return true;
}

bool PrivateVolumeTexture::subsampleReferenceField(const NaVolumeData::Reader& volumeReader)
{
    QTime time2;
    time2.start();
    const Image4DProxy<My4DImage>& refProxy = volumeReader.getReferenceImageProxy();
    // qDebug() << "starting reference resample";
    // Label field file can be either 8 or 16 bits.
    // Our sammpled version must be 16 bits
    if (1 == refProxy.su) { // 8 bit input
        ReferenceSampler<uint8_t, uint32_t> sampler(refProxy, neuronSignalTexture);
    }
    else { // 16 bit input
        ReferenceSampler<uint16_t, uint32_t> sampler(refProxy, neuronSignalTexture);
    }
    // qDebug() << "reference resample took" << time2.elapsed() << "milliseconds";
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
    double xScale = (double)getOriginalImageSize().x() / (double)getUsedTextureSize().x();
    double yScale = (double)getOriginalImageSize().y() / (double)getUsedTextureSize().y();
    double zScale = (double)getOriginalImageSize().z() / (double)getUsedTextureSize().z();
    // qDebug() << "x, y, z Scale =" << xScale << yScale << zScale << __FILE__ << __LINE__;
    std::vector<double> channelIntensities(imageProxy.sc + 1, 0.0); // For colorReader::blend() interface; +1 for reference channel
    if (zEnd < 0) // -1 means actual final z
        zEnd = (int)getUsedTextureSize().z();
    if (zEnd > getUsedTextureSize().z())
        zEnd = (int)getUsedTextureSize().z();
    int channelCount = imageProxy.sc;
    if (volumeReader.hasReferenceImage())
        channelCount += 1;
    for(int z = zBegin; z < zEnd; ++z)
    {
        // qDebug() << z << __FILE__ << __LINE__;
        int z0 = (int)(z * zScale + 0.49);
        int z1 = (int)((z + 1) * zScale + 0.49);
        for(int y = 0; y < getUsedTextureSize().y(); ++y)
        {
            int y0 = (int)(y * yScale + 0.49);
            int y1 = (int)((y + 1) * yScale + 0.49);
            for(int x = 0; x < getUsedTextureSize().x(); ++x)
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
    // qDebug() << "PrivateVolumeTexture::loadFast3DTexture" << sx << sy << sz << __FILE__ << __LINE__;
    Dimension size(sx, sy, sz); // TODO - what about used size?
    assert(size == getPaddedTextureSize());
    neuronSignalTexture.allocateSize(getPaddedTextureSize(), getUsedTextureSize());
    size_t texture_bytes = sx * sy * sz * 4;
    // bool doParallel = false; // It's not faster in parallel
    const uint8_t* src = data;
    uint8_t* dest = (uint8_t*)neuronSignalTexture.getData();
    memcpy(dest, src, texture_bytes);
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


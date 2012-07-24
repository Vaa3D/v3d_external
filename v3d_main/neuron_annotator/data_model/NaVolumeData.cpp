#include "NaVolumeData.h"
#include "VolumeTexture.h"
#include <iostream>
#include <QFuture>
#include <cassert>
#include "../utility/ImageLoader.h"

#ifdef USE_FFMPEG
#include "../utility/loadV3dFFMpeg.h"
#endif

using namespace std;
using namespace jfrc;


/////////////////////////////////////////
// NaVolumeDataLoadableStack methods //
/////////////////////////////////////////

class SleepThread : QThread {
public:
    SleepThread() {}
    void msleep(int milliseconds) {
        QThread::msleep(milliseconds);
    }
};

NaVolumeDataLoadableStack::NaVolumeDataLoadableStack(My4DImage* stackpParam, QString filenameParam, int stackIndexParam)
   : QObject(NULL)
   , stackp(stackpParam)
   , filename(filenameParam)
   , stackIndex(stackIndexParam)
   , progressValue(0)
   , progressMin(0)
   , progressMax(100)
{
}

bool NaVolumeDataLoadableStack::load()
{
    setRelativeProgress(0.02f); // getting here *is* finite progress
    // qDebug() << "NaVolumeData::LoadableStack::load() filename=" << filename;
    QString fullFilepath=determineFullFilepath();
    if (! QFile(fullFilepath).exists())
        return false;
    ImageLoader imageLoader;
    imageLoader.setProgressIndex(stackIndex);
    connect(&imageLoader, SIGNAL(progressValueChanged(int,int)),
            this, SIGNAL(progressValueChanged(int,int)));
    connect(&imageLoader, SIGNAL(progressAborted(int)),
            this, SIGNAL(failed()));
    connect(&imageLoader, SIGNAL(progressMessageChanged(QString)),
            this, SIGNAL(progressMessageChanged(QString)));
    imageLoader.loadImage(stackp, fullFilepath);
    /*
    // stackp->isEmpty() is returning 'true' for correctly loaded images.
    // so I'm commenting out this block.
    if (stackp->isEmpty()) {
        emit failed();
        return false;
    }
    */
    setRelativeProgress(0.75);
    if (! stackp->p_vmin)
        stackp->updateminmaxvalues();
    setRelativeProgress(1.0);
    emit finished();
    return true;
}

void NaVolumeDataLoadableStack::setRelativeProgress(float relativeProgress)
{
    int newProgressValue = (int) (progressMin + relativeProgress * (progressMax - progressMin) + 0.5);
    assert(newProgressValue >= progressMin);
    assert(newProgressValue <= progressMax);
    if (newProgressValue == progressValue) return;
    progressValue = newProgressValue;
    emit progressValueChanged(progressValue, stackIndex);
}

QString NaVolumeDataLoadableStack::determineFullFilepath() const
{
    if (QFile(filename).exists())
        return filename;
    const char * extensions[] = {
#ifdef USE_FFMPEG
        ".mp4",
#endif
        ".v3dpbd",
        ".v3draw",
        ".tif",
        ".tif" // extra entry for when USE_FFMPEG is undefined
    };
    for (int e = 0; e < 4; ++e)
    {
        QString fn = filename + extensions[e];
        if (QFile(fn).exists())
            return fn;
    }
    return filename + ".tif"; // even though the file doesn't exist...
}



//////////////////////////
// NaVolumeData methods //
//////////////////////////

/* explicit */
NaVolumeData::NaVolumeData()
    : originalImageStack(NULL)
    , neuronMaskStack(NULL)
    , referenceStack(NULL)
    , emptyImage(new My4DImage())
    , originalImageProxy(emptyImage)
    , neuronMaskProxy(emptyImage)
    , referenceImageProxy(emptyImage)
    , currentProgress(0)
    , bDoUpdateSignalTexture(true)
    , volumeTexture(NULL)
    , doFlipY(true)
{
    // Connect specific signals to general ones
    connect(this, SIGNAL(channelLoaded(int)),
            this, SIGNAL(dataChanged()));
    connect(this, SIGNAL(channelsLoaded(int)),
            this, SIGNAL(dataChanged()));
    connect(this, SIGNAL(referenceLoaded()),
            this, SIGNAL(dataChanged()));
    connect(this, SIGNAL(neuronMaskLoaded()),
            this, SIGNAL(dataChanged()));
}

NaVolumeData::~NaVolumeData()
{
    bAbortWrite = true;
    Writer volumeWriter(*this); // Wait for readers to finish before deleting
    volumeWriter.clearImageData();
}

void NaVolumeData::setTextureInput(const VolumeTexture* texture)
{
    volumeTexture = texture;
}

bool NaVolumeData::loadVolumeFromTexture()
{
    if (NULL == volumeTexture) {
        emit progressAborted("Volume texture not found");
        return false;
    }
    bool bSucceeded = false;
    emit progressMessageChanged("Copying volume from 3D texture"); // emit outside of lock block
    {
        Writer writer(*this);
        if (writer.loadVolumeFromTexture(volumeTexture))
            bSucceeded = true;
    }

    if (bSucceeded) {
        bDoUpdateSignalTexture = false; // because it was set upstream
        emit channelsLoaded(3);
    }
    else {
        emit progressAborted("Volume load failed");
    }
    return bSucceeded;
}

/* slot */
void NaVolumeData::setStackLoadProgress(int progressValue, int stackIndex)
{
    // qDebug() << "setStackLoadProgress()" << progressValue << stackIndex;
    if (stackIndex < 0) {
        qDebug() << "stack index less than zero";
        return;
    }
    if (stackIndex >= stackLoadProgressValues.size()) {
        qDebug() << "stack index out of range";
        return;
    }
    if (progressValue == stackLoadProgressValues[stackIndex])
        return; // no change
    // TODO - use different weights for each stack depending on file size.
    stackLoadProgressValues[stackIndex] = progressValue;
    float totalProgressValue = 0.0;
    for (int i = 0; i < stackLoadProgressValues.size(); ++i) {
        totalProgressValue += stackLoadProgressValues[i] * 1.0 / stackLoadProgressValues.size();
    }
    setProgressValue((int) (totalProgressValue + 0.5));
}

/* slot */
void NaVolumeData::setProgressValue(int progressValue)
{
    if (progressValue < 0) return;
    if (progressValue > 100) return;
    if (progressValue == currentProgress) return;
    currentProgress = progressValue;
    // qDebug() << "NaVolumeData load progress =" << currentProgress;
    emit progressValueChanged(currentProgress);
}

// You might ask why I don't use My4DImage::flip(axis):
// Method My4DImage::flip(axis) does not work for multichannel images, and is 5 times slower
// than this flipY method.
void flipY(My4DImage* img)
{
    if (NULL == img) return;
    const long su = img->getUnitBytes();
    const long sx = img->getXDim();
    const long sy = img->getYDim();
    const long sz = img->getZDim();
    const long sc = img->getCDim();
    size_t rowBytes = su * sx;
    size_t sliceBytes = rowBytes * sy;
    size_t chanBytes = sliceBytes * sz;
    size_t halfY = sy / 2;
    std::vector<unsigned char> rowSwapBuf(rowBytes);
    unsigned char* rowBuf = &rowSwapBuf[0];
    // qDebug() << sx << sy << sz << sc << halfY;
    // qDebug() << img->getTotalBytes() << img->getTotalUnitNumber() << sx * sy * sz * sc << img->getTotalUnitNumberPerPlane() << sliceBytes;
    unsigned char* rawData = img->getRawData();
    unsigned char* chanPtr;
    unsigned char* slicePtr;
    for (int c = 0; c < sc; ++c)
    {
        chanPtr = rawData + c * chanBytes;
        for (int z = 0; z < sz; ++z)
        {
            slicePtr = chanPtr + z * sliceBytes;
            for (int y = 0; y <= halfY; ++y)
            {
                // swap scan line y with scan line (sz - y - 1)
                unsigned char* rowA = slicePtr + y * rowBytes;
                unsigned char* rowB = slicePtr + (sy - 1 - y) * rowBytes;
                if (rowA == rowB)
                    continue;
                memcpy(rowBuf, rowA, rowBytes);
                memcpy(rowA, rowB, rowBytes);
                memcpy(rowB, rowBuf, rowBytes);
            }
        }
    }
}

/* slot */
void NaVolumeData::loadVolumeDataFromFiles()
{
    QTime stopwatch;
    stopwatch.start();
    // qDebug() << "NaVolumeData::loadVolumeDataFromFiles()" << stopwatch.elapsed() << __FILE__ << __LINE__;

    bool stacksLoaded = false;
    emit progressMessageChanged("Loading image stack files"); // emit outside of lock block
    emit progressValueChanged(1); // show a bit of blue
    { // Allocate writer on the stack so write lock will be automatically released when method returns
        Writer volumeWriter(*this);
        // qDebug() << "NaVolumeData::loadVolumeDataFromFiles()" << stopwatch.elapsed() << __FILE__ << __LINE__;
        volumeWriter.clearImageData();
        // qDebug() << "NaVolumeData::loadVolumeDataFromFiles()" << stopwatch.elapsed() << __FILE__ << __LINE__;
        stacksLoaded = volumeWriter.loadStacks();
        // qDebug() << "NaVolumeData::loadVolumeDataFromFiles()" << stopwatch.elapsed() << __FILE__ << __LINE__;

        // Temporary kludge to counteract complicated flipping that occurs during neuron separation.
        if (stacksLoaded) {
            if (doFlipY) {
                qDebug() << "Loading image data into memory from disk took " << stopwatch.elapsed() / 1000.0 << " seconds";
                qDebug() << "Flipping Y-axis of images to compensate for unfortunate 2011-2012 data issues" << stopwatch.elapsed() << __FILE__ << __LINE__;
                // Data images are flipped relative to reference image.  I turned off flipping in
                // method NaVolumeData::Writer::normalizeReferenceStack(), rather than revert it here.
                emit benchmarkTimerPrintRequested("Starting to flip Y-axis");
                flipY(originalImageStack);
                flipY(neuronMaskStack);
                emit benchmarkTimerPrintRequested("Finished flipping Y-axis");
                // flipY(referenceStack);
                // qDebug() << stopwatch.elapsed();
            }
        }
    } // release locks before emit
    if (! stacksLoaded) {
        emit progressAborted(QString("Problem loading stacks"));
        return;
    }

    // nerd report
    size_t data_size = 0;
    data_size += originalImageStack->getTotalBytes();
    data_size += referenceStack->getTotalBytes();
    data_size += neuronMaskStack->getTotalBytes();
    qDebug() << "Loading 16-bit image data from disk took " << stopwatch.elapsed() / 1000.0 << " seconds";
    qDebug() << "Loading 16-bit image data from disk absorbed "
            << (double)data_size / double(1e6) << " MB of RAM"; // kibibytes boo hoo whatever...

    // bDoUpdateSignalTexture = true; // because it needs update now

    // qDebug() << "NaVolumeData::loadVolumeDataFromFiles()" << stopwatch.elapsed() / 1000.0 << "seconds" << __FILE__ << __LINE__;
    emit progressCompleted();
    // qDebug() << "emitting NaVolumeData::channelsLoaded" << __FILE__ << __LINE__;
    emit channelsLoaded(originalImageProxy.sc);
}

/* slot */
bool NaVolumeData::loadChannels(QString fileName) // includes loading general volumes
{
    bool bSucceeded = false;
    int channel_count = 0;
    emit progressMessageChanged("Loading single volume file"); // emit outside of lock block
    {
        Writer writer(*this);
        channel_count = writer.loadChannels(fileName);
        if (channel_count > 0)
            bSucceeded = true;
    } // release lock before emitting
    if (bSucceeded) {
        emit channelsLoaded(channel_count);
    }
    else
        emit progressAborted("Data stack load failed");
    return bSucceeded;
}

/* slot */
/*
bool NaVolumeData::loadSingleImageMovieVolume(QString fileName)
{
    bool bSucceeded = false;
    emit progressMessageChanged("Loading single volume file"); // emit outside of lock block
    emit progressValueChanged(1); // show a bit of blue
    {
        NaVolumeData::Writer volumeWriter(*this);
        bSucceeded = volumeWriter.loadSingleImageMovieVolume(fileName);
    } // release lock before emit
    if (bSucceeded) {
        bDoUpdateSignalTexture = true;
        emit progressCompleted();
        emit channelsLoaded(3);
    }
    else {
        emit progressAborted("Volume load failed");
    }
    return bSucceeded;
}
*/

/* slot */
bool NaVolumeData::loadReference(QString fileName)
{
    bool bSucceeded = false;
    emit progressMessageChanged("Loading reference channel"); // emit outside of lock block
    {
        Writer writer(*this);
        if (writer.loadReference(fileName))
            bSucceeded = true;
    }

    if (bSucceeded)
        emit referenceLoaded();
    else
        emit progressAborted("Reference load failed");
    return bSucceeded;
}

/* slot */
bool NaVolumeData::loadOneChannel(QString fileName, int channel_index) // includes loading general volumes
{
    bool bSucceeded = false;
    emit progressMessageChanged(QString("Loading data channel %1").arg(channel_index)); // emit outside of lock block
    {
        Writer writer(*this);
        if (writer.loadOneChannel(fileName, channel_index))
            bSucceeded = true;
    }
    if (bSucceeded)
        emit channelLoaded(channel_index);
    else
        emit progressAborted("Channel load failed");
    return bSucceeded;
}

/* slot */
bool NaVolumeData::loadNeuronMask(QString fileName)
{
    bool bSucceeded = false;
    emit progressMessageChanged("Loading neuron mask"); // emit outside of lock block
    {
        Writer writer(*this);
        if (writer.loadNeuronMask(fileName))
            bSucceeded = true;
    }
    if (bSucceeded)
        emit neuronMaskLoaded();
    else
        emit progressAborted("Neuron mask load failed");
    return bSucceeded;
}


//////////////////////////////////
// NaVolumeData::Writer methods //
//////////////////////////////////

void NaVolumeData::Writer::clearLandmarks()
{
    if (m_data->originalImageStack != NULL)
        m_data->originalImageStack->listLandmarks.clear();
}

void NaVolumeData::Writer::setLandmarks(const QList<LocationSimple> locations)
{
    if (m_data->originalImageStack != NULL)
        m_data->originalImageStack->listLandmarks = locations;
}

void NaVolumeData::Writer::clearImageData()
{
    if (m_data->originalImageStack != NULL) {
        delete m_data->originalImageStack;
        m_data->originalImageStack = NULL;
    }
    if (m_data->neuronMaskStack != NULL) {
        delete m_data->neuronMaskStack;
        m_data->neuronMaskStack = NULL;
    }
    if (m_data->referenceStack != NULL) {
        delete m_data->referenceStack;
        m_data->referenceStack = NULL;
    }
    if (m_data->emptyImage != NULL) {
        delete m_data->emptyImage;
        m_data->emptyImage = NULL;
    }
}

bool NaVolumeData::Writer::loadSingleImageMovieVolume(QString fileName)
{
    qDebug() << "NaVolumeData::Writer::loadSingleImageMovieVolume" << fileName;
#ifdef USE_FFMPEG
    My4DImage* img = new My4DImage();
    if (! loadStackFFMpeg(fileName.toStdString().c_str(), *img) ) {
        delete img;
        return false;
    }
    if (! setSingleImageVolume(img))
        return false;
    return true;
#else
    return false;
#endif
}

int NaVolumeData::Writer::loadChannels(QString fileName) // includes loading general volumes
{
    qDebug() << "NaVolumeData::Writer::loadChannels()" << fileName;
    My4DImage* img = new My4DImage();
    ImageLoader loader;
    if (! loader.loadImage(img, fileName) ) {
        delete img;
        return 0;
    }
    if (! setSingleImageVolume(img)) {
        delete img;
        return 0;
    }
    return img->getCDim();
}

bool NaVolumeData::Writer::setSingleImageVolume(My4DImage* img)
{
    qDebug() << "NaVolumeData::Writer::setSingleImageVolume";
    if (m_data->originalImageStack == img)
        return false; // no change
    if (NULL == img)
        return false;
    if (NULL != m_data->originalImageStack)
    {
        delete m_data->originalImageStack;
        m_data->originalImageStack = NULL;
    }
    m_data->originalImageStack = img;
    if (! img->p_vmin)
        img->updateminmaxvalues();
    m_data->originalImageProxy = Image4DProxy<My4DImage>(m_data->originalImageStack);
    m_data->originalImageProxy.set_minmax(m_data->originalImageStack->p_vmin, m_data->originalImageStack->p_vmax);
    return true;
}

// Convert two-channel image to three channels to avoid crash
My4DImage* ensureThreeChannel(My4DImage* input)
{
    if (NULL == input)
        return input;
    if (3 == input->getCDim())
        return input;
    qDebug() << "converting image to 3 channels" << __FILE__ << __LINE__;
    My4DImage* volImg = new My4DImage();
    volImg->createImage(
            input->getXDim(),
            input->getYDim(),
            input->getZDim(),
            3, // three color channels
            input->getDatatype()); // 1 => 8 bits per value
    size_t channelBytes = volImg->getXDim() * volImg->getYDim() * volImg->getZDim() * volImg->getUnitBytes();
    bool haveMinMax = (NULL != input->p_vmin);
    if (haveMinMax) {
        volImg->p_vmin = new double[3];
        volImg->p_vmax = new double[3];
    }
    for (int c = 0; c < 3; ++c)
    {
        int c_in = c;
        if (c_in >= input->getCDim())
            c_in = input->getCDim() - 1; // fill other channels with final channel
        memcpy(volImg->getRawData() + c * channelBytes,
               input->getRawData() + c_in * channelBytes,
               channelBytes);
        if (haveMinMax) {
            volImg->p_vmin[c] = input->p_vmin[c_in];
            volImg->p_vmax[c] = input->p_vmax[c_in];
        }
    }
    delete input;
    return volImg;
}

bool NaVolumeData::Writer::loadStacks()
{
    if (m_data->bAbortWrite) return false;
    QTime stopwatch;
    stopwatch.start();

    // Prepare to track progress of 3 file load operations
    m_data->stackLoadProgressValues.assign(3, 0);
    m_data->currentProgress = -1; // to make sure progress value changes on the next line
    m_data->setProgressValue(0);
    QCoreApplication::processEvents(); // ensure that progress bar gets displayed

    m_data->originalImageStack = new My4DImage();
    LoadableStack originalStack(m_data->originalImageStack, m_data->originalImageStackFilePath, 0);
    connect(&originalStack, SIGNAL(progressValueChanged(int, int)),
            m_data, SLOT(setStackLoadProgress(int, int)));
    connect(&originalStack, SIGNAL(progressMessageChanged(QString)),
            m_data, SIGNAL(progressMessageChanged(QString)));
    qDebug() << "NaVolumeData::Writer::loadStacks() starting originalStack.load()";
    // Pass stack pointer instead of stack reference to avoid problem with lack of QObject copy constructor.

    m_data->neuronMaskStack = new My4DImage();
    LoadableStack maskStack(m_data->neuronMaskStack, m_data->maskLabelFilePath, 1);
    connect(&maskStack, SIGNAL(progressValueChanged(int, int)),
            m_data, SLOT(setStackLoadProgress(int, int)));
    connect(&maskStack, SIGNAL(progressMessageChanged(QString)),
            m_data, SIGNAL(progressMessageChanged(QString)));
    qDebug() << "NaVolumeData::Writer::loadStacks() starting maskStack.load()";

    m_data->referenceStack = new My4DImage();
    My4DImage* initialReferenceStack = m_data->referenceStack;
    LoadableStack referenceStack(initialReferenceStack, m_data->referenceStackFilePath, 2);
    connect(&referenceStack, SIGNAL(progressValueChanged(int, int)),
            m_data, SLOT(setStackLoadProgress(int, int)));
    connect(&referenceStack, SIGNAL(progressMessageChanged(QString)),
            m_data, SIGNAL(progressMessageChanged(QString)));
    qDebug() << "NaVolumeData::Writer::loadStacks() starting referenceStack.load()";

    if (m_data->bAbortWrite) return false;

    // There are some bugs with multithreaded image loading, so make it an option.
    bool bUseMultithreadedLoader = true;
    // Tiff loading is not reentrant, so don't multithread tiff loading.
    int tiff_count = 0;
    if (originalStack.determineFullFilepath().endsWith(".tif"))
        ++tiff_count;
    if (maskStack.determineFullFilepath().endsWith(".tif"))
        ++tiff_count;
    if (referenceStack.determineFullFilepath().endsWith(".tif"))
        ++tiff_count;
    if (tiff_count > 1)
    {
        bUseMultithreadedLoader = false;
        qDebug() << "Using single thread loader because there are nonreentrant tiff files to load.";
    }
    if (bUseMultithreadedLoader)
    {
        // Load each file in a separate thread.  This assumes that loading code is reentrant...
        QList< QFuture<void> > loaderList;

        QFuture<void> originalLoader = QtConcurrent::run(&originalStack, &LoadableStack::load);
        loaderList.append(originalLoader);

        QFuture<void> maskLoader = QtConcurrent::run(&maskStack, &LoadableStack::load);
        loaderList.append(maskLoader);

        QFuture<void> referenceLoader = QtConcurrent::run(&referenceStack, &LoadableStack::load);
        loaderList.append(referenceLoader);


        while(1) {
            SleepThread st;
            st.msleep(1000);
            int doneCount=0;
            for (int i=0;i<loaderList.size();i++) {
                QFuture<void> loader=loaderList.at(i);
                if (loader.isFinished()) {
                    doneCount++;
                }
            }
            int stillActive=loaderList.size()-doneCount;
            if (stillActive==0) {
                break;
            } else {
                qDebug() << "Waiting on " << stillActive << " loaders";
            }
            QCoreApplication::processEvents(); // let progress signals through
        }
        if (m_data->bAbortWrite) return false;
    }
    else {
        // Non-threaded sequential loading
        m_data->setProgressMessage("Loading multicolor brain images...");
        if (! originalStack.load()) {
            qDebug() << "ERROR loading signal volume" << m_data->originalImageStackFilePath;
            return false;
        }
        if (m_data->bAbortWrite) return false;
        m_data->setProgressMessage("Loading neuron fragment locations...");
        if (! maskStack.load()) {
            qDebug() << "ERROR loading label volume" << m_data->maskLabelFilePath;
            return false;
        }
        if (m_data->bAbortWrite) return false;
        m_data->setProgressMessage("Loading nc82 synaptic reference image...");
        if (! referenceStack.load()) {
            qDebug() << "ERROR loading reference volume" << m_data->referenceStackFilePath;
            return false;
        }
        if (m_data->bAbortWrite) return false;
    }

    // Convert 2-channel image to 3-channels to avoid crash
    m_data->originalImageStack = ensureThreeChannel(m_data->originalImageStack);

    qDebug() << "NaVolumeData::Writer::loadStacks() done loading all stacks in " << stopwatch.elapsed() / 1000.0 << " seconds";

    if (! m_data->originalImageStack->p_vmin)
        m_data->originalImageStack->updateminmaxvalues();
    m_data->originalImageProxy = Image4DProxy<My4DImage>(m_data->originalImageStack);
    m_data->originalImageProxy.set_minmax(m_data->originalImageStack->p_vmin, m_data->originalImageStack->p_vmax);

    if (! m_data->neuronMaskStack->p_vmin)
        m_data->neuronMaskStack->updateminmaxvalues();
    m_data->neuronMaskProxy = Image4DProxy<My4DImage>(m_data->neuronMaskStack);
    m_data->neuronMaskProxy.set_minmax(m_data->neuronMaskStack->p_vmin, m_data->neuronMaskStack->p_vmax);

    if (m_data->bAbortWrite) return false;

    // qDebug() << "Calling normalizeReferenceStack...";
    // normalizeReferenceStack(initialReferenceStack);
    // qDebug() << "Done calling normalizeReferenceStack";
    if (! m_data->referenceStack->p_vmin)
        m_data->referenceStack->updateminmaxvalues();
    m_data->referenceImageProxy = Image4DProxy<My4DImage>(m_data->referenceStack);
    m_data->referenceImageProxy.set_minmax(m_data->referenceStack->p_vmin, m_data->referenceStack->p_vmax);

    return true;
}

bool NaVolumeData::Writer::loadReference(QString fileName)
{
    assert(false); // TODO
    return false;
}

bool NaVolumeData::Writer::loadOneChannel(QString fileName, int channel_index) // includes loading general volumes
{
    assert(false); // TODO
    return false;
}

bool NaVolumeData::Writer::loadNeuronMask(QString fileName)
{
    assert(false); // TODO
    return false;
}

bool NaVolumeData::Writer::loadVolumeFromTexture(const VolumeTexture* texture)
{
    // TODO
    qDebug() << "NaVolumeData::Writer::loadVolumeFromTexture()"
            << __FILE__ << __LINE__;
    if (NULL == texture)
        return false;
    {
        QTime stopwatch;
        stopwatch.start();
        jfrc::VolumeTexture::Reader textureReader(*texture); // acquire read lock
        jfrc::Dimension size = textureReader.paddedTextureSize();
        size_t sx = size.x();
        size_t sy = size.y();
        size_t sz = size.z();
        My4DImage* volImg = new My4DImage();
        volImg->createImage(sx, sy, sz,
                3, // RGB
                V3D_UINT8); // 1 => 8 bits per value
        My4DImage* refImg = new My4DImage();
        refImg->createImage(sx, sy, sz,
                1,
                V3D_UINT8);
        // TODO - perform multithreaded copy in z slabs
        // Precomputing these offsets speed debug mode from
        // 7 seconds for loop to 1.1 seconds for loop
        uint8_t* red_offset = volImg->getRawData() + 0 * sx * sy * sz;
        uint8_t* green_offset = volImg->getRawData() + 1 * sx * sy * sz;
        uint8_t* blue_offset = volImg->getRawData() + 2 * sx * sy * sz;
        uint8_t* nc82_offset = refImg->getRawData() + 0 * sx * sy * sz;
        const uint8_t* textureData = (const uint8_t*)textureReader.signalData3D();
        for (int z = 0; z < sz; ++z) {
            const uint8_t* z_offset1 = textureData + z * sx * sy * 4;
            uint8_t* red_z_offset = red_offset + z * sx * sy * 1;
            uint8_t* green_z_offset = green_offset + z * sx * sy * 1;
            uint8_t* blue_z_offset = blue_offset + z * sx * sy * 1;
            uint8_t* nc82_z_offset = nc82_offset + z * sx * sy * 1;
            for (int y = 0; y < sy; ++y) {
                const uint8_t* y_offset1 = z_offset1 + y * sx * 4;
                uint8_t* red = red_z_offset + y * sx * 1;
                uint8_t* green = green_z_offset + y * sx * 1;
                uint8_t* blue = blue_z_offset + y * sx * 1;
                uint8_t* nc82 = nc82_z_offset + y * sx * 1;
                for (int x = 0; x < sx; ++x) {
                    const uint8_t* rgba_x = y_offset1 + x * 4;
                    red[x] = rgba_x[2]; // texture order is BGRA, so swap R/B
                    green[x] = rgba_x[1];
                    blue[x] = rgba_x[0]; // texture order is BGRA, so swap R/B
                    nc82[x] = rgba_x[3];
                }
            }
        }
        qDebug() << "Copying texture took" << stopwatch.elapsed() << "milliseconds";
        stopwatch.restart();
        setSingleImageVolume(volImg);
        qDebug() << "setSingleImageVolume took" << stopwatch.elapsed() << "milliseconds";
    } // release read lock
    // TODO - copy RGBA to data and reference in multiple threads

    return true;
}


//////////////////////////////////
// NaVolumeData::Reader methods //
//////////////////////////////////

const Image4DProxy<My4DImage>& NaVolumeData::Reader::getNeuronMaskProxy() const
{
    return m_data->neuronMaskProxy;
}

const Image4DProxy<My4DImage>& NaVolumeData::Reader::getOriginalImageProxy() const
{
    return m_data->originalImageProxy;
}

const Image4DProxy<My4DImage>& NaVolumeData::Reader::getReferenceImageProxy() const
{
    return m_data->referenceImageProxy;
}

bool NaVolumeData::Reader::doUpdateSignalTexture() const
{
    return m_data->bDoUpdateSignalTexture;
}



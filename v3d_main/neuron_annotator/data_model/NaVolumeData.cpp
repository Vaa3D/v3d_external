#include "NaVolumeData.h"
#include <iostream>
#include <QFuture>
#include <cassert>

#include "../utility/ImageLoader.h"
#include "../utility/loadV3dFFMpeg.h"

using namespace std;


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

QString NaVolumeDataLoadableStack::determineFullFilepath()
{
    QString f1=filename;
    QString v3dpbdTestFilename=f1.append(".v3dpbd");
    QFile v3dpbdTestFile(v3dpbdTestFilename);
    if (v3dpbdTestFile.exists())
        return v3dpbdTestFilename;

    QString f2=filename;
    QString v3drawTestFilename=f2.append(".v3draw");
    QFile v3drawTestFile(v3drawTestFilename);
    if (v3drawTestFile.exists())
        return v3drawTestFilename;

    QString f3=filename;
    QString tifFilename=f3.append(".tif");
    return tifFilename;
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
{
}

NaVolumeData::~NaVolumeData()
{
    bAbortWrite = true;
    Writer volumeWriter(*this); // Wait for readers to finish before deleting
    volumeWriter.clearImageData();
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
            qDebug() << "Flipping Y-axis of images to compensate for unfortunate 2011-2012 data issues" << stopwatch.elapsed() << __FILE__ << __LINE__;
            flipY(originalImageStack);
            flipY(neuronMaskStack);
            // Data images are flipped relative to reference image.  I turned off flipping in
            // method NaVolumeData::Writer::normalizeReferenceStack(), rather than revert it here.
            // flipY(referenceStack);
            // qDebug() << stopwatch.elapsed();

            // populate histograms
            histograms.assign((size_t)(originalImageProxy.sc + 1), IntensityHistogram());
            for(int c = 0; c < originalImageProxy.sc; ++c)
                histograms[c].populate(originalImageProxy, c);
            histograms[originalImageProxy.sc].populate(referenceImageProxy);
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


    // qDebug() << "NaVolumeData::loadVolumeDataFromFiles()" << stopwatch.elapsed() / 1000.0 << "seconds" << __FILE__ << __LINE__;
    emit progressCompleted();
    // qDebug() << "emitting NaVolumeData::dataChanged" << __FILE__ << __LINE__;
    emit dataChanged();
}

/* slot */
bool NaVolumeData::loadSingleImageMovieVolume(QString fileName)
{
    NaVolumeData::Writer volumeWriter(*this);
    if (! volumeWriter.loadSingleImageMovieVolume(fileName) )
        return false;
    emit dataChanged();
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
    return setSingleImageVolume(img);
#else
    return false;
#endif
}

bool NaVolumeData::Writer::setSingleImageVolume(My4DImage* img)
{
    qDebug() << "NaVolumeData::Writer::loadSingleImageMovieVolume";
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
    img->updateminmaxvalues();
    m_data->originalImageProxy = Image4DProxy<My4DImage>(m_data->originalImageStack);
    m_data->originalImageProxy.set_minmax(m_data->originalImageStack->p_vmin, m_data->originalImageStack->p_vmax);
    return true;
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
    bool bUseMultithreadedLoader = false;
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
    }
    else {
        // Non-threaded sequential loading
        m_data->setProgressMessage("Loading multicolor brain images...");
        originalStack.load();
        if (m_data->bAbortWrite) return false;
        m_data->setProgressMessage("Loading neuron fragment locations...");
        maskStack.load();
        if (m_data->bAbortWrite) return false;
        m_data->setProgressMessage("Loading nc82 synaptic reference image...");
        referenceStack.load();
        if (m_data->bAbortWrite) return false;
    }

    qDebug() << "NaVolumeData::Writer::loadStacks() done loading all stacks in " << stopwatch.elapsed() / 1000.0 << " seconds";

    // m_data->originalImageStack->updateminmaxvalues();
    m_data->originalImageProxy = Image4DProxy<My4DImage>(m_data->originalImageStack);
    m_data->originalImageProxy.set_minmax(m_data->originalImageStack->p_vmin, m_data->originalImageStack->p_vmax);

    // m_data->neuronMaskStack->updateminmaxvalues();
    m_data->neuronMaskProxy = Image4DProxy<My4DImage>(m_data->neuronMaskStack);
    m_data->neuronMaskProxy.set_minmax(m_data->neuronMaskStack->p_vmin, m_data->neuronMaskStack->p_vmax);

    if (m_data->bAbortWrite) return false;

    // qDebug() << "Calling normalizeReferenceStack...";
    // normalizeReferenceStack(initialReferenceStack);
    // qDebug() << "Done calling normalizeReferenceStack";
    m_data->referenceStack->updateminmaxvalues();
    m_data->referenceImageProxy = Image4DProxy<My4DImage>(m_data->referenceStack);
    m_data->referenceImageProxy.set_minmax(m_data->referenceStack->p_vmin, m_data->referenceStack->p_vmax);

    return true;
}

//////////////////////////////////
// NaVolumeData::Reader methods //
//////////////////////////////////

const IntensityHistogram& NaVolumeData::Reader::getHistogram(int channelIndex) const
{
    return m_data->histograms[channelIndex];
}

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



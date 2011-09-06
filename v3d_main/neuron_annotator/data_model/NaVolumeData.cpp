#include "NaVolumeData.h"
#include <iostream>
#include <QFuture>

using namespace std;


/////////////////////////////////////////
// NaVolumeData::LoadableStack methods //
/////////////////////////////////////////

class SleepThread : QThread {
public:
    SleepThread() {}
    void msleep(int milliseconds) {
        QThread::msleep(milliseconds);
    }
};

NaVolumeData::LoadableStack::LoadableStack(My4DImage* stackp, QString filename)
{
    this->stackp=stackp;
    this->filename=filename;
}

bool NaVolumeData::LoadableStack::load()
{
    qDebug() << "NaVolumeData::LoadableStack::load() filename=" << filename;
    stackp->loadImage(filename.toAscii().data());
    if (stackp->isEmpty()) {
        return false;
    }
    stackp->updateminmaxvalues();
    return true;
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
{
}

NaVolumeData::~NaVolumeData()
{
    Writer volumeWriter(*this); // Wait for readers to finish before deleting
    volumeWriter.clearImageData();
}

/* slot */
void NaVolumeData::loadVolumeDataFromFiles()
{
    QTime stopwatch;
    stopwatch.start();

    // Allocate writer on the stack so write lock will be automatically released when method returns
    {
        Writer volumeWriter(*this);
        volumeWriter.clearImageData();

        if (!volumeWriter.loadStacks()) {
            volumeWriter.unlock();
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

        volumeWriter.unlock();
    } // release locks before emit
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

bool NaVolumeData::Writer::loadStacks()
{
    QTime stopwatch;
    stopwatch.start();

    QList< QFuture<void> > loaderList;

    m_data->originalImageStack = new My4DImage();
    LoadableStack originalStack(m_data->originalImageStack, m_data->originalImageStackFilePath);
    qDebug() << "NaVolumeData::Writer::loadStacks() starting originalStack.load()";
    QFuture<void> originalLoader = QtConcurrent::run(originalStack, &LoadableStack::load);
    loaderList.append(originalLoader);

    m_data->neuronMaskStack = new My4DImage();
    LoadableStack maskStack(m_data->neuronMaskStack, m_data->maskLabelFilePath);
    qDebug() << "NaVolumeData::Writer::loadStacks() starting maskStack.load()";
    QFuture<void> maskLoader = QtConcurrent::run(maskStack, &LoadableStack::load);
    loaderList.append(maskLoader);

    My4DImage* initialReferenceStack = new My4DImage();
    LoadableStack referenceStack(initialReferenceStack, m_data->referenceStackFilePath);
    qDebug() << "NaVolumeData::Writer::loadStacks() starting referenceStack.load()";
    QFuture<void> referenceLoader = QtConcurrent::run(referenceStack, &LoadableStack::load);
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
    }

    qDebug() << "NaVolumeData::Writer::loadStacks() done loading all stacks in " << stopwatch.elapsed() / 1000.0 << " seconds";

    m_data->originalImageProxy = Image4DProxy<My4DImage>(m_data->originalImageStack);
    m_data->originalImageProxy.set_minmax(m_data->originalImageStack->p_vmin, m_data->originalImageStack->p_vmax);

    m_data->neuronMaskProxy = Image4DProxy<My4DImage>(m_data->neuronMaskStack);
    m_data->neuronMaskProxy.set_minmax(m_data->neuronMaskStack->p_vmin, m_data->neuronMaskStack->p_vmax);

    normalizeReferenceStack(initialReferenceStack);

    return true;
}

bool NaVolumeData::Writer::normalizeReferenceStack(My4DImage* initialReferenceStack)
{
    m_data->referenceStack=new My4DImage();
    My4DImage * referenceStack = m_data->referenceStack;
    referenceStack->loadImage(initialReferenceStack->getXDim(), initialReferenceStack->getYDim(), initialReferenceStack->getZDim(), 1 /* number of channels */, 1 /* bytes per channel */);
    Image4DProxy<My4DImage> initialProxy(initialReferenceStack);
    Image4DProxy<My4DImage> referenceProxy(referenceStack);

    double initialMin=initialReferenceStack->getChannalMinIntensity(0);
    double initialMax=initialReferenceStack->getChannalMaxIntensity(0);

    qDebug() << "Populating reference with initial data";
    double initialRange=initialMax-initialMin;
    qDebug() << "Reference lsm initialMin=" << initialMin << " initialMax=" << initialMax << " initialRange=" << initialRange;
    int zDim=initialReferenceStack->getZDim();
    int yDim=initialReferenceStack->getYDim();
    int xDim=initialReferenceStack->getXDim();
    for (int z=0;z<zDim;z++) {
        for (int y=0;y<yDim;y++) {
            for (int x=0;x<xDim;x++) {
                int value= (255.0*(*initialProxy.at_uint16(x,y,z,0))-initialMin)/initialRange;
                if (value<0) {
                    value=0;
                } else if (value>255) {
                    value=255;
                }
                referenceProxy.put8bit_fit_at(x,(yDim-y)-1,z,0,value); // For some reason, the Y-dim seems to need inversion
            }
        }
    }
    initialReferenceStack->cleanExistData();
    delete initialReferenceStack;
    referenceStack->updateminmaxvalues();

    m_data->referenceImageProxy = Image4DProxy<My4DImage>(m_data->referenceStack);
    m_data->referenceImageProxy.set_minmax(m_data->referenceStack->p_vmin, m_data->referenceStack->p_vmax);

    qDebug() << "Finished loading reference stack";

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



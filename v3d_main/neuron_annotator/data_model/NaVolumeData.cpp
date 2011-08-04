#include "NaVolumeData.h"
#include <iostream>

using namespace std;


//////////////////////////
// NaVolumeData methods //
//////////////////////////

/* explicit */
NaVolumeData::NaVolumeData(QObject * parentParam /* = NULL */)
    : NaLockableData(parentParam)
    , originalImageStack(NULL)
    , neuronMaskStack(NULL)
    , referenceStack(NULL)
    , maxNeuronIndex(-1)
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
    Writer volumeWriter(*this);
    volumeWriter.clearImageData();

    if (! volumeWriter.loadOriginalImageStack()) {
        volumeWriter.unlock(); // unlock before emit
        emit progressAborted(QString("Problem loading volume image"));
        return;
    }

    if (! volumeWriter.loadNeuronMaskStack()) {
        volumeWriter.unlock(); // unlock before emit
        emit progressAborted(QString("Problem loading neuron mask"));
        return;
    }

    if (! volumeWriter.loadReferenceStack()) {
        volumeWriter.unlock(); // unlock before emit
        emit progressAborted(QString("Problem loading reference image"));
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

    volumeWriter.unlock(); // unlock before emit
    emit dataChanged();
}

//////////////////////////////////
// NaVolumeData::Writer methods //
//////////////////////////////////

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
    m_data->maxNeuronIndex = -1;
}

bool NaVolumeData::Writer::loadOriginalImageStack()
{
    QString msgPrefix("NaVolumeData::loadOriginalImageStack()");
    m_data->originalImageStack = new My4DImage;
    My4DImage * originalImageStack = m_data->originalImageStack;
    if (!originalImageStack) {
        cerr << msgPrefix.toStdString() << " : problem creating My4DImage" << endl;
        return false;
    }
    originalImageStack->loadImage(m_data->originalImageStackFilePath.toAscii().data());
    if (originalImageStack->isEmpty()) {
        cerr << msgPrefix.toStdString() << ": originalImageStack is empty after loading\n";
        return false;
    }
    cout << "Loaded original image stack with dimensions X=" << originalImageStack->getXDim() << " Y=" << originalImageStack->getYDim()
            << " Z=" << originalImageStack->getZDim() << " C=" << originalImageStack->getCDim() << "\n";
    return true;
}

bool NaVolumeData::Writer::loadNeuronMaskStack() {
    QString msgPrefix("NaVolumeData::loadNeuronMaskStack()");
    if (m_data->originalImageStack==0) {
        cerr << msgPrefix.toStdString() << " error : originalImageStack must be created before this function is called" << endl;
        return false;
    }
    m_data->neuronMaskStack = new My4DImage;
    My4DImage * neuronMaskStack = m_data->neuronMaskStack;
    if (!neuronMaskStack) {
        cerr << msgPrefix.toStdString() << " : problem creating My4DImage" << endl;
        return false;
    }
    neuronMaskStack->loadImage(m_data->maskLabelFilePath.toAscii().data());

    // measure range of neuron fragment indices
    double minFragmentIndex = 1e9;
    double maxFragmentIndex = -1e9;
    Image4DProxy<My4DImage> maskProxy(const_cast<My4DImage*>(m_data->neuronMaskStack));
    int imageX = m_data->neuronMaskStack->getXDim();
    int imageY = m_data->neuronMaskStack->getYDim();
    int imageZ = m_data->neuronMaskStack->getZDim();
    for (int z = 0; z < imageZ; z++) {
        for (int y = 0; y < imageY; y++) {
            for (int x = 0; x < imageX; x++) {
                double fragmentIndex = maskProxy.value_at(x,y,z,0);
                if (fragmentIndex > maxFragmentIndex)
                    maxFragmentIndex = fragmentIndex;
                if (fragmentIndex < minFragmentIndex)
                    minFragmentIndex = fragmentIndex;
            }
        }
    }
    qDebug() << "Maximum fragment index = " << maxFragmentIndex;
    qDebug() << "Minimum fragment index = " << minFragmentIndex;
    m_data->maxNeuronIndex = maxFragmentIndex;

    return true;
}

bool NaVolumeData::Writer::loadReferenceStack()
{
    // Phase 1: load the data
    QString msgPrefix("NaVolumeData::loadReferenceStack()");
    qDebug() << msgPrefix << " : start";
    My4DImage* initialReferenceStack=new My4DImage();
    initialReferenceStack->loadImage(m_data->referenceStackFilePath.toAscii().data());
    if (initialReferenceStack->isEmpty()) {
        cerr << msgPrefix.toStdString() << ": initialReferenceStack is empty after loading\n";
        return false;
    }
    cout << "Loaded reference stack stack with dimensions X=" << initialReferenceStack->getXDim() << " Y=" << initialReferenceStack->getYDim()
            << " Z=" << initialReferenceStack->getZDim() << " C=" << initialReferenceStack->getCDim() << "\n";

    // Phase 2: normalize to 8-bit
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
    qDebug() << "Finished loading reference stack";

    return true;
}

//////////////////////////////////
// NaVolumeData::Reader methods //
//////////////////////////////////

const Image4DProxy<My4DImage> NaVolumeData::Reader::getNeuronMaskProxy() const
{
    // Image4DProxy class is a bit const-retarded, so const_cast here.
    // (It's OK; we are returning a const object in the end.)
    return Image4DProxy<My4DImage>(const_cast<My4DImage*>(m_data->neuronMaskStack));
}

const Image4DProxy<My4DImage> NaVolumeData::Reader::getOriginalImageProxy() const
{
    // Image4DProxy class is a bit const-retarded, so const_cast here.
    // (It's OK; we are returning a const object in the end.)
    return Image4DProxy<My4DImage>(const_cast<My4DImage*>(m_data->originalImageStack));
}

const Image4DProxy<My4DImage> NaVolumeData::Reader::getReferenceImageProxy() const
{
    // Image4DProxy class is a bit const-retarded, so const_cast here.
    // (It's OK; we are returning a const object in the end.)
    return Image4DProxy<My4DImage>(const_cast<My4DImage*>(m_data->referenceStack));
}



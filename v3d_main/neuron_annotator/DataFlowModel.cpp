#include "DataFlowModel.h"
#include "utility/FooDebug.h"
#include "utility/url_tools.h"
#include <QtAlgorithms>
#include <iostream>
#include <cassert>

using namespace std;

const int DataFlowModel::REFERENCE_MIP_INDEX=0;
const int DataFlowModel::BACKGROUND_MIP_INDEX=1;

/* explicit */
DataFlowModel::DataFlowModel(QObject* parentParam /* = NULL */)
    : QObject(parentParam)
    , multiColorImageStackNode(NULL)
    , neuronAnnotatorResultNode(NULL)
    // Allocate data flow objects, in order, to automatically set up multithreaded data stream
    , volumeData(/* this */) // load from disk (cannot move qobject with a parent to a QThread)
    , neuronSelectionModel(volumeData) // which layers are shown?
    , dataColorModel(volumeData) // choose colors
    , neuronFragmentData(volumeData)
    , zSliceColors(volumeData, dataColorModel, neuronSelectionModel)
    , mipFragmentData(volumeData /* , this */) // project in Z, slice on fragment index
    , mipFragmentColors(mipFragmentData, dataColorModel) // color 'em
    , galleryMipImages(mipFragmentColors) // shrink 'em
    , mipMergedData(volumeData, mipFragmentData, dataColorModel, neuronSelectionModel)
{
    // Prepare to load 16-bit volume data from disk in a separate QThread
    connect(this, SIGNAL(volumeDataNeeded()),
            &volumeData, SLOT(loadVolumeDataFromFiles()));

    volumeTexture.setDataFlowModel(this);

    // qDebug() << "Address of regular data color model =" << dataColorModel.dataPtr();
    // qDebug() << "Address of fast 3D data color model =" << fast3DColorModel.dataPtr();
    // qDebug() << "Address of slow 3D data color model =" << slow3DColorModel.dataPtr();

    // wire up 3d viewer fast color update system
    fast3DColorModel.setIncrementalColorSource(dataColorModel, slow3DColorModel);
    dataColorModel.initializeRgba48();
    slow3DColorModel.initializeRgba48();

    slow3DColorModel.setDataFlowModel(this);
    connect(&getVolumeTexture(), SIGNAL(signalMetadataChanged()),
            &getSlow3DColorModel(), SLOT(updateVolumeTextureMetadata()));

    volumeData.setTextureInput(&volumeTexture);
    // TODO - wire this up
    // connect(&volumeTexture, SIGNAL(volumeLoadSequenceCompleted()),
    //         &volumeData, SLOT(continueStagedLoad()));

    connect(&volumeData, SIGNAL(benchmarkTimerPrintRequested(QString)),
            this, SIGNAL(benchmarkTimerPrintRequested(QString)));
    connect(&volumeData, SIGNAL(benchmarkTimerResetRequested()),
            this, SIGNAL(benchmarkTimerResetRequested()));
    connect(&volumeTexture, SIGNAL(benchmarkTimerPrintRequested(QString)),
            this, SIGNAL(benchmarkTimerPrintRequested(QString)));
    connect(&volumeTexture, SIGNAL(benchmarkTimerResetRequested()),
            this, SIGNAL(benchmarkTimerResetRequested()));
#ifdef USE_FFMPEG
    connect(&fast3DTexture, SIGNAL(benchmarkTimerPrintRequested(QString)),
            this, SIGNAL(benchmarkTimerPrintRequested(QString)));
    connect(&fast3DTexture, SIGNAL(benchmarkTimerResetRequested()),
            this, SIGNAL(benchmarkTimerResetRequested()));
#endif

    // For debugging
    // connect(&dataColorModel, SIGNAL(dataChanged()),
    //         this, SLOT(debugColorModel()));
    // Kludge to keep color models synchronized
    connect(&dataColorModel, SIGNAL(dataChanged()),
            this, SLOT(synchronizeColorModels()));
}

DataFlowModel::~DataFlowModel()
{
    cancel();
    NaVolumeData::Writer writer(getVolumeData());
    if (multiColorImageStackNode!=0) {
        delete multiColorImageStackNode;
    }
    if (neuronAnnotatorResultNode!=0) {
        delete neuronAnnotatorResultNode;
    }
}

/* slot */
void DataFlowModel::cancel()
{
    mipMergedData.invalidate();
    volumeData.invalidate();
}

/* slot */
// kludge to accomodate disparate color models caused by
// different orders and types of staged loaded files.
// Keeps the data range roughly the same between
// 1) dataColorModel and 2) slow3DColorModel
void DataFlowModel::synchronizeColorModels()
{
    // We might want to change slow data/hdr ranges to
    // match regular DataColorModel.
    // -1 means "don't change"
    float dataMaxima[4] = {-1,-1,-1,-1};
    float dataMinima[4] = {-1,-1,-1,-1};
    float hdrMaxima[4]  = {-1,-1,-1,-1};
    float hdrMinima[4]  = {-1,-1,-1,-1};
    {
        DataColorModel::Reader reader1(dataColorModel);
        if (dataColorModel.readerIsStale(reader1))
            return;
        DataColorModel::Reader reader3(slow3DColorModel);
        if (slow3DColorModel.readerIsStale(reader3))
            return;
        for (int c = 0; c < reader1.getNumberOfDataChannels(); ++c)
        {
            if (c > 3) break; // more than 4 channels?!
            float ratio = reader1.getChannelDataMax(c) /
                    float(reader3.getChannelDataMax(c));
            if ((ratio > 4.0) || (ratio < 0.25)) {
                // color models are out of sync
                // but don't actually change the data until the read locks are released
                dataMaxima[c] = int(ratio * reader3.getChannelDataMax(c));
                dataMinima[c] = int(ratio * reader3.getChannelDataMin(c));
                hdrMaxima[c] = int(ratio * reader3.getChannelHdrMax(c));
                hdrMinima[c] = int(ratio * reader3.getChannelHdrMin(c));
            }
        }
    } // release read locks
    for (int c = 0; c < 4; ++c)
    {
        if (dataMaxima[c] == -1) continue;
        {
            DataColorModel::Writer writer(slow3DColorModel);
            slow3DColorModel.setChannelDataRange(c, dataMinima[c], dataMaxima[c]);
            slow3DColorModel.setChannelHdrRange(c, hdrMinima[c], hdrMaxima[c]);
        }
    }
}

/* slot */
void DataFlowModel::debugColorModel()
{
    {
        DataColorModel::Reader reader1(dataColorModel);
        DataColorModel::Reader reader2(fast3DColorModel);
        DataColorModel::Reader reader3(slow3DColorModel);
        fooDebug()
                << "Color model red data range ="
                << reader1.getChannelDataMin(0)
                << reader1.getChannelDataMax(0)
                << "hdr range ="
                << reader1.getChannelHdrMin(0)
                << reader1.getChannelHdrMax(0);
        fooDebug()
                << "Fast model red data range ="
                << reader2.getChannelDataMin(0)
                << reader2.getChannelDataMax(0)
                << "hdr range ="
                << reader2.getChannelHdrMin(0)
                << reader2.getChannelHdrMax(0);
        fooDebug()
                << "Slow model red data range ="
                << reader3.getChannelDataMin(0)
                << reader3.getChannelDataMax(0)
                << "hdr range ="
                << reader3.getChannelHdrMin(0)
                << reader3.getChannelHdrMax(0);
    }
}

bool DataFlowModel::loadLsmMetadata()
{
    if (NULL == multiColorImageStackNode)
        return false;
    QList<QUrl> lsmMetadataFilepathList=multiColorImageStackNode->getPathsToLsmMetadataFiles();
    if (lsmMetadataFilepathList.size()==0) {
        // Empty list is OK, as long as voxel size is isotropic, or
        // if Optical Resolution is availabel from entity model.
        // qDebug() << "DataFlowModel::loadLsmMetadata() received empty list of lsm metadata files";
        return false;
    } else {
        UrlStream stream(lsmMetadataFilepathList.at(0));
        if (stream.io() == NULL)
            return false;
        QStringList fileContents;
        while(! stream.io()->atEnd()) {
            fileContents.append(stream.io()->readLine());
        }
        bool parseSuccess=false;
        for (int i=fileContents.size()-1;i>=0;i--) {
            QString line=fileContents.at(i);
            if (line.trimmed().length()>0) {
                QStringList doubleArgs = line.trimmed().split(QRegExp("\\s+"));
                if (doubleArgs.length()!=3) {
                    qDebug() << "Could not parse line which was expected to have 3 doubles = " << line;
                    for (int j=0;j<doubleArgs.length();j++) {
                        qDebug() << j << " " << doubleArgs.at(j);
                    }
                    return false;
                }
                QString d1String=doubleArgs.at(0);
                double d1=d1String.toDouble();
                QString d2String=doubleArgs.at(1);
                double d2=d2String.toDouble();
                QString d3String=doubleArgs.at(2);
                double d3=d3String.toDouble();
                setZRatio(d3/d1);
                // zRatio=d3/d1;
                // qDebug() << "Using lsm dimension ratios " << d1 << " " << d2 << " " << d3 << " setting zRatio=" << zRatio;
                parseSuccess=true;
                break;
            }
        }
        if (!parseSuccess) {
            qDebug() << "DataFlowModel::loadLsmMetadata could not parse file to determine zRatio";
            return false;
        }
        return true;
    }

}

bool DataFlowModel::loadVolumeData()
{
    {
        // Allocate writer on the stack so write lock will be automatically released when method returns
        NaVolumeData::Writer volumeWriter(volumeData);

        // Set file names of image files so VolumeData will know what to load.
        volumeWriter.setOriginalImageStackFileUrl(
                multiColorImageStackNode->getPathToOriginalImageStackFile());
        volumeWriter.setMaskLabelFileUrl(
                multiColorImageStackNode->getPathToMulticolorLabelMaskFile());
        volumeWriter.setReferenceStackFileUrl(
                multiColorImageStackNode->getPathToReferenceStackFile());
    } // release locks before emit
    emit volumeDataNeeded(); // load data in a separate QThread

    return true;
}



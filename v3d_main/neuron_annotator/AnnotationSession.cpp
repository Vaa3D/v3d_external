#include "AnnotationSession.h"
#include "gui/RendererNeuronAnnotator.h"
#include <QtAlgorithms>
#include <iostream>
#include <cassert>

using namespace std;

const int AnnotationSession::REFERENCE_MIP_INDEX=0;
const int AnnotationSession::BACKGROUND_MIP_INDEX=1;

/* explicit */
AnnotationSession::AnnotationSession(QObject* parentParam /* = NULL */)
    : QObject(parentParam)
    , multiColorImageStackNode(NULL)
    , neuronAnnotatorResultNode(NULL)
    , volumeData(/* this */) // cannot move qobject with a parent to a QThread
    , mipFragmentData(volumeData /* , this */) // cannot move qobject with parent to a QThread
    , dataColorModel(volumeData)
    , mipFragmentColors(mipFragmentData, dataColorModel)
{
    // Prepare to load 16-bit volume data from disk in a separate QThread
    connect(this, SIGNAL(volumeDataNeeded()),
            &volumeData, SLOT(loadVolumeDataFromFiles()));
    // TODO connect mip color update signal to something
}

AnnotationSession::~AnnotationSession()
{
    if (multiColorImageStackNode!=0) {
        delete multiColorImageStackNode;
    }
    if (neuronAnnotatorResultNode!=0) {
        delete neuronAnnotatorResultNode;
    }
    for (int i=0;i<neuronMipList.size();i++) {
        delete neuronMipList.at(i);
    }
    for (int i=0;i<overlayMipList.size();i++) {
        delete overlayMipList.at(i);
    }
}

// TBD
bool AnnotationSession::save() {
    return true;
}

// TBD
bool AnnotationSession::load(long annotationSessionID) {
    return true;
}

bool AnnotationSession::loadLsmMetadata() {
    QStringList lsmMetadataFilepathList=multiColorImageStackNode->getPathsToLsmMetadataFiles();
    if (lsmMetadataFilepathList.size()==0) {
        qDebug() << "AnnotationSession::loadLsmMetadata() received empty list of lsm metadata files";
        return false;
    } else {
        QString filePath=lsmMetadataFilepathList.at(0);
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            cerr << "Could not open file=" << filePath.toStdString() << " to read\n";
            return false;
        }
        QStringList fileContents;
        while(!file.atEnd()) {
            fileContents.append(file.readLine());
        }
        file.close();
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
                zRatio=d3/d1;
                qDebug() << "Using lsm dimension ratios " << d1 << " " << d2 << " " << d3 << " setting zRatio=" << zRatio;
                parseSuccess=true;
                break;
            }
        }
        if (!parseSuccess) {
            qDebug() << "AnnotationSession::loadLsmMetadata could not parse file to determine zRatio";
            return false;
        }
        return true;
    }

}

bool AnnotationSession::loadVolumeData()
{
    // Allocate writer on the stack so write lock will be automatically released when method returns
    NaVolumeData::Writer volumeWriter(volumeData);

    // Set file names of image files so VolumeData will know what to load.
    volumeWriter.setOriginalImageStackFilePath(
            multiColorImageStackNode->getPathToOriginalImageStackFile());
    volumeWriter.setMaskLabelFilePath(
            multiColorImageStackNode->getPathToMulticolorLabelMaskFile());
    volumeWriter.setReferenceStackFilePath(
            multiColorImageStackNode->getPathToReferenceStackFile());
    volumeWriter.unlock(); // unlock before emit
    emit volumeDataNeeded(); // load data in a separate QThread

    return true;
}

bool AnnotationSession::prepareLabelIndex()
{
    // Read lock is allocated on the stack, so it will be automatically released when the read operation
    // is complete and the read lock falls out of scope.
    NaVolumeData::Reader volumeReader(volumeData);
    if (! volumeReader.hasReadLock()) return false;
    int maxMaskIndex=volumeReader.getNumberOfNeurons();
    volumeReader.unlock();

    qDebug() << "Using maxMaskIndex=" << maxMaskIndex;

    for (int i=0;i<maxMaskIndex;i++) {
        bool status=false;
        maskStatusList.append(status);
        neuronSelectList.append(status);
    }

    return true;
}

// This function populates the neuronMipList<QPixmap> indexed 0..<num-stacks>
// and the overlayMipList<QPixmap> with Reference in position 0 and background signal in position 1

bool AnnotationSession::populateMipLists()
{
    // Read lock is allocated on the stack, so it will be automatically released when the read operation
    // is complete and the read lock falls out of scope.
    NaVolumeData::Reader volumeReader(volumeData);
    if (! volumeReader.hasReadLock()) return false;

    qDebug() << "AnnotationSession::populateMaskMipList() start";

    const Image4DProxy<My4DImage> originalProxy = volumeReader.getOriginalImageProxy();
    const Image4DProxy<My4DImage> maskProxy = volumeReader.getNeuronMaskProxy();
    const Image4DProxy<My4DImage> referenceProxy = volumeReader.getReferenceImageProxy();

    int z,y,x;
    int imageX=originalProxy.sx;
    int imageY=originalProxy.sy;
    int imageZ=originalProxy.sz;
    int imageC=originalProxy.sc;
    if (imageC != 3) {
        qDebug() << "Expected original image stack to have channel dim=3 but it has channel dim=" << imageC;
        return false;
    }

    neuronMipList.clear();
    overlayMipList.clear();
    assert(neuronMipList.size() == 0);
    assert(overlayMipList.size() == 0);

    // Neuron MIP List and Background
    for (int maskIndex=0;maskIndex<maskStatusList.size();maskIndex++) {
        QImage* mip=new QImage(imageX, imageY, QImage::Format_RGB888);
        mip->fill(0);
        neuronMipList.append(mip);
    }

    // Order is important here
    QImage* referenceImage = new QImage(imageX, imageY, QImage::Format_RGB888);
    referenceImage->fill(0);
    overlayMipList.append(referenceImage);

    QImage* backgroundImage = new QImage(imageX, imageY, QImage::Format_RGB888);
    backgroundImage->fill(0);
    overlayMipList.append(backgroundImage);


    bool falseBool=false;
    overlayStatusList.append(falseBool); // Reference status
    overlayStatusList.append(falseBool); // Background status

    for (z=0;z<imageZ;z++) {
        for (y=0;y<imageY;y++) {
            for (x=0;x<imageX;x++) {

                // Reference
                int referenceIntensity = referenceProxy.value8bit_at(x,y,z,0);
                QRgb referenceColor = qRgb(referenceIntensity, referenceIntensity, referenceIntensity); // gray
                QRgb previousReferenceColor = referenceImage->pixel(x,y);
                if (z==0) {
                    referenceImage->setPixel(x,y,referenceColor);
                } else {
                    int previousIntensity = qRed(previousReferenceColor) + qGreen(previousReferenceColor) + qBlue(previousReferenceColor);
                    int currentIntensity = qRed(referenceColor) + qGreen(referenceColor) + qBlue(referenceColor);
                    if (currentIntensity > previousIntensity) {
                        referenceImage->setPixel(x,y,referenceColor);
                    }
                }

                // Neurons and Background
                unsigned char maskIndex = maskProxy.value8bit_at(x,y,z,0);
                if (maskIndex > neuronMipList.size()) {
                    int totalMaskEntries=neuronMipList.size(); // since position 0 is background
                    qDebug() << "Found mask index=" << maskIndex << " but the total number of mask entries is=" << totalMaskEntries;
                    return false;
                }
                int red = originalProxy.value8bit_at(x,y,z,0);
                int green = originalProxy.value8bit_at(x,y,z,1);
                int blue = originalProxy.value8bit_at(x,y,z,2);
                QRgb color=qRgb(red,green,blue);
                QImage* maskImage=0;
                if (maskIndex==0) {
                    // Background
                    maskImage=overlayMipList[AnnotationSession::BACKGROUND_MIP_INDEX];
                } else {
                    maskImage=neuronMipList[maskIndex-1];
                }
                QRgb previousColor = maskImage->pixel(x,y);
                if (z==0) {
                    maskImage->setPixel(x,y,color);
                } else {
                    int previousIntensity = qRed(previousColor) + qGreen(previousColor) + qBlue(previousColor);
                    int currentIntensity = qRed(color) + qGreen(color) + qBlue(color);
                    if (currentIntensity > previousIntensity) {
                        maskImage->setPixel(x,y,color);
                    }
                }
            }
        }
        if (! volumeReader.refreshLock()) return false; // Try to reacquire lock every 25 ms
    }

    qDebug() << "AnnotationSession::populateMaskMipList() done";
    return true;
}

void AnnotationSession::overlayUpdate(int index, bool status) {
    int statusValue=(status ? 1 : 0);
    qDebug() << "AnnotationSession::overlayUpdate index=" << index << " status=" << status;
    overlayStatusList.replace(index, statusValue);
    QString overlayUpdateString=QString("FULL_UPDATE");
    emit modelUpdated(overlayUpdateString);
}

void AnnotationSession::neuronMaskUpdate(int index, bool status) {
    qDebug() << "AnnotationSession::neuronMaskUpdate index=" << index << " status=" << status;
    int statusValue=(status ? 1 : 0);
    maskStatusList.replace(index, statusValue);
    QString neuronMaskUpdateString=QString("NEURONMASK_UPDATE %1 %2").arg(index).arg(statusValue);
    emit modelUpdated(neuronMaskUpdateString);
}

void AnnotationSession::neuronMaskFullUpdate() {
    qDebug() << "AnnotationSession::neuronMaskFullUpdate() - emitting modelUpdated()";
    QString updateString=QString("FULL_UPDATE");
    emit modelUpdated(updateString);
}

void AnnotationSession::setOverlayStatus(int index, bool status) {
    overlayStatusList.replace(index, status);
}

void AnnotationSession::setNeuronMaskStatus(int index, bool status) {
    maskStatusList.replace(index, status);
}


// switch status of selected neuron
void AnnotationSession::switchSelectedNeuron(int index)
{
    if(neuronSelectList.at(index) == true)
    {
        neuronSelectList.replace(index, false);
    }
    else
    {
        neuronSelectList.replace(index, true);
    }
}

void AnnotationSession::switchSelectedNeuronUniquelyIfOn(int index) {
    bool alreadySelected=neuronSelectList.at(index);
    if (!alreadySelected) {
        // We want to ensure this selection is unique
        for (int i=0;i<neuronSelectList.size();i++) {
            neuronSelectList.replace(i,false);
        }
        neuronSelectList.replace(index, true);
    } else {
        neuronSelectList.replace(index, false);
    }
}

// show selected neuron
void AnnotationSession::showSelectedNeuron(QList<int> overlayList)
{
    int selectionIndex=-1;
    for (int i=0;i<neuronSelectList.size();i++) {
        if (neuronSelectList.at(i)) {
            selectionIndex=i;
            break;
        }
    }
    if (selectionIndex<0 || selectionIndex>=maskStatusList.size()) {
        // nothing to do
        return;
    }
    for (int i=0;i<overlayStatusList.size();i++) {
        overlayStatusList.replace(i, false);
    }
    for (int i=0;i<overlayList.size();i++) {
        overlayStatusList.replace(overlayList.at(i), true);
    }
    for (int i=0;i<maskStatusList.size();i++) {
        maskStatusList.replace(i, false);
    }
    maskStatusList.replace(selectionIndex, true);
    neuronMaskFullUpdate();
    emit scrollBarFocus(selectionIndex);
    emit deselectNeuron();
}

// show all neurons
void AnnotationSession::showAllNeurons(QList<int> overlayList)
{
    for (int i=0;i<overlayStatusList.size();i++) {
        overlayStatusList.replace(i, false);
    }
    for (int i=0;i<overlayList.size();i++) {
        overlayStatusList.replace(overlayList.at(i), true);
    }
    for (int i=0;i<maskStatusList.size();i++) {
        maskStatusList.replace(i, true);
    }
    neuronMaskFullUpdate();
}

// clear all neurons
void AnnotationSession::clearAllNeurons()
{
    // deselect background and reference
    for (int i=0;i<overlayStatusList.size();i++) {
        overlayStatusList.replace(i, false);
    }

    // deselect neurons
    for (int i=0;i<neuronSelectList.size();i++) {
        neuronSelectList.replace(i, false);
    }
    for (int i=0;i<maskStatusList.size();i++) {
        maskStatusList.replace(i, false);
    }
    neuronMaskFullUpdate();
}

// update Neuron Select List
void AnnotationSession::updateNeuronSelectList(int index)
{
    qDebug()<<"AnnotationSession neuron index ..."<<index;

    for (int i=0;i<neuronSelectList.size();i++) {
        neuronSelectList.replace(i, false);
        maskStatusList.replace(i, false);
    }

    neuronSelectList.replace(index, true);
    maskStatusList.replace(index, true);
}


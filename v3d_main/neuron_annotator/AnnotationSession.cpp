#include "AnnotationSession.h"
#include "gui/RendererNeuronAnnotator.h"
#include <QtAlgorithms>
#include <iostream>
#include <cassert>

using namespace std;

const int AnnotationSession::REFERENCE_MIP_INDEX=0;
const int AnnotationSession::BACKGROUND_MIP_INDEX=1;

AnnotationSession::AnnotationSession()
{
    multiColorImageStackNode=0;
    neuronAnnotatorResultNode=0;
    originalImageStack=0;
    neuronMaskStack=0;
    referenceStack=0;
}

AnnotationSession::~AnnotationSession() {
    if (multiColorImageStackNode!=0) {
        delete multiColorImageStackNode;
    }
    if (neuronAnnotatorResultNode!=0) {
        delete neuronAnnotatorResultNode;
    }
    if (originalImageStack!=0) {
        delete originalImageStack;
    }
    if (neuronMaskStack!=0) {
        delete neuronMaskStack;
    }
    if (referenceStack!=0) {
        delete referenceStack;
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

bool AnnotationSession::loadOriginalImageStack() {
    QString msgPrefix("AnnotationSession::loadOriginalImageStack()");
    QString originalImageStackFilePath=multiColorImageStackNode->getPathToOriginalImageStackFile();
    originalImageStack = new My4DImage;
    if (!originalImageStack) {
        cerr << msgPrefix.toStdString() << " : problem creating My4DImage" << endl;
        return false;
    }
    originalImageStack->loadImage(originalImageStackFilePath.toAscii().data());
    if (originalImageStack->isEmpty()) {
        cerr << msgPrefix.toStdString() << ": originalImageStack is empty after loading\n";
        return false;
    }
    cout << "Loaded original image stack with dimensions X=" << originalImageStack->getXDim() << " Y=" << originalImageStack->getYDim()
            << " Z=" << originalImageStack->getZDim() << " C=" << originalImageStack->getCDim() << "\n";
    return true;
}

bool AnnotationSession::loadNeuronMaskStack() {
    QString msgPrefix("AnnotationSession::loadNeuronMaskStack()");
    if (originalImageStack==0) {
        cerr << msgPrefix.toStdString() << " error : originalImageStack must be created before this function is called" << endl;
        return false;
    }
    int imageXSize=originalImageStack->getXDim();
    int imageYSize=originalImageStack->getYDim();
    int imageZSize=originalImageStack->getZDim();
    QString maskLabelFilePath=multiColorImageStackNode->getPathToMulticolorLabelMaskFile();
    neuronMaskStack = new My4DImage;
    if (!neuronMaskStack) {
        cerr << msgPrefix.toStdString() << " : problem creating My4DImage" << endl;
        return false;
    }
    neuronMaskStack->loadImage(maskLabelFilePath.toAscii().data());

    return true;

    // Create blank image
    //neuronMaskStack->loadImage(imageXSize, imageYSize, imageZSize, 1, 1); // single 4D slice, 8-bit color, respectively
    //if (neuronMaskStack->isEmpty()) {
    //    cerr << msgPrefix.toStdString() << ": neuronMaskStack is empty after loading\n";
    //    return false;
    //}

    // Load binary mask data
    //bool readMaskStatus = MultiColorImageStackNode::readMaskFileToMy4DImage(neuronMaskStack, maskLabelFilePath);
    //return readMaskStatus;
}

bool AnnotationSession::loadReferenceStack() {

    // Phase 1: load the data
    QString msgPrefix("AnnotationSession::loadReferenceStack()");
    qDebug() << msgPrefix << " : start";
    QString referenceStackFilePath=multiColorImageStackNode->getPathToReferenceStackFile();
    My4DImage* initialReferenceStack=new My4DImage();
    initialReferenceStack->loadImage(referenceStackFilePath.toAscii().data());
    if (initialReferenceStack->isEmpty()) {
        cerr << msgPrefix.toStdString() << ": initialReferenceStack is empty after loading\n";
        return false;
    }
    cout << "Loaded reference stack stack with dimensions X=" << initialReferenceStack->getXDim() << " Y=" << initialReferenceStack->getYDim()
            << " Z=" << initialReferenceStack->getZDim() << " C=" << initialReferenceStack->getCDim() << "\n";

    // Phase 2: normalize to 8-bit
    referenceStack=new My4DImage();
    referenceStack->loadImage(initialReferenceStack->getXDim(), initialReferenceStack->getYDim(), initialReferenceStack->getZDim(), 1 /* number of channels */, 1 /* bytes per channel */);
    Image4DProxy<My4DImage> initialProxy(initialReferenceStack);
    Image4DProxy<My4DImage> referenceProxy(referenceStack);

    double initialMin=initialReferenceStack->getChannalMinIntensity(0);
    double initialMax=initialReferenceStack->getChannalMaxIntensity(0);

    qDebug() << "Populating reference with initial data";
    double initialRange=initialMax-initialMin;
    qDebug() << "Reference lsm initialMin=" << initialMin << " initialMax=" << initialMax << " initialRange=" << initialRange;
    for (int z=0;z<initialReferenceStack->getZDim();z++) {
        for (int y=0;y<initialReferenceStack->getYDim();y++) {
            for (int x=0;x<initialReferenceStack->getXDim();x++) {
                int value= (255.0*(*initialProxy.at_uint16(x,y,z,0))-initialMin)/initialRange;
                if (value<0) {
                    value=0;
                } else if (value>255) {
                    value=255;
                }
                referenceProxy.put8bit_fit_at(x,y,z,0,value);
            }
        }
    }
    initialReferenceStack->cleanExistData();
    delete initialReferenceStack;
    qDebug() << "Finished loading reference stack";

    return true;
}

bool AnnotationSession::prepareLabelIndex() {
    int z,y,x;
    int imageX=originalImageStack->getXDim();
    int imageY=originalImageStack->getYDim();
    int imageZ=originalImageStack->getZDim();
    int imageC=originalImageStack->getCDim();
    if (imageC != 3) {
        qDebug() << "Expected original image stack to have channel dim=3 but it has channel dim=" << imageC;
        return false;
    }
    Image4DProxy<My4DImage> maskProxy(neuronMaskStack);

    int maxMaskIndex=0;

    for (z=0;z<imageZ;z++) {
        for (y=0;y<imageY;y++) {
            for (x=0;x<imageX;x++) {
                unsigned char mask = maskProxy.value8bit_at(x,y,z,0);
                if (mask>maxMaskIndex) {
                    maxMaskIndex=mask;
                }
            }
        }
    }

    qDebug() << "Using maxMaskIndex=" << maxMaskIndex;

    for (int i=0;i<maxMaskIndex;i++) {
        NeuronMaskEntry entry(0,0,i);
        maskEntryList.append(entry);
    }

    for (int i=0;i<maskEntryList.size();i++) {
        bool status=false;
        maskStatusList.append(status);
        neuronSelectList.append(status);
    }
    return true;
}

// This function populates the neuronMipList<QPixmap> indexed 0..<num-stacks>
// and the overlayMipList<QPixmap> with Reference in position 0 and background signal in position 1

bool AnnotationSession::populateMipLists() {
    qDebug() << "AnnotationSession::populateMaskMipList() start";
    int z,y,x;
    int imageX=originalImageStack->getXDim();
    int imageY=originalImageStack->getYDim();
    int imageZ=originalImageStack->getZDim();
    int imageC=originalImageStack->getCDim();
    if (imageC != 3) {
        qDebug() << "Expected original image stack to have channel dim=3 but it has channel dim=" << imageC;
        return false;
    }
    Image4DProxy<My4DImage> originalProxy(originalImageStack);
    Image4DProxy<My4DImage> maskProxy(neuronMaskStack);
    Image4DProxy<My4DImage> referenceProxy(referenceStack);

    neuronMipList.clear();
    overlayMipList.clear();
    assert(neuronMipList.size() == 0);
    assert(overlayMipList.size() == 0);

    // Neuron MIP List and Background
    for (int maskIndex=0;maskIndex<maskEntryList.size();maskIndex++) {
        neuronMipList.append(QImage(imageX, imageY, QImage::Format_RGB888));
    }

    // Order is important here
    overlayMipList.append((QImage(imageX, imageY, QImage::Format_RGB888))); // Reference
    overlayMipList.append((QImage(imageX, imageY, QImage::Format_RGB888))); // Background

    QImage& referenceImage = overlayMipList[AnnotationSession::REFERENCE_MIP_INDEX];
    QImage& backgroundImage = overlayMipList[AnnotationSession::BACKGROUND_MIP_INDEX];

    bool falseBool=false;
    overlayStatusList.append(falseBool); // Reference status
    overlayStatusList.append(falseBool); // Background status

    for (z=0;z<imageZ;z++) {
        for (y=0;y<imageY;y++) {
            for (x=0;x<imageX;x++) {

                // Reference
                int referenceIntensity = referenceProxy.value8bit_at(x,y,z,0);
                QRgb referenceColor = qRgb(referenceIntensity, referenceIntensity, referenceIntensity); // gray
                QRgb previousReferenceColor = referenceImage.pixel(x,y);
                if (previousReferenceColor==0) {
                    referenceImage.setPixel(x,y,referenceColor);
                } else {
                    int previousIntensity = qRed(previousReferenceColor) + qGreen(previousReferenceColor) + qBlue(previousReferenceColor);
                    int currentIntensity = qRed(referenceColor) + qGreen(referenceColor) + qBlue(referenceColor);
                    if (currentIntensity > previousIntensity) {
                        referenceImage.setPixel(x,y,referenceColor);
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
                    maskImage=&(overlayMipList[AnnotationSession::BACKGROUND_MIP_INDEX]);
                } else {
                    maskImage=&(neuronMipList[maskIndex-1]);
                }
                QRgb previousColor = maskImage->pixel(x,y);
                if (previousColor==0) {
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
    }

    qDebug() << "AnnotationSession::populateMaskMipList() done";
    return true;
}

void AnnotationSession::overlayUpdate(int index, bool status) {
    int statusValue=(status ? 1 : 0);
    overlayStatusList.replace(index, statusValue);
    QString overlayUpdateString=QString("FULL_UPDATE");
    emit modelUpdated(overlayUpdateString);
}

void AnnotationSession::neuronMaskUpdate(int index, bool status) {
    int statusValue=(status ? 1 : 0);
    maskStatusList.replace(index, statusValue);
    QString neuronMaskUpdateString=QString("NEURONMASK_UPDATE %1 %2").arg(index).arg(statusValue);
    emit modelUpdated(neuronMaskUpdateString);
}

void AnnotationSession::neuronMaskFullUpdate() {
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
    maskStatusList.replace(selectionIndex,true);
    neuronMaskFullUpdate();
    emit scrollBarFocus(selectionIndex);
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



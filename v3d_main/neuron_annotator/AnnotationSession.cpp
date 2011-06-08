#include "AnnotationSession.h"
#include "RendererNeuronAnnotator.h"
#include <QtAlgorithms>
#include <iostream>

using namespace std;

AnnotationSession::AnnotationSession()
{
    multiColorImageStackNode=0;
    neuronAnnotatorResultNode=0;
    originalImageStack=0;
    neuronMaskStack=0;
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
    // Create blank image
    neuronMaskStack->loadImage(imageXSize, imageYSize, imageZSize, 1, 1); // single 4D slice, 8-bit color, respectively
    if (neuronMaskStack->isEmpty()) {
        cerr << msgPrefix.toStdString() << ": neuronMaskStack is empty after loading\n";
        return false;
    }
    // Load binary mask data
    bool readMaskStatus = MultiColorImageStackNode::readMaskFileToMy4DImage(neuronMaskStack, maskLabelFilePath);
    return readMaskStatus;
}

bool AnnotationSession::loadMaskLabelIndexFile() {
    QString msgPrefix("AnnotationSession::loadMaskLabelIndexFile()");
    maskEntryList.clear();
    QString maskLabelIndexFilePath=multiColorImageStackNode->getPathToMulticolorLabelMaskIndexFile();
    QFile maskLabelFile(maskLabelIndexFilePath);
    QFileInfo maskLabelFileInfo(maskLabelFile);
    if (!maskLabelFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        cerr << msgPrefix.toStdString() << " : could not open file=" << maskLabelFileInfo.absolutePath().toStdString() << " to read" << endl;
        return false;
    }
    QTextStream in(&maskLabelFile);
    while (!in.atEnd()) {
        QString indexLine = in.readLine();
        if (!indexLine.startsWith("#")) {
            NeuronMaskEntry entry;
            if (!entry.populateFromString(indexLine)) {
                maskLabelFile.close();
                return false;
            }
            maskEntryList.append(entry);
        }
    }
    maskLabelFile.close();
    qSort(maskEntryList.begin(), maskEntryList.end());
    for (int i=0;i<maskEntryList.size()+1;i++) { // +1 is for background position
        bool status=false;
        maskStatusList.append(status);
        neuronSelectList.append(status);
    }
    return true;
}

// This function populates the maskMipList<QPixmap> with the background
// density data in index=0, and the neuron masks in indices 1..<num-masks>

bool AnnotationSession::populateMaskMipList() {
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

    QList<QImage*> maskImageList;
    // Position 0 is background, 1..n are the neuron masks
    for (int maskIndex=0;maskIndex<maskEntryList.size()+1;maskIndex++) {
        QImage* qImage = new QImage(imageX, imageY, QImage::Format_RGB888);
        maskImageList.append(qImage);
    }
    for (z=0;z<imageZ;z++) {
        for (y=0;y<imageY;y++) {
            for (x=0;x<imageX;x++) {
                unsigned char mask = maskProxy.value8bit_at(x,y,z,0);
                if (mask>maskImageList.size()) {
                    qDebug() << "Found mask index=" << mask << " but the total number of mask entries is=" << maskImageList.size();
                    return false;
                }
                int red = originalProxy.value8bit_at(x,y,z,0);
                int green = originalProxy.value8bit_at(x,y,z,1);
                int blue = originalProxy.value8bit_at(x,y,z,2);
                QRgb color=qRgb(red,green,blue);
                QImage* maskImage = maskImageList.at(mask);
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
    for (int maskIndex=0;maskIndex<maskEntryList.size()+1;maskIndex++) {
        QPixmap pixmap;
        QImage* maskImage=maskImageList.at(maskIndex);
        if (!pixmap.convertFromImage(*maskImage)) {
            qDebug() << "Error creating pixmap from MIP image index=" << maskIndex;
            return false;
        }
        delete maskImage; // assuming this is OK and will not corrupt QPixmap
        maskMipList.append(pixmap);
    }
    qDebug() << "AnnotationSession::populateMaskMipList() done";
    return true;
}

void AnnotationSession::neuronMaskUpdate(int index, bool status) {
    int statusValue=(status ? 1 : 0);
    maskStatusList.replace(index, statusValue);
    QString neuronMaskUpdateString=QString("NEURONMASK %1 %2").arg(index).arg(statusValue);
    emit modelUpdated(neuronMaskUpdateString);
}

void AnnotationSession::setNeuronMaskStatus(int index, bool status) {
    if (index>=maskStatusList.size()) {
        qDebug() << "AnnotationSession::setNeuronMaskStatus() index=" << index << " status=" << status << " : ignoring since list size=" << maskStatusList.size();
        return;
    }
    maskStatusList.replace(index, status);
}

// swith status of selected neuron
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
void AnnotationSession::showSelectedNeuron(bool background)
{
    if(background)
    {
        qDebug()<<"with bg ...";

        // on background
        setNeuronMaskStatus(0, true);
        //neuronMaskUpdate(0, true);
    }
    else
    {
        qDebug()<<"without bg ...";

        // off background
        setNeuronMaskStatus(0, false);
        //neuronMaskUpdate(0, false);
    }

    // show neuron
    int index = -1;

    for(int i=0; i<neuronSelectList.size(); i++)
    {
        if(neuronSelectList.at(i))
        {
            index = i;
            break; // only one neuron selected at once
        }
    }

    if(index<0)
    {
        qDebug()<<"no neuron selected ...";
        return;
    }

    for(int i=1; i<maskStatusList.size(); i++)
    {
        if(i==index) {
            setNeuronMaskStatus(i, true);
            //neuronMaskUpdate(i, true);
        }
        else {
            setNeuronMaskStatus(i, false);
            //neuronMaskUpdate(i, false);
        }
    }

    emit neuronMaskStatusSet();
    emit scrollBarFocus(index);
}

// show all neurons
void AnnotationSession::showAllNeurons(bool background)
{
    if(background)
    {
        qDebug()<<"with bg ...";

        // on background
        setNeuronMaskStatus(0, true);
    }
    else
    {
        qDebug()<<"without bg ...";

        // off background
        setNeuronMaskStatus(0, false);
    }

    // show neurons
    for(int i=1; i<maskStatusList.size(); i++)
    {
        setNeuronMaskStatus(i, true);
    }

    emit neuronMaskStatusSet();
    emit scrollBarFocus(1);
}


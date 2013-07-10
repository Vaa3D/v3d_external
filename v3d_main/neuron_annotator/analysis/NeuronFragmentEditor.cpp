#include "NeuronFragmentEditor.h"
#include "../utility/ImageLoader.h"
#include "AnalysisTools.h"
#include "SleepThread.h"
#include <climits>

const int NeuronFragmentEditor::MODE_UNDEFINED=-1;
const int NeuronFragmentEditor::MODE_COMBINE=0;
const int NeuronFragmentEditor::MODE_COMBINE_MASK=1;
const int NeuronFragmentEditor::MODE_REVERSE_LABEL=2;
const int NeuronFragmentEditor::MODE_MIPS=3;

NeuronFragmentEditor::NeuronFragmentEditor()
{
    mode=MODE_COMBINE; // default
    sourceImage=0L;
    labelImage=0L;
    label8=0L;
    label16=0L;
    outputPrefix="";
    xdim=ydim=zdim=0;
    maxThreadCount=0;
}

NeuronFragmentEditor::~NeuronFragmentEditor()
{
}

int NeuronFragmentEditor::processArgs(vector<char*> *argList)
{
    for (int i=0;i<argList->size();i++) {
        QString arg=(*argList)[i];
        if (arg=="-mode") {
            QString modeString=(*argList)[++i];
            if (modeString=="combine") {
                mode=MODE_COMBINE;
            } else if (modeString=="combine-mask") {
                mode=MODE_COMBINE_MASK;
            } else if (modeString=="reverse-label") {
                mode=MODE_REVERSE_LABEL;
            } else if (modeString=="mips") {
                mode=MODE_MIPS;
            } else {
                mode=MODE_UNDEFINED;
            }
        } else if (arg=="-sourceImage") {
            sourceImageFilepath=(*argList)[++i];
        } else if (arg=="-labelIndex") {
            inputLabelIndexFilepath=(*argList)[++i];
        } else if (arg=="-fragments") {
            fragmentListString=(*argList)[++i];
            QList<QString> fragmentListBeforeInt=fragmentListString.split(QRegExp(","));
            for (int f=0;f<fragmentListBeforeInt.length();f++) {
                QString fragmentString=fragmentListBeforeInt.at(f);
                int fragmentInt=fragmentString.toInt();
                fragmentList.append(fragmentInt);
            }
        } else if (arg=="-outputMip") {
            outputMipFilepath=(*argList)[++i];
        } else if (arg=="-outputStack") {
            outputStackFilepath=(*argList)[++i];
        } else if (arg=="-outputDir") {
            outputDirPath=(*argList)[++i];
        } else if (arg=="-outputPrefix") {
            outputPrefix=(*argList)[++i];
        } else if (arg=="-maskFiles") {
            do {
                QString possibleMaskFile=(*argList)[++i];
                if (possibleMaskFile.startsWith("-")) {
                    break;
                } else {
                    // Assume is mask file
                    maskFilePaths.append(possibleMaskFile);
                }
            } while( i < (argList->size()-1) );
        } else if (arg=="-maxThreadCount") {
            QString maxThreadString=(*argList)[++i];
            maxThreadCount=maxThreadString.toInt();
        }
    }
    bool argError=false;
    if (mode!=MODE_COMBINE && mode!=MODE_COMBINE_MASK && mode!=MODE_REVERSE_LABEL && mode!=MODE_MIPS) {
        qDebug() << "Do not recognize valid mode";
        argError=true;
    }
    if (mode==MODE_COMBINE || mode==MODE_REVERSE_LABEL || mode==MODE_MIPS) {
        if (sourceImageFilepath.length() < 1) {
            qDebug() << "-sourceImageFilepath is required";
            argError=true;
        }
        if (inputLabelIndexFilepath.length() < 1) {
            qDebug() << "-labelIndex is required";
            argError=true;
        }
    }
    if (mode==MODE_COMBINE || mode==MODE_COMBINE_MASK) {
        if (outputMipFilepath.length() < 1) {
            qDebug() << "-outputMip is required";
            argError=true;
        }
        if (outputStackFilepath.length() < 1) {
            qDebug() << "-outputStack is required";
            argError=true;
        }
    }
    if (mode==MODE_COMBINE) {
        if (fragmentListString.length() < 1) {
            qDebug() << "-fragments list is required";
            argError=true;
        }
        if (fragmentList.size() < 1) {
            qDebug() << "fragment list must contain at least one fragment index number";
            argError=true;
        }
    } else if (mode==MODE_COMBINE_MASK) {
        if (maskFilePaths.size() < 1) {
            qDebug() << "-maskFiles list is required";
            argError=true;
        }
    } else if (mode==MODE_REVERSE_LABEL || mode==MODE_MIPS) {
        if (outputDirPath.size() < 1) {
            qDebug() << "-outputDir is required";
            argError=true;
        }
        if (outputPrefix.size() < 1) {
            qDebug() << "-outputPrefix is required";
            argError=true;
        }
    }
    if (argError) {
        return 1;
    }
    return 0;
}


bool NeuronFragmentEditor::execute()
{
    if (maxThreadCount>0) {
        QThreadPool *globalThreadPool = QThreadPool::globalInstance();
        qDebug() << "Setting max thread count=" << maxThreadCount;
        globalThreadPool->setMaxThreadCount(maxThreadCount);
    }
    if (mode==MODE_COMBINE) {
        return createFragmentComposite();
    } else if (mode==MODE_COMBINE_MASK) {
        return createMaskComposite();
    } else if (mode==MODE_REVERSE_LABEL) {
        return reverseLabel();
    } else if (mode==MODE_MIPS) {
        return createMips();
    } else {
        return false;
    }
}

bool NeuronFragmentEditor::loadSourceAndLabelImages()
{
    // Open consolidated signal label file
    ImageLoader sourceLoader;
    sourceImage=sourceLoader.loadImage(sourceImageFilepath);
    xdim=sourceImage->getXDim();
    ydim=sourceImage->getYDim();
    zdim=sourceImage->getZDim();
    cdim=sourceImage->getCDim();

    qDebug() << "Using source x=" << xdim << " y=" << ydim << " z=" << zdim << " c=" << cdim << " datatype=" << sourceImage->getDatatype();

    ImageLoader labelLoader;
    labelImage=labelLoader.loadImage(inputLabelIndexFilepath);

    qDebug() << "Label Image has datatype=" << labelImage->getDatatype() << " , checking source and label dimension correspondence";

    if (labelImage->getXDim()!=xdim) {
        qDebug() << "source and label xdim do not match";
        return false;
    }
    if (labelImage->getYDim()!=ydim) {
        qDebug() << "source and label ydim do not match";
        return false;
    }
    if (labelImage->getZDim()!=zdim) {
        qDebug() << "source and label zdim do not match";
        return false;
    }
//    if (cdim<3) {
//        qDebug() << "Expected source image to contain at least 3 channels";
//        return false;
//    }
    return true;
}

bool NeuronFragmentEditor::createFragmentComposite()
{
    if (!loadSourceAndLabelImages()) {
        return false;
    }
    createImagesFromFragmentList(fragmentList, outputStackFilepath, outputMipFilepath);
}


bool NeuronFragmentEditor::createImagesFromFragmentList(QList<int> fragmentList, QString stackFilename, QString mipFilename)
{
    qDebug() << "createImagesFromFragmentList() start";

    My4DImage * compositeImage = new My4DImage();
    compositeImage->loadImage(xdim, ydim, zdim, cdim, V3D_UINT8);

    qDebug() << "Setting up channel pointers";

    v3d_uint16 * label=(v3d_uint16*)(labelImage->getRawDataAtChannel(0));

    v3d_uint8 * sourceR=sourceImage->getRawDataAtChannel(0);
    v3d_uint8 * sourceG=sourceImage->getRawDataAtChannel(1);
    v3d_uint8 * sourceB=sourceImage->getRawDataAtChannel(2);

    v3d_uint8 * compR=compositeImage->getRawDataAtChannel(0);
    v3d_uint8 * compG=compositeImage->getRawDataAtChannel(1);
    v3d_uint8 * compB=compositeImage->getRawDataAtChannel(2);

    qDebug() << "Creating fragment membership array";

    int fragmentListSize=fragmentList.size();
    int * fragmentArr = new int[fragmentListSize];
    for (int f=0;f<fragmentListSize;f++) {
        fragmentArr[f]=fragmentList.at(f);
        qDebug() << "Adding entry for fragment=" << fragmentArr[f];
    }

    qDebug() << "Populating new image, getting max intensity";

    v3d_uint8 maxIntensity=0;

    // First get max intensity
    for (V3DLONG z=0;z<zdim;z++) {
        for (V3DLONG y=0;y<ydim;y++) {
            for (V3DLONG x=0;x<xdim;x++) {
                V3DLONG offset=z*ydim*xdim + y*xdim + x;
                int labelValue=label[offset];
                bool include=false;
                for (int f=0;f<fragmentListSize;f++) {
                    if (labelValue==fragmentArr[f]) {
                        include=true;
                    }
                }
                if (include) {
                    v3d_uint8 r=sourceR[offset];
                    v3d_uint8 g=sourceG[offset];
                    v3d_uint8 b=sourceB[offset];
                    if (r>maxIntensity) {
                        maxIntensity=r;
                    }
                    if (g>maxIntensity) {
                        maxIntensity=g;
                    }
                    if (b>maxIntensity) {
                        maxIntensity=b;
                    }
                }
            }
        }
    }

    if (maxIntensity==0) {
        maxIntensity=5; // any non-zero should work
    }

    qDebug() << "Max intensity=" << maxIntensity << ", populating new image";

    for (V3DLONG z=0;z<zdim;z++) {
        for (V3DLONG y=0;y<ydim;y++) {
            for (V3DLONG x=0;x<xdim;x++) {
                V3DLONG offset=z*ydim*xdim + y*xdim + x;
                int labelValue=label[offset];
                bool include=false;
                for (int f=0;f<fragmentListSize;f++) {
                    if (labelValue==fragmentArr[f]) {
                        include=true;
                    }
                }
                if (include) {
                    v3d_uint8 r=sourceR[offset];
                    v3d_uint8 g=sourceG[offset];
                    v3d_uint8 b=sourceB[offset];
                    double intensityFactor=254.0/(double)maxIntensity;
                    r=((double)r*intensityFactor);
                    g=((double)g*intensityFactor);
                    b=((double)b*intensityFactor);
                    compR[offset]=r;
                    compG[offset]=g;
                    compB[offset]=b;
                } else {
                    compR[offset]=0;
                    compG[offset]=0;
                    compB[offset]=0;
                }
            }
        }
    }

    qDebug() << "Creating mip";

    My4DImage * compositeMIP = AnalysisTools::createMIPFromImage(compositeImage);

    if (stackFilename.length()>0) {

        qDebug() << "Saving composite";

        ImageLoader compositeLoader;
        compositeLoader.saveImage(compositeImage, stackFilename);

    }

    if (mipFilename.length()>0) {

        qDebug() << "Saving mip";

        ImageLoader compositeMIPSaver;
        compositeMIPSaver.saveImage(compositeMIP, mipFilename);

    }

    qDebug() << "Starting cleanup";

    delete [] fragmentArr;
    delete compositeImage;
    delete compositeMIP;
    return true;
}

bool NeuronFragmentEditor::reverseLabel()
{
    if (!loadSourceAndLabelImages()) {
        return false;
    }

    QDir outputDir(outputDirPath);
    if (!outputDir.exists()) {
        QDir().mkdir(outputDirPath);
    }

    // First, we will profile and index all label voxels
    if (labelImage->getDatatype()==V3D_UINT8) {
        label8=(v3d_uint8*)(labelImage->getRawDataAtChannel(0));
    } else {
        label16=(v3d_uint16*)(labelImage->getRawDataAtChannel(0));
    }

    const int MAX_LABEL=65536;
    labelIndex=new long[MAX_LABEL];
    for (int i=0;i<MAX_LABEL;i++) {
        labelIndex[i]=0;
    }

    for (long z=0;z<zdim;z++) {
        for (long y=0;y<ydim;y++) {
            for (long x=0;x<xdim;x++) {
                long offset=z*ydim*xdim + y*xdim + x;
                int labelValue=-1;
                if (label8>0L) {
                    labelValue=label8[offset];
                } else {
                    labelValue=label16[offset];
                }
                if (labelValue>=0) {
                    labelIndex[labelValue]++;
                }
            }
        }
    }

    QList<int> labelList;
    for (int i=1;i<MAX_LABEL;i++) { // ignore 0
        if (labelIndex[i]>0) {
            labelList.append(i);
            qDebug() << "Found label=" << i;
        }
    }

    // For the outermost loop, we iterate through each label and score
    // each axis for efficiency.
    QList< QFuture<bool> > labelProcessList;
    maskChan.setSourceImage(sourceImage);
    maskChan.setLabelImage(labelImage);
    for (int l=0;l<labelList.size();l++) {
        int label=labelList[l];
        qDebug() << "Processing label=" << label << " voxels=" << labelIndex[label];
	QString maskFullPath=createFullPathFromLabel(label, ".mask");
	QString chanFullPath=createFullPathFromLabel(label, ".chan");
        QFuture<bool> labelJob = QtConcurrent::run(maskChan, &MaskChan::createMaskChanForLabel, label, maskFullPath, chanFullPath, &mutex);
        labelProcessList.append(labelJob);
    }

    while(1) {
        SleepThread st;
        st.msleep(1000);
        int doneCount=0;
        for (int i=0;i<labelProcessList.size();i++) {
            QFuture<bool> labelJob = labelProcessList.at(i);
            if (labelJob.isFinished()) {
                if (labelJob.result()==true) {
                    doneCount++;
                } else {
                    qDebug() << "Label job returned false";
                    return false;
                }
            }
        }
        int stillActive=labelProcessList.size()-doneCount;
        if (stillActive==0) {
            break;
        } else {
            qDebug() << "Waiting on " << stillActive << " label process jobs";
        }
    }

    // Global cleanup
    delete sourceImage;
    delete labelImage;

    return true;
}

bool NeuronFragmentEditor::createMaskComposite()
{
    My4DImage* outputStack = 0L;

    // Use the first mask file to dictate the dimensions
    FILE* fid=0L;
    xdim=-1;
    ydim=-1;
    zdim=-1;
    cdim=-1;

    for (int i=0;i<maskFilePaths.size();i++) {
        QString maskFilePath=maskFilePaths[i];
        if ( (fid=fopen(maskFilePath.toAscii().data(), "rb"))==0) {
            qDebug() << "Could not open file " << maskFilePath << " to read";
            return false;
        }

        long chanVoxels=0L;
        unsigned char chanChannelCount=0;
        unsigned char chanRecRed=0;
        unsigned char chanRecGreen=0;
        unsigned char chanRecBlue=0;
        unsigned char chanBytesPerChannel=0;
        unsigned char* chanData=0L;
        long chanTotalDataBytes=0L;
        QString chanFilePath=QString("");
        int maskFilePrefixPosition=maskFilePath.lastIndexOf(".");
        if (maskFilePrefixPosition > 0) {
            chanFilePath.append(maskFilePath.left(maskFilePrefixPosition));
            chanFilePath.append(".chan");
            FILE* fid2=0L;
            if ( (fid2=fopen(chanFilePath.toAscii().data(), "rb"))>0) {
                fread(&chanVoxels, sizeof(long), 1, fid2);
                fread(&chanChannelCount, sizeof(unsigned char), 1, fid2);
                fread(&chanRecRed, sizeof(unsigned char), 1, fid2);
                fread(&chanRecGreen, sizeof(unsigned char), 1, fid2);
                fread(&chanRecBlue, sizeof(unsigned char), 1, fid2);
                fread(&chanBytesPerChannel, sizeof(unsigned char), 1, fid2);
                qDebug() << "File " << chanFilePath << " has chanBytesPerChannel=" << chanBytesPerChannel;
                chanTotalDataBytes=chanBytesPerChannel*chanVoxels*chanChannelCount;
                chanData=new unsigned char[chanTotalDataBytes];
                fread(chanData, sizeof(unsigned char), chanTotalDataBytes, fid2);
                fclose(fid2);
                if (cdim==-1) {
                    cdim=chanChannelCount;
                } else {
                    if (cdim!=chanChannelCount) {
                        qDebug() << "Expected channel counts to match between mask files, cdim=" << cdim << " chanChannelCount=" << chanChannelCount;
                        return false;
                    }
                }
            }
        }

        long xsize=0L;
        long ysize=0L;
        long zsize=0L;
        fread(&xsize, sizeof(long), 1, fid);
        fread(&ysize, sizeof(long), 1, fid);
        fread(&zsize, sizeof(long), 1, fid);

        if (xdim==-1) {
            xdim=xsize;
        } else {
            if (xsize!=xdim) {
                qDebug() << "xsize " << xsize << " in file " << maskFilePath << " does not match " << xdim;
            }
        }

        if (ydim==-1) {
            ydim=ysize;
        } else {
            if (ysize!=ydim) {
                qDebug() << "ysize " << ysize << " in file " << maskFilePath << " does not match " << ydim;
            }
        }

        if (zdim==-1) {
            zdim=zsize;
        } else {
            if (zsize!=zdim) {
                qDebug() << "zsize " << zsize << " in file " << maskFilePath << " does not match " << zdim;
            }
        }

        if (outputStack==0L) {
            outputStack=new My4DImage();
            if (chanBytesPerChannel==0 || chanBytesPerChannel==1) {
                outputStack->loadImage(xdim, ydim, zdim, cdim, V3D_UINT8);
            } else if (chanBytesPerChannel==2) {
                outputStack->loadImage(xdim, ydim, zdim, cdim, V3D_UINT16);
            } else {
                qDebug() << "Do not understand chanBytesPerChannel=" << chanBytesPerChannel;
                return false;
            }
        }

        float xmicrons=0.0;
        float ymicrons=0.0;
        float zmicrons=0.0;

        // we ignore these for now
        fread(&xmicrons, sizeof(float), 1, fid);
        fread(&ymicrons, sizeof(float), 1, fid);
        fread(&zmicrons, sizeof(float), 1, fid);

        long x0;
        long x1;
        long y0;
        long y1;
        long z0;
        long z1;

        // bounding box
        fread(&x0, sizeof(long), 1, fid);
        fread(&x1, sizeof(long), 1, fid);
        fread(&y0, sizeof(long), 1, fid);
        fread(&y1, sizeof(long), 1, fid);
        fread(&z0, sizeof(long), 1, fid);
        fread(&z1, sizeof(long), 1, fid);

        long totalVoxels;
        fread(&totalVoxels, sizeof(long), 1, fid);

        if (chanVoxels>0L && chanVoxels!=totalVoxels) {
            qDebug() << "Mask and Chan totalVoxel counts do not match";
            return false;
        }

        unsigned char axis;
        fread(&axis, sizeof(unsigned char), 1, fid);

        long readVoxels=0L;
        long planePosition=0L;

        v3d_uint8** data8=new v3d_uint8*[cdim];
        v3d_uint16** data16=new v3d_uint16*[cdim];

        for (int c=0;c<cdim;c++) {
            if (chanBytesPerChannel==0 || chanBytesPerChannel==1) {
                data8[c]=outputStack->getRawDataAtChannel(c);
                data16[c]=0L;
            } else {
                data8[c]=0L;
                data16[c]=(v3d_uint16*)outputStack->getRawDataAtChannel(c);
            }
        }

        v3d_uint16* chanData16=(v3d_uint16*)chanData;

        while(readVoxels<totalVoxels) {
            // Start new ray
            long skip=0L;
            fread(&skip, sizeof(long), 1, fid);
            planePosition+=skip;
            long pairs=0L;
            fread(&pairs, sizeof(long), 1, fid);
            for (long p=0;p<pairs;p++) {
                long start=0L;
                long end=0L;
                fread(&start, sizeof(long), 1, fid);
                fread(&end, sizeof(long), 1, fid);
                if (axis==0) { // yz(x)
                    long y=planePosition/zdim;
                    long z=planePosition-(zdim*y);
                    for (int x=start;x<end;x++) {
                        long offset=z*ydim*xdim+y*xdim+x;
                        if (chanBytesPerChannel==0) {
                            for (int c=0;c<cdim;c++) {
                                data8[c][offset]=255;
                            }
                        } else if (chanBytesPerChannel==1) {
                            for (int c=0;c<cdim;c++) {
                                data8[c][offset]=chanData[readVoxels+(x-start)+c*chanVoxels];
                            }
                        } else if (chanBytesPerChannel==2) {
                            for (int c=0;c<cdim;c++) {
                                data16[c][offset]=chanData16[readVoxels+(x-start)+c*chanVoxels];
                            }
                        }
                    }
                } else if (axis==1) { // xz(y)
                    long x=planePosition/zdim;
                    long z=planePosition-(zdim*x);
                    for (int y=start;y<end;y++) {
                        long offset=z*ydim*xdim+y*xdim+x;
                        if (chanBytesPerChannel==0) {
                            for (int c=0;c<cdim;c++) {
                                data8[c][offset]=255;
                            }
                        } else if (chanBytesPerChannel==1) {
                            for (int c=0;c<cdim;c++) {
                                data8[c][offset]=chanData[readVoxels+(y-start)+c*chanVoxels];
                            }
                        } else if (chanBytesPerChannel==2) {
                            for (int c=0;c<cdim;c++) {
                                data16[c][offset]=chanData16[readVoxels+(y-start)+c*chanVoxels];
                            }
                        }
                    }
                } else if (axis==2) { // xy(z)
                    long x=planePosition/ydim;
                    long y=planePosition-(ydim*x);
                    for (int z=start;z<end;z++) {
                        long offset=z*ydim*xdim+y*xdim+x;
                        if (chanBytesPerChannel==0) {
                            for (int c=0;c<cdim;c++) {
                                data8[c][offset]=255;
                            }
                        } else if (chanBytesPerChannel==1) {
                            for (int c=0;c<cdim;c++) {
                                data8[c][offset]=chanData[readVoxels+(z-start)+c*chanVoxels];
                            }
                        } else if (chanBytesPerChannel==2) {
                            for (int c=0;c<cdim;c++) {
                                data16[c][offset]=chanData16[readVoxels+(z-start)+c*chanVoxels];
                            }
                        }
                    }
                }
                readVoxels+=(end-start);
            }
            planePosition++;
        }
        fclose(fid);

        if (chanData!=0L) {
            delete [] chanData;
        }
    }

    qDebug() << "Saving composite";

    ImageLoader compositeLoader;
    compositeLoader.saveImage(outputStack, outputStackFilepath);

    qDebug() << "Creating mip";

    My4DImage * compositeMIP = AnalysisTools::createMIPFromImage(outputStack);

    qDebug() << "Saving mip";

    ImageLoader compositeMIPSaver;
    compositeMIPSaver.saveImage(compositeMIP, outputMipFilepath);

    qDebug() << "Starting cleanup";

    delete outputStack;
    delete compositeMIP;

    return true;

}

bool NeuronFragmentEditor::createMips()
{
    QDir outputDir(outputDirPath);
    if (!outputDir.exists()) {
        QDir().mkdir(outputDirPath);
    }

    if (!loadSourceAndLabelImages()) {
        qDebug() << "Error loading source and label images";
        return false;
    }

    QList<int> labelList=maskChan.getFragmentListFromLabelStack();

    for (int i=0;i<labelList.size();i++) {
        int label=labelList[i];
        QList<int> singletonList;
        singletonList.append(label);
        QString outputStackFilename="";
        QString outputMipFilename=createFullPathFromLabel(label, ".tif");
        qDebug() << "Creating " << outputMipFilename;
        if (!createImagesFromFragmentList(singletonList, outputStackFilename, outputMipFilename)) {
            qDebug() << "createImagesFromFragmentList() failed at label=" << label;
            return false;
        }
    }

    return true;
}

QString NeuronFragmentEditor::createFullPathFromLabel(int label, QString extension)
{
    QString filename=outputDirPath;
    filename.append("/");
    if (outputPrefix.length()>0) {
        filename.append(outputPrefix);
        filename.append("_");
    }
    filename.append(QString::number(label-1));
    filename.append(extension);

    return filename;
}




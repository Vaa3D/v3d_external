#include "NeuronFragmentEditor.h"
#include "../utility/ImageLoader.h"
#include "AnalysisTools.h"
#include <climits>

const int NeuronFragmentEditor::MODE_UNDEFINED=-1;
const int NeuronFragmentEditor::MODE_COMBINE=0;
const int NeuronFragmentEditor::MODE_REVERSE_LABEL=1;

NeuronFragmentEditor::NeuronFragmentEditor()
{
    mode=MODE_COMBINE; // default
    sourceImage=0L;
    labelImage=0L;
    label8=0L;
    label16=0L;
    outputPrefix="";
    xdim=ydim=zdim=0;
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
            } else if (modeString=="reverse-label") {
                mode=MODE_REVERSE_LABEL;
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
        }
    }
    bool argError=false;
    if (mode!=MODE_COMBINE && mode!=MODE_REVERSE_LABEL) {
        qDebug() << "Do not recognize valid mode";
        argError=true;
    }
    if (sourceImageFilepath.length() < 1) {
        qDebug() << "-sourceImageFilepath is required";
        argError=true;
    }
    if (inputLabelIndexFilepath.length() < 1) {
        qDebug() << "-labelIndex is required";
        argError=true;
    }
    if (mode==MODE_COMBINE) {
        if (fragmentListString.length() < 1) {
            qDebug() << "-fragments list is required";
            argError=true;
        }
        if (outputMipFilepath.length() < 1) {
            qDebug() << "-outputMip is required";
            argError=true;
        }
        if (outputStackFilepath.length() < 1) {
            qDebug() << "-outputStack is required";
            argError=true;
        }
        if (fragmentList.size() < 1) {
            qDebug() << "fragment list must contain at least one fragment index number";
            argError=true;
        }
    } else if (mode==MODE_REVERSE_LABEL) {
        if (outputDirPath.size() < 1) {
            qDebug() << "-outputDir is required";
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
    if (mode==MODE_COMBINE) {
        return createFragmentComposite();
    } else if (mode==MODE_REVERSE_LABEL) {
        return reverseLabel();
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

    qDebug() << "Using source x=" << xdim << " y=" << ydim << " z=" << zdim << " c=" << cdim;

    ImageLoader labelLoader;
    labelImage=labelLoader.loadImage(inputLabelIndexFilepath);

    qDebug() << "Checking source and label dimension correspondence";

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
    if (cdim<3) {
        qDebug() << "Expected source image to contain at least 3 channels";
        return false;
    }
    return true;
}

bool NeuronFragmentEditor::createFragmentComposite()
{
    if (!loadSourceAndLabelImages()) {
        return false;
    }

    qDebug() << "Creating compositeImage";

    My4DImage * compositeImage = new My4DImage();
    compositeImage->loadImage(xdim, ydim, zdim, cdim, V3D_UINT8);

    qDebug() << "Setting up channel pointeres";

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
                    if (labelValue==fragmentArr[f]+1) {
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
                    if (labelValue==fragmentArr[f]+1) {
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

    qDebug() << "Saving composite";

    ImageLoader compositeLoader;
    compositeLoader.saveImage(compositeImage, outputStackFilepath);

    qDebug() << "Creating mip";

    My4DImage * compositeMIP = AnalysisTools::createMIPFromImage(compositeImage);

    qDebug() << "Saving mip";

    ImageLoader compositeMIPSaver;
    compositeMIPSaver.saveImage(compositeMIP, outputMipFilepath);

    qDebug() << "Starting cleanup";

    delete [] fragmentArr;
    delete compositeImage;
    delete compositeMIP;
    return true;
}

/*

  Format for mask and channel files.

  Mask files:

  long xsize; // space
  long ysize; // space
  long zsize; // space
  float xMicrons; // voxel size x
  float yMicrons; // voxel size y
  float zMicrons; // voxel size z
  long x0; // bounding box
  long x1; // bounding box, such that x0 is inclusive, x1 exclusive, etc
  long y0; // bb
  long y1; // bb
  long z0; // bb
  long z1; // bb
  long totalVoxels;
  unsigned char axis; // 0=yz(x), 1=xz(y), 2=xy(z)
  { // For each ray
    long skip;
    long pairs;
    { // For each pair
        long start;
        long end; // such that end-start is length, i.e., end is exclusive
    }
  }

  Channel files:

  long totalVoxels;
  unsigned char channels; // number of channels
  unsigned char recommendedRedChannel;
  unsigned char recommendedGreenChannel;
  unsigned char recommendedBlueChannel;
  unsigned char bytesPerChannel; // 1=8-bit, 2=16-bit
  { // For each channel
    { // For each voxel
        B value;
    }
  }

 */

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
    long* labelIndex=new long[MAX_LABEL];
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
    for (int l=0;l<labelList.size();l++) {
        int label=labelList[l];
        qDebug() << "Processing label=" << label << " voxels=" << labelIndex[label];

        QList<long> pairCountList; // x=0, y=1, z=2
        QList<MaskRay*> xRayList;
        QList<MaskRay*> yRayList;
        QList<MaskRay*> zRayList;

        QList<long> x0List;
        QList<long> x1List;
        QList<long> y0List;
        QList<long> y1List;
        QList<long> z0List;
        QList<long> z1List;

        for (int direction=0;direction<3;direction++) {

            QList<MaskRay*> * rayList;
            long pairCount=0L;
            long countCheck=0L;

            if (direction==0) {
                rayList=&xRayList;
            } else if (direction==1) {
                rayList=&yRayList;
            } else {
                rayList=&zRayList;
            }

            long x0,x1,y0,y1,z0,z1;

            axisTracer(direction, label, rayList, pairCount, countCheck, x0, x1, y0, y1, z0, z1);

            x0List.append(x0);
            x1List.append(x1);
            y0List.append(y0);
            y1List.append(y1);
            z0List.append(z0);
            z1List.append(z1);

            pairCountList.append(pairCount);
            pairCount=0L;

            if (countCheck!=labelIndex[label]) {
                qDebug() << "Count check failed : direction=" << direction << " countCheck=" << countCheck << " labelIndex=" << labelIndex[label];
                return false;
            } else {
                qDebug() << "Direction " << direction << " passed voxel count check";
            }

        }

        // We have computed the ray set for each axis, we will save the one with the
        // smallest size.

        for (int s=0;s<pairCountList.size();s++) {
            qDebug() << "pairCount " << s << " : " << pairCountList[s];
        }

        unsigned char smallestSize=0;
        if (pairCountList[1]<pairCountList[smallestSize]) {
            smallestSize=1;
        }
        if (pairCountList[2]<pairCountList[smallestSize]) {
            smallestSize=2;
        }

        qDebug() << "Using axis " << smallestSize;

        // Write out the mask file
        QString maskFilename="";
        if (outputPrefix.length()>0) {
            maskFilename.append(outputPrefix);
            maskFilename.append("_");
        }
        maskFilename.append(QString::number(label));
        maskFilename.append(".mask");

        QString maskFullPath=outputDirPath;
        maskFullPath.append("/");
        maskFullPath.append(maskFilename);

        qDebug() << "Writing to file " << maskFullPath;

        FILE* fid = fopen(maskFullPath.toAscii().data(), "wb");
        if (!fid) {
            qDebug() << "Could not open file " << maskFullPath << " to write";
            return false;
        }

        fwrite(&xdim, sizeof(long), 1, fid);
        fwrite(&ydim, sizeof(long), 1, fid);
        fwrite(&zdim, sizeof(long), 1, fid);

        float xMicrons=0.0;
        float yMicrons=0.0;
        float zMicrons=0.0;

        fwrite(&xMicrons, sizeof(float), 1, fid);
        fwrite(&yMicrons, sizeof(float), 1, fid);
        fwrite(&zMicrons, sizeof(float), 1, fid);

        long x0=x0List[smallestSize];
        long x1=x1List[smallestSize];
        long y0=y0List[smallestSize];
        long y1=y1List[smallestSize];
        long z0=z0List[smallestSize];
        long z1=z1List[smallestSize];

        fwrite(&x0, sizeof(long), 1, fid);
        fwrite(&x1, sizeof(long), 1, fid);
        fwrite(&y0, sizeof(long), 1, fid);
        fwrite(&y1, sizeof(long), 1, fid);
        fwrite(&z0, sizeof(long), 1, fid);
        fwrite(&z1, sizeof(long), 1, fid);

        long totalVoxels=labelIndex[label];
        fwrite(&totalVoxels, sizeof(long), 1, fid);
        fwrite(&smallestSize, sizeof(unsigned char), 1, fid);

        if (smallestSize==0) {
            writeMaskList(fid, xRayList);
        } else if (smallestSize==1) {
            writeMaskList(fid, yRayList);
        } else {
            writeMaskList(fid, zRayList);
        }
        fflush(fid);
        fclose(fid);
        fid=0L;

        // Write out the channel file

        // First, clear the previous masks
        for (int d=0;d<3;d++) {
            QList<MaskRay*> * rayList;
            if (d==0) {
                rayList=&xRayList;
            } else if (d==1) {
                rayList=&yRayList;
            } else {
                rayList=&zRayList;
            }
            for (int i=0;i<rayList->size();i++) {
                MaskRay* ray = (*rayList)[i];
                delete ray;
            }
            rayList->clear();
        }

        QString channelFilename="";
        if (outputPrefix.length()>0) {
            channelFilename.append(outputPrefix);
            channelFilename.append("_");
        }
        channelFilename.append(QString::number(label));
        channelFilename.append(".chan");

        QString channelFullPath=outputDirPath;
        channelFullPath.append("/");
        channelFullPath.append(channelFilename);

        qDebug() << "Writing to file " << channelFullPath;

        fid = fopen(channelFullPath.toAscii().data(), "wb");
        if (!fid) {
            qDebug() << "Could not open file " << channelFullPath << " to write";
            return false;
        }

        fwrite(&totalVoxels, sizeof(long), 1, fid);
        unsigned char numChannels=sourceImage->getCDim();
        fwrite(&numChannels, sizeof(unsigned char), 1, fid);

        unsigned char recRed=0;
        unsigned char recGreen=1;
        unsigned char recBlue=2;

        fwrite(&recRed, sizeof(unsigned char), 1, fid);
        fwrite(&recGreen, sizeof(unsigned char), 1, fid);
        fwrite(&recBlue, sizeof(unsigned char), 1, fid);

        unsigned char datatype=1; // 8-bit
        if (label8==0L) {
            datatype=2; // 16-bit
        }
        fwrite(&datatype, sizeof(unsigned char), 1, fid);

        // allocate space for data
        long unitsNeeded=totalVoxels*cdim;
        void* data=0L;
        if (sourceImage->getDatatype()==V3D_UINT8) {
            data=(void*) new v3d_uint8[unitsNeeded];
        } else {
            data=(void*) new v3d_uint16[unitsNeeded];
        }

        // re-run axis-tracer and populate channel intensity data
        QList<MaskRay*> * rayList;
        long pairCount=0L;
        long countCheck=0L;

        if (smallestSize==0) {
            rayList=&xRayList;
        } else if (smallestSize==1) {
            rayList=&yRayList;
        } else {
            rayList=&zRayList;
        }

        qDebug() << "calling axisTracer 2nd pass data=" << data;

        axisTracer(smallestSize, label, rayList, pairCount, countCheck, x0, x1, y0, y1, z0, z1, data, totalVoxels);

        if (countCheck!=totalVoxels) {
            qDebug() << "In second pass of axisTracer, countCheck=" << countCheck << " totalVoxels=" << totalVoxels;
            exit(1);
        }

        fwrite(data, totalVoxels*cdim, (datatype+1), fid);
        fflush(fid);
        fclose(fid);

        qDebug() << "Wrote " << channelFullPath;
    }

    // Global cleanup
    delete sourceImage;
    delete labelImage;
}

void NeuronFragmentEditor::writeMaskList(FILE* fid, QList<MaskRay*>& list)
{
    for (int i=0;i<list.size();i++) {
        fwrite(&(list[i]->skipCount), sizeof(long), 1, fid);
        long pairListSize=list[i]->endList.size();
        fwrite(&pairListSize, sizeof(long), 1, fid);
        //qDebug() << "skip=" << list[i]->skipCount << " pairs=" << pairListSize;
        for(int j=0;j<list[i]->endList.size();j++) {
            long start=list[i]->startList[j];
            fwrite(&start, sizeof(long), 1, fid);
            long end=list[i]->endList[j];
            fwrite(&end, sizeof(long), 1, fid);
            //qDebug() << "start=" << start << " end=" << end;
        }
    }
}

void NeuronFragmentEditor::axisTracer(int direction, int label, QList<MaskRay*> * rayList, long& pairCount, long& voxelCount,
        long& x0, long& x1, long& y0, long& y1, long& z0, long& z1, void* data, long assumedVoxelCount)
{

    bool debug=false;
//    if (label==23) {
//        debug=true;
//    }
    long skipCount=0L;
    long start=-1;
    long end=-1;
    MaskRay* ray=0L;
    long D0, D1, D2;

    x0=LONG_MAX;
    x1=0L;
    y0=LONG_MAX;
    y1=0L;
    z0=LONG_MAX;
    z1=0L;

    if (direction==0) { // X-axis
        D0=ydim;
        D1=zdim;
        D2=xdim;
    } else if (direction==1) { // Y-axis
        D0=xdim;
        D1=zdim;
        D2=ydim;
    } else if (direction==2) { // Z-axis
        D0=xdim;
        D1=ydim;
        D2=zdim;
    }

    long cdim=sourceImage->getCDim();
    long cOffset=sourceImage->getTotalUnitNumberPerChannel();
    v3d_uint8* source8=0L;
    v3d_uint16* source16=0L;
    v3d_uint8* data8=0L;
    v3d_uint16* data16=0L;
    if (sourceImage->getDatatype()==V3D_UINT8) {
        source8=(v3d_uint8*)sourceImage->getRawData();
        data8=(v3d_uint8*)data;
    } else {
        source16=(v3d_uint16*)sourceImage->getRawData();
        data16=(v3d_uint16*)data;
    }

    //qDebug() << "Inside axisTracer data8=" << data8 << " source8=" << source8 <<" data16=" << data16 << " source16=" << source16;


    voxelCount=0L;
    pairCount=0L;
    long dataPosition=0L;

    const int START_STATE_BEGIN=0;
    const int START_STATE_JUST_STARTED=1;
    const int START_STATE_STARTED_IN_LAST_POSITION=2;

    for (long d0=0;d0<D0;d0++) {
        for (long d1=0;d1<D1;d1++) {
            start=-1;
            end=-1;
            for (long d2=0;d2<D2;d2++) {
                int startState=START_STATE_BEGIN;
                long offset=0L;
                if (direction==0) {
                    offset=d1*ydim*xdim + d0*xdim + d2;
                } else if (direction==1) {
                    offset=d1*ydim*xdim + d2*xdim + d0;
                } else { // direction==2
                    offset=d2*ydim*xdim + d1*xdim + d0;
                }
                int labelValue=-1;
                if (label8>0L) {
                    labelValue=label8[offset];
                } else {
                    labelValue=label16[offset];
                }
                if (start==-1 && labelValue==label) {
                    startState=START_STATE_JUST_STARTED;
                    if (ray==0L) {
                        ray=new MaskRay();
                        ray->skipCount=skipCount;
                        skipCount=0L;
                    }
                    start=d2;
                    end=-1;
                    ray->startList.append(start);
                    if (d2==(D2-1)) {
                        startState=START_STATE_STARTED_IN_LAST_POSITION;
                    }
                }
                if (startState==START_STATE_STARTED_IN_LAST_POSITION ||
                        (startState==START_STATE_BEGIN && (start>=0 && ( labelValue!=label || ( labelValue==label && d2==(D2-1) ) ) ) ) ) {
                    end=d2;
                    if (labelValue==label && d2==(D2-1)) { // handle last-case
                        end=D2;
                    }
                    ray->endList.append(end);
                    voxelCount+=(end-start);
                    if (debug) {
                        qDebug() << "start=" << start << " end=" << end << " voxelCount=" << voxelCount;
                    }
                    if (data>0L) {
                        if (voxelCount > assumedVoxelCount) {
                            qDebug() << "Error: assumedVoxelCount greater than voxelCount";
                            exit(1);
                        }
                        for (int i=start;i<end;i++) {
                            long sourcePosition=0L;
                            if (direction==0) {
                                sourcePosition=d1*ydim*xdim + d0*xdim + i;
                            } else if (direction==1) {
                                sourcePosition=d1*ydim*xdim + i*xdim + d0;
                            } else { // direction==2
                                sourcePosition=i*ydim*xdim + d1*xdim + d0;
                            }
                            for (long c=0;c<cdim;c++) {
                                long cof=c*cOffset;
                                long dof=c*assumedVoxelCount;
                                //qDebug() << "dataPosition=" << dataPosition << " c=" << c << " dof=" << dof << " cof=" << cof << " voxelCount=" << voxelCount;
                                if (data8>0L) {
                                    data8[dataPosition+dof]=source8[sourcePosition+cof];
                                } else {
                                    data16[dataPosition+dof]=source16[sourcePosition+cof];
                                }
                            }
                            dataPosition++;
                        }
                    }
                    start=-1;
                    end=-1;
                }
            }
            if (ray>0L) {
                pairCount+=ray->endList.size();
                // Update max values - we will just check first and last values
                long start=ray->startList[0];
                long end=ray->endList[ray->endList.size()-1];
                if (direction==0) { // x=d2 y=d0 z=d1
                    if (start<x0) x0=start;
                    if (end>x1) x1=end;
                    if (d0<y0) y0=d0;
                    if (d0>y1) y1=d0;
                    if (d1<z0) z0=d1;
                    if (d1>z1) z1=d1;
                } else if (direction==1) { // x=d0 y=d2 z=d1
                    if (d0<x0) x0=d0;
                    if (d0>x1) x1=d0;
                    if (start<y0) y0=start;
                    if (end>y1) y1=end;
                    if (d1<z0) z0=d1;
                    if (d1>z1) z1=d1;
                } else if (direction==2) { // x=d0 y=d1 z=d2
                    if (d0<x0) x0=d0;
                    if (d0>x1) x1=d0;
                    if (d1<y0) y0=d1;
                    if (d1>y1) y1=d1;
                    if (start<z0) z0=start;
                    if (end>z1) z1=end;
                }
                rayList->append(ray);
                ray=0L;
            } else {
                skipCount++;
            }
        }
    }

}








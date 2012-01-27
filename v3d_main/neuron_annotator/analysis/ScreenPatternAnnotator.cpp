#include "ScreenPatternAnnotator.h"
#include "../utility/ImageLoader.h"
#include "../../v3d/histogramsimple.h"

const int ScreenPatternAnnotator::MODE_UNDEFINED=-1;
const int ScreenPatternAnnotator::MODE_ANNOTATE=0;
const int ScreenPatternAnnotator::MODE_COMPARTMENT_INDEX=1;

ScreenPatternAnnotator::ScreenPatternAnnotator()
{
    mode=MODE_UNDEFINED;
    inputImage=0;
    patternChannelIndex=-1;
    lut16Color=0;
    imageGlobal16ColorImage=0;
}

ScreenPatternAnnotator::~ScreenPatternAnnotator()
{
    if (inputImage!=0) {
        delete inputImage;
    }
    if (lut16Color!=0) {
        delete [] lut16Color;
    }
    if (imageGlobal16ColorImage!=0) {
        delete imageGlobal16ColorImage;
    }
}

bool ScreenPatternAnnotator::execute()
{
    if (mode==MODE_COMPARTMENT_INDEX) {
        return createCompartmentIndex();
    } else if (mode==MODE_ANNOTATE) {
        return annotate();
    } else if (mode==MODE_UNDEFINED) {
        return false;
    }
}

bool ScreenPatternAnnotator::createCompartmentIndex() {

    // What we are doing here is to load a large set of compartment mask files and consolidate
    // them into a single index-mapped stack, in which each voxel contains the index of a
    // particular mask. This version will be 8-bit, which limits us to 256 masks.

    // We will progressively load each mask, and as we do so, we will populate the index stack.
    // We will also parse the name of the file and build an index-lookup file, so we can keep
    // track of which index corresponds to which compartment abbreviation.

    // This loader assumes the mask files have the format: mask-<#>-<Abbreviation>.tif

    if (compartmentIndexImage!=0) {
        delete compartmentIndexImage;
    }
    compartmentIndexImage=0;
    compartmentIndexAbbreviationMap.clear();

    // Create Output Directory
    if (!QFileInfo(outputResourceDirPath).isDir()) {
        QDir().mkpath(outputResourceDirPath);
    }

    QFileInfoList fileList = QDir(topLevelCompartmentMaskDirPath).entryInfoList();
    qDebug() << "In directory=" << topLevelCompartmentMaskDirPath << " found " << fileList.size() << " files to evaluate";
    for (int i=0;i<fileList.size();i++) {
        QFileInfo fileInfo=fileList.at(i);
        qDebug() << "Evaluating file=" << fileInfo.fileName();
        if (fileInfo.fileName().endsWith(".tif")) {
            int index=getIndexFromCompartmentMaskFilename(fileInfo.fileName());
            if (index < 0) {
                qDebug() << "ScreenPatternAnnotator::createCompartmentIndex() could not parse mask index from filename=" << fileInfo.fileName();
                return false;
            }
            if (index>255) {
                qDebug() << "ScreenPatternAnnotator::createCompartmentIndex() only supports up to 256 masks";
                return false;
            }
            QString abbreviation=getAbbreviationFromCompartmentMaskFilename(fileInfo.fileName());
            if (!abbreviation.size()>0) {
                qDebug() << "ScreenPatternAnnotator::createCompartmentIndex() could not parse abbreviation from filename=" << fileInfo.fileName();
                return false;
            }
            compartmentIndexAbbreviationMap.insert(index, abbreviation);
            qDebug() << "File=" << fileInfo.fileName() << " index=" << index << " abbreviation=" << abbreviation;
            ImageLoader maskLoader;
            qDebug() << "Loading " << fileInfo.absoluteFilePath();
            My4DImage * maskImage = maskLoader.loadImage(fileInfo.absoluteFilePath());
            if (maskImage->getCDim()>1) {
                qDebug() << "Mask file has channel count > 1, of=" << maskImage->getCDim() << " , will use channel 0";
            }
            qDebug() << "Done loading";
            if (compartmentIndexImage==0) {
                compartmentIndexImage=new My4DImage();
                compartmentIndexImage->loadImage(maskImage->getXDim(), maskImage->getYDim(), maskImage->getZDim(), 1 /* index channel */, V3D_UINT8);
                v3d_uint8 * ciData=compartmentIndexImage->getRawData();
                for (V3DLONG ci=0;ci<compartmentIndexImage->getTotalBytes();ci++) {
                    ciData[ci]=0;
                }
            } else {
                if (maskImage->getXDim()==compartmentIndexImage->getXDim() &&
                    maskImage->getYDim()==compartmentIndexImage->getYDim() &&
                    maskImage->getZDim()==compartmentIndexImage->getZDim()) {
                    qDebug() << "Verified dimension match";
                } else {
                    qDebug() << "ERROR: Dimensions " << maskImage->getXDim() << " " << maskImage->getYDim() << " " << maskImage->getZDim() << " do not match the previous dimensiions "
                             << compartmentIndexImage->getXDim() << " " << compartmentIndexImage->getYDim() << " " << compartmentIndexImage->getZDim();
                    return false;
                }
            }
            // We now have the info to populate the compartmentIndexImage for the current mask
            V3DLONG maskSizeCount=0;
            v3d_uint8 * maskData=maskImage->getRawDataAtChannel(0L);
            v3d_uint8 * indexData=compartmentIndexImage->getRawData();
            V3DLONG maskChannelUnitSize=maskImage->getXDim()*maskImage->getYDim()*maskImage->getZDim();
            for (V3DLONG p=0;p<maskChannelUnitSize;p++) {
                if (maskData[p]>0) {
                    indexData[p]=index;
                    maskSizeCount++;
                }
            }
            qDebug() << "Found mask of size=" << maskSizeCount;
            delete maskImage;
        }
    }
    QString outputResourceDirPathCopy=outputResourceDirPath;
    outputResourceDirPathCopy.append(QDir::separator()).append("compartmentIndex.v3dpbd");
    ImageLoader compartmentIndexSaver;
    compartmentIndexSaver.saveImage(compartmentIndexImage, outputResourceDirPathCopy);
    QString outputResourceDirPathCopy2=outputResourceDirPath;
    outputResourceDirPathCopy2.append(QDir::separator()).append("compartmentAbbreviationIndex.txt");
    QFile abbreviationFile(outputResourceDirPathCopy2);
    if (!abbreviationFile.open(QIODevice::WriteOnly)) {
        qDebug() << "Could not open file=" << outputResourceDirPathCopy2;
        return false;
    }
    QTextStream abbreviationOutputStream(&abbreviationFile);
    QList<int> indexList=compartmentIndexAbbreviationMap.keys();
    for (int k=0;k<indexList.size();k++) {
        int index=indexList.at(k);
        QString abbreviation=compartmentIndexAbbreviationMap[index];
        abbreviationOutputStream << index << " " << abbreviation << "\n";
    }
    abbreviationOutputStream.flush();
    abbreviationFile.close();
    return true;
}

QString ScreenPatternAnnotator::getAbbreviationFromCompartmentMaskFilename(QString filename) {
    QString abbreviation;
    if (!filename.startsWith("mask-")) {
        return abbreviation;
    }
    QStringList dashList=filename.split("-");
    QString abbrString=dashList.at(2);
    abbrString=abbrString.left(abbrString.size()-4); // removes trailing ".tif"
    return abbrString;
}

int ScreenPatternAnnotator::getIndexFromCompartmentMaskFilename(QString filename) {
    if (!filename.startsWith("mask-")) {
        return -1;
    }
    QStringList dashList=filename.split("-");
    QString indexString=dashList.at(1);
    while(indexString.size()>1 && indexString.startsWith("0")) {
        indexString=indexString.right(indexString.size()-1);
    }
    return indexString.toInt();
}


bool ScreenPatternAnnotator::annotate() {

    // Load Input Stack
    inputImage=new My4DImage();
    ImageLoader imageLoader;
    imageLoader.loadImage(inputImage, inputStackFilepath);
    if (inputImage->getDatatype()!=V3D_UINT8) {
        qDebug() << "ScreenPatternGenerator currently only supports 8-bit input data";
        return false;
    }

    // Compute Global 256-bin Histogram
    V3DLONG xSize=inputImage->getXDim();
    V3DLONG ySize=inputImage->getYDim();
    V3DLONG zSize=inputImage->getZDim();
    V3DLONG channelSize=xSize*ySize*zSize;
    global256BinHistogram.compute(inputImage->getRawData() + channelSize*patternChannelIndex ,channelSize);
    V3DLONG* global256BinHistogramArray=global256BinHistogram.getHistogram();

    // Create Output Directory
    if (!QFileInfo(outputDirectoryPath).isDir()) {
        QDir().mkpath(outputDirectoryPath);
    }

    // Create Global 16-Color Image
    lut16Color=create16Color8BitLUT();
    imageGlobal16ColorImage=create3DHeatmapFromChannel(inputImage, patternChannelIndex, lut16Color);
    ImageLoader imageLoaderForSave;
    QString filepathToSave(returnFullPathWithOutputPrefix("heatmap16Color.v3dpbd"));
    qDebug() << "Saving global 16-color heatmap to file=" << filepathToSave;
    bool saveHeatmapStatus=imageLoaderForSave.saveImage(imageGlobal16ColorImage, filepathToSave);
    if (!saveHeatmapStatus) {
        qDebug() << "ScreenPatternAnnotator::execute() Error during save of heatmap file";
        return false;
    }

    // Create MIP
    ImageLoader imageLoaderForMip;
    imageLoaderForMip.create2DMIPFromStack(imageGlobal16ColorImage, returnFullPathWithOutputPrefix("heatmap16ColorMIP.tif"));

    // Load Compartment Index
    ImageLoader compartmentIndexLoader;
    if (compartmentIndexImage!=0) {
        delete compartmentIndexImage;
    }
    compartmentIndexImage=new My4DImage();
    QString resourceDirectoryPathCopy=resourceDirectoryPath;
    QString compartmentIndexImageFilepath=resourceDirectoryPathCopy.append(QDir::separator()).append("compartmentIndex.v3dpbd");
    if (!compartmentIndexLoader.loadImage(compartmentIndexImage, compartmentIndexImageFilepath)) {
        qDebug() << "Could not load compartmentIndexImage from file=" << compartmentIndexImageFilepath;
        return false;
    }

    // Load Abbreviation Index Map
    compartmentIndexAbbreviationMap.clear();
    QString resourceDirectoryPathCopy2=resourceDirectoryPath;
    QString abbreviationMapFilepath=resourceDirectoryPathCopy2.append(QDir::separator()).append("compartmentAbbreviationIndex.txt");
    QFile abbreviationMapFile(abbreviationMapFilepath);
    if (!abbreviationMapFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open file=" << abbreviationMapFilepath << " to read";
        return false;
    }
    while(!abbreviationMapFile.atEnd()) {
        QString abLine=abbreviationMapFile.readLine();
        QList<QString> abList=abLine.split(" ");
        QString indexString=abList.at(0);
        int indexKey=indexString.toInt();
        QString abbreviationValue=abList.at(1);
        compartmentIndexAbbreviationMap[indexKey]=abbreviationValue;
    }
    abbreviationMapFile.close();

    // Perform Compartment Annotations
    QList<int> compartmentIndexList=compartmentIndexAbbreviationMap.keys();
    for (int k=0;k<compartmentIndexList.size();k++) {
        int index=compartmentIndexList.at(k);
        QString abbreviation=compartmentIndexAbbreviationMap[index];
        createCompartmentAnnotation(index, abbreviation);
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  This method generates the annotation artifacts for each compartment.
//
//  Here are the steps:
//
//     (1) A bounding-box (set of coordinates)
//
//     (2) A 16-color heatmap based on the raw image values
//
//     (3) A MIP based on the global 16-color heatmap
//
//     (4) A 16-color heatmap based on the auto-normalization of the compartment mask itself
//
//     (5) A MIP based on #5
//
//     (6.1, 6.2, 6.3, 6.4, 6.5) A re-sampled volume (RSV) version, which is 256x256xZ, where 256 is the largest XY dimension of the box, and Z is equiv re-scaled Z
//
//  The above are the graphics artifacts. Below are the numerical measurements to support search, with attribute strings:
//
//     (7) A measure of the overall pattern strength within the compartment, the sum of the 16-color heatmap normalized by volume
//
//     (8) The coefficient of variance of #8
//
//     (9) A measure of the distribution of the pattern within the compartment, the sum of the auto-normalized heatmap normalized by volume
//
//     (10) The coefficient of variance of #10
//
//     (11) Using a super-voxel (5x5x5) cubified set of the original volume, the coefficient of variance of the intensity of this set
//
//     (12) The percentage of 5x5x5 cubes containing 50% of the total intensity for all cubes
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScreenPatternAnnotator::createCompartmentAnnotation(int index, QString abbreviation) {

    SPA_BoundingBox bb=findBoundingBoxFromIndex(index);
    qDebug() << "Bounding box for " << abbreviation << " = " << bb.x0 << " " << bb.x1 << " " << bb.y0 << " " << bb.y1 << " " << bb.z0 << " " << bb.z1;

    My4DImage * compartmentHeatmap=createSub3DImageFromMask(imageGlobal16ColorImage, index, bb);
    if (compartmentHeatmap==0) {
        qDebug() << "compartmentHeatmap returned as 0";
        return;
    }
    qDebug() << "Created subimage from compartment mask";

    My4DImage * compartmentHeatmapMIP=createMIPFromImage(compartmentHeatmap);
    if (compartmentHeatmapMIP==0) {
        qDebug() << "compartmentHeatmapMIP returned as 0";
        return;
    }
    qDebug() << "Created MIP from compartment heatmap";

}

My4DImage * ScreenPatternAnnotator::createMIPFromImage(My4DImage * image) {

    if (image->getDatatype()!=V3D_UINT8) {
        qDebug() << "createMIPFromImage only supports datatype 1";
        return 0;
    }
    Image4DProxy<My4DImage> stackProxy(image);
    My4DImage * mip = new My4DImage();
    mip->loadImage( stackProxy.sx, stackProxy.sy, 1 /* z */, stackProxy.sc, V3D_UINT8 );
    memset(mip->getRawData(), 0, mip->getTotalBytes());
    Image4DProxy<My4DImage> mipProxy(mip);

    for (int y=0;y<stackProxy.sy;y++) {
        for (int x=0;x<stackProxy.sx;x++) {
            V3DLONG maxIntensity=0;
            int maxPosition=0;
            for (int z=0;z<stackProxy.sz;z++) {
                V3DLONG currentIntensity=0;
                for (int c=0;c<stackProxy.sc;c++) {
                    currentIntensity+=(*stackProxy.at(x,y,z,c));
                }
                if (currentIntensity>maxIntensity) {
                    maxIntensity=currentIntensity;
                    maxPosition=z;
                }
            }
            for (int c=0;c<stackProxy.sc;c++) {
                mipProxy.put_at(x,y,0,c,(*stackProxy.at(x,y,maxPosition,c)));
            }
        }
    }
    return mip;
}


// Here we will iterate through the mask and add the corresponding
My4DImage * ScreenPatternAnnotator::createSub3DImageFromMask(My4DImage * sourceImage, int index, SPA_BoundingBox bb) {

    V3DLONG xmax=compartmentIndexImage->getXDim();
    V3DLONG ymax=compartmentIndexImage->getYDim();
    V3DLONG zmax=compartmentIndexImage->getZDim();

    v3d_uint8 * iData=compartmentIndexImage->getRawData();
    v3d_uint8 * rData=sourceImage->getRawDataAtChannel(0);
    v3d_uint8 * gData=sourceImage->getRawDataAtChannel(1);
    v3d_uint8 * bData=sourceImage->getRawDataAtChannel(2);

    My4DImage * subImage = new My4DImage();
    V3DLONG s_xlen=(bb.x1-bb.x0)+1;
    V3DLONG s_ylen=(bb.y1-bb.y0)+1;
    V3DLONG s_zlen=(bb.z1-bb.z0)+1;
    subImage->loadImage( s_xlen, s_ylen, s_zlen , 3 /* rgb */, V3D_UINT8 /* datatype */);
    v3d_uint8 * sRaw=subImage->getRawData();
    for (V3DLONG i=0;i<subImage->getTotalBytes();i++) {
        sRaw[i]=0;
    }

    v3d_uint8 * s_rData=subImage->getRawDataAtChannel(0);
    v3d_uint8 * s_gData=subImage->getRawDataAtChannel(1);
    v3d_uint8 * s_bData=subImage->getRawDataAtChannel(2);

    for (int z=bb.z0;z<=bb.z1;z++) {
        V3DLONG zoffset=z*ymax*xmax;
        V3DLONG s_zoffset=(z-bb.z0)*s_ylen*s_xlen;
        for (int y=bb.y0;y<=bb.y1;y++) {
            V3DLONG yoffset = y*xmax + zoffset;
            V3DLONG s_yoffset = (y-bb.y0)*s_xlen + s_zoffset;
            for (int x=bb.x0;x<=bb.x1;x++) {
                V3DLONG position = yoffset + x;
                V3DLONG s_position = s_yoffset + (x-bb.x0);
                if (iData[position]==index) {
                    s_rData[s_position]=rData[position];
                    s_gData[s_position]=gData[position];
                    s_bData[s_position]=bData[position];
                }
            }
        }
    }
    return subImage;
}

// This method determines the bounding box for the given compartment index
SPA_BoundingBox ScreenPatternAnnotator::findBoundingBoxFromIndex(int index) {
    V3DLONG xmax=compartmentIndexImage->getXDim();
    V3DLONG ymax=compartmentIndexImage->getYDim();
    V3DLONG zmax=compartmentIndexImage->getZDim();
    V3DLONG x0=xmax;
    V3DLONG x1=-1;
    V3DLONG y0=ymax;
    V3DLONG y1=-1;
    V3DLONG z0=zmax;
    V3DLONG z1=-1;
    V3DLONG offset=0;
    v3d_uint8 * data=compartmentIndexImage->getRawData();
    V3DLONG totalBytes=compartmentIndexImage->getTotalBytes();
    V3DLONG totalUnits=compartmentIndexImage->getTotalUnitNumber();
    V3DLONG totalCalcUnits=xmax*ymax*zmax;
    for (V3DLONG z=0;z<zmax;z++) {
        V3DLONG zoffset=z*ymax*xmax;
        for (V3DLONG y=0;y<ymax;y++) {
            V3DLONG yoffset = y*xmax + zoffset;
            for (V3DLONG x=0;x<xmax;x++) {
                offset = yoffset + x;
                if (offset >= totalBytes) {
                    qDebug() << "ERROR  offset=" << offset << " totalBytes=" << totalBytes << " x=" << x << " y=" << y << " z=" << z;
                }
                if (data[offset]==index) {
                    if (x>x1) {
                        x1=x;
                    }
                    if (x<x0) {
                        x0=x;
                    }
                    if (y>y1) {
                        y1=y;
                    }
                    if (y<y0) {
                        y0=y;
                    }
                    if (z>z1) {
                        z1=z;
                    }
                    if (z<z0) {
                        z0=z;
                    }
                }
            }
        }
    }
    SPA_BoundingBox bb;
    bb.x0=x0;
    bb.x1=x1;
    bb.y0=y0;
    bb.y1=y1;
    bb.z0=z0;
    bb.z1=z1;
    return bb;
}

QString ScreenPatternAnnotator::returnFullPathWithOutputPrefix(QString filename) {
    QString outputDirectoryPathCopy=outputDirectoryPath;
    QString outputPrefixCopy=outputPrefix;
    return outputDirectoryPathCopy.append(QDir::separator()).append(outputPrefixCopy).append("_").append(filename);
}

int ScreenPatternAnnotator::processArgs(vector<char*> *argList)
{
    for (int i=0;i<argList->size();i++) {
        QString arg=(*argList)[i];
        bool done=false;
        if (arg=="-input") {
            do {
                QString possibleFile=(*argList)[++i];
                if (!possibleFile.startsWith("-")) {
                    inputStackFilepath=possibleFile;
                } else {
                    done=true;
                    i--; // rewind
                }
            } while(!done && i<(argList->size()-1));
        } else if (arg=="-pattern_channel") {
            QString patternChannelIndexString=(*argList)[++i];
            patternChannelIndex=patternChannelIndexString.toInt();
        } else if (arg=="-prefix") {
            outputPrefix=(*argList)[++i];
        } else if (arg=="-outputDir") {
            outputDirectoryPath=(*argList)[++i];
        } else if (arg=="-topLevelCompartmentMaskDir") {
            topLevelCompartmentMaskDirPath=(*argList)[++i];
        } else if (arg=="-outputResourceDir") {
            outputResourceDirPath=(*argList)[++i];
        } else if (arg=="-resourceDir") {
            resourceDirectoryPath=(*argList)[++i];
        }
    }
    if (topLevelCompartmentMaskDirPath.length()>0 && outputResourceDirPath.length()>0) {
        mode=MODE_COMPARTMENT_INDEX;
    } else {
        if (inputStackFilepath.length()<1) {
            qDebug() << "inputStackFilepath has invalid length";
            return 1;
        } else if (patternChannelIndex<0) {
            qDebug() << "patternChannelIndex must be greater than zero";
            return 1;
        } else if (outputPrefix.length()<1) {
            qDebug() << "outputPrefix has invalid length";
            return 1;
        } else if (outputDirectoryPath.length()<1) {
            qDebug() << "outputDirectoryPath has invalid length";
            return 1;
        } else if (resourceDirectoryPath.length()<1) {
            qDebug() << "resourceDirectoryPath has invalid length";
            return 1;
        }
        mode=MODE_ANNOTATE;
    }
    return 0;
}

v3d_uint8 * ScreenPatternAnnotator::create16Color8BitLUT()
{
    v3d_uint8 * lut16 = new v3d_uint8[256*3];

    for (int i=0;i<16;i++) {
        for (int j=0;j<16;j++) {
            int index=i*16+j;
            if (i==0) {
                lut16[index]    = 0;
                lut16[index+256]= 0;
                lut16[index+512]= 0;
            } else if (i==1) {
                lut16[index]    = 1;
                lut16[index+256]= 1;
                lut16[index+512]= 171;
            } else if (i==2) {
                lut16[index]    = 1;
                lut16[index+256]= 1;
                lut16[index+512]= 224;
            } else if (i==3) {
                lut16[index]    = 0;
                lut16[index+256]= 110;
                lut16[index+512]= 255;
            } else if (i==4) {
                lut16[index]    = 1;
                lut16[index+256]= 171;
                lut16[index+512]= 254;
            } else if (i==5) {
                lut16[index]    = 1;
                lut16[index+256]= 224;
                lut16[index+512]= 254;
            } else if (i==6) {
                lut16[index]    = 1;
                lut16[index+256]= 254;
                lut16[index+512]= 1;
            } else if (i==7) {
                lut16[index]    = 190;
                lut16[index+256]= 255;
                lut16[index+512]= 0;
            } else if (i==8) {
                lut16[index]    = 255;
                lut16[index+256]= 255;
                lut16[index+512]= 0;
            } else if (i==9) {
                lut16[index]    = 255;
                lut16[index+256]= 224;
                lut16[index+512]= 0;
            } else if (i==10) {
                lut16[index]    = 255;
                lut16[index+256]= 141;
                lut16[index+512]= 0;
            } else if (i==11) {
                lut16[index]    = 250;
                lut16[index+256]= 94;
                lut16[index+512]= 0;
            } else if (i==12) {
                lut16[index]    = 245;
                lut16[index+256]= 0;
                lut16[index+512]= 0;
            } else if (i==13) {
                lut16[index]    = 245;
                lut16[index+256]= 0;
                lut16[index+512]= 185;
            } else if (i==14) {
                lut16[index]    = 222;
                lut16[index+256]= 180;
                lut16[index+512]= 222;
            } else if (i==15) {
                lut16[index]    = 255;
                lut16[index+256]= 255;
                lut16[index+512]= 255;
            }
        }
    }
    return lut16;
}

// This method below assumes the heatmap has 256 entries in the order 256 entries for red, then 256 entries for green, then 256 entries for blue.
My4DImage * ScreenPatternAnnotator::create3DHeatmapFromChannel(My4DImage * sourceImage, V3DLONG sourceChannel, v3d_uint8 * lookupTable)
{
    if (sourceImage->getDatatype()!=V3D_UINT8) {
        qDebug() << "ScreenPatternAnnotator::create3DHeatmapFromChannel() only supports datatype=1";
        return 0;
    }
    My4DImage * targetImage = new My4DImage();
    targetImage->loadImage(sourceImage->getXDim(), sourceImage->getYDim(), sourceImage->getZDim(), 3 /* R, G, B */, sourceImage->getDatatype());
    v3d_uint8 * sourceData = sourceImage->getRawDataAtChannel(sourceChannel);
    v3d_uint8 * targetDataR = targetImage->getRawDataAtChannel(0L);
    v3d_uint8 * targetDataG = targetImage->getRawDataAtChannel(1L);
    v3d_uint8 * targetDataB = targetImage->getRawDataAtChannel(2L);
    V3DLONG totalVoxels=sourceImage->getXDim()*sourceImage->getYDim()*sourceImage->getZDim();
    for (V3DLONG i=0;i<totalVoxels;i++) {
        v3d_uint8 value=sourceData[i];
        targetDataR[i]=lookupTable[value];
        targetDataG[i]=lookupTable[value+256];
        targetDataB[i]=lookupTable[value+512];
    }
    return targetImage;
}



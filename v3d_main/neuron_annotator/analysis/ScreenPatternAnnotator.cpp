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

    return true;
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



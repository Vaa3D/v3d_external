#if defined(USE_Qt5)
//    #include <QtConcurrent>
#endif 


#include "ScreenPatternAnnotator.h"
#include "../utility/ImageLoader.h"
#include "../../v3d/histogramsimple.h"

#include "../terafly/src/presentation/theader.h"  //2015May PHC

const int ScreenPatternAnnotator::MODE_UNDEFINED=-1;
const int ScreenPatternAnnotator::MODE_ANNOTATE=0;
const int ScreenPatternAnnotator::MODE_COMPARTMENT_INDEX=1;
const int ScreenPatternAnnotator::MODE_INDEX=2;
const int ScreenPatternAnnotator::MODE_MASK_GUIDE=3;
const int ScreenPatternAnnotator::MODE_ARNIM_SCORE=4;
const int ScreenPatternAnnotator::MODE_SIMILARITY_SCORE=5;
const int ScreenPatternAnnotator::MODE_HEATMAP=6;

const int VIEWABLE_DIMENSION = 256;
const int VIEWABLE_BORDER = 10;
const double NORMALIZATION_CUTOFF=0.01;
const int CUBE_SIZE = 5;

const int LOWER_ZONE_THRESHOLD=31;
const int BA_20X_XYPLANE_Z_INDEX=105;
const int GHOST_INTENSITY_LEVEL=15;

const QString MIPS_SUBDIR("mips");
const QString SUPPORTING_SUBDIR("supportingFiles");
const QString NORMALIZED_SUBDIR("normalized");



ScreenPatternAnnotator::ScreenPatternAnnotator()
{
    mode=MODE_UNDEFINED;
    inputImage=0;
    inputImageCubified=0;
    patternChannelIndex=-1;
    lut16Color=0;
    imageGlobal16ColorImage=0;
    compositeMaskImage=0;
    compartmentIndexImage=0;
    compartmentIndexImageCubified=0;
    flipYWhenLoadingMasks=true;
}

ScreenPatternAnnotator::~ScreenPatternAnnotator()
{
    if (inputImage!=0) {
        delete inputImage;
    }
    if (inputImageCubified!=0) {
        delete inputImageCubified;
    }
    if (lut16Color!=0) {
        delete [] lut16Color;
    }
    if (imageGlobal16ColorImage!=0) {
        delete imageGlobal16ColorImage;
    }
    if (compartmentIndexImage!=0) {
        delete compartmentIndexImage;
    }
    if (compositeMaskImage!=0) {
      delete compositeMaskImage;
    }
    if (compartmentIndexImageCubified!=0) {
        delete compartmentIndexImageCubified;
    }
}

bool ScreenPatternAnnotator::execute()
{
    if (mode==MODE_COMPARTMENT_INDEX) {
        return createCompartmentIndex();
    } else if (mode==MODE_ANNOTATE) {
        return annotate();
    } else if (mode==MODE_INDEX) {
        return updateIndex();
    } else if (mode==MODE_MASK_GUIDE) {
      return createMaskGuide();
    } else if (mode==MODE_ARNIM_SCORE) {
      return arnimScore();
    } else if (mode==MODE_UNDEFINED) {
      return false;
    } else if (mode==MODE_SIMILARITY_SCORE) {
      return createSimilarityList();
    } else if (mode==MODE_HEATMAP) {
      return createV2Heatmap();
    }
    return false;
}

int ScreenPatternAnnotator::processArgs(vector<char*> *argList)
{
    bool flipYSet=false;
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
        } else if (arg=="-flipYWhenLoadingMasks") {
            QString tf = (*argList)[++i];
            if (tf=="true") {
                flipYWhenLoadingMasks=true;
                flipYSet=true;
            } else if (tf=="false") {
                flipYWhenLoadingMasks=false;
                flipYSet=true;
            } else {
                qDebug() << "-flipYWhenLoadingMasks must be followed by true or false";
                return 0;
            }
        } else if (arg=="-compartmentIndexFile") {
	  compartmentIndexFile=(*argList)[++i];
	} else if (arg=="-compartmentNameIndexFile") {
	  compartmentNameIndexFile=(*argList)[++i];
	} else if (arg=="-maskBinaryFile") {
	  maskBinaryFile=(*argList)[++i];
	} else if (arg=="-outputIndexFile") {
	  outputIndexFile=(*argList)[++i];
	} else if (arg=="-outputNameIndexFile") {
	  outputNameIndexFile=(*argList)[++i];
	} else if (arg=="-outputRGBFile") {
	  outputRGBFile=(*argList)[++i];
	} else if (arg=="-inputNameIndexFile") {
	  inputNameIndexFile=(*argList)[++i];
	} else if (arg=="-inputRGBFile") {
	  inputMaskRGBFile=(*argList)[++i];
	} else if (arg=="-outputMaskDirectory") {
	  outputMaskDirectoryPath=(*argList)[++i];
	} else if (arg=="-arnimScoreOutputFile") {
	  arnimScoreOutputFilepath=(*argList)[++i];
	} else if (arg=="-targetStack") {
	  targetStackFilepath=(*argList)[++i];
	} else if (arg=="-subjectStackList") {
	  subjectStackListFilepath=(*argList)[++i];
	} else if (arg=="-outputSimilarityList") {
	  outputSimilarityFilepath=(*argList)[++i];
	} else if (arg=="-convertStackHeatmapV1ToV2") {
	  heatmapV1Filepath=(*argList)[++i];
	  heatmapV2Filepath=(*argList)[++i];
	}
    }
    if (topLevelCompartmentMaskDirPath.length()>0 && outputResourceDirPath.length()>0 && flipYSet) {
      mode=MODE_COMPARTMENT_INDEX;
    } else if (compartmentIndexFile.length()>0) {
      mode=MODE_INDEX;
    } else if (outputMaskDirectoryPath.length()>0) {
      mode=MODE_MASK_GUIDE;
    } else if (arnimScoreOutputFilepath.length()>0) {
      mode=MODE_ARNIM_SCORE;
    } else if (outputSimilarityFilepath.length()>0) {
      mode=MODE_SIMILARITY_SCORE;
    } else if (heatmapV1Filepath.length()>0) {
      mode=MODE_HEATMAP;
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


bool ScreenPatternAnnotator::createMaskGuide() {

  // Step 1: Load the Name Index File
  QList<QString> maskNameList;
  QList<int> redList;
  QList<int> greenList;
  QList<int> blueList;
  QFile nameIndexQFile(inputNameIndexFile);
  if (!nameIndexQFile.open(QIODevice::ReadOnly)) {
    qDebug() << "Could not open file=" << inputNameIndexFile << " to read";
    return false;
  }
  int listIndex=0;
  while(!nameIndexQFile.atEnd()) {
    QString nameLine=nameIndexQFile.readLine();
    nameLine=nameLine.trimmed();
    if (nameLine.length()>0) {
      QList<QString> nameList=nameLine.split(QRegExp("\\s+"));
      QString indexString=nameList.at(0);
      int indexKey=indexString.toInt();
      if (indexKey!=listIndex+1) {
	qDebug() << "Out of sync with index parse at line=" << nameLine;
	return false;
      }
      QString name=nameList.at(1);
      int listLength=nameList.length();
      if (listLength<8) {
	qDebug() << "Could not parse numbers from line=" << nameLine;
	return false;
      }
      QString redString=nameList.at(listLength-4);
      QString greenString=nameList.at(listLength-3);
      QString blueString=nameList.at(listLength-2);
      int red=redString.toInt();
      int green=greenString.toInt();
      int blue=blueString.toInt();
      maskNameList.append(name);
      redList.append(red);
      greenList.append(green);
      blueList.append(blue);
      listIndex++;
    }
  }
  nameIndexQFile.close();

  // Step 2: Create Mask Output Directory
  QDir maskOutputQDir(outputMaskDirectoryPath);
  if (!maskOutputQDir.exists()) {
    QDir().mkdir(outputMaskDirectoryPath);
  }

  // Step 3: Load RGB File and save copy to mask dir
  QFile rgbMaskQFile(inputMaskRGBFile);
  QFileInfo rgbMaskQFileInfo=QFileInfo(rgbMaskQFile);
  ImageLoader rgbSourceImageLoader;

  My4DImage * rgbSourceMask = rgbSourceImageLoader.loadImage(rgbMaskQFileInfo.absoluteFilePath());
  QString maskPath(outputMaskDirectoryPath);
  maskPath.append(QDir::separator());
  maskPath.append("Mask.v3dpbd");
  rgbSourceImageLoader.saveImage(rgbSourceMask, maskPath);

  // Step 4: Create MIP for main RGB file
  My4DImage * rgbSourceMIP = AnalysisTools::createMIPFromImage(rgbSourceMask);
  ImageLoader rgbSourceMIPSaver;
  QString maskMIPPath(outputMaskDirectoryPath);
  maskMIPPath.append(QDir::separator());
  maskMIPPath.append("Mask_MIP.tif");
  rgbSourceMIPSaver.saveImage(rgbSourceMIP, maskMIPPath);
  delete rgbSourceMIP;

  // Step 5: Step through each mask, and create a mask stack with ghost frame
  for (int maskIndex=0;maskIndex<maskNameList.length();maskIndex++) {
    QString maskName=maskNameList.at(maskIndex);

    // Create ghost stack
    My4DImage * ghostImage = createMaskIndexGhostImage(rgbSourceMask, maskIndex+1, redList, greenList, blueList);
    ImageLoader ghostSaver;
    QString ghostStackFilename(outputMaskDirectoryPath);
    ghostStackFilename.append(QDir::separator());
    ghostStackFilename.append(maskName);
    ghostStackFilename.append(".v3dpbd");
    ghostSaver.saveImage(ghostImage, ghostStackFilename);

    // Create MIP
    My4DImage * ghostImageMIP = AnalysisTools::createMIPFromImage(ghostImage);
    ImageLoader ghostMIPSaver;
    QString ghostMIPFilename(outputMaskDirectoryPath);
    ghostMIPFilename.append(QDir::separator());
    ghostMIPFilename.append(maskName);
    ghostMIPFilename.append("_MIP");
    ghostMIPFilename.append(".tif");
    ghostMIPSaver.saveImage(ghostImageMIP, ghostMIPFilename);

    // Cleanup
    delete ghostImageMIP;
    delete ghostImage;
  }

  // Final cleanup
  delete rgbSourceMask;
  return true;
}

// This method creates an image stack which is entirely empty, except for two
// subspaces: (1) mask corresponding to index is copied from mask (2) area outside
// the mask is made a shade of gray.

My4DImage * ScreenPatternAnnotator::createMaskIndexGhostImage(My4DImage * rgbMask, int index, QList<int> redList, QList<int> greenList, QList<int> blueList) {
  V3DLONG cdim=rgbMask->getCDim();
  V3DLONG xdim=rgbMask->getXDim();
  V3DLONG ydim=rgbMask->getYDim();
  V3DLONG zdim=rgbMask->getZDim();
  My4DImage * ghostImage = new My4DImage();
  ghostImage->loadImage(xdim, ydim, zdim, 3, V3D_UINT8);

  v3d_uint8 * rChan=rgbMask->getRawDataAtChannel(0);
  v3d_uint8 * gChan=rgbMask->getRawDataAtChannel(1);
  v3d_uint8 * bChan=rgbMask->getRawDataAtChannel(2);

  v3d_uint8 * rG=ghostImage->getRawDataAtChannel(0);
  v3d_uint8 * gG=ghostImage->getRawDataAtChannel(1);
  v3d_uint8 * bG=ghostImage->getRawDataAtChannel(2);

  int maskIndexLength=redList.length();
  for (V3DLONG z=0;z<zdim;z++) {
    for (V3DLONG y=0;y<ydim;y++) {
      for (V3DLONG x=0;x<xdim;x++) {
	V3DLONG p=z*ydim*xdim+y*xdim+x;
	int r=rChan[p];
	int g=gChan[p];
	int b=bChan[p];
	if (r==0 && g==0 && b==0) {
	  rG[p]=0;
	  gG[p]=0;
	  bG[p]=0;
	} else {
	  int m=0;
	  for (;m<maskIndexLength;m++) {
	    if (redList.at(m)==r &&
		greenList.at(m)==g &&
		blueList.at(m)==b) {
	      break;
	    }
	  }
	  if (m==maskIndexLength) {
	    qDebug() << "Could not locate color in mask color list r=" << r << " g=" << g << " b=" << b;
	    return NULL;
	  }
	  if (m==index-1) {
	    rG[p]=rChan[p];
	    gG[p]=gChan[p];
	    bG[p]=bChan[p];
	  } else {
	    V3DLONG grayValue=(m*7633997)%32;
	    char gray=grayValue+16;
	    rG[p]=gray;
	    gG[p]=gray;
	    bG[p]=gray;
	  }
	}
      }
    }
  }
  return ghostImage;
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
    if (compartmentIndexImageCubified!=0) {
        delete compartmentIndexImageCubified;
    }
    compartmentIndexImageCubified=0;
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
            if (!(abbreviation.size()>0)) {
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

            V3DLONG zmax=maskImage->getZDim();
            V3DLONG ymax=maskImage->getYDim();
            V3DLONG xmax=maskImage->getXDim();

            for (V3DLONG z=0;z<zmax;z++) {
                V3DLONG zoffset=z*xmax*ymax;
                for (V3DLONG y=0;y<ymax;y++) {
                    V3DLONG yoffset=y*xmax+zoffset;
                    V3DLONG invertedYOffset=(ymax-1-y)*xmax+zoffset;
                    for (V3DLONG x=0;x<xmax;x++) {
                        V3DLONG offset=x+yoffset;
                        V3DLONG invertedOffset=x+invertedYOffset;
                        if (maskData[offset]>0) {
                            if (flipYWhenLoadingMasks) {
                                indexData[invertedOffset]=index;
                            } else {
                                indexData[offset]=index;
                            }
                            maskSizeCount++;
                        }
                    }
                }
            }

            qDebug() << "For abbreviation " << abbreviation << " found mask of size=" << maskSizeCount << " at index " << index;
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
    for (int i=0;i<dashList.size();i++) {
      qDebug() << "Dash " << i << "=" << dashList.at(i);
    }
    QString abbrString=dashList.at(2);
    qDebug() << "abbrString=" << abbrString;
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

QString ScreenPatternAnnotator::getMipsSubdirectoryPath() {
    return getOutputSubdirectory(MIPS_SUBDIR);
}

QString ScreenPatternAnnotator::getSupportingSubdirectoryPath() {
    return getOutputSubdirectory(SUPPORTING_SUBDIR);
}

QString ScreenPatternAnnotator::getNormalizedSubdirectoryPath() {
    return getOutputSubdirectory(NORMALIZED_SUBDIR);
}

QString ScreenPatternAnnotator::getOutputSubdirectory(QString dirName) {
    QString dirPath("");
    if (outputDirectoryPath.length()>0) {
        dirPath.append(outputDirectoryPath);
        dirPath.append(QDir::separator());
        dirPath.append(dirName);
    }
    return dirPath;
}

bool ScreenPatternAnnotator::createOrVerifyDirectory(QString dirPath) {
    if (!QFileInfo(dirPath).isDir()) {
        return QDir().mkpath(dirPath);
    } else {
        return true;
    }
}

bool ScreenPatternAnnotator::createOutputDirTree() {
    if (outputDirectoryPath.length()<1) {
        qDebug() << "createOutputDirTree() outputDirectoryPath must be defined";
        return false;
    }
    QString mipsDirPath=getMipsSubdirectoryPath();
    QString supportingDirPath=getSupportingSubdirectoryPath();
    QString normalizedDirPath=getNormalizedSubdirectoryPath();
    if ( createOrVerifyDirectory(outputDirectoryPath) &&
         createOrVerifyDirectory(mipsDirPath) &&
         createOrVerifyDirectory(supportingDirPath) &&
         createOrVerifyDirectory(normalizedDirPath) ) {
        return true;
    } else {
        return false;
    }
}

bool ScreenPatternAnnotator::loadCompartmentIndex()
{
    // Load Compartment Index
    ImageLoader compartmentIndexLoader;
    if (compartmentIndexImage!=0) {
        delete compartmentIndexImage;
    }
    compartmentIndexImage=new My4DImage();
    QString resourceDirectoryPathCopy=resourceDirectoryPath;
    QString compartmentIndexImageFilepath=resourceDirectoryPathCopy.append(QDir::separator()).append("maskIndex.v3dpbd");
    if (!compartmentIndexLoader.loadImage(compartmentIndexImage, compartmentIndexImageFilepath)) {
        qDebug() << "loadCompartmentIndex: Could not load compartmentIndexImage from file=" << compartmentIndexImageFilepath;
        return false;
    }
    return true;
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
    qDebug() << "Cubifying inputImage";
    inputImageCubified=AnalysisTools::cubifyImage(inputImage, CUBE_SIZE, AnalysisTools::CUBIFY_TYPE_AVERAGE);
    qDebug() << "Done";

    // Compute Global 256-bin Histogram
    V3DLONG xSize=inputImage->getXDim();
    V3DLONG ySize=inputImage->getYDim();
    V3DLONG zSize=inputImage->getZDim();
    V3DLONG channelSize=xSize*ySize*zSize;
    global256BinHistogram.compute(inputImage->getRawData() + channelSize*patternChannelIndex ,channelSize);
    V3DLONG* global256BinHistogramArray=global256BinHistogram.getHistogram();

    // Compute Zone Thresholds using Histogram
    zoneThresholds[0]=LOWER_ZONE_THRESHOLD; // hard-coded
    V3DLONG total=0;
    V3DLONG weight=0;
    for (int h=(LOWER_ZONE_THRESHOLD+1);h<256;h++) {
        total+=global256BinHistogramArray[h];
        weight+=h*global256BinHistogramArray[h];
    }
    V3DLONG A1 = LOWER_ZONE_THRESHOLD+1;
    V3DLONG As = A1+1;
    V3DLONG A2 = As+1;
    if (total>0) {
        As = weight/total;
        A1 = (As-LOWER_ZONE_THRESHOLD)/2+LOWER_ZONE_THRESHOLD;
        total=0;
        weight=0;
        for (int h=As;h<256;h++) {
            total+=global256BinHistogramArray[h];
            weight+=h*global256BinHistogramArray[h];
        }
        A2 = weight/total;
    }
    qDebug() << "As=" << As << " A1=" << A1 << " A2=" << A2;
    zoneThresholds[1]=A1;
    zoneThresholds[2]=As;
    zoneThresholds[3]=A2;
    for (int t=0;t<4;t++) {
        quantifierList.append(QString("Global.t%1=%2").arg(t).arg(zoneThresholds[t]));
    }

    // Compute Global Quantifiers
    V3DLONG zt[5];
    for (int g=0;g<5;g++) {
        zt[g]=0;
    }
    v3d_uint8 * pData=inputImage->getRawDataAtChannel(patternChannelIndex);
    V3DLONG totalVoxels=0;
    for (V3DLONG p=0;p<inputImage->getTotalUnitNumberPerChannel();p++) {
        v3d_uint8 v=pData[p];
        totalVoxels++;
        if (v<=zoneThresholds[0]) {
            zt[0]++;
        } else if (v<=zoneThresholds[1]) {
            zt[1]++;
        } else if (v<=zoneThresholds[2]) {
            zt[2]++;
        } else if (v<=zoneThresholds[3]) {
            zt[3]++;
        } else {
            zt[4]++;
        }
    }
    // Normalize
    for (int g=0;g<5;g++) {
        if (totalVoxels>0) {
            globalZoneLevels[g]=(zt[g]*1.0)/(1.0*totalVoxels);
        } else {
            globalZoneLevels[g]=0;
        }
        QString gLine=QString("Global.z%1=%2").arg(g).arg(globalZoneLevels[g]);
        quantifierList.append(gLine);
    }

    if (!createOutputDirTree()) {
        qDebug() << "ERROR: could not create output directory path=" << outputDirectoryPath;
        return false;
    }

    // Create Global 16-Color Image
    lut16Color=AnalysisTools::create16Color8BitLUT_fiji();
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
    // First - we need to add an outline for the pattern to make sense - we will add a faint border from a
    // compartment channel plane.
    My4DImage * mip=AnalysisTools::createMIPFromImageByLUT(imageGlobal16ColorImage, lut16Color);
    addXYGhostPlaneFrom3DTo2D(inputImage, BA_20X_XYPLANE_Z_INDEX /* Z index */, 2 /* channel */, mip);
    ImageLoader imageLoaderForMip;
    qDebug() << "Saving heatmap16ColorMIP.tif using AnalysisTools::createMIPFromImageByLUT";
    imageLoaderForMip.saveImage(mip, returnFullPathWithOutputPrefix("heatmap16ColorMIP.tif", MIPS_SUBDIR));

    // Load Compartment Index
    if (!loadCompartmentIndex()) {
      qDebug() << "Error calling loadCompartmentIndex()";
      return false;
    }

    qDebug() << "Cubifying compartmentIndexImage";
    compartmentIndexImageCubified=AnalysisTools::cubifyImage(compartmentIndexImage, CUBE_SIZE, AnalysisTools::CUBIFY_TYPE_MODE);
    qDebug() << "Done with cubifyImage()";

    // Create Composite Mask
    ImageLoader imageLoaderForComposite;
    if (compositeMaskImage!=0) {
      delete compositeMaskImage;
    }
    compositeMaskImage = createCompositeMaskImage(imageGlobal16ColorImage, compartmentIndexImage);
    QString compositeFilepathToSave(returnFullPathWithOutputPrefix("compositeMask.v3dpbd"));
    qDebug() << "Saving compositeMaskImage to file=" << compositeFilepathToSave;
    bool saveCompositeStatus=imageLoaderForComposite.saveImage(compositeMaskImage, compositeFilepathToSave);
    if (!saveCompositeStatus) {
      qDebug() << "ScreenPatternAnnotator::execute() Error during save of composite mask image";
      return false;
    }

    // Create composite MIP
    My4DImage * compositeMip=AnalysisTools::createMIPFromImageByLUT(compositeMaskImage, lut16Color);
    ImageLoader imageLoaderForCompositeMip;
    qDebug() << "Saving compositeMaskMIP.tif using AnalysisTools::createMIPFromImageByLUT";
    imageLoaderForCompositeMip.saveImage(compositeMip, returnFullPathWithOutputPrefix("compositeMaskMIP.tif", MIPS_SUBDIR));

    // Load Abbreviation Index Map
    if (!loadMaskNameIndex()) {
      qDebug() << "Error calling loadMaskNameIndex()";
      return false;
    }

    // Perform Compartment Annotations
    QList<int> compartmentIndexList=compartmentIndexAbbreviationMap.keys();
    for (int k=0;k<compartmentIndexList.size();k++) {
        int index=compartmentIndexList.at(k);
        QString abbreviation=compartmentIndexAbbreviationMap[index];
        createCompartmentAnnotation(index, abbreviation);
    }

    // Write out Quantifier File
    QString quantifierFilePath(returnFullPathWithOutputPrefix("quantifiers.txt", SUPPORTING_SUBDIR));
    QFile quantifierFile(quantifierFilePath);
    if (!quantifierFile.open(QIODevice::WriteOnly)) {
        qDebug() << "Could not open file=" << quantifierFilePath;
        return false;
    }
    QTextStream quantifierOutputStream(&quantifierFile);
    for (int q=0;q<quantifierList.size();q++) {
        quantifierOutputStream << quantifierList.at(q) << "\n";
    }
    quantifierOutputStream.flush();
    quantifierFile.close();

    return true;
}

bool ScreenPatternAnnotator::loadMaskNameIndex()
{
    // Load Abbreviation Index Map
    compartmentIndexAbbreviationMap.clear();
    QString resourceDirectoryPathCopy2=resourceDirectoryPath;
    QString abbreviationMapFilepath=resourceDirectoryPathCopy2.append(QDir::separator()).append("maskNameIndex.txt");
    QFile abbreviationMapFile(abbreviationMapFilepath);
    qDebug() << "Opening abbreviationMap file=" << abbreviationMapFilepath;
    if (!abbreviationMapFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open file=" << abbreviationMapFilepath << " to read";
        return false;
    }
    qDebug() << "Successfully opened file";
    while(!abbreviationMapFile.atEnd()) {
        QString abLine=abbreviationMapFile.readLine();
        abLine=abLine.trimmed();
	if (abLine.length()>0) {
	  QList<QString> abList=abLine.split(QRegExp("\\s+"));
	  QString indexString=abList.at(0);
	  int indexKey=indexString.toInt();
	  QString abbreviationValue=abList.at(1);
	  compartmentIndexAbbreviationMap[indexKey]=abbreviationValue;
	}
    }
    abbreviationMapFile.close();
    qDebug() << "Done with abbreviationMapFile";
    return true;
}

// Here, we will treat any non-zero position in the index image as being part-of the composite, and then
// filter the sourceImage accordingly

My4DImage * ScreenPatternAnnotator::createCompositeMaskImage(My4DImage * sourceImage, My4DImage * indexImage) {
  My4DImage * compositeImage=new My4DImage();
  compositeImage->loadImage(sourceImage->getXDim(), sourceImage->getYDim(), sourceImage->getZDim(), sourceImage->getCDim(), V3D_UINT8);
  int xdim=indexImage->getXDim();
  int ydim=indexImage->getYDim();
  int zdim=indexImage->getZDim();
  int cdim=sourceImage->getCDim();
  if (xdim!=sourceImage->getXDim() ||
      ydim!=sourceImage->getYDim() ||
      zdim!=sourceImage->getZDim()) {
    qDebug() << "ScreenPatternAnnotator::createCompositeMaskImage() source and index dimensions do not match";
    return 0;
  }
  QList<v3d_uint8*> sdataList;
  QList<v3d_uint8*> cdataList;
  for (int c=0;c<cdim;c++) {
    v3d_uint8* sdata=sourceImage->getRawDataAtChannel(c);
    sdataList.append(sdata);
    v3d_uint8* cdata=compositeImage->getRawDataAtChannel(c);
    cdataList.append(cdata);
  }
  v3d_uint8* idata=indexImage->getRawDataAtChannel(0);
  for (V3DLONG z=0;z<zdim;z++) {
    for (V3DLONG y=0;y<ydim;y++) {
      for (V3DLONG x=0;x<xdim;x++) {
	V3DLONG position=z*y*x+y*x+x;
	for (int c=0;c<cdim;c++) {
	  v3d_uint8* sdata=sdataList[c];
	  v3d_uint8* cdata=cdataList[c];
	  if (idata[position]==0) {
	    cdata[position]=0;
	  } else {
	    cdata[position]=sdata[position];
	  }
	}
      }
    }
  }
  return compositeImage;
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
//     (a) We will create 5 zones of intensity ranges:
//
//           (1) zone 1 is 0-31
//
//           Call As the average of all voxels above 31, A1 the average of those below As but above 31, and A2 the average of those above As
//
//           (2) zone 2 is 32-A1
//
//           (3) zone 3 is A1-As
//
//           (4) zone 4 is As-A2
//
//           (5) zone 5 is A2 and above
//
//     (b) We will score the % of voxels in each zone for each compartment
//
//     (c) We will take the 5x5x5 cubified version and then re-score the percentages
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////


void ScreenPatternAnnotator::createCompartmentAnnotation(int index, QString abbreviation) {

    BoundingBox3D bb=findBoundingBox3DFromIndex(index);
    qDebug() << "Bounding box for " << abbreviation << " = " << bb.x0 << " " << bb.x1 << " " << bb.y0 << " " << bb.y1 << " " << bb.z0 << " " << bb.z1;

    My4DImage * compartmentHeatmap=createSub3DImageFromMask(imageGlobal16ColorImage, index, bb);
    if (compartmentHeatmap==0) {
        qDebug() << "compartmentHeatmap returned as 0";
        return;
    }
    qDebug() << "Created compartment heatmap with dimensions " << compartmentHeatmap->getXDim() << " " << compartmentHeatmap->getYDim() << " " << compartmentHeatmap->getZDim();

    My4DImage * compartmentImage=AnalysisTools::getChannelSubImageFromMask(inputImage, compartmentIndexImage, patternChannelIndex, index, bb, false /* normalize */, 0.0);
    qDebug() << "Created compartmentImage using AnalysisTools::getChannelSubImageFromMask";

    My4DImage * compartmentNormalizedImage=AnalysisTools::getChannelSubImageFromMask(inputImage, compartmentIndexImage, patternChannelIndex, index, bb, true /* normalize */, NORMALIZATION_CUTOFF);
    qDebug() << "Created normalized image for compartment";

    My4DImage * normalizedCompartmentHeatmap=create3DHeatmapFromChannel(compartmentNormalizedImage, 0 /* only one channel */, lut16Color);
    qDebug() << "Created normalizedCompartmentHeatmap with dimensions " << normalizedCompartmentHeatmap->getXDim() << " " <<
                normalizedCompartmentHeatmap->getYDim() << " " << normalizedCompartmentHeatmap->getZDim();

    My4DImage * normalizedCompartmentHeatmapMIP=AnalysisTools::createMIPFromImageByLUT(normalizedCompartmentHeatmap, lut16Color);
    qDebug() << "Created normalizedCompartmentHeatmapMIP";

    ImageLoader imageLoaderForSave;
    bool saveStatus=false;

    My4DImage * compartmentHeatmapFullSize=createViewableImage(compartmentHeatmap, VIEWABLE_BORDER);
    QString filenameCompartmentHeatmapFullSize=abbreviation;
    filenameCompartmentHeatmapFullSize.append("_heatmap16Color.v3dpbd");
    QString savepathCompartmentHeatmapFullSize(returnFullPathWithOutputPrefix(filenameCompartmentHeatmapFullSize));
    qDebug() << "Saving " << abbreviation << " 16-color heatmap to file=" << savepathCompartmentHeatmapFullSize;
    saveStatus=imageLoaderForSave.saveImage(compartmentHeatmapFullSize, savepathCompartmentHeatmapFullSize);
    if (!saveStatus) {
        qDebug() << "ScreenPatternAnnotator::createCompartmentAnnotation() Error during save of " << savepathCompartmentHeatmapFullSize;
    }
    My4DImage * viewableCompartmentMIP=AnalysisTools::createMIPFromImageByLUT(compartmentHeatmapFullSize, lut16Color);
    QString filenameCompartmentHeatmapFullSizeMIP=abbreviation;
    filenameCompartmentHeatmapFullSizeMIP.append("_heatmap16ColorMIP.tif");
    QString savepathCompartmentHeatmapFullSizeMIP(returnFullPathWithOutputPrefix(filenameCompartmentHeatmapFullSizeMIP, MIPS_SUBDIR));
    qDebug() << "Saving " << abbreviation << " 16-color heatmap MIP to file=" << savepathCompartmentHeatmapFullSizeMIP;
    saveStatus=imageLoaderForSave.saveImage(viewableCompartmentMIP, savepathCompartmentHeatmapFullSizeMIP);
    if (!saveStatus) {
        qDebug() << "ScreenPatternAnnotator::createCompartmentAnnotation() Error during save of " << savepathCompartmentHeatmapFullSizeMIP;
    }
    delete viewableCompartmentMIP;
    delete compartmentHeatmapFullSize;

    My4DImage * normalizedCompartmentHeatmapFullSize=createViewableImage(normalizedCompartmentHeatmap, VIEWABLE_BORDER);
    QString filenameNormalizedCompartmentHeatmapFullSize=abbreviation;
    filenameNormalizedCompartmentHeatmapFullSize.append("_normalized_heatmap16Color.v3dpbd");
    QString savepathNormalizedCompartmentHeatmapFullSize(returnFullPathWithOutputPrefix(filenameNormalizedCompartmentHeatmapFullSize, NORMALIZED_SUBDIR));
    qDebug() << "Saving " << abbreviation << " normalized 16-color heatmap to file=" << savepathNormalizedCompartmentHeatmapFullSize;
    saveStatus=imageLoaderForSave.saveImage(normalizedCompartmentHeatmapFullSize, savepathNormalizedCompartmentHeatmapFullSize);
    if (!saveStatus) {
        qDebug() << "ScreenPatternAnnotator::createCompartmentAnnotation() Error during save of " << savepathNormalizedCompartmentHeatmapFullSize;
    }
    My4DImage * viewableNormalizedCompartmentMIP=AnalysisTools::createMIPFromImageByLUT(normalizedCompartmentHeatmapFullSize, lut16Color);
    QString filenameNormalizedCompartmentHeatmapFullSizeMIP=abbreviation;
    filenameNormalizedCompartmentHeatmapFullSizeMIP.append("_normalized_heatmap16ColorMIP.tif");
    QString savepathNormalizedCompartmentHeatmapFullSizeMIP(returnFullPathWithOutputPrefix(filenameNormalizedCompartmentHeatmapFullSizeMIP, MIPS_SUBDIR));
    qDebug() << "Saving " << abbreviation << " 16-color normalized heatmap MIP to file=" << savepathNormalizedCompartmentHeatmapFullSizeMIP;
    saveStatus=imageLoaderForSave.saveImage(viewableNormalizedCompartmentMIP, savepathNormalizedCompartmentHeatmapFullSizeMIP);
    if (!saveStatus) {
        qDebug() << "ScreenPatternAnnotator::createCompartmentAnnotation() Error during save of " << savepathNormalizedCompartmentHeatmapFullSizeMIP;
    }
    delete viewableNormalizedCompartmentMIP;
    delete normalizedCompartmentHeatmapFullSize;

    double * compartmentZoneFractions = quantifyCompartmentZones(inputImage, compartmentIndexImage, index, bb);

    for (int g=0;g<5;g++) {
        QString gLine=QString("%1.z%2=%3").arg(abbreviation).arg(g).arg(compartmentZoneFractions[g]);
        quantifierList.append(gLine);
    }

    BoundingBox3D cubeBB;
    cubeBB.x0 = bb.x0/CUBE_SIZE;
    cubeBB.x1 = bb.x1/CUBE_SIZE;
    cubeBB.y0 = bb.y0/CUBE_SIZE;
    cubeBB.y1 = bb.y1/CUBE_SIZE;
    cubeBB.z0 = bb.z0/CUBE_SIZE;
    cubeBB.z1 = bb.z1/CUBE_SIZE;

    V3DLONG cubeXmax=inputImageCubified->getXDim();
    V3DLONG cubeYmax=inputImageCubified->getYDim();
    V3DLONG cubeZmax=inputImageCubified->getZDim();

    if (cubeBB.x1>=cubeXmax) {
      cubeBB.x1=cubeXmax-1;
      qDebug() << "cubeBB.x1=" << cubeBB.x1;
    }
    if (cubeBB.y1>=cubeYmax) {
      cubeBB.y1=cubeYmax-1;
      qDebug() << "cubeBB.y1=" << cubeBB.y1;
    }
    if (cubeBB.z1>=cubeZmax) {
      cubeBB.z1=cubeZmax-1;
      qDebug() << "cubeBB.z1=" << cubeBB.z1;
    }

    double * compartmentCubeZoneFractions = quantifyCompartmentZones(inputImageCubified, compartmentIndexImageCubified, index, cubeBB);

    for (int g=0;g<5;g++) {
        QString gLine=QString("%1.c%2=%3").arg(abbreviation).arg(g).arg(compartmentCubeZoneFractions[g]);
        quantifierList.append(gLine);
    }

    // Cleanup
    delete compartmentHeatmap;
    delete compartmentImage;
    delete compartmentNormalizedImage;
    delete normalizedCompartmentHeatmap;
    delete normalizedCompartmentHeatmapMIP;
    delete [] compartmentZoneFractions;
    delete [] compartmentCubeZoneFractions;
}

double * ScreenPatternAnnotator::quantifyCompartmentZones(My4DImage * sourceImage, My4DImage * compartmentIndex, int index, BoundingBox3D bb) {

    V3DLONG cz[5];
    for (int z=0;z<5;z++) {
        cz[z]=0;
    }
    v3d_uint8 * pData=sourceImage->getRawDataAtChannel(patternChannelIndex);
    v3d_uint8 * iData=compartmentIndex->getRawData();
    V3DLONG zmax=sourceImage->getZDim();
    V3DLONG ymax=sourceImage->getYDim();
    V3DLONG xmax=sourceImage->getXDim();
    V3DLONG compartmentVoxelCount=0;

    for (V3DLONG z=bb.z0;z<=bb.z1;z++) {
        V3DLONG zoffset=z*ymax*xmax;
        for (V3DLONG y=bb.y0;y<=bb.y1;y++) {
            V3DLONG yoffset=y*xmax+zoffset;
            for (V3DLONG x=bb.x0;x<=bb.x1;x++) {
                V3DLONG offset=x+yoffset;
                if (iData[offset]==index) {
                    compartmentVoxelCount++;
                    v3d_uint8 v=pData[offset];
                    if (v<=zoneThresholds[0]) {
                        cz[0]++;
                    } else if (v<=zoneThresholds[1]) {
                        cz[1]++;
                    } else if (v<=zoneThresholds[2]) {
                        cz[2]++;
                    } else if (v<=zoneThresholds[3]) {
                        cz[3]++;
                    } else {
                        cz[4]++;
                    }
                }
            }
        }
    }

    double * compartmentZoneFractions = new double[5];
    for (int z=0;z<5;z++) {
        double c=cz[z];
        double v=compartmentVoxelCount;
        double r=0.0;
        if (v>0.0) {
            r=c/v;
        }
        compartmentZoneFractions[z]=r;
    }
    return compartmentZoneFractions;
}

My4DImage * ScreenPatternAnnotator::createViewableImage(My4DImage * sourceImage, int borderSize) {

    V3DLONG xmax=sourceImage->getXDim();
    V3DLONG ymax=sourceImage->getYDim();
    V3DLONG zmax=sourceImage->getZDim();
    V3DLONG cmax=sourceImage->getCDim();

    My4DImage * targetImage = new My4DImage();
    double targetRatio=1.0;
    V3DLONG xViewCenterOffset=0;
    V3DLONG yViewCenterOffset=0;
    if (xmax>=ymax) {
        targetRatio=( (VIEWABLE_DIMENSION-2*borderSize)*1.0) / (xmax*1.0);
        double originalYPosition=borderSize+(ymax*targetRatio)/2.0;
        double viewYPosition=VIEWABLE_DIMENSION/2.0;
        yViewCenterOffset=viewYPosition-originalYPosition;
    } else {
        targetRatio=( (VIEWABLE_DIMENSION-2*borderSize)*1.0) / (ymax*1.0);
        double originalXPosition=borderSize+(xmax*targetRatio)/2.0;
        double viewXPosition=VIEWABLE_DIMENSION/2.0;
        xViewCenterOffset=viewXPosition-originalXPosition;
    }
    V3DLONG xv=VIEWABLE_DIMENSION;
    V3DLONG yv=VIEWABLE_DIMENSION;
    V3DLONG zv=zmax*targetRatio+2*borderSize;

    targetImage->loadImage(xv, yv, zv, sourceImage->getCDim(), V3D_UINT8 /* datatype */);
    memset(targetImage->getRawData(), 0, targetImage->getTotalBytes());

    for (int c=0;c<sourceImage->getCDim();c++) {
        v3d_uint8 * sData=sourceImage->getRawDataAtChannel(c);
        v3d_uint8 * vData=targetImage->getRawDataAtChannel(c);
        for (V3DLONG z=borderSize;z<(zv-borderSize);z++) {
            for (V3DLONG y=borderSize;y<(yv-borderSize);y++) {
                for (V3DLONG x=borderSize;x<(xv-borderSize);x++) {
                    V3DLONG sx=((x-borderSize)*1.0+0.5)/targetRatio;
                    V3DLONG sy=((y-borderSize)*1.0+0.5)/targetRatio;
                    V3DLONG sz=((z-borderSize)*1.0+0.5)/targetRatio;
                    bool outOfBounds=false;
                    if (sx<0) {
                        outOfBounds=true;
                    } else if (sx>=xmax) {
                        outOfBounds=true;
                    }
                    if (sy<0) {
                        outOfBounds=true;
                    } else if (sy>=ymax) {
                        outOfBounds=true;
                    }
                    if (sz<0) {
                        outOfBounds=true;
                    } else if (sz>=zmax) {
                        outOfBounds=true;
                    }
                    V3DLONG sPosition=sz*xmax*ymax+sy*xmax+sx;
                    V3DLONG tPosition=z*xv*yv+(y+yViewCenterOffset)*xv+(x+xViewCenterOffset);
                    if (tPosition>targetImage->getTotalUnitNumberPerChannel()) {
                        qDebug() << "ERROR: tPosition=" << tPosition << " and total channel unit size=" << targetImage->getTotalUnitNumberPerChannel();
                        return 0;
                    }
                    if (outOfBounds) {
                        vData[tPosition]=0;
                    } else {
                        vData[tPosition]=sData[sPosition];
                    }
                }
            }
        }
    }
    return targetImage;
}





My4DImage * ScreenPatternAnnotator::createNormalizedImage(My4DImage * sourceImage, int maskIndex) {
    if (sourceImage->getDatatype()!=V3D_UINT8) {
        qDebug() << "ERROR - createNormalizedImage only supports datatype=1";
        return 0;
    }
    sourceImage->updateminmaxvalues();
    My4DImage * normalizedImage=new My4DImage();
    normalizedImage->loadImage(sourceImage->getXDim(), sourceImage->getYDim(), sourceImage->getZDim(), sourceImage->getCDim(), V3D_UINT8);
    for (int c=0;c<sourceImage->getCDim();c++) {
        double min=sourceImage->getChannalMinIntensity(c);
        double max=sourceImage->getChannalMaxIntensity(c);
        qDebug() << "createNormalizedImage for c=" << c << " min=" << min << " max=" << max;
        v3d_uint8 * cdata=sourceImage->getRawDataAtChannel(c);
        v3d_uint8 * ndata=normalizedImage->getRawDataAtChannel(c);
        for (V3DLONG p=0;p<sourceImage->getTotalUnitNumberPerChannel();p++) {
            double value=cdata[p];
            double nvalue=0.0;
            if ((max-min)!=0.0) {
                nvalue=255.0*((value-min)/(max-min));
            }
            if (nvalue>255.0) {
                nvalue=255.0;
            } else if (nvalue<0.0) {
                nvalue=0.0;
            }
            ndata[p] = nvalue;
        }
    }
    return normalizedImage;
}

// Here we will iterate through the mask and add the corresponding
My4DImage * ScreenPatternAnnotator::createSub3DImageFromMask(My4DImage * sourceImage, int index, BoundingBox3D bb) {

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
BoundingBox3D ScreenPatternAnnotator::findBoundingBox3DFromIndex(int index) {
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
    BoundingBox3D bb;
    bb.x0=x0;
    bb.x1=x1;
    bb.y0=y0;
    bb.y1=y1;
    bb.z0=z0;
    bb.z1=z1;
    return bb;
}

QString ScreenPatternAnnotator::returnFullPathWithOutputPrefix(QString filename, QString subdir) {
    QString outputDirectoryPathCopy=outputDirectoryPath;
    QString outputPrefixCopy=outputPrefix;
    return outputDirectoryPathCopy.append(QDir::separator()).append(subdir).append(QDir::separator()).append(outputPrefixCopy).append("_").append(filename);
}

QString ScreenPatternAnnotator::returnFullPathWithOutputPrefix(QString filename) {
    QString outputDirectoryPathCopy=outputDirectoryPath;
    QString outputPrefixCopy=outputPrefix;
    return outputDirectoryPathCopy.append(QDir::separator()).append(outputPrefixCopy).append("_").append(filename);
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


void ScreenPatternAnnotator::addXYGhostPlaneFrom3DTo2D(My4DImage* stackImage, int zOffset, int stackChannel, My4DImage* image)
{
    v3d_uint8* sData=stackImage->getRawDataAtChannel(stackChannel);
    V3DLONG xmax=stackImage->getXDim();
    V3DLONG ymax=stackImage->getYDim();
    V3DLONG zmax=stackImage->getZDim();
    if (xmax!=image->getXDim()) {
        qDebug() << "addXYGhostPlaneFrom3Dto2D: x dimensions do not match";
        return;
    }
    if (ymax!=image->getYDim()) {
        qDebug() << "addXYGhostPlaneFrom3Dto2D: y dimensions do not match";
    }
    V3DLONG sStart=zOffset*xmax*ymax;
    v3d_uint8* rData=image->getRawDataAtChannel(0);
    v3d_uint8* gData=image->getRawDataAtChannel(1);
    v3d_uint8* bData=image->getRawDataAtChannel(2);
    for (V3DLONG i=0;i<xmax*ymax;i++) {
        v3d_uint8 sv=sData[sStart+i];
        if (sv>0) {
            v3d_uint8 rv=rData[i];
            if (rv<GHOST_INTENSITY_LEVEL) {
                rData[i]=GHOST_INTENSITY_LEVEL;
            }
            v3d_uint8 gv=gData[i];
            if (gv<GHOST_INTENSITY_LEVEL) {
                gData[i]=GHOST_INTENSITY_LEVEL;
            }
            v3d_uint8 bv=bData[i];
            if (bv<GHOST_INTENSITY_LEVEL) {
                bData[i]=GHOST_INTENSITY_LEVEL;
            }
        }
    }
}

/*

updateIndex()

This is a custom method for creating a new mask index file (and corresponding name and RGB file) from
pre-existing files.

The input files are:

1) compartment index file
2) compartment name file
3) binary mask file

The outputs are:

1) mask index file
2) mask name file
3) RGB file showing colored mask indices

These are the steps for the process:

1) load the compartment index file

2) load the binary mask file

3) create an index mask file

4) populate the index mask file with entries mapped-to the compartment file, assigning a special index
to positions which are active in the mask file but do not belong to a compartment

5) create a virtual-box-partition of the mask space (e.g., 20x20x20 voxels) and sequentially assign
the non-active but masked voxels to these boxes, as they are non-zero

6) output the new mask index file

7) using the name-map, create a new name index file

8) using random colors for each index, create a new RGB file

*/

bool ScreenPatternAnnotator::updateIndex() {

  ImageLoader compartmentIndexLoader;
  My4DImage * compartmentIndexImage = compartmentIndexLoader.loadImage(compartmentIndexFile);

  ImageLoader binaryMaskLoader;
  My4DImage * binaryMaskImage = binaryMaskLoader.loadImage(maskBinaryFile);

  V3DLONG xdim=binaryMaskImage->getXDim();
  V3DLONG ydim=binaryMaskImage->getYDim();
  V3DLONG zdim=binaryMaskImage->getZDim();

  if (xdim!=compartmentIndexImage->getXDim()) {
    qDebug() << "Xdims do not match";
    return false;
  }

  if (ydim!=compartmentIndexImage->getYDim()) {
    qDebug() << "Ydims do not match";
    return false;
  }

  if (zdim!=compartmentIndexImage->getZDim()) {
    qDebug() << "Zdims do not match";
    return false;
  }

  My4DImage * compartmentMaskImage = new My4DImage();
  compartmentMaskImage->loadImage(binaryMaskImage->getXDim(), binaryMaskImage->getYDim(), binaryMaskImage->getZDim(), 1, V3D_UINT8);

  My4DImage * outsideMaskImage = new My4DImage();
  outsideMaskImage->loadImage(binaryMaskImage->getXDim(), binaryMaskImage->getYDim(), binaryMaskImage->getZDim(),
			      1, V3D_UINT8);

  v3d_uint8 * maskData=binaryMaskImage->getRawDataAtChannel(0);

  v3d_uint8 * cmaskData=compartmentMaskImage->getRawDataAtChannel(0);
  v3d_uint8 * omaskData=outsideMaskImage->getRawDataAtChannel(0);

  v3d_uint8 * compartmentData=compartmentIndexImage->getRawDataAtChannel(0);

  QMap<int,int> compartmentToMaskMap;
  QMap<QString,int> outsideToMaskMap;

  double OUTSIDE_CUBE_PERC = 0.10;
  V3DLONG OUTSIDE_CUBE_LENGTH = OUTSIDE_CUBE_PERC * xdim;

  qDebug() << "Using OUTSIDE_CUBE_LENGTH=" << OUTSIDE_CUBE_LENGTH;

  for (V3DLONG z=0;z<zdim;z++) {
    for (V3DLONG y=0;y<ydim;y++) {
      for (V3DLONG x=0;x<xdim;x++) {
	V3DLONG position=z*ydim*xdim+y*xdim+x;
	int maskValue=maskData[position];
	if (maskValue==0) {
	  cmaskData[position]=0;
	  omaskData[position]=0;
	} else {
	  int compartmentValue=compartmentData[position];
	  int maskIndexValue=0;
	  if (compartmentValue==0) {
	    // Outside
	    int outsideX=x/OUTSIDE_CUBE_LENGTH;
	    int outsideY=y/OUTSIDE_CUBE_LENGTH;
	    int outsideZ=z/OUTSIDE_CUBE_LENGTH;
	    QString outsideKey=QString("%1 %2 %3").arg(outsideX).arg(outsideY).arg(outsideZ);
	    int outsideIndexValue=0;
	    if (outsideToMaskMap.contains(outsideKey)) {
	      outsideIndexValue=outsideToMaskMap[outsideKey];
	    } else {
	      int nextOutsideIndexValue=outsideToMaskMap.size()+1;
	      outsideToMaskMap[outsideKey]=nextOutsideIndexValue;
	      outsideIndexValue=nextOutsideIndexValue;
	    }
	    omaskData[position]=outsideIndexValue;
	  } else {
	    // Inside
	    int maskIndexValue=0;
	    if (compartmentToMaskMap.contains(compartmentValue)) {
	      maskIndexValue=compartmentToMaskMap[compartmentValue];
	    } else {
	      int nextMaskIndexValue=compartmentToMaskMap.size()+1;
	      compartmentToMaskMap[compartmentValue]=nextMaskIndexValue;
	      maskIndexValue=nextMaskIndexValue;
	    }
	    cmaskData[position]=maskIndexValue;
	  }
	}
      }
    }
  }

  // We now have the compartment and outside masks populated, so we need to build the final
  // index by combining both results. We will do this by re-using the compartment results,
  // but add the outside results starting with the highest compartment index + 1.
  V3DLONG dataLength=xdim*ydim*zdim;
  int startingOutsideIndex=compartmentToMaskMap.size()+1;
  for (V3DLONG i=0;i<dataLength;i++) {
    if (omaskData[i]>0) {
      int index=omaskData[i]+startingOutsideIndex-1; // -1 is to handle omask starts at 1
      cmaskData[i]=index;
    }
  }

  // Now, cmaskData contains the whole index
  ImageLoader cmaskSaver;
  cmaskSaver.saveImage(compartmentMaskImage, outputIndexFile);

  // Load the original abbreviations from the compartment index file
  QMap<int,QString> compartmentNameMap;
  QFile inputNameIndexQFile(compartmentNameIndexFile);
  qDebug() << "Opening compartmentNameIndexFile=" << compartmentNameIndexFile;
  if (!inputNameIndexQFile.open(QIODevice::ReadOnly)) {
    qDebug() << "Could not open file=" << compartmentNameIndexFile << " to read";
    return false;
  }
  qDebug() << "Successfully opened file";
  while(!inputNameIndexQFile.atEnd()) {
    QString abLine=inputNameIndexQFile.readLine();
    abLine=abLine.trimmed();
    if (abLine.length()>0) {
      QList<QString> abList=abLine.split(QRegExp("\\s+"));
      if (abList.length()>1) {
	QString indexString=abList.at(0);
	int indexNumber=indexString.toInt();
	QString abbreviation=abList.at(1);
	compartmentNameMap[indexNumber]=abbreviation;
	qDebug() << "Assigned " << indexString << " to " << abbreviation;
      }
    }
  }
  inputNameIndexQFile.close();
  qDebug() << "Done with abbreviationMapFile";

  // Migrate to mask name map
  QMap<int,QString> maskNameMap;
  QList<int> compartmentKeyList=compartmentToMaskMap.keys();
  for (int i=0;i<compartmentKeyList.length();i++) {
    int compartmentIndex=compartmentKeyList.at(i);
    int maskIndex=compartmentToMaskMap[compartmentIndex];
    QString compartmentName=compartmentNameMap[compartmentIndex];
    maskNameMap[maskIndex]=compartmentName;
    qDebug() << "Adding mask index=" << maskIndex << " as name=" << compartmentName;
  }

  // Add outside components
  QList<QString> outsideKeyList=outsideToMaskMap.keys();
  for (int i=0;i<outsideKeyList.length();i++) {
    QString outsideKey=outsideKeyList.at(i);
    int baseIndex=outsideToMaskMap[outsideKey];
    int outsideIndex=baseIndex+startingOutsideIndex-1;
    QString outsideName=QString("NonComp-%1").arg(baseIndex);
    maskNameMap[outsideIndex]=outsideName;
    qDebug() << "Adding outside index=" << outsideIndex << " as name=" << outsideName << " for key=" << outsideKey;
  }

  // Generate RGB file to visualize result
  QMap<int,int> Rmap;
  QMap<int,int> Gmap;
  QMap<int,int> Bmap;
  QMap<int,QString> rgbStringMap;
  QList<int> indexList=maskNameMap.keys();
  for (int k=0;k<indexList.size();k++) {
    int index=indexList.at(k);
    int r=qrand() % 256;
    int g=qrand() % 256;
    int b=qrand() % 256;
    Rmap[index]=r;
    Gmap[index]=g;
    Bmap[index]=b;
    QString rgbString=QString("( %1 %2 %3 )").arg(r).arg(g).arg(b);
    rgbStringMap[index]=rgbString;
  }
  My4DImage * rgbImage=new My4DImage();
  rgbImage->loadImage(xdim,ydim,zdim,3,V3D_UINT8);
  v3d_uint8 * r0=rgbImage->getRawDataAtChannel(0);
  v3d_uint8 * r1=rgbImage->getRawDataAtChannel(1);
  v3d_uint8 * r2=rgbImage->getRawDataAtChannel(2);
  for (V3DLONG p=0;p<dataLength;p++) {
    int maskValue=cmaskData[p];
    if (maskValue==0) {
      r0[p]=0;
      r1[p]=0;
      r2[p]=0;
    } else {
      int r=Rmap[maskValue];
      int g=Gmap[maskValue];
      int b=Bmap[maskValue];
      r0[p]=r;
      r1[p]=g;
      r2[p]=b;
    }
  }
  ImageLoader rgbSaver;
  rgbSaver.saveImage(rgbImage, outputRGBFile);

  // Create mask name file
  QFile nameFile(outputNameIndexFile);
  if (!nameFile.open(QIODevice::WriteOnly)) {
    qDebug() << "Could not open file=" << outputNameIndexFile;
    return false;
  }
  QTextStream nameOutputStream(&nameFile);
  for (int k=0;k<indexList.size();k++) {
    int index=indexList.at(k);
    QString name=maskNameMap[index];
    QString rgbString=rgbStringMap[index];
    nameOutputStream << index << " " << name << " " << rgbString << "\n";
    qDebug() << "Wrote index=" << index << " name=" << name << " rgb=" << rgbString;
  }
  nameOutputStream.flush();
  nameFile.close();

  return true;
}

bool ScreenPatternAnnotator::arnimScore()
{
  // Load Input Stack
  inputImage=new My4DImage();
  ImageLoader imageLoader;
  imageLoader.loadImage(inputImage, inputStackFilepath);
  if (inputImage->getDatatype()!=V3D_UINT8) {
    qDebug() << "ScreenPatternGenerator currently only supports 8-bit input data";
    return false;
  }

  if (!loadCompartmentIndex()) {
    qDebug() << "Error calling loadCompartmentIndex()";
    return false;
  }

  if (!loadMaskNameIndex()) {
    qDebug() << "Error calling loadMaskNameIndex()";
    return false;
  }

  QFile outputFile(arnimScoreOutputFilepath);
  if (!outputFile.open(QIODevice::WriteOnly)) {
    qDebug() << "Could not open file=" << arnimScoreOutputFilepath;
    return false;
  }
  QTextStream scoreOutput(&outputFile);

  // Perform Compartment Annotations
  QList<int> compartmentIndexList=compartmentIndexAbbreviationMap.keys();
  for (int k=0;k<compartmentIndexList.size();k++) {
    int index=compartmentIndexList.at(k);
    QString abbreviation=compartmentIndexAbbreviationMap[index];
    BoundingBox3D bb=findBoundingBox3DFromIndex(index);
    int * arnimScores = quantifyArnimCompartmentScores(inputImage, compartmentIndexImage, index, bb);
    int a1=arnimScores[0];
    int a2=arnimScores[1];
    int a3=arnimScores[2];
    int a4=arnimScores[3];
    int a5=arnimScores[4];

    scoreOutput << index << " " << abbreviation << " " << a1 << " " << a2 << " " << a3 << " " << a4 << " " << a5 << "\n";

  }
  scoreOutput.flush();
  outputFile.close();
  return true;
}


/* Arnim's Compartment Annotation

His system is implemented with the following scripts:

1) wrapperWrap-120621.sh

This goes through the set of lsm images for the 20x gal4 samples and calls wrapper111122.ijm on each one.

2) wrapper120621.ijm

This is applied to a single lsm file and iterates through the mask list, calling the isolateRegion9.ijm script on each one.

3) isolateRegion9.ijm

* This gets the volume (voxel count) of the mask
* Calls THREE_D_HISTOGRAM(1, 255, 1) presumably to get overall voxel count
* Has the function THREE_D_HISTOGRAM(hmin, hmax, nbins) which does this:
   - returns raw peakBin, and the value of the raw peakBin
   - creates valArray, which is simply a bin index, offset by hmin
   - For each bin, outputs: min, max, mean, stdev, sum
   - outputs overall stats for histarray
   - then, for each bin, outputs normalized statistics, such that the mask volume is used to generate a ratio
   - then, generates a 'fusionArray' which is the histArray weighted by the index
   - the last lines of the output file are n bins with <index> <value index> <hist> <norm>

This script crops and generates histogram statistics on each mask.

4) autoAnnoIntensity.hist16.7.sh

* This computes the intensities and distributions used as the final scores per mask
* The claim is the the output is (i1 i2 i3 d1 d2), in which i1 and d1 (?) are the official outputs
* An array with 3 intensity thresholds is used (40 10 0)
* Histogram bins with 16 values are expected per mask
* The number of output bins is 5, with the number of fused bins being 2
* For each of the thresholds in (40 10 0) :
    - Initialize sum=0
    - For each of the N=16 bins:
        + Read the normalized-by-volume floating point value
        + Multiply by 100,000
        + Increment the sum
        + Pool in sets of 3, so that (0,1,2) (3,4,5) (6,7,8) (9,10,11) (12,13,14) (15) are together
        + If the sum is greater than the theshold (which is 40, 10, or 0) assign rank=5,4,3,2,1,0 until this is true
        + If after each pool is accumulated, if not over threshold, then decrement rank
 * The raw rank is the 'intensity' value
 * Then, based on the rank for threshold 40, we assign 'medslop' and 'noslop' as sums of subsets of histogram bins (of the 16 bins)
 * Then, based on threshold, these 'medslop' and 'noslop' values are assigned a final value for 'distribution'
 * The two distribution values generated are for $medSlop and then $noSlop

 This method returns a 1D array with values:

 0 Intensity-40
 1 Intensity-10
 2 Intensity-0
 3 Distribution medslop
 4 Distribution noslop

*/

int * ScreenPatternAnnotator::quantifyArnimCompartmentScores(My4DImage * sourceImage, My4DImage * compartmentIndex, int index, BoundingBox3D bb){

  // Initialize threshold values
  v3d_uint8 threshold[3];
  threshold[0]=40;
  threshold[1]=10;
  threshold[2]=0;

  int thresholdIntensityResult[3];

  V3DLONG histogram[16];
  for (int h=0;h<16;h++) histogram[h]=0;

  // Compute normalized mask histogram
  v3d_uint8 * pData=sourceImage->getRawDataAtChannel(patternChannelIndex);
  v3d_uint8 * iData=compartmentIndex->getRawData();
  V3DLONG zmax=sourceImage->getZDim();
  V3DLONG ymax=sourceImage->getYDim();
  V3DLONG xmax=sourceImage->getXDim();
  V3DLONG compartmentVoxelCount=0;
  for (V3DLONG z=bb.z0;z<=bb.z1;z++) {
    V3DLONG zoffset=z*ymax*xmax;
    for (V3DLONG y=bb.y0;y<=bb.y1;y++) {
      V3DLONG yoffset=y*xmax+zoffset;
      for (V3DLONG x=bb.x0;x<=bb.x1;x++) {
	V3DLONG offset=x+yoffset;
	if (iData[offset]==index) {
	  compartmentVoxelCount++;
	  v3d_uint8 v=pData[offset];
	  int hbin=15-v/16; // invert
	  histogram[hbin]++;
	}
      }
    }
  }
  double normHistogram[16];
  double compartmentVoxelCountDouble=compartmentVoxelCount*1.0;
  for (int h=0;h<16;h++) {
    normHistogram[h]=histogram[h]/compartmentVoxelCountDouble;
    //qDebug() << "h=" << h << " hist=" << histogram[h] << " norm=" << normHistogram[h];
  }

  // Do the bin ranking for each threshold case
  for (int tindex=0;tindex<3;tindex++) {
    double t=threshold[tindex] * 1.0;
    double sum=0.0;
    int hbin=0;
    int fusedBins=2;
    int s=0;
    int stop=0;
    int readout=0;
    int intensityScore=0;
    int resultRank=5;
    while (hbin<16 && stop==0) {
      double normH=normHistogram[hbin]*100000;
      sum+=normH;
      //qDebug() << "Incrementing, normH=" << normH << " sum=" << sum;
      if (s<fusedBins) {
	readout=0;
      } else {
	readout=1;
      }
      s++;
      if (readout==1 || hbin>=15) {
	//qDebug() << "tindex=" << tindex << " hbin=" << hbin << " comparing sum=" << sum << " to t=" << t;
	if (sum>t && stop==0) {
	  intensityScore=resultRank;
	  stop=1;
	}
	s=0;
	readout=0;
	resultRank--;
      }
      thresholdIntensityResult[tindex]=intensityScore;
      hbin++;
    } // histogram
  } // threshold

  double medSlopTotal=0.0;
  double noSlopTotal=0.0;

  int distributionIntensity=thresholdIntensityResult[0]; // threshold=40

  //qDebug() << "Using distributionIntensity=" << distributionIntensity;

  if (distributionIntensity==0) {
    medSlopTotal=normHistogram[15];
    noSlopTotal=normHistogram[15];
  } else if (distributionIntensity==1) {
    for (int i=12;i<15;i++) {
      medSlopTotal+=normHistogram[i];
      noSlopTotal+=normHistogram[i];
    }
  } else if (distributionIntensity==2) {
    for (int i=9;i<14;i++) {
      medSlopTotal+=normHistogram[i];
    }
    for (int i=9;i<12;i++) {
      noSlopTotal+=normHistogram[i];
    }
  } else if (distributionIntensity==3) {
    for (int i=6;i<11;i++) {
      medSlopTotal+=normHistogram[i];
    }
    for (int i=6;i<9;i++) {
      noSlopTotal+=normHistogram[i];
    }
  } else if (distributionIntensity==4) {
    for (int i=3;i<8;i++) {
      medSlopTotal+=normHistogram[i];
    }
    for (int i=3;i<6;i++) {
      noSlopTotal+=normHistogram[i];
    }
  } else if (distributionIntensity==5) {
    for (int i=0;i<5;i++) {
      medSlopTotal+=normHistogram[i];
    }
    for (int i=0;i<3;i++) {
      noSlopTotal+=normHistogram[i];
    }
  }

  int medSlopDistribution=0;
  int noSlopDistribution=0;

  double medSlopDouble=medSlopTotal*100000.0;
  double noSlopDouble=noSlopTotal*100000.0;

  //qDebug() << "medSlop=" << medSlopDouble;
  //qDebug() << "noSlop=" << noSlopDouble;

  if (medSlopDouble > 70000) {
    medSlopDistribution=5;
  } else if (medSlopDouble > 26000) {
    medSlopDistribution=4;
  } else if (medSlopDouble > 16000) {
    medSlopDistribution=3;
  } else if (medSlopDouble > 7000) {
    medSlopDistribution=2;
  } else if (medSlopDouble > 1000) {
    medSlopDistribution=1;
  } else {
    medSlopDistribution=0;
  }

  if (noSlopDouble > 70000) {
    noSlopDistribution=5;
  } else if (noSlopDouble > 26000) {
    noSlopDistribution=4;
  } else if (noSlopDouble > 16000) {
    noSlopDistribution=3;
  } else if (noSlopDouble > 7000) {
    noSlopDistribution=2;
  } else if (noSlopDouble > 1000) {
    noSlopDistribution=1;
  } else {
    noSlopDistribution=0;
  }

  int * result = new int[5];

  result[0] = thresholdIntensityResult[0];
  result[1] = thresholdIntensityResult[1];
  result[2] = thresholdIntensityResult[2];
  result[3] = medSlopDistribution;
  result[4] = noSlopDistribution;

  return result;
}

bool ScreenPatternAnnotator::createSimilarityList()
{
  if (targetStackFilepath.length()<1) {
    qDebug() << "targetStackFilepath must be defined";
    return false;
  }
  if (subjectStackListFilepath.length()<1) {
    qDebug() << "subjectStackListFilepath must be defined";
    return false;
  }
  if (outputSimilarityFilepath.length()<1) {
    qDebug() << "outputSimilarityFilepath must be defined";
    return false;
  }

  lut16Color=AnalysisTools::create16Color8BitLUT_fiji();

  ImageLoader targetStackLoader;
  targetStack = targetStackLoader.loadImage(targetStackFilepath);
  if (targetStack->getCDim()!=3) {
    qDebug() << "createSimilarityList only supports 3-channel 8-bit data";
    return false;
  }

  QFile subjectStackListQFile(subjectStackListFilepath);
  if (!subjectStackListQFile.open(QIODevice::ReadOnly)) {
    qDebug() << "Could not open file=" << subjectStackListFilepath << " to read";
    return false;
  }
  while(!subjectStackListQFile.atEnd()) {
    QString stackLine=subjectStackListQFile.readLine();
    stackLine=stackLine.trimmed();
    subjectFilepathList.append(stackLine);
  }
  subjectStackListQFile.close();

  QList<SortableStringDouble> scoredStackList;
  QList< QFuture<SortableStringDouble> > futureList;
  QList<bool> doneList;
  for (int stackIndex=0;stackIndex<subjectFilepathList.size();stackIndex++) {
    doneList.append(false);
    QFuture<SortableStringDouble> f = QtConcurrent::run(this, &ScreenPatternAnnotator::computeStackSimilarityManager, stackIndex);
    futureList.append(f);
  }

  while(1) {
    SleepThread st;
    st.msleep(1000);
    int doneCount=0;
    for (int i=0;i<futureList.size();i++) {
      QFuture<SortableStringDouble> f=futureList.at(i);
      if (f.isFinished()) {
	if (doneList[i]==false) {
	  qDebug() << "job " << i << " is done - processing result";
	  doneList[i]=true;
	  SortableStringDouble result=f.result();
	  scoredStackList.append(result);
	}
	doneCount++;
      }
    }
    int stillActive=futureList.size()-doneCount;
    if (stillActive==0) {
      break;
    } else {
      qDebug() << "Waiting on " << stillActive << " similarity tasks";
    }
  }

  qDebug() << "Done all tasks - sorting results";
  qSort(scoredStackList.begin(), scoredStackList.end());

  qDebug() << "Writing result file";
  QFile outputFile(outputSimilarityFilepath);
  if (!outputFile.open(QIODevice::WriteOnly)) {
    qDebug() << "Could not open file=" << outputSimilarityFilepath << " to write";
    return false;
  }
  QTextStream outputStream(&outputFile);
  for (int i=0;i<scoredStackList.size();i++) {
    SortableStringDouble scoreObject=scoredStackList[i];
    outputStream << scoreObject.theDouble << " " << scoreObject.theString << "\n";
  }
  outputStream.flush();
  outputFile.close();
  return true;
}

SortableStringDouble ScreenPatternAnnotator::computeStackSimilarityManager(int subjectStackIndex)
{
  ImageLoader loader;
  QString subjectStackFilepath=subjectFilepathList[subjectStackIndex];
  My4DImage* subjectStack = loader.loadImage(subjectStackFilepath);
  double result=computeStackSimilarity(targetStack, subjectStack);
  delete subjectStack;
  SortableStringDouble sortObject;
  sortObject.theString=subjectStackFilepath;
  sortObject.theDouble=result;
  return sortObject;
}

double ScreenPatternAnnotator::computeStackSimilarity(My4DImage* targetStack, My4DImage* subjectStack)
{
  const v3d_uint8 T1 = 10;
  const v3d_uint8 T2 = 20;

  V3DLONG xsize=targetStack->getXDim();
  V3DLONG ysize=targetStack->getYDim();
  V3DLONG zsize=targetStack->getZDim();
  V3DLONG csize=targetStack->getCDim();

  if (subjectStack->getXDim()!=xsize ||
      subjectStack->getYDim()!=ysize ||
      subjectStack->getZDim()!=zsize ||
      subjectStack->getCDim()!=csize) {
    qDebug() << "computeStackSimilarity: dimensions do not match\n";
    return -1.0;
  }

  v3d_uint8** targetChannelSet = new v3d_uint8*[csize];
  v3d_uint8** subjectChannelSet = new v3d_uint8*[csize];

  for (V3DLONG c=0;c<csize;c++) {
    targetChannelSet[c]=targetStack->getRawDataAtChannel(c);
    subjectChannelSet[c]=subjectStack->getRawDataAtChannel(c);
  }

  V3DLONG productTotal=0;
  V3DLONG offset=0;
  V3DLONG imageSize=zsize*ysize*xsize;

  for (V3DLONG i=0;i<imageSize;i++) {

    v3d_uint8 tr=targetChannelSet[0][i];
    v3d_uint8 tg=targetChannelSet[1][i];
    v3d_uint8 tb=targetChannelSet[2][i];

    v3d_uint8 sr=subjectChannelSet[0][i];
    v3d_uint8 sg=subjectChannelSet[1][i];
    v3d_uint8 sb=subjectChannelSet[2][i];

    v3d_uint8 targetValue=AnalysisTools::getReverse16ColorLUT(lut16Color, tr, tg, tb);
    v3d_uint8 subjectValue=AnalysisTools::getReverse16ColorLUT(lut16Color, sr, sg, sb);

    if (targetValue>100) {
      targetValue=100;
    } else if (targetValue<5) {
      targetValue=5;
    }
    if (subjectValue>100) {
      subjectValue=100;
    } else if (subjectValue<5) {
      subjectValue=5;
    }
    V3DLONG product=((subjectValue-targetValue)*(subjectValue-targetValue))/(subjectValue+targetValue);
    productTotal+=product;
  }
  return (sqrt(productTotal*1.0))/(imageSize*1.0);
}



bool ScreenPatternAnnotator::createV2Heatmap()
{
  v3d_uint8 * lutV1=create16Color8BitLUT_V3();
  v3d_uint8 * lutV2=AnalysisTools::create16Color8BitLUT_fiji();

  ImageLoader v1Loader;
  My4DImage * stackV1 = v1Loader.loadImage(heatmapV1Filepath);
  V3DLONG zsize=stackV1->getZDim();
  V3DLONG ysize=stackV1->getYDim();
  V3DLONG xsize=stackV1->getXDim();
  if (stackV1->getCDim()!=3) {
    qDebug() << "createV2Heatmap() : expected 3 dimensions for input stack";
    return false;
  }

  My4DImage * stackV2 = new My4DImage();
  stackV2->loadImage(xsize, ysize, zsize, 3, V3D_UINT8);

  v3d_uint8 * rH1 = stackV1->getRawDataAtChannel(0);
  v3d_uint8 * gH1 = stackV1->getRawDataAtChannel(1);
  v3d_uint8 * bH1 = stackV1->getRawDataAtChannel(2);

  v3d_uint8 * rH2 = stackV2->getRawDataAtChannel(0);
  v3d_uint8 * gH2 = stackV2->getRawDataAtChannel(1);
  v3d_uint8 * bH2 = stackV2->getRawDataAtChannel(2);

  // Check if stack1 is already V2
  bool v2Flag=false;
  for (V3DLONG z=0;z<zsize && !v2Flag;z++) {
    for (V3DLONG y=0;y<ysize && !v2Flag;y++) {
      for (V3DLONG x=0;x<xsize && !v2Flag;x++) {
	V3DLONG offset=ysize*xsize*z+y*xsize+x;

	v3d_uint8 v1_r=rH1[offset];
	v3d_uint8 v1_g=gH1[offset];
	v3d_uint8 v1_b=bH1[offset];

    v3d_uint8 v1Index = AnalysisTools::getReverse16ColorLUT(lutV1, v1_r, v1_g, v1_b);
    v3d_uint8 v2Index = AnalysisTools::getReverse16ColorLUT(lutV2, v1_r, v1_g, v1_b);

	if (v1Index==0 && v2Index > 0) {
	  // This implies V2
	  v2Flag=true;
	}
      }
    }
  }

  v3d_uint8 v1_r, v1_g, v1_b;
  v3d_uint8 v2_r, v2_g, v2_b;

  for (V3DLONG z=0;z<zsize;z++) {
    for (V3DLONG y=0;y<ysize;y++) {
      for (V3DLONG x=0;x<xsize;x++) {
	V3DLONG offset=ysize*xsize*z+y*xsize+x;

	v1_r=rH1[offset];
	v1_g=gH1[offset];
	v1_b=bH1[offset];

	if (!v2Flag) {
      v3d_uint8 v1Index = AnalysisTools::getReverse16ColorLUT(lutV1, v1_r, v1_g, v1_b);
	  v2_r=lutV2[v1Index];
	  v2_g=lutV2[v1Index+256];
	  v2_b=lutV2[v1Index+512];
	} else {
	  v2_r=v1_r;
	  v2_g=v1_g;
	  v2_b=v1_b;
	}

	rH2[offset]=v2_r;
	gH2[offset]=v2_g;
	bH2[offset]=v2_b;
      }
    }
  }

  // Create mip
  int splitPosition=heatmapV2Filepath.lastIndexOf(".");
  QString mipFilename=heatmapV2Filepath.left(splitPosition);
  mipFilename.append("MIP.tif");
  qDebug() << "Mip name=" << mipFilename;

  ImageLoader saver;
  saver.saveImage(stackV2, heatmapV2Filepath);

  My4DImage * mip = AnalysisTools::createMIPFromImageByLUT(stackV2, lutV2);
  ImageLoader mipSaver;
  mipSaver.saveImage(mip, mipFilename);

  delete stackV1;
  delete stackV2;
  delete mip;
  return true;
}

v3d_uint8 * ScreenPatternAnnotator::create16Color8BitLUT_V2()
{
    v3d_uint8 * lut16 = new v3d_uint8[256*3];

    for (int i=0;i<16;i++) {
        for (int j=0;j<16;j++) {
            int index=i*16+j;
            if (i==0) {
                lut16[index]    = 0;
                lut16[index+256]= 0;
                lut16[index+512]= 0 + ((171*j)/16);
            } else if (i==1) {
                lut16[index]    = 1;
                lut16[index+256]= 1;
                lut16[index+512]= 171 + (((224-171)*j)/16);
            } else if (i==2) {
                lut16[index]    = 1;
                lut16[index+256]= 1 + (((110-1)*j)/16);
                lut16[index+512]= 224;
            } else if (i==3) {
                lut16[index]    = 0;
                lut16[index+256]= 110 + (((171-110)*j)/16);
                lut16[index+512]= 255;
            } else if (i==4) {
                lut16[index]    = 1;
                lut16[index+256]= 171 + (((224-171)*j)/16);
                lut16[index+512]= 254;
            } else if (i==5) {
                lut16[index]    = 1;
                lut16[index+256]= 224;
                lut16[index+512]= 254 - (((254-1)*j)/16);
            } else if (i==6) {
                lut16[index]    = 1 + (((190-1)*j)/16);
                lut16[index+256]= 254;
                lut16[index+512]= 1;
            } else if (i==7) {
                lut16[index]    = 190 + (((255-190)*j)/16);
                lut16[index+256]= 255;
                lut16[index+512]= 0;
            } else if (i==8) {
                lut16[index]    = 255;
                lut16[index+256]= 255 - (((255-224)*j)/16);
                lut16[index+512]= 0;
            } else if (i==9) {
                lut16[index]    = 255;
                lut16[index+256]= 224 - (((224-141)*j)/16);
                lut16[index+512]= 0;
            } else if (i==10) {
                lut16[index]    = 255 - (((255-250)*j)/16);
                lut16[index+256]= 141 - (((141-94)*j)/16);
                lut16[index+512]= 0;
            } else if (i==11) {
                lut16[index]    = 250 - (((250-245)*j)/16);
                lut16[index+256]= 94 - (((94-0)*j)/16);
                lut16[index+512]= 0;
            } else if (i==12) {
                lut16[index]    = 245;
                lut16[index+256]= 0;
                lut16[index+512]= 0 + (((185-0)*j)/16);
            } else if (i==13) {
                lut16[index]    = 245 - (((245-222)*j)/16);
                lut16[index+256]= 0 + (((180-0)*j)/16);
                lut16[index+512]= 185 + (((222-185)*j)/16);
            } else if (i==14) {
                lut16[index]    = 222 + (((237-222)*j)/16);
                lut16[index+256]= 180 + (((215-180)*j)/16);
                lut16[index+512]= 222 + (((237-222)*j)/16);
            } else if (i==15) {
                lut16[index]    = 237 + (((255-237)*j)/16);
                lut16[index+256]= 215 + (((255-215)*j)/16);
                lut16[index+512]= 237 + (((255-237)*j)/16);
            }
            if (index==255) {
                lut16[index]    = 255;
                lut16[index+256]= 255;
                lut16[index+512]= 255;
            }
        }
    }
    // Norm intensity
    for (int i=0;i<256;i++) {
      double id=i*3.0;
      double preTotal=0.0;
      preTotal+=(lut16[i]*1.0);
      preTotal+=(lut16[i+256]*1.0);
      preTotal+=(lut16[i+512]*1.0);
      double ratio=id/preTotal;
      int i1=lut16[i]*ratio;
      int i2=lut16[i+256]*ratio;
      int i3=lut16[i+512]*ratio;
      if (i1<256) {
	lut16[i]=i1;
      } else {
	lut16[i]=255;
      }
      if (i2<256) {
	lut16[i+256]=i2;
      } else {
	lut16[i+256]=255;
      }
      if (i3<256) {
	lut16[i+512]=i3;
      } else {
	lut16[i+512]=255;
      }
    }
    return lut16;
}

v3d_uint8 * ScreenPatternAnnotator::create16Color8BitLUT_V3()
{
    v3d_uint8 * lut16 = new v3d_uint8[256*3];

    for (int i=0;i<16;i++) {
      for (int j=0;j<16;j++) {
	int index=i*16+j;
	if (i==0) {
	  lut16[index]    = 0;
	  lut16[index+256]= 0;
	  lut16[index+512]= 0; // Total=0
	} else if (i==1) {
	  lut16[index]    = 1;
	  lut16[index+256]= 1;
	  lut16[index+512]= 60; // Total=62
	} else if (i==2) {
	  lut16[index]    = 1;
	  lut16[index+256]= 1;
	  lut16[index+512]= 100; // Total=102
	} else if (i==3) {
	  lut16[index]    = 0;
	  lut16[index+256]= 32;
	  lut16[index+512]= 101; // Total=133
	} else if (i==4) {
	  lut16[index]    = 1;
	  lut16[index+256]= 60;
	  lut16[index+512]= 102; // Total=163
	} else if (i==5) {
	  lut16[index]    = 0;
	  lut16[index+256]= 103;
	  lut16[index+512]= 103; // Total=206
	} else if (i==6) {
	  lut16[index]    = 0;
	  lut16[index+256]= 207;
	  lut16[index+512]= 0; // Total=207
	} else if (i==7) {
	  lut16[index]    = 20;
	  lut16[index+256]= 208;
	  lut16[index+512]= 0; // Total=228
	} else if (i==8) {
	  lut16[index]    = 120;
	  lut16[index+256]= 209;
	  lut16[index+512]= 0; // Total=329
	} else if (i==9) {
	  lut16[index]    = 210;
	  lut16[index+256]= 120;
	  lut16[index+512]= 0; // Total=330
	} else if (i==10) {
	  lut16[index]    = 211;
	  lut16[index+256]= 200;
	  lut16[index+512]= 0; // Total=411
	} else if (i==11) {
	  lut16[index]    = 230;
	  lut16[index+256]= 150;
	  lut16[index+512]= 35; // Total=415
	} else if (i==12) {
	  lut16[index]    = 235;
	  lut16[index+256]= 100; // Total=435
	  lut16[index+512]= 100;
	} else if (i==13) {
	  lut16[index]    = 240;
	  lut16[index+256]= 0;
	  lut16[index+512]= 200; // Total=440
	} else if (i==14) {
	  lut16[index]    = 241;
	  lut16[index+256]= 50;
	  lut16[index+512]= 241; // Total=xxx
	} else if (i==15) {
	  lut16[index]    = 242;
	  lut16[index+256]= 205;
	  lut16[index+512]= 242; // Total=xxx
	}
	if (index==255) {
	  lut16[index]    = 255;
	  lut16[index+256]= 255;
	  lut16[index+512]= 255; // Total=xxx
	}
      }
    }
    return lut16;
}




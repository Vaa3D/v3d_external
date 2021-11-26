#if defined(USE_Qt5)
    //#include <QtConcurrent>
#endif 


#include "NeuronFragmentEditor.h"
#include "../utility/ImageLoader.h"
#include "AnalysisTools.h"
#include "SleepThread.h"
#include <climits>

#include "../terafly/src/presentation/theader.h"  //2015May PHC

const int NeuronFragmentEditor::MODE_UNDEFINED=-1;
const int NeuronFragmentEditor::MODE_COMBINE=0;
const int NeuronFragmentEditor::MODE_COMBINE_MASK=1;
const int NeuronFragmentEditor::MODE_REVERSE_LABEL=2;
const int NeuronFragmentEditor::MODE_MIPS=3;
const int NeuronFragmentEditor::MODE_MASK_FROM_STACK=4;
const int NeuronFragmentEditor::MODE_SURFACE_VERTEX=5;

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
    channel=0;
    threshold=0.0;
    normalizeVertexFile=false;
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
            } else if (modeString=="mask-from-stack") {
	        mode=MODE_MASK_FROM_STACK;
	    } else if (modeString=="surface-from-stack") {
	      mode=MODE_SURFACE_VERTEX;
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
	    qDebug() << "Set -outputMip = " << outputMipFilepath;
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
		  i--;
                    break;
                } else {
                    // Assume is mask file
                    maskFilePaths.append(possibleMaskFile);
                }
            } while( i < (argList->size()-1) );
        } else if (arg=="-maxThreadCount") {
            QString maxThreadString=(*argList)[++i];
            maxThreadCount=maxThreadString.toInt();
        } else if (arg=="-channel") {
	  QString channelString=(*argList)[++i];
	  channel=channelString.toInt();
	} else if (arg=="-threshold") {
	  QString thresholdString=(*argList)[++i];
	  threshold=thresholdString.toDouble();
	} else if (arg=="-surfaceVertexFile") {
	  surfaceVertexFilepath=(*argList)[++i];
	} else if (arg=="-normalizeVertexFile") {
	  normalizeVertexFile=true;
	}
    }
    bool argError=false;
    if (mode!=MODE_COMBINE &&
	mode!=MODE_COMBINE_MASK &&
	mode!=MODE_REVERSE_LABEL &&
	mode!=MODE_MIPS &&
	mode!=MODE_MASK_FROM_STACK &&
	mode!=MODE_SURFACE_VERTEX) {
        qDebug() << "Do not recognize valid mode";
        argError=true;
    }
    if (mode==MODE_COMBINE || mode==MODE_REVERSE_LABEL || mode==MODE_MIPS || mode==MODE_MASK_FROM_STACK || mode==MODE_SURFACE_VERTEX) {
        if (sourceImageFilepath.length() < 1) {
            qDebug() << "-sourceImageFilepath is required";
            argError=true;
        }
    }
    if (mode==MODE_COMBINE || mode==MODE_REVERSE_LABEL || mode==MODE_MIPS) {
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
    } else if (mode==MODE_REVERSE_LABEL || mode==MODE_MIPS || mode==MODE_MASK_FROM_STACK) {
        if (outputDirPath.size() < 1) {
            qDebug() << "-outputDir is required";
            argError=true;
        }
        if (outputPrefix.size() < 1) {
            qDebug() << "-outputPrefix is required";
            argError=true;
        }
    } else if (mode==MODE_SURFACE_VERTEX) {
      if (surfaceVertexFilepath.size() < 1) {
	qDebug() << "-surfaceVertexFile is required";
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
    } else if (mode==MODE_MASK_FROM_STACK) {
      return createMaskFromStack();
    } else if (mode==MODE_SURFACE_VERTEX) {
      return createSurfaceVertex();
    } else {
        return false;
    }
}

bool NeuronFragmentEditor::createMaskFromStack()
{
  ImageLoader preLoader;
  QFileInfo sourceFileInfo(sourceImageFilepath);
  if (!sourceFileInfo.exists()) {
    qDebug() << "Could not find source image file=" << sourceImageFilepath;
    return false;
  }

  My4DImage* preImage=preLoader.loadImage(sourceImageFilepath);

  xdim=preImage->getXDim();
  ydim=preImage->getYDim();
  zdim=preImage->getZDim();
  cdim=preImage->getCDim();

  qDebug() << "Using source x=" << xdim << " y=" << ydim << " z=" << zdim << " c=" << cdim << " datatype=" << preImage->getDatatype();

  sourceImage = new My4DImage();
  sourceImage->loadImage(xdim, ydim, zdim, 1, preImage->getDatatype());

  // For the label image, we will create an artificial label stack, and use the label field with a '1' when the source intensity is
  // above threshold, and a '0' when it is below

  labelImage = new My4DImage();
  labelImage->loadImage(xdim, ydim, zdim, 1, V3D_UINT8);

  v3d_uint8* label8 = (v3d_uint8*)(labelImage->getRawDataAtChannel(0));

  if (sourceImage->getDatatype()==V3D_UINT8) {
    v3d_uint8* pre8 = (v3d_uint8*)(preImage->getRawDataAtChannel(channel));
    v3d_uint8* source8 = (v3d_uint8*)(sourceImage->getRawDataAtChannel(0));
    v3d_uint8 t = 255.0*threshold;
    long rawSize=xdim*ydim*zdim;
    for (long i=0;i<rawSize;i++) {
      source8[i]=pre8[i];
      if (pre8[i]>t) {
	label8[i]=1;
      } else {
	label8[i]=0;
      }
    }
  } else if (sourceImage->getDatatype()==V3D_UINT16) {
    v3d_uint16* pre16 = (v3d_uint16*)(preImage->getRawDataAtChannel(channel));
    v3d_uint16* source16 = (v3d_uint16*)(sourceImage->getRawDataAtChannel(0));
    v3d_uint16 t = 4095.0*threshold;
    long rawSize=xdim*ydim*zdim;
    for (long i=0;i<rawSize;i++) {
      source16[i]=pre16[i];
      if (pre16[i]>t) {
	label8[i]=1;
      } else {
	label8[i]=0;
      }
    }
  } else {
    qDebug() << "Do not recognize image type";
    return false;
  }

  delete preImage;

  maskChan.setSourceImage(sourceImage);
  maskChan.setLabelImage(labelImage);

  // For output paths, we will use the prefix of the source image stack

  QDir outputDir(outputDirPath);
  if (!outputDir.exists()) {
    QDir().mkdir(outputDirPath);
  }

  QString filename=outputDirPath;
  filename.append("/");
  if (outputPrefix.length()>0) {
    filename.append(outputPrefix);
  }

  QString maskFullPath=filename;
  maskFullPath.append(".mask");
  QString chanFullPath=filename;
  chanFullPath.append(".chan");

  QReadWriteLock* nullMutex=0L;
  bool resultStatus= maskChan.createMaskChanForLabel(1, maskFullPath, chanFullPath, nullMutex);

  delete labelImage;
  delete sourceImage;

  labelImage=0L;
  sourceImage=0L;
}

bool NeuronFragmentEditor::loadSourceAndLabelImages()
{

  qDebug() << "NeuronFragmentEditor::loadSourceAndLabelImages()";

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

    maskChan.setSourceImage(sourceImage);
    maskChan.setLabelImage(labelImage);

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
                if (label8 != 0L) {
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
  MaskChan maskChan;
  My4DImage* outputStack = maskChan.createImageFromMaskFiles(maskFilePaths);

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

bool NeuronFragmentEditor::createSurfaceVertex()
{
  // We want to materialize the 3D stack, and then do a 3-way x,y,z approach, detecting transitions in and out
  // of the mask.
  My4DImage* sourceMask=0L;

  if (sourceImageFilepath.endsWith(".mask")) {
    QStringList fileList;
    fileList.append(sourceImageFilepath);
    MaskChan mc;
    sourceMask=mc.createImageFromMaskFiles(fileList);
  } else {
    ImageLoader il;
    sourceMask=il.loadImage(sourceImageFilepath);
  }

  QList<double> xList;
  QList<double> yList;
  QList<double> zList;

  int cdim=sourceMask->getCDim();
  int xdim=sourceMask->getXDim();
  int ydim=sourceMask->getYDim();
  int zdim=sourceMask->getZDim();

  int mdim=xdim;
  if (ydim>mdim) {
    mdim=ydim;
  }
  if (zdim>mdim) {
    mdim=zdim;
  }
  double mnorm=mdim*1.0;

  v3d_uint8** cdata=new v3d_uint8*[cdim];

  for (int c=0;c<cdim;c++) {
    cdata[c]=sourceMask->getRawDataAtChannel(c);
  }

  bool priorPosition=false;

  int xySize=ydim*xdim;

  // Do x-pass
  priorPosition=false;
  for (int z=0;z<zdim;z++) {
    for (int y=0;y<ydim;y++) {
      for (int x=0;x<xdim;x++) {
	bool ot=false;
	int offset=z*xySize+y*xdim+x;
	for (int c=0;c<cdim;c++) {
	  if (cdata[c][offset]>0) {
	    ot=true;
	    break;
	  }
	}
	if ( (priorPosition && !ot) ||
	     (!priorPosition && ot) ||
             (x==(xdim-1) && ot) ||
             (y==(ydim-1) && ot) ||
             (z==(zdim-1) && ot) ) {
	  double xv=1.0*x;
	  double yv=1.0*y+0.5;
	  double zv=1.0*z+0.5;
	  if (normalizeVertexFile) {
	    xv/=mnorm;
	    yv/=mnorm;
	    zv/=mnorm;
	  }
	  xList.append(xv);
	  yList.append(yv);
	  zList.append(zv);
	}
	priorPosition=ot;
      }
      priorPosition=false;
    }
  }

  // Do y-pass
  priorPosition=false;
  for (int z=0;z<zdim;z++) {
    for (int x=0;x<xdim;x++) {
      for (int y=0;y<ydim;y++) {
	bool ot=false;
	int offset=z*xySize+y*xdim+x;
	for (int c=0;c<cdim;c++) {
	  if (cdata[c][offset]>0) {
	    ot=true;
	    break;
	  }
	}
	if ( (priorPosition && !ot) ||
	     (!priorPosition && ot) ||
	     (x==(xdim-1) && ot) ||
             (y==(ydim-1) && ot)  ||
	     (z==(zdim-1) && ot) ) {
	  double xv=1.0*x+0.5;
	  double yv=1.0*y;
	  double zv=1.0*z+0.5;
	  if (normalizeVertexFile) {
	    xv/=mnorm;
	    yv/=mnorm;
	    zv/=mnorm;
	  }
	  xList.append(xv);
	  yList.append(yv);
	  zList.append(zv);
	}
	priorPosition=ot;
      }
      priorPosition=false;
    }
  }


  // Do z-pass
  priorPosition=false;
  for (int y=0;y<ydim;y++) {
    for (int x=0;x<xdim;x++) {
      for (int z=0;z<zdim;z++) {
	bool ot=false;
	int offset=z*xySize+y*xdim+x;
	for (int c=0;c<cdim;c++) {
	  if (cdata[c][offset]>0) {
	    ot=true;
	    break;
	  }
	}
	if ( (priorPosition && !ot) ||
	     (!priorPosition && ot) ||
	     (x==(xdim-1) && ot) ||
	     (y==(ydim-1) && ot) ||
	     (z==(zdim-1) && ot) ) {
	  double xv=1.0*x+0.5;
	  double yv=1.0*y+0.5;
	  double zv=1.0*z;
	  if (normalizeVertexFile) {
	    xv/=mnorm;
	    yv/=mnorm;
	    zv/=mnorm;
	  }
	  xList.append(xv);
	  yList.append(yv);
	  zList.append(zv);
	}
	priorPosition=ot;
      }
      priorPosition=false;
    }
  }

  // Now write output file
  QFile vertexFile(surfaceVertexFilepath);
  vertexFile.open(QIODevice::WriteOnly | QIODevice::Text);
  QTextStream out(&vertexFile);
  for (int i=0;i<xList.size();i++) {
    out << "v " << xList[i] << " " << yList[i] << " " << zList[i] << "\n";
  }
  vertexFile.close();

  return true;
}




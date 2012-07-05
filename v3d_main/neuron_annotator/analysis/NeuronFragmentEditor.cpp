#include "NeuronFragmentEditor.h"
#include "../utility/ImageLoader.h"

NeuronFragmentEditor::NeuronFragmentEditor()
{
}

NeuronFragmentEditor::~NeuronFragmentEditor()
{
}

int NeuronFragmentEditor::processArgs(vector<char*> *argList)
{
    for (int i=0;i<argList->size();i++) {
        QString arg=(*argList)[i];
	if (arg=="-sourceImage") {
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
	}
    }
    bool argError=false;
    if (inputLabelIndexFilepath.length() < 1) {
      qDebug() << "-labelIndex is required";
      argError=true;
    }
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
    if (argError) {
      return 1;
    }
    return 0;
}


bool NeuronFragmentEditor::execute()
{
  return createFragmentComposite();
}

bool NeuronFragmentEditor::createFragmentComposite()
{
  // Open consolidated signal label file
  ImageLoader sourceLoader;
  My4DImage * sourceImage=sourceLoader.loadImage(sourceImageFilepath);
  V3DLONG xdim=sourceImage->getXDim();
  V3DLONG ydim=sourceImage->getYDim();
  V3DLONG zdim=sourceImage->getZDim();
  V3DLONG cdim=sourceImage->getCDim();

  ImageLoader labelLoader;
  My4DImage * labelImage=labelLoader.loadImage(inputLabelIndexFilepath);

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

  My4DImage * compositeImage = new My4DImage();
  compositeImage->loadImage(xdim, ydim, zdim, cdim, V3D_UINT8);

  v3d_uint8 * label=labelImage->getRawDataAtChannel(0);

  v3d_uint8 * sourceR=sourceImage->getRawDataAtChannel(0);
  v3d_uint8 * sourceG=sourceImage->getRawDataAtChannel(1);
  v3d_uint8 * sourceB=sourceImage->getRawDataAtChannel(2);

  v3d_uint8 * compR=compositeImage->getRawDataAtChannel(0);
  v3d_uint8 * compG=compositeImage->getRawDataAtChannel(1);
  v3d_uint8 * compB=compositeImage->getRawDataAtChannel(2);
  
  int fragmentListSize=fragmentList.size();
  int * fragmentArr = new int[fragmentListSize];
  for (int f=0;f<fragmentListSize;f++) {
    fragmentArr[f]=fragmentList.at(f);
  }

  for (V3DLONG z=0;z<xdim;z++) {
    for (V3DLONG y=0;y<ydim;y++) {
      for (V3DLONG x=0;x<zdim;x++) {
	V3DLONG offset=z*ydim*xdim + y*xdim + x;
	int labelValue=label[offset];
	bool include=false;
	for (int f=0;f<fragmentListSize;f++) {
	  if (labelValue==fragmentArr[f]) {
	    include=true;
	  }
	}
	if (include) {
	  compR[offset]=sourceR[offset];
	  compG[offset]=sourceG[offset];
	  compB[offset]=sourceB[offset];
	} else {
	  compR[offset]=0;
	  compG[offset]=0;
	  compB[offset]=0;
	}
      }
    }
  }

  delete [] fragmentArr;

  ImageLoader compositeLoader;
  compositeLoader.saveImage(compositeImage, outputStackFilepath);

  My4DImage * compositeMIP = createMIPFromImage(compositeImage);
  ImageLoader compositeMIPSaver;
  compositeMIPSaver.saveImage(compositeMIP, outputMipFilepath);

  delete compositeImage;
  delete compositeMIP;

}

My4DImage * NeuronFragmentEditor::createMIPFromImage(My4DImage * image) {

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



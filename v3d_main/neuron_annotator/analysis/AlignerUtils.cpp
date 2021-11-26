#include "AlignerUtils.h"
#include "../utility/ImageLoader.h"

const int AlignerUtils::MODE_UNDEFINED=0;
const int AlignerUtils::MODE_REFERENCE_RANK=1;

#include "../neuron_annotator/analysis/SleepThread.h" 

AlignerUtils::AlignerUtils()
{
    mode=MODE_UNDEFINED;
}

AlignerUtils::~AlignerUtils()
{
}

bool AlignerUtils::execute()
{
    if (mode==MODE_UNDEFINED) {
        return false;
    } else if (mode==MODE_REFERENCE_RANK) {
        return referenceRank();
    }
    return false;
}

int AlignerUtils::processArgs(vector<char*> *argList)
{
    for (int i=0;i<argList->size();i++) {
        QString arg=(*argList)[i];
	if (arg=="-mode") {
	  QString modeString=(*argList)[++i];
	  if (modeString=="reference_rank") {
	    mode=MODE_REFERENCE_RANK;
	  }
        } else if (arg=="-sourceStack") {
            sourceStackFilepath=(*argList)[++i];
        }
    }
    if (mode==MODE_REFERENCE_RANK && sourceStackFilepath.size()<1) {
      qDebug() << "For mode reference_rank, -sourceStack must be specified";
      return 1;
    }
    return 0;
}

/*
  The approach we will take is to:

1) generate a histogram of the intensity data for each channel
2) compute the percentage of voxels above average intensity (127) - These are 'high'
3) compute the percentage which are dark but non-zero (from 5 through 30) - These are 'low'
4) compute the percentage which are visible (from 10 through max) - These are 'visible'
5) Reference is the channel with the highest ratio of visible/high
6) The other channels are ranked by 'high' percentage

 */

bool AlignerUtils::referenceRank() {
  ImageLoader sourceLoader;
  My4DImage* inputImage = sourceLoader.loadImage(sourceStackFilepath);
  V3DLONG xSize=inputImage->getXDim();
  V3DLONG ySize=inputImage->getYDim();
  V3DLONG zSize=inputImage->getZDim();
  V3DLONG channelSize=xSize*ySize*zSize;
  My4DImage* sourceImage = 0L;

  if (inputImage->getDatatype()==V3D_UINT8) {
    sourceImage=inputImage;
  } else if(inputImage->getDatatype()==V3D_UINT16) {
    sourceImage=new My4DImage();
    sourceImage->loadImage(xSize, ySize, zSize, inputImage->getCDim(), V3D_UINT8);
    V3DLONG datalength=xSize*ySize*zSize*inputImage->getCDim();
    v3d_uint16* p2=(v3d_uint16*)inputImage->getRawData();
    v3d_uint8* p1=sourceImage->getRawData();
    for (int i=0;i<datalength;i++) {
      int v=p2[i]/16;
      if (v<0) {
	v=0;
      } else if (v>255) {
	v=255;
      }
      p1[i]=v;
    }
    delete inputImage;
  } else {
    qDebug() << "Do not recognize datatype=" << inputImage->getDatatype();
  }
  V3DLONG** histogramChannelSet=new V3DLONG*[sourceImage->getCDim()];
  double highestVH=0.0;
  int highestVHIndex=0;
  int* highScore=new int[sourceImage->getCDim()];
  for (int i=0;i<sourceImage->getCDim();i++) {
    histogramChannelSet[i]=new V3DLONG[256];
    histogram.compute(sourceImage->getRawData() + channelSize*i, channelSize);
    V3DLONG* tmpHistogram=histogram.getHistogram();
    V3DLONG voxelTotal=0L;
    V3DLONG voxelHigh=0L;
    V3DLONG voxelLow=0L;
    V3DLONG voxelVisible=0L;
    for (int j=0;j<256;j++) {
      voxelTotal+=tmpHistogram[j];
      if (j>4 && j<31) {
	voxelLow+=tmpHistogram[j];
      }
      if (j>127) {
	voxelHigh+=tmpHistogram[j];
      }
      if (j>9) {
	voxelVisible+=tmpHistogram[j];
      }
      histogramChannelSet[i][j]=tmpHistogram[j];
      //      qDebug() << " " << i << " " << j << " " << histogramChannelSet[i][j];
    }
    double lowPerc = (voxelLow*1.0)/voxelTotal;
    double highPerc = (voxelHigh*1.0)/voxelTotal;
    double visiblePerc = (voxelVisible*1.0)/voxelTotal;
    double lowHighRatio=lowPerc/highPerc;
    double visibleHighRatio=visiblePerc/highPerc;
    if (visibleHighRatio > highestVH) {
      highestVHIndex=i;
    }
    highScore[i]=visiblePerc;
  }
  printf("%d", highestVHIndex);
  if (sourceImage->getCDim()>1) {
    // For each rank value, starting with 0
    for (int i=0;i<sourceImage->getCDim()-1;i++) {
      // We consider each non-Reference channel
      for (int c=0;c<sourceImage->getCDim();c++) {
	if (c!=highestVHIndex) {
	  int rank=0;
	  // And determine its rank
	  for (int j=0;j<sourceImage->getCDim();j++) {
	    if (j!=highestVHIndex) {
	      if (highScore[j]>highScore[c]) {
		rank++;
	      }
	    }
	  }
	  // And if its rank matches the current position, we report it
	  if (rank==i) {
	    printf(" %d", c);
	  }
	}
      }
    }
  }
  printf("\n");
  delete sourceImage;
  return true;
}

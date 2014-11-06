/*

When samples are processed and their corresponding index files are generated (before they are integrated into the unified index), this is their structure:

1) The samples are formatted in sequence x,y,z, according to the global index specification file

2) When the fragment/sample-intensity data is extracted and used to populate the unified index, we want the individual sections to be efficiently distributed

3) Therefore, as we write the fragment data, it should be in (x,y,z) chunks that correspond to these subvolumes, in the order of the original x,y,z top-level data

4) Therefore, we will try this format:

# General index info
<int, 1> : x
<int, 1> : y
<int, 1> : z
<int, 1> : unit
<int, 1> : bits

# Info for 1st stage index
<long, 1> : sampleID
<int, 1> : length of owner
<char, ...> : ascii bytes of owner
<int, 1> : sample data length, should match above params
<byte, 2 bits * x/unit * y/unit * z/unit | mod 8 > : sample data

# Begin 2nd-stage subvolume series for sample data over threshold
<for each 1st-stage subvolume...
  <int, 1> : x-start
  <int, 1> : y-start
  <int, 1> : z-start
  <long, 1> : sampleID
  <long, 1> : sampleID (for compatibility with fragments - if both match then sample)
  <int, 1> : length of this subvolume in voxels (will be variable at edges)
  <all-zero-flag byte for sample for this subvolume>
  <2nd-stage byte sequence, either 2-bit or 4-bit>
>

# Begin 2nd-stage subvolume series for fragments
<for each fragment...
  <for each 1st-stage subvolume...
    <int, 1> : x-start
    <int, 1> : y-start
    <int, 1> : z-start
    <long, 1> : fragmentID
    <long, 1> : parent sampleID
    <int, 1> : length of this subvolume in voxels (will be variable at edges)
    <all-zero-flag byte>
    <<byte, (2 or 4 bits) * unit-m * unit-m * unit-m | mod 8 > : unit-m == unit, except within edge-cubes when matches border lengths
  >
>

To construct this, we need a file for each sample that provides the metadata to generate the index file.

This "Sample Specification File" will be available for each sample, and have this structure:

SAMPLE id
OWNER
ALIGNMENT_RESULT id
SPACE
ALIGNED_STACK id
NCC_SCORE
NEURON_SEPARATION id
CONSOLIDATED_LABEL
CONSOLIDATED_SIGNAL
<list of...>
FRAGMENT id
INDEX
MASK_PATH

These will be parsed from the "sampleInfo.txt" files, and put into objects IF they match the alignment space
specified by the index specification, which is in the SPACE attribute.

If there are multiple matching alignments, the one with the most recent "NEURON SEPARATION id" will be chosen.


 */

#include "VolumeIndex.h"
#include "../utility/ImageLoader.h"

const int VolumeIndex::MODE_UNDEFINED=-1;
const int VolumeIndex::MODE_CREATE_SAMPLE_INDEX_FILE=0;
const int VolumeIndex::MODE_SEARCH=1;
const int VolumeIndex::MODE_ADD_SAMPLE_INDEX_FILE_TO_CONSOLIDATED_INDEX=2;

///////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SampleSort::operator()(const SampleSpecification *s1, const SampleSpecification *s2 ) const {
  if (s1->neuronSeparationId>s2->neuronSeparationId) {
    return true;
  } 
  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

VolumeIndex::VolumeIndex()
{
  fid=0;
  mode=MODE_UNDEFINED;
  DEBUG_FLAG=false;
  indexSpecification=0L;
  sampleSpecification=0L;
  sampleConsolidatedSignalImage=0L;
  firstStageIndex=0L;
  secondStageIndex=0L;
}

VolumeIndex::~VolumeIndex()
{}

bool VolumeIndex::execute()
{
  if (mode==MODE_UNDEFINED) {
    return false;
  } else if (mode==MODE_CREATE_SAMPLE_INDEX_FILE) {
    return createSampleIndexFile();
  } else if (mode==MODE_SEARCH) {
    return doSearch();
  } else if (mode==MODE_ADD_SAMPLE_INDEX_FILE_TO_CONSOLIDATED_INDEX) {
    return addSampleToConsolidatedIndex();
  }
  return false;
}

int VolumeIndex::processArgs(vector<char*> *argList)
{
    for (int i=0;i<argList->size();i++) {
        QString arg=(*argList)[i];
        if (arg=="-mode") {
            modeString=(*argList)[++i];
        }
	if (arg=="-indexSpecificationFile") {
	  indexSpecificationFilepath=(*argList)[++i];
	}
	if (arg=="-sampleIndexFile") {
	  sampleIndexFilepath=(*argList)[++i];
	}
	if (arg=="-sampleSpecificationFile") {
	  sampleSpecificationFilepath=(*argList)[++i];
	}
	if (arg=="-query") {
	  queryFilepath=(*argList)[++i];
	}
	if (arg=="-maxHits") {
	  maxHitsString=(*argList)[++i];
	  maxHits=maxHitsString.toInt();
	}
    }
    if (modeString.size()>0) {
      if (modeString=="sample") {
	mode=MODE_CREATE_SAMPLE_INDEX_FILE;
      } else if (modeString=="search") {
	mode=MODE_SEARCH;
      } else if (modeString=="index") {
	mode=MODE_ADD_SAMPLE_INDEX_FILE_TO_CONSOLIDATED_INDEX;
      }
    }
    return 0;
}

bool VolumeIndex::createSampleIndexFile()
{
  if (indexSpecificationFilepath.isNull()) {
    qDebug() << "indexSpecificationFilepath not set";
    return false;
  }
  if (!readIndexSpecificationFile()) {
    qDebug() << "Could not read file " << indexSpecificationFilepath;
    return false;
  }
  if (sampleSpecificationFilepath.isNull()) {
    qDebug() << "sampleSpecificationFilepath not set";
    return false;
  }
  if (!readSampleSpecificationFile()) {
    qDebug() << "Could not read file " << sampleSpecificationFilepath;
    return false;
  }
  if (sampleSpecification==0L) {
    qDebug() << "Could not find matching alignment entry";
    return true;
  }
  QString consolidatedSignalFilepath=sampleSpecification->consolidatedSignalPath;
  if (!loadSampleConsolidatedSignalImage()) {
    qDebug() << "Could not load consolidatedSignalFilepath=" << consolidatedSignalFilepath;
    return false;
  }
  if (!createFirstStageIndex()) {
    qDebug() << "Error creating first-stage index data";
    return false;
  }
  
  return true;
}

bool VolumeIndex::doSearch()
 {
   return true;
 }

 bool VolumeIndex::addSampleToConsolidatedIndex()
 {
   return true;
 }

bool VolumeIndex::readIndexSpecificationFile()
{
  QFile indexSpecificationQFile(indexSpecificationFilepath);
  if (!indexSpecificationQFile.open(QIODevice::ReadOnly)) {
    qDebug() << "Could not open file=" << indexSpecificationFilepath << " to read";
    return false;
  }
  QString eqString("=");
  indexSpecification=new IndexSpecification();
  while(!indexSpecificationQFile.atEnd()) {
    QString line=indexSpecificationQFile.readLine();
    line=line.trimmed();
    if (line.contains(eqString)) {
      QRegExp eqSplit("=");
      QStringList eqList=line.split(eqSplit);
      if (eqList.size()==2) {
	if (eqList[0]=="space") {
	  indexSpecification->space=eqList[1];
	} else if (eqList[0]=="optical_x") {
	  indexSpecification->optical_x=eqList[1].toDouble();
	} else if (eqList[0]=="optical_y") {
	  indexSpecification->optical_y=eqList[1].toDouble();
	} else if (eqList[0]=="optical_z") {
	  indexSpecification->optical_z=eqList[1].toDouble();
	} else if (eqList[0]=="pixel_x") {
	  indexSpecification->pixel_x=eqList[1].toInt();
	} else if (eqList[0]=="pixel_y") {
	  indexSpecification->pixel_y=eqList[1].toInt();
	} else if (eqList[0]=="pixel_z") {
	  indexSpecification->pixel_z=eqList[1].toInt();
	} else if (eqList[0]=="index_unit") {
	  indexSpecification->index_unit=eqList[1].toInt();
	} else if (eqList[0]=="bits") {
	  indexSpecification->bit_depth=eqList[1].toInt();
	} else if (eqList[0]=="threshold") {
	  indexSpecification->binary_threshold=eqList[1].toInt();
	} else if (eqList[0]=="root") {
	  indexSpecification->rootPath=eqList[1];
	}
      }
    }
  }
  indexSpecificationQFile.close();
  qDebug() << "Index Specification:\n" << indexSpecification->toString();
  return true;
}

bool VolumeIndex::readSampleSpecificationFile()
{
  QFile sampleSpecificationQFile(sampleSpecificationFilepath);
  if (!sampleSpecificationQFile.open(QIODevice::ReadOnly)) {
    qDebug() << "Could not open file=" << sampleSpecificationFilepath << " to read";
    return false;
  }
  if (indexSpecification==0L) {
    qDebug() << "IndexSpecification must be populated before the readSampleSpecificationFile() method is called";
    return false;
  }

  // In case there is more than one qualifying aligned sample, we need to collect them and pick
  // the most recent.

  const int STATE_INITIAL=0;
  const int STATE_ALIGNMENT_RESULT=1;
  const int STATE_ALIGNED_STACK=2;
  const int STATE_NEURON_SEPARATION=3;
  const int STATE_FRAGMENT=4;

  int state=STATE_INITIAL;
  SampleSpecification* ss=0L;
  FragmentSpecification* fs=0L;

  long sampleId=0L;
  QString owner;
  bool reprocessLineFlag=false;
  QString line;

  // First, read the file and create the (possibly multiple) sample specification objects
  while(!sampleSpecificationQFile.atEnd()) {
    if (!reprocessLineFlag) {
      line=sampleSpecificationQFile.readLine();
      line=line.trimmed();
    } else {
      reprocessLineFlag=false;
    }
    if (state==STATE_INITIAL) {
      if (line.startsWith("SAMPLE")) {
	QStringList sl=line.split("id=");
	if (sl.size()!=2) {
	  qDebug() << "Could not parse line=" << line;
	  sampleSpecificationQFile.close();
	  return false;
	}
	sampleId=sl[1].toLong();
      } else if (line.startsWith("ALIGNMENT_RESULT")) {
	if (!ss==0L) {
	  qDebug() << "Should not have active sampleAlignmentSpecification here. line=" << line;
	  sampleSpecificationQFile.close();
	  return false;
	}
	ss=new SampleSpecification();
	ss->sampleId=sampleId;
	ss->owner=owner;
	state=STATE_ALIGNMENT_RESULT;
	QStringList sl=line.split("id=");
	if (sl.size()!=2) {
	  qDebug() << "Could not parse line=" << line;
	  sampleSpecificationQFile.close();
	  return false;
	}
	ss->alignmentResultId=sl[1].toLong();
      } else if (line.startsWith("OWNER")) {
	QStringList sl=line.split("=");
	if (sl.size()!=2) {
	  qDebug() << "Could not parse line=" << line;
	  sampleSpecificationQFile.close();
	  return false;
	}
	owner=sl[1];
      }
    } else if (state==STATE_ALIGNMENT_RESULT) {
      if (line.startsWith("SPACE")) {
	QStringList sl=line.split("=");
	if (sl.size()!=2) {
	  qDebug() << "Could not parse line=" << line;
	  sampleSpecificationQFile.close();
	  return false;
	}
	ss->space=sl[1];
      } else if (line.startsWith("ALIGNED_STACK")) {
	QStringList sl=line.split("id=");
	if (sl.size()!=2) {
	  qDebug() << "Could not parse line=" << line;
	  sampleSpecificationQFile.close();
	  return false;
	}
	ss->alignedStackId=sl[1].toLong();
	state=STATE_ALIGNED_STACK;
      }
    } else if (state==STATE_ALIGNED_STACK) {
      if (line.startsWith("NCC_SCORE")) {
	QStringList sl=line.split("=");
	if (sl.size()!=2) {
	  qDebug() << "Could not parse line=" << line;
	  sampleSpecificationQFile.close();
	  return false;
	}
	ss->nccScore=sl[1].toDouble();
      } else if (line.startsWith("NEURON_SEPARATION")) {
	state=STATE_NEURON_SEPARATION;
	QStringList sl=line.split("id=");
	if (sl.size()!=2) {
	  qDebug() << "Could not parse line=" << line;
	  sampleSpecificationQFile.close();
	  return false;
	}
	ss->neuronSeparationId=sl[1].toLong();
      }
    } else if (state==STATE_NEURON_SEPARATION) {
      if (line.startsWith("CONSOLIDATED_LABEL")) {
	QStringList sl=line.split("=");
	if (sl.size()!=2) {
	  qDebug() << "Could not parse line=" << line;
	  sampleSpecificationQFile.close();
	  return false;
	}
	ss->consolidatedLabelPath=sl[1];
      } else if (line.startsWith("CONSOLIDATED_SIGNAL")) {
	QStringList sl=line.split("=");
	if (sl.size()!=2) {
	  qDebug() << "Could not parse line=" << line;
	  sampleSpecificationQFile.close();
	  return false;
	}
	ss->consolidatedSignalPath=sl[1];
      } else if (line.startsWith("FRAGMENT")) {
	QStringList sl=line.split("id=");
	if (sl.size()!=2) {
	  qDebug() << "Could not parse line=" << line;
	  sampleSpecificationQFile.close();
	  return false;
	}
	fs=new FragmentSpecification();
	fs->fragmentId=sl[1].toLong();
	state=STATE_FRAGMENT;
      }
    } else if (state==STATE_FRAGMENT) {
      if (line.startsWith("INDEX")) {
	QStringList sl=line.split("=");
	if (sl.size()!=2) {
	  qDebug() << "Could not parse line=" << line;
	  sampleSpecificationQFile.close();
	  return false;
	}
	fs->index=sl[1].toInt();
      } else if (line.startsWith("MASK_PATH")) {
	QStringList sl=line.split("=");
	if (sl.size()!=2) {
	  qDebug() << "Could not parse line=" << line;
	  sampleSpecificationQFile.close();
	  return false;
	}
	fs->maskPath=sl[1];
	ss->fragmentList.append(fs);
      } else if (line.startsWith("FRAGMENT")) {
	QStringList sl=line.split("id=");
	if (sl.size()!=2) {
	  qDebug() << "Could not parse line=" << line;
	  sampleSpecificationQFile.close();
	  return false;
	}
	fs=new FragmentSpecification();
	fs->fragmentId=sl[1].toLong();
      } else {
	// Assume this means we are done with all fragments for this alignment
	addAlignmentIfMatchingSpace(ss);
	ss=0L;
	state=STATE_INITIAL;
	reprocessLineFlag=true;
      }
    }
  }
  // Capture last entry
  sampleSpecificationQFile.close();
  if (ss!=0L) {
    addAlignmentIfMatchingSpace(ss);
    ss=0L;
    state=STATE_INITIAL;
  }
  
  // Next, sort the sample list and choose the most recent by NeuronSeparation
  qDebug() << "sampleSpecificationList has " << sampleSpecificationList.size() << "entries";
  if (sampleSpecificationList.size()==0) {
    qDebug() << "Could not find any matching sample specification info";
    return true;
  } else if (sampleSpecificationList.size()==1) {
      sampleSpecification=sampleSpecificationList[0];
      return true;
  } else {
    // Need to sort
    qSort(sampleSpecificationList.begin(), sampleSpecificationList.end(), SampleSort());
    sampleSpecification=sampleSpecificationList[0];
  }
  return true;
}

void VolumeIndex::addAlignmentIfMatchingSpace(SampleSpecification* ss) {
  if (ss->space==indexSpecification->space) {
    sampleSpecificationList.append(ss);
  } else {
    qDebug() << "Skipping alignment for non-matching space " << ss->space;
  }
}

bool VolumeIndex::loadSampleConsolidatedSignalImage()
{
  ImageLoader loader;
  QString consolidatedSignalFilepath=sampleSpecification->consolidatedSignalPath;
  if (consolidatedSignalFilepath.isNull()) {
    qDebug() << "consolidatedSignalFilepath is null";
    return false;
  }
  sampleConsolidatedSignalImage=new My4DImage();
  if (!loader.loadImage(sampleConsolidatedSignalImage, consolidatedSignalFilepath)) {
    qDebug() << "Could not load file=" << consolidatedSignalFilepath;
    return false;
  }
  return validateImageSize(sampleConsolidatedSignalImage);
}

bool VolumeIndex::validateImageSize(My4DImage* image)
{
  if (image->getXDim()==indexSpecification->pixel_x &&
      image->getYDim()==indexSpecification->pixel_y &&
      image->getZDim()==indexSpecification->pixel_z) {
    return true;
  } else {
    qDebug() << "image dimensions do not match index " << image->getXDim() << " " << image->getYDim() << " " << image->getZDim();
    return false;
  }
}

bool VolumeIndex::createFirstStageIndex()
{
  if (sampleConsolidatedSignalImage==0L) {
    qDebug() << "createFirstStageIndex() depends on sampleConsolidatedSignalImage being instantiated";
    return false;
  }
  // Estimate a buffer size for the first stage index
  const int x_size=indexSpecification->pixel_x;
  const int y_size=indexSpecification->pixel_y;
  const int z_size=indexSpecification->pixel_z;
  const int unit=indexSpecification->index_unit;
  const int bits=indexSpecification->bit_depth;
  const long sampleId=sampleSpecification->sampleId;
  QString owner=sampleSpecification->owner;

  const int x1_size=divideDimensionByUnit(x_size, unit);
  const int y1_size=divideDimensionByUnit(y_size, unit);
  const int z1_size=divideDimensionByUnit(z_size, unit);

  const int BUFFER_SIZE = 
    sizeof(int) * 5 + // x, y, z, unit, bits
    sizeof(long) + // sampleID
    sizeof(int) + // length of owner
    sizeof(char) * 256 + // 256 max length
    sizeof(int) + // sample data length for 1st stage
    ((x_size/unit + 1) * (y_size/unit + 1) * (z_size/unit + 1))/8 + 1; // size of 2-bit mask 
  firstStageIndex=new char[BUFFER_SIZE];

  // Populate the data-independent part of the buffer
  int* ip=(int*)firstStageIndex;
  *ip=x_size;
  ip++;
  *ip=y_size;
  ip++;
  *ip=z_size;
  ip++;
  *ip=unit;
  ip++;
  *ip=bits;
  ip++;
  long* lp=(long*)ip;
  *lp=sampleId;
  lp++;
  ip=(int*)lp;
  int ownerLength=owner.length();
  if (ownerLength>255) {
    delete [] firstStageIndex;
    qDebug() << "ownerLength has exceeded 255";
    return false;
  }
  *ip=ownerLength;
  ip++;
  char* cp=(char*)ip;
  const char* od=owner.toUtf8().constData();
  for (int i=0;i<ownerLength;i++) {
    *cp=od[i];
    cp++;
  }
  int s1TotalSize=x1_size*y1_size*z1_size;
  int s1MaskSize=s1TotalSize/8;
  if (s1MaskSize*8<s1TotalSize) {
    s1MaskSize++;
  }
  ip=(int*)cp;
  *ip=s1MaskSize;
  ip++;
  cp=(char*)ip;
  int subsampledBinaryMaskSize=subsampleAndThresholdToBinaryMask(sampleConsolidatedSignalImage, cp, unit, indexSpecification->binary_threshold);
  if (subsampledBinaryMaskSize!=s1MaskSize) {
    qDebug() << "Error, result mask does not equal expected mask size. Result=" << subsampledBinaryMaskSize << " expected=" << s1MaskSize;
    return false;
  } else {
    qDebug() << "First stage mask matches expected length";
  }
  cp+=subsampledBinaryMaskSize;
  firstStageIndexLength=cp-firstStageIndex;
  return true;
}

int VolumeIndex::divideDimensionByUnit(int originalSize, int unit)
{
  int a1=originalSize/unit;
  if (a1*unit<originalSize) {
    a1++;
  }
  return a1;
}

int VolumeIndex::subsampleAndThresholdToBinaryMask(My4DImage* sourceImage, char* targetMask, int unit, int threshold)
{
  if (sourceImage->getDatatype()!=V3D_UINT8) {
    qDebug() << "only compatible with 8-bit source image";
    return 0;
  }

  if (threshold>255) {
    qDebug() << "Only supports threshold up to 255";
    return 0;
  }

  v3d_uint8** rArr=new v3d_uint8*[sourceImage->getCDim()];

  for (int i=0;i<sourceImage->getCDim();i++) {
    rArr[i]=sourceImage->getRawDataAtChannel(i);
  }

  const v3d_uint8 cThreshold=threshold;

  const int x_dim=sourceImage->getXDim();
  const int y_dim=sourceImage->getYDim();
  const int z_dim=sourceImage->getZDim();
  const int c_dim=sourceImage->getCDim();

  const int x1_size=divideDimensionByUnit(x_dim, unit);
  const int y1_size=divideDimensionByUnit(y_dim, unit);
  const int z1_size=divideDimensionByUnit(z_dim, unit);

  // Outer Loop
  int offset1=-1;
  int bytePosition=0;

  for (int z1=0;z1<z1_size;z1++) {
    for (int y1=0;y1<y1_size;y1++) {
      for (int x1=0;x1<x1_size;x1++) {
	offset1++;
	bytePosition=offset1/8;
	int bitOffset=offset1 % 8;
	if (bitOffset==0) {
	  targetMask[bytePosition]=0; // clear as we move forward
	}

	// Inner Loop
	int z2_limit=(z1+1)*unit;
	if (z2_limit>z_dim) {
	  z2_limit=z_dim;
	}

	int y2_limit=(y1+1)*unit;
	if (y2_limit>y_dim) {
	  y2_limit=y_dim;
	}

	int x2_limit=(x1+1)*unit;
	if (x2_limit>x_dim) {
	  x2_limit=x_dim;
	}

	for (int z2=z1*unit;z2<z2_limit;z2++) {
	  int zyxOffset=z2*x_dim*y_dim;
	  for (int y2=y1*unit;y2<y2_limit;y2++) {

	    bool overThreshold=false;
	    int yxOffset=y2*x_dim;

	    for (int x2=x1*unit;x2<x2_limit;x2++) {
	      int rOffset=zyxOffset + yxOffset + x2;
	      for (int c=0;c<c_dim;c++) {
		if (rArr[c][rOffset] > cThreshold) {
		  overThreshold=true;
		  break;
		}
	      }
	    }

	    if (overThreshold) {
	      char oneC=1;
	      oneC << bitOffset;
	      targetMask[bytePosition] = targetMask[bytePosition] | oneC;
	    }

	  }
	}


      }
    }
  }

  return bytePosition+1;
}


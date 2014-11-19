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

# NOTE: First stage index has complete data for all super-voxels, whereas 2nd-stage is non-zero entries only. The first stage
# Also only contains sample data, and no fragment data.

# Info for 1st stage index

  <long, 1> : sampleID
  <int, 1> : length of owner
  <char, ...> : ascii bytes of owner
  <int, 1> : number of non-zero voxels in first stage, at resolution of 1st-stage
  <byte, 2 bits * x/unit * y/unit * z/unit | mod 8 > : sample data

# NOTE: The second stage is NOT in any kind of order wrt the fragment sub-volume positions. This is because the fragment
# data is read one-at-a-time from mask files, and all entries for one fragment are generated before the next fragment
# is considered.

# Begin 2nd-stage subvolume series for sample data over threshold - the first entry is the sample signal data, followed
# by a series of fragments. Each of these entries (the sample and following fragments), in turn, have a potentially
# large number of subvolume entries.

  <char, 1> : Entry-begin code (93)
  <int, 1> : x-start
  <int, 1> : y-start
  <int, 1> : z-start
  <long, 1> : fragmentID (which is the sampleID for the sample case)
  <long, 1> : sampleID (for compatibility with fragments - if both match then sample)
  <int, 1> : total non-zero voxels for this sample or fragment at 2nd-stage resolution (repeated for each subvolume entry)
  <2nd-stage byte sequence, either 2-bit or 4-bit> (note the length of this section is variable at the edge regions)

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

const char VolumeIndex::ENTRY_CODE=93;

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

  X_SIZE=Y_SIZE=Z_SIZE=UNIT=BITS=0;
  SAMPLE_ID=0L;
  OWNER="";
  X1_SIZE=Y1_SIZE=Z1_SIZE=0;
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
    return false;
  }
  if (!initParamsFromSpecification()) {
    qDebug() << "Problem initializing working parameters";
    return false;
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
  if (!writeSampleIndexHeaderAndFirstStage()) {
    qDebug() << "Error writing sample index header and first stage";
    return false;
  }
  if (!createSecondStageIndex()) {
    qDebug() << "Error creating second-stage index data";
    return false;
  }
  fclose(fid);
  delete sampleConsolidatedSignalImage;
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
	  QRegExp commaSplit(",");
	  QStringList cList=eqList[1].split(commaSplit);
	  if (cList.size()==0) {
	    qDebug() << "Could not parse threshold line=" << eqList[1];
	    return false;
	  }
	  indexSpecification->thresholdCount=eqList.size();
	  indexSpecification->thresholdArray=new int[indexSpecification->thresholdCount];
	  for (int i=0;i<eqList.size();i++) {
	    indexSpecification->thresholdArray[i]=eqList[i].trimmed().toInt();
	  }
	} else if (eqList[0]=="root") {
	  indexSpecification->rootPath=eqList[1];
	}
      }
    }
  }
  if (indexSpecification->bit_depth!=indexSpecification->thresholdCount) {
    qDebug() << "indexSpecification bit_depth does not match thresholdCount";
    return false;
  }
  if (indexSpecification->bit_depth!=2 && indexSpecification->bit_depth!=4) {
    qDebug() << "only bit_depth of 2 or 4 is supported";
    return false;
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

  const int BUFFER_SIZE = 
    sizeof(int) * 5 + // x, y, z, unit, bits
    sizeof(long) + // sampleID
    sizeof(int) + // length of owner
    sizeof(char) * 256 + // 256 max length
    sizeof(int) + // non-zero voxel count for this 1st-stage entry
    ((X_SIZE/UNIT + 1) * (Y_SIZE/UNIT + 1) * (Z_SIZE/UNIT + 1))/8 + 1; // size of 2-bit mask 
  firstStageIndex=new char[BUFFER_SIZE];

  // Populate the data-independent part of the buffer
  int* ip=(int*)firstStageIndex;
  *ip=X_SIZE;
  ip++;
  *ip=Y_SIZE;
  ip++;
  *ip=Z_SIZE;
  ip++;
  *ip=UNIT;
  ip++;
  *ip=BITS;
  ip++;
  long* lp=(long*)ip;
  *lp=sampleSpecification->sampleId;
  lp++;
  ip=(int*)lp;
  int ownerLength=sampleSpecification->owner.length();
  if (ownerLength>255) {
    delete [] firstStageIndex;
    qDebug() << "ownerLength has exceeded 255";
    return false;
  }
  *ip=ownerLength;
  ip++;
  char* cp=(char*)ip;
  const char* od=sampleSpecification->owner.toUtf8().constData();
  for (int i=0;i<ownerLength;i++) {
    *cp=od[i];
    cp++;
  }
  // Estimate expected size of mask data
  int s1TotalSize=X1_SIZE*Y1_SIZE*Z1_SIZE;
  int s1MaskSize=s1TotalSize/8;
  if (s1MaskSize*8<s1TotalSize) {
    s1MaskSize++;
  }
  int* nonZeroPlaceholder=(int*)cp;
  ip=(int*)cp;
  ip++;
  cp=(char*)ip;
  int* sbmResults=subsampleAndThresholdToBinaryMask(sampleConsolidatedSignalImage, cp, UNIT, indexSpecification->thresholdArray[0]);
  int subsampledBinaryMaskSize = sbmResults[0];
  *nonZeroPlaceholder=sbmResults[1];
  if (subsampledBinaryMaskSize!=s1MaskSize) {
    qDebug() << "Error, result mask does not equal expected mask size. Result=" << subsampledBinaryMaskSize << " expected=" << s1MaskSize;
    return false;
  } else {
    qDebug() << "First stage mask matches expected length";
  }
  cp+=subsampledBinaryMaskSize;
  firstStageIndexLength=cp-firstStageIndex;
  delete [] sbmResults;
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

int* VolumeIndex::subsampleAndThresholdToBinaryMask(My4DImage* sourceImage, char* targetMask, int unit, int threshold)
{
  int* sbmResults=new int[2]; // 0 is total size, 1 is non-zero count
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

  int nonZeroCount=0;

  // Outer Loop
  int offset1=-1;
  int bytePosition=0;

  for (int z1=0;z1<Z1_SIZE;z1++) {
    for (int y1=0;y1<Y1_SIZE;y1++) {
      for (int x1=0;x1<X1_SIZE;x1++) {
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
	      nonZeroCount++;
	      char oneC=1;
	      oneC << bitOffset;
	      targetMask[bytePosition] = targetMask[bytePosition] | oneC;
	    }

	  }
	}

      }
    }
  }
  delete [] rArr;
  sbmResults[0] = bytePosition+1;
  sbmResults[1] = nonZeroCount;
  return sbmResults;
}

bool VolumeIndex::initParamsFromSpecification()
{
  X_SIZE=indexSpecification->pixel_x;
  Y_SIZE=indexSpecification->pixel_y;
  Z_SIZE=indexSpecification->pixel_z;
  UNIT=indexSpecification->index_unit;
  BITS=indexSpecification->bit_depth;
  SAMPLE_ID=sampleSpecification->sampleId;
  OWNER=sampleSpecification->owner;
  X1_SIZE=divideDimensionByUnit(X_SIZE, UNIT);
  Y1_SIZE=divideDimensionByUnit(Y_SIZE, UNIT);
  Z1_SIZE=divideDimensionByUnit(Z_SIZE, UNIT);
}

void VolumeIndex::clearSecondStageData()
{
  for (int i=0;i<secondStageIndex.size();i++) {
    delete [] secondStageIndex[i];
  }
  secondStageIndex.clear();
  secondStageIndexLength.clear();
  secondStageVoxelCount=0;
}

bool VolumeIndex::createSecondStageIndex()
{
  if (sampleConsolidatedSignalImage==0L) {
    qDebug() << "sampleConsolidatedSignalImage must be populated before createSecondStageIndex";
    return false;
  }

  // Estimate buffer size 
  int secondStageBufferSizePerMask = 1; // entry code

  qDebug() << "Creating index entry for sample from " << sampleSpecification->consolidatedSignalPath;
  if (!createSecondStageEntry(sampleConsolidatedSignalImage, sampleSpecification->sampleId, sampleSpecification->sampleId, false)) {
    qDebug() << "Error generating second-stage index";
    return false;
  }

  if (!writeSampleIndexUpdate()) {
    qDebug() << "Error writing sample index";
    return false;
  }

  clearSecondStageData();

  for (int i=0;i<sampleSpecification->fragmentList.size();i++) {

    FragmentSpecification* fs = sampleSpecification->fragmentList[i];
    QString imagePath=fs->maskPath;
    My4DImage* fragmentImage = new My4DImage();
    ImageLoader loader;
    qDebug() << "Loading fragment path=" << imagePath;
    if (!loader.loadImage(fragmentImage, imagePath)) {
      qDebug() << "Could not load fragment image=" << imagePath;
      return false;
    }
    qDebug() << "Creating index entry for fragment from " << fs->maskPath;
    if (!createSecondStageEntry(fragmentImage, fs->fragmentId, sampleSpecification->sampleId, true)) {
      qDebug() << "Error generating second-stage index";
      return false;
    }

    if (!writeSampleIndexUpdate()) {
      qDebug() << "Error writing sample index";
      return false;
    }

    clearSecondStageData();

    delete fragmentImage;
  }
}

char* VolumeIndex::getAddressOfFirstStageMaskAtCoordinate(int x, int y, int z)
{
  int unitOffset = z*Y1_SIZE*X1_SIZE + y*X1_SIZE + x;
  int byteOffset = unitOffset/8;
  int headerFirstPartSize = sizeof(int) * 5 + // x,y,z, unit, bits
    sizeof(long) + // sampleId
    sizeof(int) + // length of owner
    sizeof(char) * 256 + // 256 max length
    sizeof(int); // non-zero voxel count for this 1st-stage entry
  char* dataStart = firstStageIndex + headerFirstPartSize;
  char* p=dataStart+byteOffset;
  return p;
}

bool VolumeIndex::validatePositiveFirstStageEntry(int x, int y, int z)
{
  int unitOffset = z*Y1_SIZE*X1_SIZE + y*X1_SIZE + x;
  int byteOffset = unitOffset/8;
  char* p=getAddressOfFirstStageMaskAtCoordinate(x, y, z);
  char v=*p;
  char bitOffset = unitOffset - 8*byteOffset;
  v >> bitOffset;
  char bitMask=1;
  char v2 = v & bitMask;
  if (v2>0) {
    // already set properly
    return true;
  } else {
    bitMask << bitOffset;
    *p = *p | bitMask; 
    return false;
  }
}

bool VolumeIndex::createSecondStageEntry(My4DImage* image, long fragmentId, long sampleId, bool updateFirstStage)
{
  int positionsPerByte=8;
  int nonZeroCount=0;

  if (BITS==4) {
    positionsPerByte=4;
  }

  const int c_dim=image->getCDim();
  v3d_uint8** rArr=new v3d_uint8*[c_dim];
  for (int i=0;i<c_dim;i++) {
    rArr[i]=image->getRawDataAtChannel(i);
  }

  int UB_SIZE=UNIT*UNIT*UNIT+100; // the 100 is for the x,y,z,sampleId, etc.
  char* UNIT_BUFFER = new char[UB_SIZE];

  // Outer Loop
  int offset1=-1;
  int offset2=-1;
  int* tArr=indexSpecification->thresholdArray;
  int tN=indexSpecification->thresholdCount;

  for (int z1=0;z1<Z1_SIZE;z1++) {
    for (int y1=0;y1<Y1_SIZE;y1++) {
      for (int x1=0;x1<X1_SIZE;x1++) {

	offset1++;

	// Inner Loop
	int z2_limit=(z1+1)*UNIT;
	if (z2_limit>Z_SIZE) {
	  z2_limit=Z_SIZE;
	}

	int y2_limit=(y1+1)*UNIT;
	if (y2_limit>Y_SIZE) {
	  y2_limit=Y_SIZE;
	}

	int x2_limit=(x1+1)*UNIT;
	if (x2_limit>X_SIZE) {
	  x2_limit=X_SIZE;
	}

	// Reset for next subvolume. Note, we will fill-in the buffer and then afterward, if all zeros, skip the entry
	// for this subvolume
	bool overFirstStageThreshold=false;
	for (int u=0;u<UB_SIZE;u++) {
	  UNIT_BUFFER[u]=0;
	}
	int sbCount=-1;
	int bytePosition=0;
	int headerOffset=0;

	// Add header info to buffer for this subvolume
	char* p=UNIT_BUFFER;
	int* ip=(int*)p;
	*ip = x1*UNIT;
	ip++; 
	*ip = y1*UNIT;
	ip++;
	*ip = z1*UNIT;
	ip++;
	long* lp = (long*)ip;
	*lp = fragmentId;
	lp++;
	*lp = sampleId;
	lp++;
	ip=(int*)lp;
	*ip=0; // total voxel count - to be filled-in later
	ip++;
	p=(char*)ip;
	headerOffset = p-UNIT_BUFFER;

	for (int z2=z1*UNIT;z2<z2_limit;z2++) {
	  int zyxOffset=z2*X_SIZE*Y_SIZE;
	  for (int y2=y1*UNIT;y2<y2_limit;y2++) {
	    int yxOffset=y2*X_SIZE;
	    for (int x2=x1*UNIT;x2<x2_limit;x2++) {
	      sbCount++;
	      offset2++;

	      bytePosition=sbCount/positionsPerByte + headerOffset;

	      int bitOffset=sbCount % positionsPerByte;
	      if (BITS==4) {
		bitOffset=2*(sbCount % positionsPerByte);
	      }

	      if (bitOffset==0) {
		UNIT_BUFFER[bytePosition]=0; // clear as we move forward
	      }

	      int rOffset=zyxOffset + yxOffset + x2;
	      char v=0;
	      v3d_uint8 maxV=0;
	      for (int c=0;c<c_dim;c++) {
		v3d_uint8 rV=rArr[c][rOffset];
		if (rV > maxV) {
		  maxV=rV;
		}
	      }
	      for (int t=tN-1;t>-1;t--) {
		if (maxV>=tArr[t]) {
		  v=t;
		  break;
		}
	      }
	      if (v>0) {
		overFirstStageThreshold=true;
	      }
	      // Now, v should be the correct level for this position
	      if (BITS==2) {
		if (v==1) {
		  char oneC=1;
		  nonZeroCount++;
		  oneC << bitOffset;
		  UNIT_BUFFER[bytePosition] = UNIT_BUFFER[bytePosition] | oneC;
		}
	      } else if (BITS==4) {
		char cv=v;
		if (cv>0) {
		  nonZeroCount++;
		  cv << bitOffset;
		  UNIT_BUFFER[bytePosition] = UNIT_BUFFER[bytePosition] | cv;
		}
	      }
	    }
	  }
	}

	if (updateFirstStage && overFirstStageThreshold) {
	  if (!validatePositiveFirstStageEntry(x1, y1, z1)) {
	    qDebug() << "Correcting first-stage entry at x=" << x1 << " y=" << y1 << " z=" << z1;
	  }
	}

	if (overFirstStageThreshold) {
	  // Then keep this entry
	  secondStageIndexLength.append(bytePosition+1);
	  char* subvolumeData = new char[bytePosition+1];
	  for (int i=0;i<bytePosition+1;i++) {
	    subvolumeData[i]=UNIT_BUFFER[i];
	  }
	  secondStageIndex.append(subvolumeData);
	}

      }
    }
  }

  delete [] UNIT_BUFFER;

  secondStageVoxelCount=nonZeroCount;

  return true;
}

bool VolumeIndex::writeSampleIndexHeaderAndFirstStage()
{
  fid=fopen(sampleIndexFilepath.toAscii().data(), "wb");
  if (!fid) {
    qDebug() << "Could not open file=" << sampleIndexFilepath << " to write";
    return false;
  }

  // General index info
  // <int, 1> : x
  // <int, 1> : y
  // <int, 1> : z
  // <int, 1> : unit
  // <int, 1> : bits

  fwrite(&X_SIZE, sizeof(int), 1, fid);
  fwrite(&Y_SIZE, sizeof(int), 1, fid);
  fwrite(&Z_SIZE, sizeof(int), 1, fid);
  fwrite(&UNIT, sizeof(int), 1, fid);
  fwrite(&BITS, sizeof(int), 1, fid);

  qDebug() << "Writing " << firstStageIndexLength << " bytes for first stage index";
  int l=fwrite(firstStageIndex, firstStageIndexLength, 1, fid);
  if (l!=1) {
    qDebug() << "fwrite failed, expected " << firstStageIndexLength << " but returned " << l;
    return false;
  }

  return true;
}

bool VolumeIndex::writeSampleIndexUpdate()
{
  for (int i=0;i<secondStageIndex.size();i++) {
    char* fp = secondStageIndex[i];
    int fpSize = secondStageIndexLength[i];
    fwrite(&ENTRY_CODE, sizeof(char), 1, fid);
    int voxelCountOffset = sizeof(char) + 3*sizeof(int) + 2*sizeof(long);
    int* vc = (int*)(fp + voxelCountOffset);
    *vc = secondStageVoxelCount;
    qDebug() << "Writing " << fpSize << " bytes for second stage index";
    int l = fwrite(fp, fpSize, 1, fid);
    if (l!=1) {
      qDebug() << "fwrite failed";
      return false;
    }
  }
  fflush(fid);
  return true;
}



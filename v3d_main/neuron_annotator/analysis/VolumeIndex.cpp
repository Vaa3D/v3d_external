/*

When samples are processed and their corresponding index files are generated (before they are integrated into the unified index), this is their structure:

1) The samples are formatted in sequence x,y,z, according to the global index specification file

2) When the fragment/sample-intensity data is extracted and used to populate the unified index, we want the individual sections to be efficiently distributed

3) Therefore, as we write the fragment data, it should be in (x,y,z) chunks that correspond to these subvolumes, in the order of the original x,y,z top-level data

4) Temporary index files created for samples will have this header corresponding to the index dimensions. This should be checked against the dimensions described by the index specification information to validate the index file before it is processed. NOTE: the actual first-stage binary index DOES NOT contain this header info, and starts with the "sampleID..." info below. In this way, the sample index files and the overall main index differ.

# General index info
<int, 1> : x
<int, 1> : y
<int, 1> : z
<int, 1> : unit
<int, 1> : bits

# NOTE: First stage index has complete data for all super-voxels, whereas 2nd-stage is non-zero entries only. The first stage
# Also only contains sample data, and no fragment data.

# Info for 1st stage index - note 1st stage is a BINARY index, whereas the 2nd-stage may be 1-bit (binary) or 2-bit (4-valued)

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
  <2nd-stage byte sequence, either 1-bit or 2-bit> (note the length of this section is variable at the edge regions)

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

=======================================================================================================================
Search
=======================================================================================================================

Overall Management:

* Search assumes that the overall score for a query vs subject can be computed by combining the independent scores from all the subvolumes
* The way the index is constructed, only subvolumes with non-zero data are included (and those are explicitly specified).
* Every sample:fragment entry needs to be scored
* Because we are weighing background matches, we need to do a complete compute for all subvoumes, including the empty ones, vs the query
* Therefore, (1) the first analysis step will be to compute, for each subvolume, the score of the query vs an empty subvolume
* This "empty" score set can be used to complement the scores for non-zero subvolumes
* The (2) second step will be to visit all subvolume indices (i.e., all subdirectories) and determine the scores, using a hash
* (3) After visiting all 2nd-stage entries, we can then cycle through each <sample:fragment> and compute the overall score for each
* (4) We will do this for each <sample:fragment> by iterating through a list of all subvolume positions and using either the non-zero score (as computed in #2), or the empty score from #1.
* Once we have all the scores, they can be sorted

Scoring Computation:

* We know the non-zero and zero counts from the binary search mask (or we need to compute it)
* We score both the non-zero and zero voxels of the query vs each subject voxel
* The difference is simply the bit-distance (0,1 for binary or 0,1,2,3 for 2-bit)

* The non-zero and zero groups are accumulated into separate groups
* The average score for each group is normalized 0-1, resulting in two numbers, both between 0.0 and 1.0, for non-zero and zero, respectively
* Then, the relative importance of non-zero to zero is taken into account
* This provides an overall score, where lower is better

Implementation

* While there is no need to preserve the results of the individual subvolume q=>s computations, the blank-subject scores need to be available to fill-in missing components from subjects in general
* The score from a subvolume needs to have several pieces of information:

1) the total number of zero query voxels
2) the total number of non-zero query voxels
3) the total score of the zero query group (the sum of the differences)
4) the total score of the non-zero query group

* A 4D array will be created to store these values for the blank case
* To do a search, we will:

1) incrementally read each entry in the 1st-stage index
2) for each 1st-stage index entry, launch a thread to compute the score
3) each thread will serially compute the score for each sub-volume, and then when done compute the overall score, in addition to the overall counts
4) each overall score and counts will be placed into a list of results
5) the list will be sorted
6) the results reported

*********************/
#if defined(USE_Qt5)
    #include <QtConcurrent>
#endif 

#include "MaskChan.h"
#include "SleepThread.h"
#include "VolumeIndex.h"
#include "DilationErosion.h"
#include "../utility/ImageLoaderBasic.h"

#include "../terafly/src/presentation/theader.h"  //2015May PHC

const int VolumeIndex::MODE_UNDEFINED=-1;
const int VolumeIndex::MODE_CREATE_SAMPLE_INDEX_FILE=0;
const int VolumeIndex::MODE_SEARCH=1;
const int VolumeIndex::MODE_ADD_SAMPLE_INDEX_FILE_TO_CONSOLIDATED_INDEX=2;

const char VolumeIndex::ENTRY_CODE=93;
const int VolumeIndex::MAX_OWNER_LENGTH=256;

const QString VolumeIndex::PRIMARY_INDEX_FILENAME="primary_stage.gindex";
const QString VolumeIndex::SECONDARY_INDEX_FILENAME="secondary_stage.gindex";

///////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SampleSort::operator()(const SampleSpecification *s1, const SampleSpecification *s2 ) const {
  if (s1->neuronSeparationId>s2->neuronSeparationId) {
    return true;
  }
  return false;
}

bool ScoreSort::operator()(const SubjectScore *s1, const SubjectScore *s2 ) const {
  if (s1->maskScore->score < s2->maskScore->score) {
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

VolumeIndex::VolumeIndex()
{
  ERROR_FLAG=false;
  fid=0;
  mainIndexFid=0;
  mode=MODE_UNDEFINED;
  DEBUG_FLAG=false;
  indexSpecification=0L;
  sampleSpecification=0L;
  sampleConsolidatedSignalImage=0L;
  firstStageIndex=0L;
  firstStageCorrectionCount=0;

  X_SIZE=Y_SIZE=Z_SIZE=UNIT=BITS=0;
  SAMPLE_ID=0L;
  OWNER="";
  X1_SIZE=Y1_SIZE=Z1_SIZE=0;

  queryThreshold=10;
  minSubjectVoxels=200;
  maxHits=100;
  backgroundWeight=1.0L;
  maskDilation=0;

  queryImage=0L;
  blankSubjectScore=0L;
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
	if (arg=="-minSubjectVoxels") {
	  QString minSubjectVoxelsString=(*argList)[++i];
	  minSubjectVoxels=minSubjectVoxelsString.toInt();
	}
	if (arg=="-queryThreshold") {
	  QString queryThresholdString=(*argList)[++i];
	  queryThreshold=queryThresholdString.toInt();
	}
	if (arg=="-maxHits") {
	  maxHitsString=(*argList)[++i];
	  maxHits=maxHitsString.toInt();
	}
	if (arg=="-backgroundWeight") {
	  QString backgroundWeightString=(*argList)[++i];
	  backgroundWeight=backgroundWeightString.toDouble();
	}
	if (arg=="-maskDilation") {
	  QString maskDilationString=(*argList)[++i];
	  maskDilation=maskDilationString.toInt();
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
  if (DEBUG_FLAG) qDebug() << "calling readSampleSpecificationFile()";
  if (!readSampleSpecificationFile()) {
    qDebug() << "Could not read file " << sampleSpecificationFilepath;
    return false;
  }
  if (sampleSpecification==0L) {
    qDebug() << "Could not find matching alignment entry";
    return false;
  }
  if (DEBUG_FLAG) qDebug() << "calling initParamsFromIndexSpecification()";
  if (!initParamsFromIndexSpecification()) {
    qDebug() << "Problem initializing working parameters";
    return false;
  }
  if (DEBUG_FLAG) qDebug() << "calling initParamsFromSampleSpecification()";
  if (!initParamsFromSampleSpecification()) {
    qDebug() << "Problem initializing working parameters";
    return false;
  }
  QString consolidatedSignalFilepath=sampleSpecification->consolidatedSignalPath;
  if (DEBUG_FLAG) qDebug() << "calling loadSampleConsolidatedSignalImage()";
  if (!loadSampleConsolidatedSignalImage()) {
    qDebug() << "Could not load consolidatedSignalFilepath=" << consolidatedSignalFilepath;
    return false;
  }
  if (DEBUG_FLAG) qDebug() << "calling createFirstStageIndexForSampleFile()";
  if (!createFirstStageIndexForSampleFile()) {
    qDebug() << "Error creating first-stage index data";
    return false;
  }
  if (DEBUG_FLAG) qDebug() << "calling writeSampleIndexHeaderAndFirstStage()";
  if (!writeSampleIndexHeaderAndFirstStage()) {
    qDebug() << "Error writing sample index header and first stage";
    return false;
  }
  if (DEBUG_FLAG) qDebug() << "calling createSecondStageIndexForSample()";
  if (!createSecondStageIndexForSampleFile()) {
    qDebug() << "Error creating second-stage index data";
    return false;
  }
  long s2position=ftell(fid);
  if (DEBUG_FLAG) qDebug() << "Final file position for creation of index=" << s2position;
  fclose(fid);
  delete sampleConsolidatedSignalImage;
  if (DEBUG_FLAG) qDebug() << "First stage correction count=" << firstStageCorrectionCount;
  return true;
}

bool VolumeIndex::doSearch()
{
  if (DEBUG_FLAG) qDebug() << "Reading index specification file";
  if (!readIndexSpecificationFile()) {
    qDebug() << "Error reading index specification file";
    return false;
  }
  if (DEBUG_FLAG) qDebug() << "Initializing index parameters";
  if (!initParamsFromIndexSpecification()) {
    qDebug() << "Problem initializing working parameters";
    return false;
  }
  if (DEBUG_FLAG) qDebug() << "Loading query image";
  if (!loadQueryImage()) {
    qDebug() << "Error loading query image " << queryFilepath;
    return false;
  }
  if (maskDilation>0) {
    qDebug() << "Dilating query mask";
    dilateQueryMask();
  }
  if (DEBUG_FLAG) qDebug() << "Computing blank subject score";
  if (!computeBlankSubjectScore()) {
    return false;
  }
  if (DEBUG_FLAG) qDebug() << "Computing subject scores";
  if (!computeSubjectScores()) {
    qDebug() << "Error computing subject scores";
    return false;
  }
  if (!displaySearchResults()) {
    qDebug() << "Error displaying search results";
    return false;
  }
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
	  indexSpecification->thresholdCount=cList.size();
	  indexSpecification->thresholdArray=new int[indexSpecification->thresholdCount];
	  for (int i=0;i<cList.size();i++) {
	    indexSpecification->thresholdArray[i]=cList[i].trimmed().toInt();
	  }
	} else if (eqList[0]=="root") {
	  indexSpecification->rootPath=eqList[1];
	}
      }
    }
  }
  if ( ! ((indexSpecification->bit_depth==1 && indexSpecification->thresholdCount==1) ||
	  (indexSpecification->bit_depth==2 && indexSpecification->thresholdCount==3))) {
    qDebug() << "indexSpecification bit_depth does not match thresholdCount";
    return false;
  }
  if (indexSpecification->bit_depth!=1 && indexSpecification->bit_depth!=2) {
    qDebug() << "only bit_depth of 1 or 2 is supported";
    return false;
  }
  indexSpecificationQFile.close();
  if (DEBUG_FLAG) qDebug() << "Index Specification:\n" << indexSpecification->toString();
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////


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
  if (DEBUG_FLAG) qDebug() << "sampleSpecificationList has " << sampleSpecificationList.size() << "entries";
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
  ImageLoaderBasic loader;
  QString consolidatedSignalFilepath=sampleSpecification->consolidatedSignalPath;
  if (consolidatedSignalFilepath.isNull()) {
    qDebug() << "consolidatedSignalFilepath is null";
    return false;
  }
  sampleConsolidatedSignalImage=new My4DImage();
  if (DEBUG_FLAG) qDebug() << "Using ImageLoaderBasic, calling loader.loadImage()";
  if (!loader.loadImage(sampleConsolidatedSignalImage, consolidatedSignalFilepath.toUtf8().constData())) {
    qDebug() << "Could not load file=" << consolidatedSignalFilepath;
    return false;
  }
  if (DEBUG_FLAG) qDebug() << "after loader.loadImage()";
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

bool VolumeIndex::createFirstStageIndexForSampleFile()
{
  if (sampleConsolidatedSignalImage==0L) {
    qDebug() << "createFirstStageIndex() depends on sampleConsolidatedSignalImage being instantiated";
    return false;
  }

  const int BUFFER_SIZE =
    sizeof(int) * 5 + // x, y, z, unit, bits
    sizeof(long) + // sampleID
    sizeof(int) + // length of owner
    sizeof(char) * MAX_OWNER_LENGTH +
    sizeof(int) + // non-zero voxel count for this 1st-stage entry
    ((X_SIZE/UNIT + 1) * (Y_SIZE/UNIT + 1) * (Z_SIZE/UNIT + 1))/8 + 1; // size of 1-bit mask
  firstStageIndex=new char[BUFFER_SIZE];
  for (int i=0;i<BUFFER_SIZE;i++) {
    firstStageIndex[i]=0;
  }

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
  if (DEBUG_FLAG) qDebug() << "Stage 1 total voxel count=" << s1TotalSize;
  int* nonZeroPlaceholder=(int*)cp;
  ip=(int*)cp;
  ip++;
  cp=(char*)ip;
  int* sbmResults=subsampleAndThresholdToBinaryMask(sampleConsolidatedSignalImage, cp, UNIT, indexSpecification->thresholdArray[0]);
  int subsampledBinaryMaskSize = sbmResults[0];
  *nonZeroPlaceholder=sbmResults[3];

  if (DEBUG_FLAG) qDebug() << "total s1 byte count=" << sbmResults[0];
  if (DEBUG_FLAG) qDebug() << "non-zero entries in 1st stage=" << sbmResults[3];
  if (DEBUG_FLAG) qDebug() << "total s2 size=" << sbmResults[1];
  if (DEBUG_FLAG) qDebug() << "non-zero entries in 2nd stage=" << sbmResults[2];

  if (subsampledBinaryMaskSize!=s1MaskSize) {
    qDebug() << "Error, result mask does not equal expected mask size. Result=" << subsampledBinaryMaskSize << " expected=" << s1MaskSize;
    return false;
  } else {
    if (DEBUG_FLAG) qDebug() << "First stage mask matches expected length";
  }
  cp+=subsampledBinaryMaskSize;
  firstStageIndexBytes=cp-firstStageIndex;
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
  if (DEBUG_FLAG) qDebug() << "subsampleAndThresholdToBinaryMask: using threshold=" << threshold;

  int* sbmResults=new int[4]; // 0 is total s1 size, 1 is s2 total size, 2 is s2 non-zero count, 3 is s1 non-zero count
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

  if (DEBUG_FLAG) qDebug() << "cThreshold=" << cThreshold;

  const int x_dim=sourceImage->getXDim();
  const int y_dim=sourceImage->getYDim();
  const int z_dim=sourceImage->getZDim();
  const int c_dim=sourceImage->getCDim();

  int s2TotalCount=0;
  int s2nonZeroCount=0;
  int s1NonZeroCount=0;

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

	int overThresholdCount=0;

	for (int z2=z1*unit;z2<z2_limit;z2++) {
	  int zyxOffset=z2*x_dim*y_dim;
	  for (int y2=y1*unit;y2<y2_limit;y2++) {
	    int yxOffset=y2*x_dim;
	    for (int x2=x1*unit;x2<x2_limit;x2++) {
	      s2TotalCount++;
	      int rOffset=zyxOffset + yxOffset + x2;
	      bool ot=false;
	      for (int c=0;c<c_dim;c++) {
		if (rArr[c][rOffset] > cThreshold) {
		  ot=true;
		}
	      }
	      if (ot) {
		overThresholdCount++;
	      }
	    }
	  }
	}

	if (overThresholdCount>0) {
	  s2nonZeroCount += overThresholdCount;
	  char oneC=1;
	  oneC <<= bitOffset;
	  char test=0;
	  test = oneC & targetMask[bytePosition];
	  if (test==0) {
	    s1NonZeroCount++;
	    targetMask[bytePosition] = targetMask[bytePosition] | oneC;
	    if (DEBUG_FLAG) qDebug() << "Adding 1st stage entry for x1=" << x1 << " y1=" << y1 << " z1=" << z1;
	  }
	}

      }
    }
  }
  delete [] rArr;
  sbmResults[0] = bytePosition+1;
  sbmResults[1] = s2TotalCount;
  sbmResults[2] = s2nonZeroCount;
  sbmResults[3] = s1NonZeroCount;
  return sbmResults;
}

bool VolumeIndex::initParamsFromIndexSpecification()
{
  if (indexSpecification==0L) {
    return false;
  }
  X_SIZE=indexSpecification->pixel_x;
  Y_SIZE=indexSpecification->pixel_y;
  Z_SIZE=indexSpecification->pixel_z;
  UNIT=indexSpecification->index_unit;
  BITS=indexSpecification->bit_depth;
  X1_SIZE=divideDimensionByUnit(X_SIZE, UNIT);
  Y1_SIZE=divideDimensionByUnit(Y_SIZE, UNIT);
  Z1_SIZE=divideDimensionByUnit(Z_SIZE, UNIT);
  return true;
}

bool VolumeIndex::initParamsFromSampleSpecification()
{
  if (sampleSpecification==0L) {
    return false;
  }
  SAMPLE_ID=sampleSpecification->sampleId;
  OWNER=sampleSpecification->owner;
  return true;
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

bool VolumeIndex::createSecondStageIndexForSampleFile()
{
  if (DEBUG_FLAG) qDebug() << "createSecondStageIndexForSample() start";

  if (sampleConsolidatedSignalImage==0L) {
    qDebug() << "sampleConsolidatedSignalImage must be populated before createSecondStageIndex";
    return false;
  }

  // Estimate buffer size
  int secondStageBufferSizePerMask = 1; // entry code

  // FIRST WE DO THE SAMPLE

  if (DEBUG_FLAG) qDebug() << "Creating index entry for sample from " << sampleSpecification->consolidatedSignalPath;
  if (!createSecondStageEntry(sampleConsolidatedSignalImage, sampleSpecification->sampleId, sampleSpecification->sampleId, false)) {
    qDebug() << "Error generating second-stage index";
    return false;
  }

  if (!writeSampleIndexUpdate()) {
    qDebug() << "Error writing sample index";
    return false;
  }

  clearSecondStageData();

  // THEN WE DO THE FRAGMENTS

  for (int i=0;i<sampleSpecification->fragmentList.size();i++) {

    FragmentSpecification* fs = sampleSpecification->fragmentList[i];
    QString imagePath=fs->maskPath;
    MaskChan maskChan;
    QStringList fragmentPathList;
    fragmentPathList.append(imagePath);
    if (DEBUG_FLAG) qDebug() << "Loading fragment path=" << imagePath;
    My4DImage* fragmentImage = maskChan.createImageFromMaskFiles(fragmentPathList);
    if (DEBUG_FLAG) qDebug() << "Creating index entry for fragment from " << fs->maskPath;
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

  return true;
}

char* VolumeIndex::getAddressOfFirstStageMaskAtCoordinate(int x, int y, int z, int ownerLength)
{
  int unitOffset = z*Y1_SIZE*X1_SIZE + y*X1_SIZE + x;
  int byteOffset = unitOffset/8;
  int headerFirstPartSize = sizeof(int) * 5 + // x,y,z, unit, bits
    sizeof(long) + // sampleId
    sizeof(int) + // length of owner
    sizeof(char) * ownerLength +
    sizeof(int); // non-zero voxel count for this 1st-stage entry
  char* dataStart = firstStageIndex + headerFirstPartSize;
  char* p=dataStart+byteOffset;
  return p;
}

bool VolumeIndex::validatePositiveFirstStageEntry(int x, int y, int z, int ownerLength)
{
  int unitOffset = z*Y1_SIZE*X1_SIZE + y*X1_SIZE + x;
  int byteOffset = unitOffset/8;
  char* p=getAddressOfFirstStageMaskAtCoordinate(x, y, z, ownerLength);
  char v=*p;
  char bitOffset = unitOffset - 8*byteOffset;
  v >>= bitOffset;
  char bitMask=1;
  char v2 = v & bitMask;
  if (v2>0) {
    // already set properly
    return true;
  } else {
    bitMask <<= bitOffset;
    *p = *p | bitMask;
    return false;
  }
}

bool VolumeIndex::createSecondStageEntry(My4DImage* image, long fragmentId, long sampleId, bool updateFirstStage)
{
  if (DEBUG_FLAG) qDebug() << "createSecondStageEntry() fragmentId=" << fragmentId;
  int positionsPerByte=8;
  int nonZeroCount=0;

  if (BITS==2) {
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

  QString owner=sampleSpecification->owner;
  int ownerLength=owner.length();

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
	      if (BITS==2) {
		bitOffset=2*(sbCount % positionsPerByte);
	      }

	      if (bitOffset==0) {
		UNIT_BUFFER[bytePosition]=0; // clear as we move forward
	      }

	      int rOffset=zyxOffset + yxOffset + x2;
	      char v=-1;
	      v3d_uint8 maxV=0;
	      for (int c=0;c<c_dim;c++) {
		v3d_uint8 rV=rArr[c][rOffset];
		if (rV > maxV) {
		  maxV=rV;
		}
	      }
	      for (int t=tN-1;t>-1;t--) {
		//		if (DEBUG_FLAG) qDebug() << "t=" << t << " tN=" << tN << " tArr=" << tArr[t];
		if (maxV>=tArr[t]) {
		  v=t;
		  //		  if (DEBUG_FLAG) qDebug() << "setting v=t with v=" << v;
		  break;
		}
	      }
	      if (v>-1) {
		overFirstStageThreshold=true;
	      }
	      // Now, v should be the correct level for this position
	      if (BITS==1) {
		if (v==0) {
		  char oneC=1;
		  nonZeroCount++;
		  oneC <<= bitOffset;
		  UNIT_BUFFER[bytePosition] = UNIT_BUFFER[bytePosition] | oneC;
		}
	      } else if (BITS==2) {
		char cv=v+1;
		if (cv>0) {
		  nonZeroCount++;
		  cv <<= bitOffset;
		  UNIT_BUFFER[bytePosition] = UNIT_BUFFER[bytePosition] | cv;
		}
	      }
	    }
	  }
	}

	if (updateFirstStage && overFirstStageThreshold) {
	  if (!validatePositiveFirstStageEntry(x1, y1, z1, ownerLength)) {
	    qDebug() << "Correcting first-stage entry at x=" << x1 << " y=" << y1 << " z=" << z1;
	    firstStageCorrectionCount++;
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

  if (DEBUG_FLAG) qDebug() << "nonZeroCount=" << nonZeroCount;

  secondStageVoxelCount=nonZeroCount;

  return true;
}

bool VolumeIndex::writeSampleIndexHeaderAndFirstStage()
{
  QByteArray ba = sampleIndexFilepath.toUtf8();
  const char* filepath = ba.constData();

  fid=fopen(filepath, "wb");
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

  // These are already in the first stage index bytes
  //  fwrite(&X_SIZE, sizeof(int), 1, fid);
  //  fwrite(&Y_SIZE, sizeof(int), 1, fid);
  //  fwrite(&Z_SIZE, sizeof(int), 1, fid);
  //  fwrite(&UNIT, sizeof(int), 1, fid);
  //  fwrite(&BITS, sizeof(int), 1, fid);

  if (DEBUG_FLAG) qDebug() << "Writing " << firstStageIndexBytes << " bytes for first stage index";
  int l=fwrite(firstStageIndex, firstStageIndexBytes, 1, fid);
  if (l!=1) {
    qDebug() << "fwrite failed, expected " << firstStageIndexBytes << " but returned " << l;
    return false;
  }

  for (int i=0;i<firstStageIndexBytes;i++) {
    int it=firstStageIndex[i];
    if (DEBUG_FLAG) qDebug() << i << " = " << it;
  }

  return true;
}

bool VolumeIndex::writeSampleIndexUpdate()
{
  if (DEBUG_FLAG) qDebug() << "writeSampleIndexUpdate() - secondStageIndex size=" << secondStageIndex.size();
  for (int i=0;i<secondStageIndex.size();i++) {
    char* fp = secondStageIndex[i];
    int fpSize = secondStageIndexLength[i];
    fwrite(&ENTRY_CODE, sizeof(char), 1, fid);
    int voxelCountOffset = 3*sizeof(int) + 2*sizeof(long);
    int* vc = (int*)(fp + voxelCountOffset);
    *vc = secondStageVoxelCount;
    if (DEBUG_FLAG) qDebug() << "Writing " << fpSize << " bytes for second stage index entry";
    int l = fwrite(fp, fpSize, 1, fid);
    if (l!=1) {
      qDebug() << "fwrite failed";
      return false;
    }
  }
  fflush(fid);
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
  What we want to do here is read in the sample index file, appending each entry to the appropriate binary index
  in the master tree.
 */

 bool VolumeIndex::addSampleToConsolidatedIndex()
 {
   if (DEBUG_FLAG) qDebug() << "addSampleToConsolidatedIndex() - starting parameter validation";
   if (indexSpecificationFilepath.isNull()) {
    qDebug() << "indexSpecificationFilepath not set";
    return false;
  }
  if (!readIndexSpecificationFile()) {
    qDebug() << "Could not read file " << indexSpecificationFilepath;
    return false;
  }
  if (!initParamsFromIndexSpecification()) {
    qDebug() << "Error initializing params from index specification";
    return false;
  }
  if (sampleIndexFilepath.isNull() || sampleIndexFilepath.length()==0) {
    qDebug() << "sampleIndexFilepath must be set";
    return false;
  }
  if (DEBUG_FLAG) qDebug() << "addSampleToConsolidatedIndex() - calling processSampleIndexToMainIndex()";
  if (!processSampleIndexToMainIndex()) {
    qDebug() << "Error processing sample Index for inclusion in main index";
    return false;
  }
   return true;
 }

bool VolumeIndex::processSampleIndexToMainIndex()
{
  char* ownerBuffer = 0L;
  if (DEBUG_FLAG) qDebug() << "Opening sampleIndexFilepath=" << sampleIndexFilepath;

  QByteArray ba = sampleIndexFilepath.toUtf8();
  const char* filepath = ba.constData();

  fid=fopen(filepath, "rb");
  if (!fid) {
    qDebug() << "Error opening file " << sampleIndexFilepath << " to read";
    return false;
  }
  int si_X_SIZE=0;
  int si_Y_SIZE=0;
  int si_Z_SIZE=0;
  int si_UNIT=0;
  int si_BITS=0;

  if (DEBUG_FLAG) qDebug() << "Reading dimensional info";

  fread(&si_X_SIZE, 1, sizeof(int), fid);
  fread(&si_Y_SIZE, 1, sizeof(int), fid);
  fread(&si_Z_SIZE, 1, sizeof(int), fid);
  fread(&si_UNIT, 1, sizeof(int), fid);
  fread(&si_BITS, 1, sizeof(int), fid);

  if (DEBUG_FLAG) qDebug() << "Validating dimensional info";

  if (si_X_SIZE != X_SIZE) {
    qDebug() << "X_SIZE does not match between sampleIndex and main index";
    return false;
  }
  if (si_Y_SIZE != Y_SIZE) {
    qDebug() << "Y_SIZE does not match between sampleIndex and main index";
    return false;
  }
  if (si_Z_SIZE != Z_SIZE) {
    qDebug() << "Z_SIZE does not match between sampleIndex and main index";
    return false;
  }
  if (si_UNIT != UNIT) {
    qDebug() << "UNIT does not match between sampleIndex and main index";
    return false;
  }
  if (si_BITS != BITS) {
    qDebug() << "BITS does not match between sampleIndex and main index";
    return false;
  }

  fread(&SAMPLE_ID, 1, sizeof(long), fid);
  int ownerLength=0;
  fread(&ownerLength, 1, sizeof(int), fid);
  ownerBuffer = new char[ownerLength+1];
  int obLength=fread(ownerBuffer, sizeof(char), ownerLength, fid);
  ownerBuffer[ownerLength]='\0';
  OWNER = QString::fromLocal8Bit(ownerBuffer);

  int X1_SIZE = divideDimensionByUnit(X_SIZE, UNIT);
  int Y1_SIZE = divideDimensionByUnit(Y_SIZE, UNIT);
  int Z1_SIZE = divideDimensionByUnit(Z_SIZE, UNIT);

  int s1TotalVoxels=X1_SIZE*Y1_SIZE*Z1_SIZE;

  // First stage is binary, regardless of 2nd-stage
  firstStageIndexBytes=s1TotalVoxels/8;
  if (s1TotalVoxels!=8*firstStageIndexBytes) {
    firstStageIndexBytes++;
  }

  int s1NonZeroCount=0;
  fread(&s1NonZeroCount, 1, sizeof(int), fid);

  firstStageIndex = new char[firstStageIndexBytes];

  int s1ReadCount=fread(firstStageIndex, sizeof(char), firstStageIndexBytes, fid);
  if (s1ReadCount!=firstStageIndexBytes) {
    qDebug() << "Error with fread for stage 1 data, firstStageIndexBytes=" << firstStageIndexBytes << " but s1ReadCount=" << s1ReadCount;
    return false;
  }

  // Before we add any data to the main index (either primary or secondary), we want to validate all entries.

  if (DEBUG_FLAG) qDebug() << "Beginning entry validation";

  long fileStartPositionForSecondStage=ftell(fid);

  char entryCodeTest=0;
  size_t entryCodeReadSize=0;

  char* s2Buffer = new char[s1TotalVoxels]; // should be plenty of space

  while(!feof(fid)) {

    entryCodeReadSize=fread(&entryCodeTest, sizeof(char), 1, fid);

    if (feof(fid)) {
      break;
    }

    if (entryCodeTest!=ENTRY_CODE) {
      qDebug() << "Unexpectedly did not receive entry code char marking beginning of next 2nd-stage entry";
      fclose(fid);
      return false;
    }
    int s2_xstart=0;
    int s2_ystart=0;
    int s2_zstart=0;
    long s2_fragment_id=0L;
    long s2_sample_id=0L;
    int s2_nonzero=0;

    fread(&s2_xstart,1,sizeof(int),fid);
    fread(&s2_ystart,1,sizeof(int),fid);
    fread(&s2_zstart,1,sizeof(int),fid);
    fread(&s2_fragment_id,1,sizeof(long),fid);
    fread(&s2_sample_id,1,sizeof(long),fid);
    fread(&s2_nonzero,1,sizeof(int),fid);

    // validate
    if (s2_xstart < 0 || s2_xstart > X_SIZE) {
      qDebug() << "Could not validate s2 x index value=" << s2_xstart;
      fclose(fid);
      return false;
    }

    if (s2_ystart < 0 || s2_ystart > Y_SIZE) {
      qDebug() << "Could not validate s2 y index value=" << s2_ystart;
      fclose(fid);
      return false;
    }

    if (s2_zstart < 0 || s2_zstart > Z_SIZE) {
      qDebug() << "Could not validate s2 z index value=" << s2_zstart;
      fclose(fid);
      return false;
    }

    if (s2_sample_id != SAMPLE_ID) {
      qDebug() << "s2 sample id does not match SAMPLE_ID=" << SAMPLE_ID;
      fclose(fid);
      return false;
    }

    // read sec data based on position
    int xlength = X_SIZE-s2_xstart;
    if (xlength > UNIT) {
      xlength=UNIT;
    }

    int ylength = Y_SIZE-s2_ystart;
    if (ylength > UNIT) {
      ylength=UNIT;
    }

    int zlength = Z_SIZE-s2_zstart;
    if (zlength > UNIT) {
      zlength=UNIT;
    }

    // reduce dimension to compression size
    int entrySize=0;
    int voxelCount=xlength*ylength*zlength;
    entrySize=getStage2DataByteCount(voxelCount);

    if (entrySize>s1TotalVoxels) {
      qDebug() << "s2 buffer size is only " << s1TotalVoxels << " but s2 entry size=" << entrySize;
      fclose(fid);
      return false;
    }

    int s2ReadSize=0;
    if ( (s2ReadSize=fread(s2Buffer,sizeof(char), entrySize, fid))!=entrySize) {
      qDebug() << "Error reading s2 entry of size=" << entrySize;
      fclose(fid);
      return false;
    }
  }

  // Write First Stage Main Index
  mainIndexFid=openPrimaryIndex("ab");
  if (!mainIndexFid) {
    qDebug() << "Error opening main index";
    return false;
  }
  fwrite(&SAMPLE_ID, sizeof(long), 1, mainIndexFid);
  fwrite(&ownerLength, sizeof(int), 1, mainIndexFid);
  fwrite(ownerBuffer, sizeof(char), ownerLength, mainIndexFid);
  fwrite(&s1NonZeroCount, sizeof(int), 1, mainIndexFid);
  int s1WriteCount=fwrite(firstStageIndex, sizeof(char), firstStageIndexBytes, mainIndexFid);
  if (DEBUG_FLAG) qDebug() << "First stage index bytes:";
  for (int i=0;i<firstStageIndexBytes;i++) {
    int it=firstStageIndex[i];
    if (DEBUG_FLAG) qDebug() << i << " = " << it;
  }
  if (s1WriteCount!=firstStageIndexBytes) {
    qDebug() << "s1WriteCount does not match firstStageIndexBytes - error writing first stage index";
    fclose(mainIndexFid);
    return false;
  }
  fflush(mainIndexFid);
  fclose(mainIndexFid);
  mainIndexFid=0L;

  // Now we are post-validation, and can re-read the data while writing to the indices...

  if (DEBUG_FLAG) qDebug() << "Beginning writing of second stage index";

  fseek(fid, fileStartPositionForSecondStage, SEEK_SET);

  while ( (entryCodeReadSize=fread(&entryCodeTest, sizeof(char), 1, fid))==sizeof(char) ) {

    if (entryCodeTest!=ENTRY_CODE) {
      qDebug() << "Unexpectedly did not receive entry code char marking beginning of next 2nd-stage entry";
      fclose(fid);
      return false;
    }
    int s2_xstart=0;
    int s2_ystart=0;
    int s2_zstart=0;
    long s2_fragment_id=0L;
    long s2_sample_id=0L;
    int s2_nonzero=0;

    fread(&s2_xstart,1,sizeof(int),fid);
    fread(&s2_ystart,1,sizeof(int),fid);
    fread(&s2_zstart,1,sizeof(int),fid);
    fread(&s2_fragment_id,1,sizeof(long),fid);
    fread(&s2_sample_id,1,sizeof(long),fid);
    fread(&s2_nonzero,1,sizeof(int),fid);

    if (DEBUG_FLAG) qDebug() << "Processing fragment_id=" << s2_fragment_id << " x=" << s2_xstart << " y=" << s2_ystart << " z=" << s2_zstart;

    // read sec data based on position
    int xlength = X_SIZE-s2_xstart;
    if (xlength > UNIT) {
      xlength=UNIT;
    }

    int ylength = Y_SIZE-s2_ystart;
    if (ylength > UNIT) {
      ylength=UNIT;
    }

    int zlength = Z_SIZE-s2_zstart;
    if (zlength > UNIT) {
      zlength=UNIT;
    }

    // reduce dimension to compression size
    int voxelCount=xlength*ylength*zlength;
    int entrySize=getStage2DataByteCount(voxelCount);

    if (entrySize>s1TotalVoxels) {
      qDebug() << "s2 buffer size is only " << s1TotalVoxels << " but s2 entry size=" << entrySize;
      fclose(fid);
      return false;
    }

    // clear buffer
    for (int i=0;i<s1TotalVoxels;i++) {
      s2Buffer[i]=0;
    }

    int s2ReadSize=0;
    if ( (s2ReadSize=fread(&s2Buffer[0], sizeof(char), entrySize, fid))!=entrySize) {
      qDebug() << "Error reading s2 entry of size=" << entrySize;
      fclose(fid);
      return false;
    }

    // Now we can flush this data to the main secondary index
    int s1_x=s2_xstart/UNIT;
    int s1_y=s2_ystart/UNIT;
    int s1_z=s2_zstart/UNIT;

    if (DEBUG_FLAG) qDebug() << "Using 1st stage x=" << s1_x << " y=" << s1_y << " z=" << s1_z;

    mainIndexFid=openSecondaryIndexToAppend(s1_x, s1_y, s1_z);
    fwrite(&ENTRY_CODE, sizeof(char), 1, mainIndexFid);
    fwrite(&s2_xstart, sizeof(int), 1, mainIndexFid);
    fwrite(&s2_ystart, sizeof(int), 1, mainIndexFid);
    fwrite(&s2_zstart, sizeof(int), 1, mainIndexFid);
    fwrite(&s2_fragment_id, sizeof(long), 1, mainIndexFid);
    fwrite(&s2_sample_id, sizeof(long), 1, mainIndexFid);
    fwrite(&s2_nonzero, sizeof(int), 1, mainIndexFid);
    fwrite(s2Buffer, sizeof(char), s2ReadSize, mainIndexFid);
    fflush(mainIndexFid);
    fclose(mainIndexFid);
    mainIndexFid=0L;
  }

  delete [] ownerBuffer;

  delete [] s2Buffer;

  if (DEBUG_FLAG) qDebug() << "Done writing stage 2 data";

  return true;
}

FILE* VolumeIndex::openPrimaryIndex(const char* filestring)
{
  if (mainIndexFid!=0L) {
    fclose(mainIndexFid);
  }
  QString primaryIndexPath=indexSpecification->rootPath;
  primaryIndexPath.append("/");
  primaryIndexPath.append(PRIMARY_INDEX_FILENAME);

  QByteArray ba = primaryIndexPath.toUtf8();
  const char* indexpath = ba.constData();

  mainIndexFid=fopen(indexpath, filestring);
  if (!mainIndexFid) {
    qDebug() << "Could not open primary index file=" << primaryIndexPath;
  }
  return mainIndexFid;
}

FILE* VolumeIndex::openSecondaryIndexToAppend(int x, int y, int z)
{
  if (mainIndexFid!=0L) {
    fclose(mainIndexFid);
  }
  QString secondaryDir=getSecondaryIndexDir(x,y,z);
  QString secondaryPath=getSecondaryIndexFilepath(x, y, z);
  if (!QDir().mkpath(secondaryDir)) {
    qDebug() << "Could not create secondary dir=" << secondaryDir;
    mainIndexFid=0L;
    return mainIndexFid;
  }
  QByteArray ba = secondaryPath.toUtf8();
  const char* path = ba.constData();

  mainIndexFid=fopen(path, "ab");
  if (!mainIndexFid) {
    qDebug() << "Could not open secondary index file=" << secondaryPath;
  }
  return mainIndexFid;
}

FILE* VolumeIndex::openSecondaryIndexToRead(int x, int y, int z)
{
  QString secondaryPath=getSecondaryIndexFilepath(x, y, z);

  QByteArray ba = secondaryPath.toUtf8();
  const char* path = ba.constData();

  FILE* fid=fopen(path, "rb");
  if (!fid) {
    qDebug() << "Could not open secondary index file=" << secondaryPath << " to read";
  }
  return fid;
}


QString VolumeIndex::getSecondaryIndexDir(int x, int y, int z)
{
  QString secondaryPath=indexSpecification->rootPath;
  secondaryPath.append("/");
  secondaryPath.append(QString::number(x));
  secondaryPath.append("/");
  secondaryPath.append(QString::number(y));
  secondaryPath.append("/");
  secondaryPath.append(QString::number(z));
  return secondaryPath;
}


QString VolumeIndex::getSecondaryIndexFilepath(int x, int y, int z)
{
  QString secondaryDir=getSecondaryIndexDir(x, y, z);
  secondaryDir.append("/");
  secondaryDir.append(SECONDARY_INDEX_FILENAME);
  return secondaryDir;
}

bool VolumeIndex::loadQueryImage()
{
  // First, load the original image
  My4DImage* originalQueryImage=0L;
  if (queryFilepath.endsWith(".mask")) {
    MaskChan maskChan;
    QStringList pathList;
    pathList.append(queryFilepath);
    if (DEBUG_FLAG) qDebug() << "Loading mask query file=" << queryFilepath;
    originalQueryImage=maskChan.createImageFromMaskFiles(pathList);
  } else {
    ImageLoaderBasic loader;
    if (queryFilepath.isNull() || queryFilepath.length()<1) {
      qDebug() << "queryFilepath is not set";
      return false;
    }
    originalQueryImage=new My4DImage();
    if (!loader.loadImage(originalQueryImage, queryFilepath.toUtf8().constData())) {
      qDebug() << "Could not load query file=" << queryFilepath;
      return false;
    }
  }
  doVoxelCountReport(originalQueryImage);

  // Next, convert query to 1-channel and binarize
  const int x_dim=originalQueryImage->getXDim();
  const int y_dim=originalQueryImage->getYDim();
  const int z_dim=originalQueryImage->getZDim();
  const int c_dim=originalQueryImage->getCDim();

  if (x_dim!=X_SIZE ||
      y_dim!=Y_SIZE ||
      z_dim!=Z_SIZE) {
    qDebug() << "query dimensions do not match index dimensions";
    return false;
  }

  queryImage=new My4DImage();
  queryImage->loadImage(x_dim, y_dim, z_dim, 1 /* channel */, V3D_UINT8);

  v3d_uint8** rArr=new v3d_uint8*[c_dim];

  for (int i=0;i<c_dim;i++) {
    rArr[i]=originalQueryImage->getRawDataAtChannel(i);
  }

  v3d_uint8* qArr=queryImage->getRawDataAtChannel(0);

  for (int z1=0;z1<Z_SIZE;z1++) {
    int offsetZ=z1*Y_SIZE*X_SIZE;
    for (int y1=0;y1<Y_SIZE;y1++) {
      int offsetY=y1*X_SIZE;
      for (int x1=0;x1<X_SIZE;x1++) {
	int offset=offsetZ + offsetY + x1;
	char maxV=0;
	for (int c=0;c<c_dim;c++) {
	  char v=rArr[c][offset];
	  if (v>maxV) {
	    maxV=v;
	  }
	}
	if (maxV>=queryThreshold) {
	  qArr[offset]=255;
	} else {
	  qArr[offset]=0;
	}
      }
    }
  }

  delete originalQueryImage;
  return true;
}

void VolumeIndex::dilateQueryMask()
{
  My4DImage* queryWorkspace=new My4DImage();
  queryWorkspace->loadImage(X_SIZE, Y_SIZE, Z_SIZE, 1 /* channel */, V3D_UINT8);
  DilationErosion de;
  unsigned char**** qd = (unsigned char****)queryImage->getData();
  unsigned char**** qdStart=qd;
  unsigned char**** qw = (unsigned char****)queryWorkspace->getData();
  unsigned char* qwc = queryWorkspace->getRawDataAtChannel(0);
  int voxelCount=X_SIZE*Y_SIZE*Z_SIZE;
  for (int i=0;i<voxelCount;i++) {
    qwc[i]=0;
  }
  for (int d=0;d<maskDilation;d++) {
    de.dilateOrErode(DilationErosion::TYPE_DILATE, X_SIZE, Y_SIZE, Z_SIZE, qd[0], qw[0], 2, 24);
    qd=qw;
    qw=qd;
  }
  if (qd!=qdStart) {
    delete queryImage;
    queryImage=queryWorkspace;
  } else {
    delete queryWorkspace;
  }
}

/* This method assumes the query is binary 0/255 and the subject is either 1-bit or 2-bit via threshold info from index spec, and
   has been preprocessed as values 0,1 or 0,1,2,3 */
MaskScore* VolumeIndex::computeSubvolumeScore(char* query, char* subject, int length)
{
  MaskScore* ms = new MaskScore();

  ms->score=0.0;
  ms->zeroCount=0;
  ms->nonzeroCount=0;
  ms->zeroScore=0;
  ms->nonzeroScore=0;
  ms->fragmentNonzeroCount=0;

  if (indexSpecification->bit_depth==1) {
    for (int i=0;i<length;i++) {
      if (subject[i]!=0) {
	ms->fragmentNonzeroCount++;
      }
      if (query[i]==0) {
	ms->zeroCount++;
	if (subject[i]==0) {
	  // do nothing since match
	} else {
	  // Assume 1
	  ms->zeroScore++;
	}
      } else {
	ms->nonzeroCount++;
	// Assume non-zero
	if (subject[i]==0) {
	  ms->nonzeroScore++;
	} else {
	  // Assume 1
	  // do nothing since match
	}
      }
    }

  } else { // Assume bit depth==2
    for (int i=0;i<length;i++) {
      if (query[i]==0) {
	ms->zeroCount++;
	ms->zeroScore += subject[i]; // should be 0,1,2,3, which is difference
      } else {
	ms->nonzeroCount++;
	ms->nonzeroScore += (3-subject[i]);
      }
    }

  }

  ms->score = 0.0; // should be computed separately
  return ms;
}

void VolumeIndex::computeMaskScore(MaskScore* maskScore)
{
  double backgroundScore=0.0;
  double foregroundScore=0.0;
  if (maskScore->zeroCount>0) {
    backgroundScore = backgroundWeight * (maskScore->zeroScore / (maskScore->zeroCount * 1.0));
  }
  if (maskScore->nonzeroCount>0) {
    foregroundScore = maskScore->nonzeroScore / (maskScore->nonzeroCount * 1.0);
  }
  maskScore->score = backgroundScore + foregroundScore;
}

// This should only be called once
bool VolumeIndex::computeBlankSubjectScore()
{
  if (blankSubjectScore!=0L) {
    qDebug() << "computeBlankSubjectScore() should only be called once";
    return false;
  }
  blankSubjectScore=new MaskScore***[Z1_SIZE];
  for (int b1=0;b1<Z1_SIZE;b1++) {
    blankSubjectScore[b1]=new MaskScore**[Y1_SIZE];
    for (int b2=0;b2<Y1_SIZE;b2++) {
      blankSubjectScore[b1][b2]=new MaskScore*[X1_SIZE];
    }
  }
  const int MAX_SUBVOLUME_SIZE=UNIT*UNIT*UNIT;
  char* queryData=new char[MAX_SUBVOLUME_SIZE]; // max size for subvolume
  char* subjectData=new char[MAX_SUBVOLUME_SIZE];
  for (int i=0;i<MAX_SUBVOLUME_SIZE;i++) {
    queryData[i]=0;
    subjectData[i]=0;
  }
  int qCount=1;
  char qtArr[1];
  qtArr[0]=1;
  for (int z1=0;z1<Z1_SIZE;z1++) {
    for (int y1=0;y1<Y1_SIZE;y1++) {
      for (int x1=0;x1<X1_SIZE;x1++) {
	int subvolumeSize=getImageSubvolumeDataByStage1Coordinates(queryImage, queryData, x1, y1, z1, qCount, qtArr);
	// We now have query data from the subvolume as an array of 0,1 and also subject data as all zeros
	MaskScore* ms=computeSubvolumeScore(queryData, subjectData, subvolumeSize);
	ms->x=x1;
	ms->y=y1;
	ms->z=z1;
	blankSubjectScore[z1][y1][x1]=ms;
      }
    }
  }
  delete [] queryData;
  delete [] subjectData;
}

/* this extracts subvolume data from an image, and uses the threshold array to consolidate the information into a single array ordered by level */
/* it returns values as 0,1,2,3 for 2-bit data, and 0,255 for 1-bit */
int VolumeIndex::getImageSubvolumeDataByStage1Coordinates(My4DImage* image, char* data, int x1, int y1, int z1, int tCount, char* tArr)
{
  v3d_uint8** rArr=new v3d_uint8*[image->getCDim()];

  int nonzeroCount=0;

  const int x_dim=image->getXDim();
  const int y_dim=image->getYDim();
  const int z_dim=image->getZDim();
  const int c_dim=image->getCDim();

  int channelLength=z_dim*y_dim*x_dim;

  for (int i=0;i<image->getCDim();i++) {
    rArr[i]=image->getRawDataAtChannel(i);
  }

  // Inner Loop
  int z2_limit=(z1+1)*UNIT;
  if (z2_limit>z_dim) {
    z2_limit=z_dim;
  }

  int y2_limit=(y1+1)*UNIT;
  if (y2_limit>y_dim) {
    y2_limit=y_dim;
  }

  int x2_limit=(x1+1)*UNIT;
  if (x2_limit>x_dim) {
    x2_limit=x_dim;
  }

  int dataPosition=-1;

  for (int z2=z1*UNIT;z2<z2_limit;z2++) {
    int zyxOffset=z2*x_dim*y_dim;
    for (int y2=y1*UNIT;y2<y2_limit;y2++) {
      int yxOffset=y2*x_dim;
      v3d_uint8 maxV=0;
      for (int x2=x1*UNIT;x2<x2_limit;x2++) {
	int rOffset=zyxOffset + yxOffset + x2;
	dataPosition++;

	maxV=0;

	for (int c=0;c<c_dim;c++) {
	  v3d_uint8 v=rArr[c][rOffset];
	  if (v > maxV) {
	    maxV=v;
	  }
	}

	int t=0;
	for (;t<tCount;t++) {
	  if (maxV<tArr[t]) {
	    break;
	  }
	}

	data[dataPosition]=t;

	if (t!=0) {
	  nonzeroCount++;
	}

      }
    }
  }

  delete [] rArr;

  dataPosition++;

  if (BITS==1) {
    for (int i=0;i<dataPosition;i++) {
      if (data[i]>0) {
	data[i]=255;
      }
    }
  } else {
    // assume BITS==2, then leave as 0,1,2,3
  }

  if (DEBUG_FLAG) qDebug() << "getImageSubvolumeDataByStage1Coordinates()  x1=" << x1 << " y1=" << y1 << " z1=" << z1 <<" dataPosition=" << dataPosition << " nonzeroCount=" << nonzeroCount;

  return dataPosition;
}

bool VolumeIndex::computeSubjectScores()
{
  searchResultList.clear();

  // We want to iterate through the 1st-stage index, spawning threads that are individually responsible for each sample. These 1st-stage
  // threads will then visit each non-zero 2nd-stage index, and launch 2nd-stage threads which will add their results to the searchResultList.
  // After all are finished, this list will be sorted and the results reported.

  char* ownerBuffer=new char[2000];

  fid=openPrimaryIndex("rb");

  sampleFutureList.clear();

  int s1TotalVoxels=X1_SIZE*Y1_SIZE*Z1_SIZE;
  int firstStageIndexBytes=s1TotalVoxels/8;
  if (s1TotalVoxels!=8*firstStageIndexBytes) {
    firstStageIndexBytes++;
  }

  int sampleIndex=0;

  int debugCounter=0;

  while(!feof(fid)) {

    debugCounter++;

    if (debugCounter > 3) {
      break;
    }

    long sampleId;
    int ownerLength;
    int firstStageVoxelCount;
    fread(&sampleId, sizeof(long), 1, fid);
    if (feof(fid)) {
      break;
    }
    fread(&ownerLength, sizeof(int), 1, fid);
    if (ownerLength>2000) {
      qDebug() << "Exceeded maximum owner name length buffer";
      return false;
    }
    fread(ownerBuffer, sizeof(char), ownerLength, fid);
    ownerBuffer[ownerLength]='\0';
    QString owner = QString::fromLocal8Bit(ownerBuffer);
    SampleThread* st=new SampleThread();
    st->sampleId=sampleId;
    st->owner=owner;
    st->fid=0L;
    fread(&firstStageVoxelCount, sizeof(int), 1, fid);

    if (DEBUG_FLAG) qDebug() << "sampleId=" << sampleId << " owner=" << owner << " firstStageVoxelCount=" << firstStageVoxelCount;

    st->firstStageVoxelCount=firstStageVoxelCount;
    st->firstStageData=new char[firstStageIndexBytes];
    int readCount=fread(st->firstStageData, sizeof(char), firstStageIndexBytes, fid);

    if (DEBUG_FLAG) qDebug() << "read " << readCount << " of " << firstStageIndexBytes << " first stage data";

    for (int i=0;i<firstStageIndexBytes;i++) {
      int it=st->firstStageData[i];
      if (DEBUG_FLAG) qDebug() << i << " = " << it;
    }

    //    st=runSampleThread(st);
    //    delete [] st->firstStageData; // to clear space
    //    st->firstStageData=0L;

    sampleThreadList.append(st);
    qDebug() << "Adding runSampleThread to sampleFutureList for sampleId=" << sampleId;
    QFuture<SampleThread*> sf = QtConcurrent::run(this, &VolumeIndex::runSampleThread, st);
    sampleFutureList.append(sf);
  }

  if (DEBUG_FLAG) qDebug() << "Check6 - sampleFutureList size=" << sampleFutureList.size();

  // THIS NEEDS TO BE RE-IMPLEMENTED WITH SEPARATE SAMPLE/FRAGMENT THREAD POOLS TO PREVENT DEADLOCK
  int doneCount=0;
  while(1) {
    if (DEBUG_FLAG) qDebug() << "Starting sleep";
    SleepThread st;
    st.msleep(5000);
    if (DEBUG_FLAG) qDebug() << "Done with sleep";
    qDebug() << "sampleFutureList has " << sampleFutureList.size() << " entries";
    for (int i=0;i<sampleFutureList.size();i++) {
      if (DEBUG_FLAG) qDebug() << "Checking sampleThread " << i;
      QFuture<SampleThread*> sf=sampleFutureList.at(i);
      if (sf.isFinished()) {
	if (DEBUG_FLAG) qDebug() << "thread is finished";
	SampleThread* st = sf.result();
	addSampleResult(st);
	sampleFutureList.removeOne(sf);
  	doneCount++;
	qDebug() << "Finished " << doneCount << " runSampleThreads";
      } else {
	if (DEBUG_FLAG) qDebug() << "thread is not finished";
      }
    }
    if (ERROR_FLAG) {
      if (DEBUG_FLAG) qDebug() << "Error during execution of SampleThreads";
      return false;
    }
    int stillActive=sampleFutureList.size();
    if (stillActive==0) {
      break;
    } else {
      if (DEBUG_FLAG) qDebug() << "Waiting on " << stillActive << " sample search threads";
    }
  }

  if (DEBUG_FLAG) qDebug() << "Finished with runSampleThread manager loop - searchResultList has " << searchResultList.size() << " entries";

  // Now sort the results
  qSort(searchResultList.begin(), searchResultList.end(), ScoreSort());

  // Cleanup
  fclose(fid);
  fid=0L;
  delete [] ownerBuffer;

  return true;
}

void VolumeIndex::addSampleResult(SampleThread* st)
{
  long sampleId=st->sampleId;
  QList<long> keyList=st->fragmentScoreMap.keys();
  for (int i=0;i<keyList.size();i++) {
    long fragmentId=keyList[i];
    MaskScore* wholeMaskScore=new MaskScore();
    SubjectScore* ss=new SubjectScore();
    ss->sampleId=st->sampleId;
    ss->fragmentId=fragmentId;
    ss->owner=st->owner;
    ss->firstStageVoxelCount=st->firstStageVoxelCount;
    QList<MaskScore*> maskScoreList=st->fragmentScoreMap[fragmentId];
    if (maskScoreList.size()>0) {
      for (int j=0;j<maskScoreList.size();j++) {
	MaskScore* partialMaskScore=maskScoreList[j];
	wholeMaskScore->append(partialMaskScore);
      }
    }
    for (int z1=0;z1<Z1_SIZE;z1++) {
      for (int y1=0;y1<Y1_SIZE;y1++) {
	for (int x1=0;x1<X1_SIZE;x1++) {
	  QString nonzeroSignature=QString::number(x1)+" "+QString::number(y1)+" "+QString::number(z1);
	  if (!fragmentStage1NonzeroSet[fragmentId].contains(nonzeroSignature)) {
	    wholeMaskScore->append(blankSubjectScore[z1][y1][x1]);
	  }
	}
      }
    }
    computeMaskScore(wholeMaskScore);
    ss->maskScore=wholeMaskScore;
    searchResultList.append(ss);
  }
}

// The sample thread is responsible for visiting each nonzero second-stage
// index. For each nonzero entry, it reads the corresponding 2nd-stage
// index, and looks for all entries associated with its sampleId.
// For each associated entry, it calcluates the MaskScore, and adds this
// to its collection.

// Once done, it iterates through all map entries, and for each one, it
// computes a global score for each fragment, which is added to the
// searchResultList.

SampleThread* VolumeIndex::runSampleThread(SampleThread* sampleThread)
{
  if (DEBUG_FLAG) qDebug() << "runSampleThread starting for id=" << sampleThread->sampleId;
  for (int z1=0;z1<Z1_SIZE;z1++) {
    for (int y1=0;y1<Y1_SIZE;y1++) {
      for (int x1=0;x1<X1_SIZE;x1++) {
	int x1_position=z1*Y1_SIZE*X1_SIZE + y1*X1_SIZE + x1;
	int byte_position=x1_position/8;
	int byteOffset = x1_position-8*byte_position;
	char v=sampleThread->firstStageData[byte_position];
	v >>= byteOffset;
	char one=1;
	if (v & one > 0) {
	  // This position has data - open 2nd-stage index
	  if (DEBUG_FLAG) qDebug() << "For sample " << sampleThread->sampleId << " opening secondary index at position x=" << x1 << " y=" << y1 << " z=" << z1;
	  sampleThread->fid=openSecondaryIndexToRead(x1, y1, z1);
	  if (!sampleThread->fid) {
	    QString secondaryIndexFilepath=getSecondaryIndexFilepath(x1, y1, z1);
	    qDebug() << "Could not open secondary index to read=" << secondaryIndexFilepath;
	    ERROR_FLAG=true;
	    return sampleThread;
	  }
	  int xlen=UNIT;
	  if (x1==(X1_SIZE-1)) {
	    xlen = X_SIZE - x1*UNIT;
	  }
	  int ylen=UNIT;
	  if (y1==(Y1_SIZE-1)) {
	    ylen = Y_SIZE - y1*UNIT;
	  }
	  int zlen=UNIT;
	  if (z1==(Z1_SIZE-1)) {
	    zlen = Z_SIZE - z1*UNIT;
	  }
	  int s2units=xlen*ylen*zlen;
	  int s2byteLength=getStage2DataByteCount(s2units);
	  char* secondStageData=new char[s2byteLength];
	  // Now iterate through index file and select qualifying entries
	  while (!feof(sampleThread->fid)) {
	    char entryCode=0;
	    fread(&entryCode, sizeof(char), 1, sampleThread->fid);
	    if (feof(sampleThread->fid)) {
	      break;
	    }
	    if (entryCode!=ENTRY_CODE) {
	      qDebug() << "Error: ENTRY_CODE does not match";
	      ERROR_FLAG=true;
	      return sampleThread;
	    } else {
	      if (DEBUG_FLAG) qDebug() << "Confirmed ENTRY_CODE is correct";
	    }
	    int xstart, ystart, zstart;
	    long fragmentId, sampleId;
	    int voxelCount;
	    fread(&xstart, sizeof(int), 1, sampleThread->fid);
	    fread(&ystart, sizeof(int), 1, sampleThread->fid);
	    fread(&zstart, sizeof(int), 1, sampleThread->fid);
	    if (!(xstart==x1*UNIT && ystart==y1*UNIT && zstart==z1*UNIT)) {
	      qDebug() << "2nd stage index x y z do not match index file";
	      ERROR_FLAG=true;
	      return sampleThread;
	    }
	    fread(&fragmentId, sizeof(long), 1, sampleThread->fid);
	    fread(&sampleId, sizeof(long), 1, sampleThread->fid);
	    fread(&voxelCount, sizeof(int), 1, sampleThread->fid);
	    fread(secondStageData, sizeof(char), s2byteLength, sampleThread->fid);
	    if (DEBUG_FLAG) qDebug() << "runSampleThread sampleId=" << sampleId << " fragmentId=" << fragmentId << " voxelCount=" << voxelCount << " minSubjectVoxels=" << minSubjectVoxels;
	    if (sampleId==sampleThread->sampleId && voxelCount >= minSubjectVoxels) {
	      if (DEBUG_FLAG) qDebug() << "Calling addMaskScoreForSecondStageEntry()";
	      addMaskScoreForSecondStageEntry(fragmentId, sampleThread, secondStageData, s2units, x1, y1, z1);
	    }
	  }
	  delete [] secondStageData;
	  if (sampleThread->fid!=0L) {
	    fclose(sampleThread->fid);
	    sampleThread->fid=0L;
	  }
	}
      }
    }
  }
  return sampleThread;
}

void VolumeIndex::addMaskScoreForSecondStageEntry(long fragmentId, SampleThread* sampleThread, char* secondStageData, int dataUnits, int x1, int y1, int z1)
{
  QString nonzeroSignature=QString::number(x1)+" "+QString::number(y1)+" "+QString::number(z1);
  fragmentStage1NonzeroSet[fragmentId].insert(nonzeroSignature);

  // First get query data ready
  int qCount=1;
  char qtArr[1];
  qtArr[0]=1;
  int maxS2Size=UNIT*UNIT*UNIT;
  char* queryData = new char[maxS2Size];
  int subvolumeSize=getImageSubvolumeDataByStage1Coordinates(queryImage, queryData, x1, y1, z1, qCount, qtArr);

  // Then get subject data ready
  int s2bytes=getStage2DataByteCount(dataUnits);
  char* subjectData = new char[maxS2Size];
  expandSubjectData(secondStageData, subjectData, dataUnits);

  // Then compute score
  MaskScore* ms=computeSubvolumeScore(queryData, subjectData, dataUnits);
  ms->x=x1;
  ms->y=y1;
  ms->z=z1;
  if (DEBUG_FLAG) qDebug() << "maskScore=" << ms->score << " x=" << ms->x << " y=" << ms->y << " z=" << ms->z << " zeroCount=" << ms->zeroCount << " nonzeroCount=" << ms->nonzeroCount << " zeroScore=" << ms->zeroScore << " nonzeroScore=" << ms->nonzeroScore;

  // Add to list
  if (DEBUG_FLAG) qDebug() << "sampleThread->fragmentScoreMap has " << sampleThread->fragmentScoreMap.size() << " entries, before addition";
  if (sampleThread->fragmentScoreMap.contains(fragmentId)) {
    if (DEBUG_FLAG) qDebug() << "appending additional score for fragment, which already has " << sampleThread->fragmentScoreMap[fragmentId].size() << " entries";
    sampleThread->fragmentScoreMap[fragmentId].append(ms);
  } else {
    if (DEBUG_FLAG) qDebug() << "Creating new MaskScore list for sampleId=" << sampleThread->sampleId << " fragmentId=" << fragmentId;
    QList<MaskScore*> fragmentMaskScoreList;
    fragmentMaskScoreList.append(ms);
    sampleThread->fragmentScoreMap[fragmentId]=fragmentMaskScoreList;
  }

  delete [] queryData;
  delete [] subjectData;
}

 void VolumeIndex::expandSubjectData(char* compressed, char* expanded, int units)
 {
   int bytePosition=0;
   char one=1;
   char three=3;
   if (BITS==1) {
     for (int i=0;i<units;i++) {
       bytePosition=i/8;
       char v=compressed[bytePosition];
       int offset=i-bytePosition*8;
       v >>= offset;
       if (one & v) {
	 expanded[i]=255;
       } else {
	 expanded[i]=0;
       }
     }
   } else { // BITS==2
     for (int i=0;i<units;i++) {
       bytePosition=i/4;
       char v=compressed[bytePosition];
       int offset=(i-bytePosition*4)*2;
       v >>= offset;
       char v2=v & three;
       expanded[i]=v2;
     }
   }
 }


int VolumeIndex::getStage2DataByteCount(int s2units)
{
  int s2bitLength=s2units; // assume 1-bit initially
  if (BITS==2) {
    s2bitLength*=2;
  }
  int s2byteLength = s2bitLength / 8;
  if (8*s2byteLength < s2bitLength) {
    s2byteLength++;
  }
  return s2byteLength;
}

// Not implemented in current version
FragmentThread* VolumeIndex::runFragmentThread(FragmentThread* fragmentThread)
{
  return 0L;
}

bool VolumeIndex::displaySearchResults()
{
  QTextStream output(stdout);
  if (DEBUG_FLAG) qDebug() << "Displaying results for " << searchResultList.size() << " hits";

  output << "===============================================================================\n";
  for (int i=0;i<searchResultList.size();i++) {
    SubjectScore* ss=searchResultList[i];
    MaskScore* ms=ss->maskScore;
    output << i << " " << ss->sampleId << " " << ss->fragmentId << " " << ss->owner << " " << ms->fragmentNonzeroCount << " " << ms->score << "\n";
  }
  // It looks like Sean means the return value to answer the question "did no errors occur?";
  // So I'm setting the value to "true", to avoid Windows compile error while he's away.
  return true;
}

void VolumeIndex::doVoxelCountReport(My4DImage* sourceImage)
{

  v3d_uint8** rArr=new v3d_uint8*[sourceImage->getCDim()];

  for (int i=0;i<sourceImage->getCDim();i++) {
    rArr[i]=sourceImage->getRawDataAtChannel(i);
  }

  const int x_dim=sourceImage->getXDim();
  const int y_dim=sourceImage->getYDim();
  const int z_dim=sourceImage->getZDim();
  const int c_dim=sourceImage->getCDim();

  int zeroCount=0;
  int nonzeroCount=0;

  for (int z1=0;z1<z_dim;z1++) {
    for (int y1=0;y1<y_dim;y1++) {
      for (int x1=0;x1<x_dim;x1++) {
	int rOffset=z1*y_dim*x_dim + y1*x_dim + x1;
	bool ot=false;
	for (int c=0;c<c_dim;c++) {
	  if (rArr[c][rOffset] > 0) {
	    ot=true;
	  }
	}
	if (ot) {
	  nonzeroCount++;
	} else {
	  zeroCount++;
	}
      }
    }
  }

  if (DEBUG_FLAG) qDebug() << "doVoxelCountReport() zeroCount=" << zeroCount << " nonzeroCount=" << nonzeroCount;

  delete [] rArr;
}


#ifndef VOLUMEINDEX_H
#define VOLUMEINDEX_H

#include <QString>
#include <QtCore>
#include <QDir>
#include "../../v3d/v3d_core.h"
#include "AnalysisTools.h"
#include "MaskChan.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////////////////

class IndexSpecification
{
 public:

  IndexSpecification() { 
    thresholdArray=0L;
  }

  QString space;

  double optical_x;
  double optical_y;
  double optical_z;

  int pixel_x;
  int pixel_y;
  int pixel_z;

  int index_unit;
  int bit_depth;
  int thresholdCount;
  int* thresholdArray;
  
  QString rootPath;

  QString toString() {

    QString q("\n");

    q.append("space=");
    q.append(space);
    q.append("\n");

    q.append("optical_x=");
    q.append(QString::number(optical_x));
    q.append("\n");

    q.append("optical_y=");
    q.append(QString::number(optical_y));
    q.append("\n");

    q.append("optical_z=");
    q.append(QString::number(optical_z));
    q.append("\n");

    q.append("pixel_x=");
    q.append(QString::number(pixel_x));
    q.append("\n");

    q.append("pixel_y=");
    q.append(QString::number(pixel_y));
    q.append("\n");

    q.append("pixel_z=");
    q.append(QString::number(pixel_z));
    q.append("\n");

    q.append("index_unit=");
    q.append(QString::number(index_unit));
    q.append("\n");

    q.append("bits=");
    q.append(QString::number(bit_depth));
    q.append("\n");

    if (thresholdArray!=0L) {
      for (int i=0;i<thresholdCount;i++) {
	int t=thresholdArray[i];
	q.append("threshold ");
	q.append(QString::number(i));
	q.append(" = ");
	q.append(QString::number(t));
	q.append("\n");
      }
    }

    q.append("root=");
    q.append(rootPath);
    q.append("\n");

    return q;
  }

};

class FragmentSpecification
{
 public:
  long fragmentId;
  int index;
  QString maskPath;

  QString toString() {

    QString q("\n");

    q.append("  fragmentId=");
    q.append(QString::number(fragmentId));
    q.append("\n");

    q.append("    index=");
    q.append(QString::number(index));
    q.append("\n");

    q.append("    maskPath=");
    q.append(maskPath);
    q.append("\n");

    return q;
  }
    
};

class SampleSpecification
{
 public:
  friend class SampleSort;
  long sampleId;
  QString owner;
  long alignmentResultId;
  QString space;
  long alignedStackId;
  double nccScore;
  long neuronSeparationId;
  QString consolidatedLabelPath;
  QString consolidatedSignalPath;
  QList<FragmentSpecification*> fragmentList;

  QString toString() {
    
    QString q("\n");

    q.append("sampleId=");
    q.append(QString::number(sampleId));
    q.append("\n");

    q.append("owner=");
    q.append(owner);
    q.append("\n");

    q.append("aligmentResultId=");
    q.append(QString::number(alignmentResultId));
    q.append("\n");

    q.append("space=");
    q.append(space);
    q.append("\n");

    q.append("alignedStackId=");
    q.append(QString::number(alignedStackId));
    q.append("\n");

    q.append("nccScore=");
    q.append(QString::number(nccScore));
    q.append("\n");

    q.append("neuronSeparationid=");
    q.append(QString::number(neuronSeparationId));
    q.append("\n");

    q.append("consolidatedLabelPath=");
    q.append(consolidatedLabelPath);
    q.append("\n");

    q.append("consolidatedSignalPath=");
    q.append(consolidatedSignalPath);
    q.append("\n");

    for (int i=0;i<fragmentList.size();i++) {
      FragmentSpecification* fs=fragmentList[i];
      QString f=fs->toString();
      q.append(f);
    }
    
    return q;
  }

};

class SampleSort
{
 public:
  bool operator()(const SampleSpecification *s1, const SampleSpecification *s2 ) const;
};

class MaskScore
{
 public:
  MaskScore() {
    score=0.0;
    x=y=z=0;
    zeroCount=nonzeroCount=nonzeroScore=zeroScore=fragmentNonzeroCount=0;
  }
  double score;
  int x;
  int y;
  int z;
  int zeroCount;
  int nonzeroCount;
  int zeroScore;
  int nonzeroScore;
  int fragmentNonzeroCount;
  
  void append(const MaskScore* m2) {
    this->zeroCount+=m2->zeroCount;
    this->nonzeroCount+=m2->nonzeroCount;
    this->zeroScore+=m2->zeroScore;
    this->nonzeroScore+=m2->nonzeroScore;
    this->fragmentNonzeroCount+=m2->fragmentNonzeroCount;
  }

};

class SubjectScore
{
 public:
  friend class ScoreSort;
  long sampleId;
  long fragmentId;
  QString owner;
  int firstStageVoxelCount; 
  MaskScore* maskScore;
};

class ScoreSort
{
 public:
  bool operator()(const SubjectScore *s1, const SubjectScore *s2 ) const;
};


// Not used in current implementation
class FragmentThread
{
 public:
  FILE* fid; 
  long sampleId;
  long fragmentId;
  SubjectScore* score;
};

class SampleThread
{
 public:
  FILE* fid;
  long sampleId;
  QString owner;
  int firstStageVoxelCount;
  char* firstStageData;
  //  QList<FragmentThread*> fragmentThreadList;
  //  QList< QFuture<FragmentThread*> > fragmentFutureList;
  QMap<long, QList<MaskScore*> > fragmentScoreMap;
};
  

/////////////////////////////////////////////////////////////////////////////////////////////////////////

class VolumeIndex
{
public:

    static const int MODE_UNDEFINED;
    static const int MODE_CREATE_SAMPLE_INDEX_FILE;
    static const int MODE_SEARCH;
    static const int MODE_ADD_SAMPLE_INDEX_FILE_TO_CONSOLIDATED_INDEX;

    static const char ENTRY_CODE;

    static const QString PRIMARY_INDEX_FILENAME;
    static const QString SECONDARY_INDEX_FILENAME;

    static const int MAX_OWNER_LENGTH;

    VolumeIndex();

    ~VolumeIndex();

    static string getCommandLineDescription() {
        return "volume-index";
    }

    static string getUsage() {
        string usage;
        usage.append("  Volume Index Tool - v2 of earlier volume-pattern-index                                                \n");
        usage.append("                                                                                                        \n");
        usage.append("   Tool for indexing and searching volumes for similar patterns                                         \n");
	usage.append("                                                                                                        \n");
	usage.append("    'sample' creates a binary sample index file from a sample, for adding to main index                 \n");
	usage.append("    'search' searches the main index                                                                    \n");
        usage.append("    'index' adds a sample file to a main consolidated index                                             \n");
        usage.append("                                                                                                        \n");
        usage.append("     -mode [sample|search|index]                                                                        \n");
	usage.append("                                                                                                        \n");
	usage.append("   For all:                                                                                             \n");
        usage.append("     -indexSpecificationFile <index specification file>                                                 \n");
	usage.append("                                                                                                        \n");
	usage.append("   For modes sample and index:                                                                          \n");
	usage.append("     -sampleIndexFile <path to sample index file>                                                       \n");
        usage.append("                                                                                                        \n");
	usage.append("   For mode sample:                                                                                     \n");
	usage.append("     -sampleSpecificationFile <path to file with metadata for stack and mask files>                     \n");
	usage.append("                                                                                                        \n");
	usage.append("   For mode search:                                                                                     \n");
	usage.append("     -query <query stack or mask file>                                                                  \n");
	usage.append("     -minSubjectVoxels <int> [200]                                                                      \n");
	usage.append("     -queryThreshold <if file non-binary, binarization threshold> [10]                                  \n");
        usage.append("     -maxHits <max number of hits> [100]                                                                \n");
	usage.append("     -backgroundWeight <float. 0=ignore 1=same >1=more than mask> [1.0]                                 \n");
	usage.append("     -maskDilation <int, num cycles. 0=none 1=1, etc> [0]                                               \n");
        usage.append("                                                                                                        \n");
        return usage;
    }

    bool execute();
    int processArgs(vector<char*> *argList);

private:
    FILE* fid;
    FILE* mainIndexFid;

    bool ERROR_FLAG;

    bool DEBUG_FLAG;
    int mode;

    QString modeString;
    QString indexSpecificationFilepath;
    QString sampleIndexFilepath;
    QString sampleSpecificationFilepath;
    QString maxHitsString;

    bool createSampleIndexFile();
    bool doSearch();
    bool addSampleToConsolidatedIndex();
    
    bool readIndexSpecificationFile();
    bool readSampleSpecificationFile();

    void addAlignmentIfMatchingSpace(SampleSpecification* ss);

    QList<SampleSpecification*> sampleSpecificationList;
    IndexSpecification* indexSpecification;
    SampleSpecification* sampleSpecification;

    My4DImage* sampleConsolidatedSignalImage;
    bool loadSampleConsolidatedSignalImage();
    bool validateImageSize(My4DImage* image);

    char* firstStageIndex;
    int firstStageIndexBytes;

    // Note: these get cleared re-written with every fragment (or sample signal)
    QList<char*> secondStageIndex; // includes x,y,z,fragmentId,sampleId,data - each entry for subvolume
    QList<int> secondStageIndexLength; // length of full entry including x,y,z,fragmentId,sampleId,data - each entry for subvolume
    int secondStageVoxelCount; // for the current sample/fragment

    void clearSecondStageData();

    int divideDimensionByUnit(int originalSize, int unit);
    int* subsampleAndThresholdToBinaryMask(My4DImage* sourceImage, char* targetMask, int unit, int threshold);
    bool createSecondStageEntry(My4DImage* image, long fragmentId, long sampleId, bool updateFirstStage);
    char* getAddressOfFirstStageMaskAtCoordinate(int x, int y, int z, int ownerLength);

    bool createFirstStageIndexForSampleFile();
    bool createSecondStageIndexForSampleFile();
    bool writeSampleIndexHeaderAndFirstStage();
    bool writeSampleIndexUpdate();

    bool validatePositiveFirstStageEntry(int x, int y, int z, int ownerLength);

    bool initParamsFromIndexSpecification();
    bool initParamsFromSampleSpecification();
    int firstStageCorrectionCount;

    // Settings for shared work
    int X_SIZE, Y_SIZE, Z_SIZE;
    int UNIT;
    int BITS;
    long SAMPLE_ID;
    QString OWNER;
    int X1_SIZE, Y1_SIZE, Z1_SIZE;

    bool processSampleIndexToMainIndex();

    FILE* openPrimaryIndex(const char* filestring);
    FILE* openSecondaryIndexToAppend(int x, int y, int z);
    FILE* openSecondaryIndexToRead(int x, int y, int z);
    QString getSecondaryIndexFilepath(int x, int y, int z);
    QString getSecondaryIndexDir(int x, int y, int z);

    QString queryFilepath;
    int minSubjectVoxels;
    int queryThreshold;
    int maxHits;
    double backgroundWeight;
    int maskDilation;

    My4DImage* queryImage;

    bool loadQueryImage();

    void dilateQueryMask();

    MaskScore* computeSubvolumeScore(char* query, char* subject, int length);

    MaskScore**** blankSubjectScore;
    bool computeBlankSubjectScore();
    int getImageSubvolumeDataByStage1Coordinates(My4DImage* image, char* data, int x1, int y1, int z1, int tCount, char* tArr);
    
    bool computeSubjectScores();
    int getStage2DataByteCount(int units);

    QList<SubjectScore*> searchResultList;
    QList<SampleThread*> sampleThreadList;
    QMap< long, QSet<QString> > fragmentStage1NonzeroSet;

    SampleThread* runSampleThread(SampleThread* sampleThread);
    FragmentThread* runFragmentThread(FragmentThread* fragmentThread);

    QList< QFuture<SampleThread*> > sampleFutureList;

    void addMaskScoreForSecondStageEntry(long fragmentId, SampleThread* sampleThread, char* secondStageData, int dataUnits, int x1, int y1, int z1);
    void computeMaskScore(MaskScore* maskScore);
    void expandSubjectData(char* compressed, char* expanded, int units);
    void addSampleResult(SampleThread* st);

    bool displaySearchResults();
    
    void doVoxelCountReport(My4DImage* image);

};

#endif // VOLUMEINDEX_H

#ifndef VOLUMEPATTERNINDEX_H
#define VOLUMEPATTERNINDEX_H

#include <QString>
#include <QtCore>
#include <QDir>
#include "../../v3d/v3d_core.h"
#include "AnalysisTools.h"
#include "MaskChan.h"

using namespace std;

class VolumePatternIndex
{
public:

    static const int DEFAULT_UNIT_SIZE;
    static const int DEFAULT_THRESHOLD_A;
    static const int DEFAULT_THRESHOLD_B;
    static const int DEFAULT_THRESHOLD_C;
    static const double DEFAULT_MIN_SCORE;
    static const int DEFAULT_MAX_HITS;
    static const QString DEFAULT_MATRIX_STRING;
    static const QString DEFAULT_FULL_MATRIX_STRING;
    static const int DEFAULT_BINARY_PROXY_VALUE;

    static const int MODE_UNDEFINED;
    static const int MODE_INDEX;
    static const int MODE_SEARCH;
    static const int MODE_APPEND;

    static const int INDEX_STATE_CREATE;
    static const int INDEX_STATE_APPEND;

    static const int FILENAME_BUFFER_SIZE;
    static const int ID_BUFFER_SIZE;

    VolumePatternIndex();

    ~VolumePatternIndex();

    static string getCommandLineDescription() {
        return "volume-pattern-index";
    }

    static string getUsage() {
        string usage;
        usage.append("  Volume Pattern Index Tool                                                                             \n");
        usage.append("                                                                                                        \n");
        usage.append("    Tool for indexing and searching volumes for similar patterns                                        \n");
	usage.append("                                                                                                        \n");
	usage.append("    'index' builds an index, whereas 'append' appends new data to preexisting index                     \n");
        usage.append("                                                                                                        \n");
        usage.append("     -mode [index|search|append]                                                                        \n");
        usage.append("     -indexFile <index file>                                                                            \n");
        usage.append("                                                                                                        \n");
        usage.append("    For modes index and append:  (note: .mask files or stacks OK - masks treated as max intensity)      \n");
        usage.append("                                                                                                        \n");
        usage.append("     -inputList <file with list of stacks to index, format=<unique-ID> <file> [ <channel#> ] >          \n");
        usage.append("     -defaultChannelToIndex <channel number>                                                            \n");
        usage.append("     [ -unitSize <voxels per cube side> : default=10 ]                                                  \n");
        usage.append("     [ -threshold \"a b c\" : default a=6, b=20, c=50 , creates 4 scoring bins ]                        \n");
        usage.append("                                                                                                        \n");
        usage.append("    For mode search:                                                                                    \n");
        usage.append("                                                                                                        \n");
        usage.append("     -query <image volume or .mask file to use as query>                                                \n");
        usage.append("     -queryChannel <channel number>                                                                     \n");
        usage.append("     -outputFile <outputFile>                                                                           \n");
	usage.append("     [ -minScore <minimum score to quality as hit> : default=-1000000.0                                 \n");
        usage.append("     [ -maxHits <maximum number of hits> : default=100 ]                                                \n");
        usage.append("     [ -skipzeros : ignores all zero-valued voxels in query during search ]                             \n");
        usage.append("     [ -matrix \"t0s0 t0s1 t0s2 t0s3 ... t3s0 t3s1 t3s2 t3s3\" : the 16 int values for score matrix ]   \n");
        usage.append("                                                                                                        \n");
        return usage;
    }

    bool execute();
    int processArgs(vector<char*> *argList);

private:
    FILE* fid;

    bool DEBUG_FLAG;

    int mode;
    int indexState;

    V3DLONG x0,x1,y0,y1,z0,z1;
    V3DLONG qx0, qx1, qy0, qy1, qz0, qz1;
    V3DLONG iXmax, iYmax, iZmax;

    QString modeString;

    QString inputFileListPath;
    int defaultChannelToIndex;
    QString indexFilePath;
    int unitSize;
    int* threshold;

    QString queryImageFilePath;
    int queryChannel;
    QString outputFilePath;
    double minScore;
    int maxHits;
    bool skipzeros;
    int* matrix;

    unsigned char* indexData;
    V3DLONG indexTotalBytes;

    unsigned char* queryIndex;
    unsigned char* queryIndexSkipPositions;
    My4DImage* queryImage;

    // These track the entry id, path, and channel - in order
    QList<V3DLONG> indexScoreList;
    QList<int> indexChannelList;
    QStringList indexIdList;
    QHash<QString,QString> idPathHash;
    QStringList indexFileList;

    QStringList parseIndexFileLine(QString line);
    bool createIndex();
    bool appendIndex();
    bool doSearch();
    bool parseThresholdString(QString thresholdString);
    bool populateIndexFileList();
    bool parseMatrixString(QString matrixString);
    bool openIndexAndWriteHeader();
    bool openIndexAndReadHeader();
    bool addEntriesToIndex();

    void indexImage(My4DImage* image, int channel, bool skipzeros);
    V3DLONG calculateIndexScore(unsigned char* queryIndex, unsigned char* subjectIndex, V3DLONG indexTotal, unsigned char* skipPositions);
    V3DLONG computeTotalBytesFromIndexTotal(V3DLONG indexTotal);

    static bool compareScores(QPair<V3DLONG, int> p1, QPair<V3DLONG, int> p2) {
        if (p1.first > p2.first) {
            return true;
        } else {
            return false;
        }
    }

};

#endif // VOLUMEPATTERNINDEX_H

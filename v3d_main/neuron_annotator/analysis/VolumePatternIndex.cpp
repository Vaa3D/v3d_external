
/*

  Index
  ----------------------------------------------------------------------------

  This utility indexes a set of 3-D images of 1-channel for search against a
  query 3-D image also of 1-channel. The dimensions of the query and index
  must match.

  1. Number of Indices

  We will start with an initial version having only a single index.

  2. Index Characteristics

  The index will consist of a reduced downsampling of the original image, the
  magnitude of which is re-mapped according to the threshold set.

  3. Bitwidth of Index

  There are four thresholds used to partition the data: (with sample values)

  0 - empty [0-5]
  1 - low [6-19]
  2 - medium [20-49]
  3 - high [50 and up]

  We will therefore use 2-bits to characterize a region within the index.

  4. Size of Region

  The region will downsample the original data by a factor of 1000, so that we
  use 10x10x10 downsampling as default.

  Search
  -------------------------------------------------------------------------------

  1. Steps

  Search will be either one-phase or two-phase. If one-phase, or "fast", then
  only the index will be used. If two-phase, then the results of the fast
  search will be further refined in the second phase by using the actual source
  image files.

  A floating-point value will be multiplied by the "maxHits" parameters and this
  number (rounded to the nearest integer) will be used as the number of fast
  results to be forwarded to the second pass.

  2. Scoring

  There will be a 4x4 scoring matrix, used for both the fast phase and the second
  (full) phase.

  The default matrix will be:

                              Subject

                  empty      low      medium      high


       empty        0        -1        -2        -4

Qry    low         -1         1         1         0

       medium      -2         1         2         2

       high        -4         0         2         4


 */



#include "VolumePatternIndex.h"
#include "../utility/ImageLoader.h"

const int VolumePatternIndex::MODE_UNDEFINED=-1;
const int VolumePatternIndex::MODE_INDEX=0;
const int VolumePatternIndex::MODE_SEARCH=1;

VolumePatternIndex::VolumePatternIndex()
{
    mode=MODE_UNDEFINED;
    fastSearch=false;
}

VolumePatternIndex::~VolumePatternIndex()
{
}

bool VolumePatternIndex::execute()
{
    if (mode==MODE_UNDEFINED) {
        return false;
    } else if (mode==MODE_INDEX) {
        return createIndex();
    } else if (mode==MODE_SEARCH) {
        return doSearch();
    }
    return false;
}

int VolumePatternIndex::processArgs(vector<char*> *argList)
{
    for (int i=0;i<argList->size();i++) {
        QString arg=(*argList)[i];
        if (arg=="-mode") {
            modeString=(*argList)[++i];
        } else if (arg=="-inputList") {
            inputFileListPath=(*argList)[++i];
        } else if (arg=="-outputIndex") {
            outputIndexFilePath=(*argList)[++i];
        } else if (arg=="-subVolume") {
            QString subVolumeString=(*argList)[++i];
            if (!parseSubVolumeString(subVolumeString)) {
                qDebug() << "Could not parse subVolumeString=" << subVolumeString;
                return 0;
            }
        } else if (arg=="-unitSize") {
            QString unitSizeString=(*argList)[++i];
            unitSize=unitSizeString.toInt();
        } else if (arg=="-threshold") {
            QString thresholdString=(*argList)[++i];
            if (!parseThresholdString(thresholdString)) {
                qDebug() << "Could not parse threshold string=" << thresholdString;
                return 0;
            }
        } else if (arg=="-query") {
            queryImageFilePath=(*argList)[++i];
        } else if (arg=="-maxHits") {
            QString maxHitsString=(*argList)[++i];
            maxHits=maxHitsString.toInt();
        } else if (arg=="-fast") {
            fastSearch=true;
        } else if (arg=="-matrix") {
            QString matrixString=(*argList)[++i];
            if (!parseMatrixString(matrixString)) {
                qDebug() << "Could not parse matrix string=" << matrixString;
            }
        }
    }
    if (modeString.size()>0) {
        if (modeString=="index") {
            mode=MODE_INDEX;
        } else if (modeString=="search") {
            mode=MODE_SEARCH;
        }
    }
    return 0;
}

bool VolumePatternIndex::parseSubVolumeString(QString subVolumeString) {
    QRegExp splitRegex("\\s+");
    QStringList volList=subVolumeString.split(splitRegex);
    if (volList.size()!=6) {
        if (volList.size()>0) {
            for (int i=0;i<volList.size();i++) {
                qDebug() << "i=" << i << " string=" << volList[i];
            }
        }
        return false;
    }
    QString x0String=volList[0];
    x0=x0String.toInt();
    QString x1String=volList[1];
    x1=x1String.toInt();
    QString y0String=volList[2];
    y0=y0String.toInt();
    QString y1String=volList[3];
    y1=y1String.toInt();
    QString z0String=volList[4];
    z0=z0String.toInt();
    QString z1String=volList[5];
    z1=z1String.toInt();
}

bool VolumePatternIndex::parseThresholdString(QString thresholdString) {
    QRegExp splitRegex("\\s+");
    QStringList tList=thresholdString.split(splitRegex);
    if (tList.size()!=4) {
        if (tList.size()>0) {
            for (int i=0;i<tList.size();i++) {
                qDebug() << "i=" << i << " string=" << tList[i];
            }
        }
        return false;
    }
    // For now we expect 4 positions
    threshold=new int[4];
    for (int i=0;i<tList.size();i++) {
        threshold[i]=tList[i].toInt();
    }
}

bool VolumePatternIndex::parseMatrixString(QString matrixString) {
    QRegExp splitRegex("\\s+");
    QStringList mList=matrixString.split(splitRegex);
    if (mList.size()!=16) {
        for (int i=0;i<mList.size();i++) {
            qDebug() << "i=" << i << " string=" << mList[i];
        }
        return false;
    }
    matrix=new float[16];
    for (int i=0;i<16;i++) {
        matrix[i]=mList[i].toFloat();
    }
}

bool VolumePatternIndex::createSubVolume() {
    return true;
}

bool VolumePatternIndex::populateIndexFileList() {
    QFile indexFileListFile(inputFileListPath);
    if (!indexFileListFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open file=" << inputFileListPath << " to read";
        return false;
    }
    while(!indexFileListFile.atEnd()) {
        QString line=indexFileListFile.readLine();
        line=line.trimmed();
        QFileInfo fileInfo(line);
        if (!fileInfo.exists()) {
            qDebug() << "Could not verify that file=" << line << " exists";
            return false;
        }
        qDebug() << "Verified " << line << " exists";
        indexFileList.append(line);
    }
    indexFileListFile.close();
}

bool VolumePatternIndex::createIndex()
{
    qDebug() << "createIndex() start";
    if (!populateIndexFileList()) {
        qDebug() << "populateIndexFileList failed";
        return false;
    }
}

bool VolumePatternIndex::doSearch()
{
    qDebug() << "doSearch() start";
}



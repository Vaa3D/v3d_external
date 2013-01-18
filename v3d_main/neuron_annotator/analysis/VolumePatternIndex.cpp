
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

  There are three thresholds used to partition the data into 4 regions:
     (with sample values) 6, 20, 50:

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

const int DEFAULT_UNIT_SIZE = 10;
const int DEFAULT_THRESHOLD_A = 6;
const int DEFAULT_THRESHOLD_B = 20;
const int DEFAULT_THRESHOLD_C = 50;
const int DEFAULT_MAX_HITS = 100;
const QString DEFAULT_MATRIX_STRING("0.0 -1.0 -2.0 -4.0 -1.0 1.0 1.0 0.0 -2.0 1.0 2.0 2.0 -4.0 0.0 2.0 4.0");

const int VolumePatternIndex::MODE_UNDEFINED=-1;
const int VolumePatternIndex::MODE_INDEX=0;
const int VolumePatternIndex::MODE_SEARCH=1;

VolumePatternIndex::VolumePatternIndex()
{
    fid=0;
    mode=MODE_UNDEFINED;
    fastSearch=false;
    defaultChannelToIndex=-1;
    queryChannel=-1;
    iXmax=-1;
    iYmax=-1;
    iZmax=-1;
    x0=x1=y0=y1=z0=z1=-1;
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
        } else if (arg=="-defaultChannelToIndex") {
            QString defaultChannelToIndexString=(*argList)[++i];
            defaultChannelToIndex=defaultChannelToIndexString.toInt();
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
        } else if (arg=="-queryChannel") {
            QString queryChannelString=(*argList)[++i];
            queryChannel=queryChannelString.toInt();
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
            if (queryChannel==-1) {
                qDebug() << "queryChannel must be defined";
                return 0;
            }
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
    if (tList.size()!=3) {
        if (tList.size()>0) {
            for (int i=0;i<tList.size();i++) {
                qDebug() << "i=" << i << " string=" << tList[i];
            }
        }
        return false;
    }
    // For now we expect 3 positions
    threshold=new int[3];
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
        int channel=-1;
        QRegExp splitRegex("\\s+");
        QStringList fList=line.split(splitRegex);
        if (fList.size()>1) {
            channel=fList[1].toInt();
        } else {
            channel=defaultChannelToIndex;
        }
        if (channel<0) {
            qDebug() << "Index channel not defined - must specify defaultChannelToIndex if channel not given";
            return false;
        } else {
            indexChannelList.append(channel);
        }
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
    V3DLONG* subregion=new V3DLONG[6];

    if (!populateIndexFileList()) {
        qDebug() << "populateIndexFileList failed";
        return false;
    }
    if (indexFileList.size()<1) {
        qDebug() << "No files to index";
        return false;
    }
    int fileIndex=0;
    for (fileIndex=0;fileIndex<indexFileList.size();fileIndex++) {
        My4DImage sourceImage;
        ImageLoader loader;
        if (!loader.loadImage(&sourceImage, indexFileList[fileIndex])) {
            qDebug() << "Could not load file=" << indexFileList[fileIndex];
            return false;
        }
        if (fileIndex==0 && x0==-1) {
            // Then assume we are to use the first image to set the selection region
            x0=0; x1=sourceImage.getXDim();
            y0=0; y1=sourceImage.getYDim();
            z0=0; z1=sourceImage.getZDim();
            subregion[0]=x0;
            subregion[1]=x1;
            subregion[2]=y0;
            subregion[3]=y1;
            subregion[4]=z0;
            subregion[5]=z1;
        }
        My4DImage* cubifiedImage=AnalysisTools::cubifyImageByChannel(&sourceImage, indexChannelList[fileIndex], unitSize, AnalysisTools::CUBIFY_TYPE_AVERAGE, subregion);
        if (fileIndex==0) {
            iXmax=cubifiedImage->getXDim();
            iYmax=cubifiedImage->getYDim();
            iZmax=cubifiedImage->getZDim();
        } else {
            if (cubifiedImage->getXDim()!=iXmax ||
                cubifiedImage->getYDim()!=iYmax ||
                cubifiedImage->getZDim()!=iZmax) {
                qDebug() << "Cubified image " << indexFileList[fileIndex] << " dimensions do not match";
                return false;
            }
        }
        // Next, we will walk through the image and gradually populate a binary structure with the scores.
        V3DLONG cubifiedTotal=iXmax*iYmax*iZmax;
        // We can store 4 2-bit values per 8-bit byte, so these are the number of bytes we need:
        V3DLONG indexTotalBytes=ceil((cubifiedTotal*1.0)/4.0);
        unsigned char* indexData=new unsigned char[indexTotalBytes];
        V3DLONG indexPosition=0;
        int bytePosition=0;
        v3d_uint8* cubifiedData=cubifiedImage->getRawDataAtChannel(0);
        for (V3DLONG z=0;z<iZmax;z++) {
            V3DLONG zoffset=z*iYmax*iXmax;
            for (V3DLONG y=0;y<iYmax;y++) {
                V3DLONG yoffset=zoffset+y*iXmax;
                for (V3DLONG x=0;x<iXmax;x++) {
                    V3DLONG offset=yoffset+x;
                    v3d_uint8 value=cubifiedData[offset];
                    if (bytePosition>3) {
                        bytePosition=0;
                        indexPosition++;
                        if (indexPosition>=indexTotalBytes) {
                            qDebug() << "Exceeded indexTotalBytes at z=" << z << " y=" << y << " x=" << x;
                            return false;
                        }
                    }
                    unsigned char* currentByte = indexData + indexPosition;
                    if (bytePosition==0) {
                        // As we begin populating a byte, clear it first
                        *currentByte=0;
                    }
                    // 0=00000000
                    // 1=00000001
                    // 2=00000010
                    // 3=00000011
                    if (*currentByte<threshold[0]) {
                        // leave as zero
                    } else if (*currentByte<threshold[1]) {
                        *currentByte |= 1;
                    } else if (*currentByte<threshold[2]) {
                        *currentByte |= 2;
                    } else {
                        *currentByte |= 3;
                    }
                    if (bytePosition<3) {
                        *currentByte <<= 2;
                    }
                    bytePosition++;
                }
            }
        }
        if (fileIndex==0) {
            if (!openIndexAndWriteHeader()) {
                qDebug() << "Could not open index file and write header";
                return false;
            }
        }

//            V3DLONG i;

//            fid = fopen(filename, "wb");
//            if (!fid)
//            {
//                return exitWithError("Fail to open file for writing");
//            }

//            /* Write header */
//                             // raw_image_stack_by_hpeng
//            char formatkey[] = "v3d_volume_pkbitdf_encod";
//            int lenkey = strlen(formatkey);

//            V3DLONG nwrite = fwrite(formatkey, 1, lenkey, fid);
//            if (nwrite!=lenkey)
//            {
//                return exitWithError("File write error");
//            }


    }
    return true;
}

bool VolumePatternIndex::openIndexAndWriteHeader() {
    fid=fopen(outputIndexFilePath.toAscii().data(), "wb");
    if (!fid) {
        qDebug() << "Could not open file=" << outputIndexFilePath << " to write";
        return false;
    }

    // First, we have to characterize the parameters of the index, so that compatibility
    // with the query can be checked.





    // Next, write the number of index files
    int numIndexFiles=indexFileList.size();
    fwrite(&numIndexFiles, sizeof(int), 1, fid);


}

bool VolumePatternIndex::doSearch()
{
    qDebug() << "doSearch() start";
    return true;
}



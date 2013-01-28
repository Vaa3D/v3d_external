
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

const int VolumePatternIndex::FILENAME_BUFFER_SIZE = 2000;

const int VolumePatternIndex::DEFAULT_UNIT_SIZE = 10;
const int VolumePatternIndex::DEFAULT_THRESHOLD_A = 6;
const int VolumePatternIndex::DEFAULT_THRESHOLD_B = 20;
const int VolumePatternIndex::DEFAULT_THRESHOLD_C = 50;
const int VolumePatternIndex::DEFAULT_MAX_HITS = 100;
const QString VolumePatternIndex::DEFAULT_MATRIX_STRING("0 -1 -2 -4 -1 1 1 0 -2 1 2 2 -4 0 2 4");

const int VolumePatternIndex::MODE_UNDEFINED=-1;
const int VolumePatternIndex::MODE_INDEX=0;
const int VolumePatternIndex::MODE_SEARCH=1;

VolumePatternIndex::VolumePatternIndex()
{
    fid=0;
    mode=MODE_UNDEFINED;
    fullSearch=false;
    defaultChannelToIndex=-1;
    queryChannel=-1;
    iXmax=-1;
    iYmax=-1;
    iZmax=-1;
    x0=x1=y0=y1=z0=z1=-1;
    qx0=qx1=qy0=qy1=qz0=qz1=-1;
    indexData=0L;
    indexTotalBytes=-1L;
    matrix=0;
    unitSize=DEFAULT_UNIT_SIZE;
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
        } else if (arg=="-indexFile") {
            indexFilePath=(*argList)[++i];
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
        } else if (arg=="-full") {
            fullSearch=true;
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
            qx0=x0;x0=-1;
            qx1=x1;x1=-1;
            qy0=y0;y0=-1;
            qy1=y1;y1=-1;
            qz0=z0;z0=-1;
            qz1=z1;z1=-1;
            if (queryChannel==-1) {
                qDebug() << "queryChannel must be defined";
                return 0;
            }
        }
    }
    if (matrix==0) {
        if (!parseMatrixString(DEFAULT_MATRIX_STRING)) {
            qDebug() << "Could not parse default matrix string=" << DEFAULT_MATRIX_STRING;
            return 0;
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
    x0=x0String.toLong();
    QString x1String=volList[1];
    x1=x1String.toLong();
    QString y0String=volList[2];
    y0=y0String.toLong();
    QString y1String=volList[3];
    y1=y1String.toLong();
    QString z0String=volList[4];
    z0=z0String.toLong();
    QString z1String=volList[5];
    z1=z1String.toLong();
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
    matrix=new int[16];
    for (int i=0;i<16;i++) {
        matrix[i]=mList[i].toInt();
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

void VolumePatternIndex::formatSubregion(V3DLONG* subregion)
{
    subregion[0]=x0;
    subregion[1]=x1;
    subregion[2]=y0;
    subregion[3]=y1;
    subregion[4]=z0;
    subregion[5]=z1;
}

bool VolumePatternIndex::createIndex()
{
    qDebug() << "createIndex() start";
    V3DLONG* subregion=0L;

    if (!populateIndexFileList()) {
        qDebug() << "populateIndexFileList failed";
        return false;
    }
    if (indexFileList.size()<1) {
        qDebug() << "No files to index";
        return false;
    }
    int fileIndex=0;
    unsigned char* indexData=0L;
    for (fileIndex=0;fileIndex<indexFileList.size();fileIndex++) {
        My4DImage sourceImage;
        ImageLoader loader;
        if (!loader.loadImage(&sourceImage, indexFileList[fileIndex])) {
            qDebug() << "Could not load file=" << indexFileList[fileIndex];
            return false;
        } else {
            qDebug() << "Indexing " << fileIndex << " of " << indexFileList.size() << " : " << indexFileList[fileIndex] << "...";
        }
        if (fileIndex==0 && x0==-1) {
            // Then assume we are to use the first image to set the selection region
            qDebug() << "createIndex::Formatting subregion. x0=" << x0;
            x0=0; x1=sourceImage.getXDim();
            y0=0; y1=sourceImage.getYDim();
            z0=0; z1=sourceImage.getZDim();
            if (subregion==0L) {
                subregion=new V3DLONG[6];
            }
            formatSubregion(subregion);
            qDebug() << "Done formatting subregion";
            for (int i=0;i<6;i++) {
                qDebug() << "subregion " << i << " =" << subregion[i];
            }
        }
        qDebug() << "Calling indexImage with subregion=" << subregion;
        indexData=indexImage(&sourceImage, indexChannelList[fileIndex], subregion, indexData);
        if (fileIndex==0) {
            if (!openIndexAndWriteHeader()) {
                qDebug() << "Could not open index file and write header";
                return false;
            }
        }
        fwrite(indexData, sizeof(unsigned char), indexTotalBytes, fid);
    }
    if (indexData!=0) {
        delete [] indexData;
    }
    if (subregion!=0) {
        delete [] subregion;
    }
    fclose(fid);
    fid=0;
    return true;
}

bool VolumePatternIndex::openIndexAndWriteHeader() {
    fid=fopen(indexFilePath.toAscii().data(), "wb");
    if (!fid) {
        qDebug() << "Could not open file=" << indexFilePath << " to write";
        return false;
    }

    // First, we have to characterize the parameters of the index, so that compatibility
    // with the query can be checked.

    // The parameters of the index are:
    //    a) the original size (before creating cubic index)
    //    b) the unit size (for creating the index)
    //    c) the thresholds (number and values)

    // 1. Image Size
    fwrite(&x0, sizeof(int), 1, fid);
    fwrite(&x1, sizeof(int), 1, fid);

    fwrite(&y0, sizeof(int), 1, fid);
    fwrite(&y1, sizeof(int), 1, fid);

    fwrite(&z0, sizeof(int), 1, fid);
    fwrite(&z1, sizeof(int), 1, fid);

    // 2. Unit Size

    fwrite(&unitSize, sizeof(int), 1, fid);

    // 3. Threshold

    int thresholdNumber=3;
    fwrite(&thresholdNumber, sizeof(int), 1, fid);
    for (int i=0;i<3;i++) {
        fwrite(threshold+i, sizeof(int), 1, fid);
    }

    // 4. Cube Size

    fwrite(&iXmax, sizeof(int), 1, fid);
    fwrite(&iYmax, sizeof(int), 1, fid);
    fwrite(&iZmax, sizeof(int), 1, fid);

    // 5. Number of Index Files

    // Next, write the number of index files
    int numIndexFiles=indexFileList.size();
    fwrite(&numIndexFiles, sizeof(int), 1, fid);

    // 6. Index File Paths w/ Channel Index

    // Next, write the paths for the index files, along with channel indices
    char* filePathBuffer=new char[FILENAME_BUFFER_SIZE];
    for (int i=0;i<indexFileList.size();i++) {
        QString filePath=indexFileList[i];
        int length=filePath.length();
        if (length>FILENAME_BUFFER_SIZE) {
            qDebug() << "File path length name buffer not large enough for=" << filePath;
            return false;
        }
        fwrite(&length, sizeof(int), 1, fid);
        strcpy(filePathBuffer, filePath.toAscii().data());
        fwrite(filePathBuffer, sizeof(char), length, fid);
        int channelIndex=indexChannelList[i];
        fwrite(&channelIndex, sizeof(int), 1, fid);
    }
}

bool VolumePatternIndex::doSearch()
{
    qDebug() << "doSearch() start - reading index header...";
    if (!openIndexAndReadHeader()) {
        qDebug() << "Could not open and read header of file=" << indexFilePath;
        return false;
    }

    qDebug() << "Read query...";
    ImageLoader queryLoader;
    if (!queryLoader.loadImage(queryImage, queryImageFilePath)) {
        qDebug() << "Could not load query image file=" << queryImageFilePath;
        return false;
    }

    qDebug() << "Convert query to index format...";
    V3DLONG* subregion=new V3DLONG[6];
    formatSubregion(subregion);
    if ((indexData=indexImage(queryImage, queryChannel, subregion, indexData))==0) {
        qDebug() << "Error indexing query";
        return false;
    }
    queryIndex=new unsigned char [indexTotalBytes];
    memcpy(queryIndex, indexData, indexTotalBytes);

    qDebug() << "Begin scoring (index phase)...";
    V3DLONG indexTotal=iXmax*iYmax*iZmax;
    for (int i=0;i<indexFileList.size();i++) {
        size_t readSize=0;
        if ( (readSize=fread(indexData, 1, indexTotalBytes, fid))!=indexTotalBytes ) {
            qDebug() << "Could not read full block of " << indexTotalBytes << " , read " << readSize << " instead";
            return false;
        }
        V3DLONG score=calculateIndexScore(queryIndex, indexData, indexTotal);
        indexScoreList.append(score);
    }

    qDebug() << "Begin sorting...";

    QList< QPair<V3DLONG, int> > pairList;

    for (int i=0;i<indexScoreList.size();i++) {
        pairList.append(qMakePair(indexScoreList[i], i));
    }

    qSort(pairList.begin(), pairList.end(), VolumePatternIndex::compareScores);

    int finalResultSize=maxHits;
    if (finalResultSize>pairList.size()) {
        finalResultSize=pairList.size();
    }

    QList< QPair<V3DLONG, int> > finalResultList;

    if (fullSearch) {

        QList< QPair<V3DLONG, int> > fullScoreList;

        // Take the top maxHits and compute the full-resolution scores
        for (int i=0;i<finalResultSize;i++) {
            QPair<V3DLONG, int> p=pairList[i];
            int index=p.second;
            QString filename=indexFileList[index];
            ImageLoader loader;
            My4DImage* currentImage;
            qDebug() << "Full search - loading " << filename << "...";
            loader.loadImage(currentImage, filename);
            qDebug() << "              scoring...";
            V3DLONG score;
            int subjectChannel=indexChannelList[index];
            if (!calculateImageScore(queryImage, currentImage, subjectChannel, &score)) {
                qDebug() << "Error calculating score for " << filename;
                return false;
            }
            fullScoreList.append(qMakePair(score, index));
        }

        qSort(fullScoreList.begin(), fullScoreList.end(), VolumePatternIndex::compareScores);

        for (int i=0;i<finalResultSize;i++) {
            finalResultList.append(fullScoreList[i]);
        }

    } else {
        // Just use initial results
        for (int i=0;i<finalResultSize;i++) {
            finalResultList.append(pairList[i]);
        }
    }

    qDebug() << "==============================================================";

    for (int i=0;i<finalResultSize;i++) {
        int position=i+1;
        QPair<V3DLONG, int> p=finalResultList[i];
        V3DLONG score=p.first;
        int index=p.second;
        QString filename=indexFileList[index];
        qDebug() << position << ". " << score << " : " << filename;
    }

    return true;
}



V3DLONG VolumePatternIndex::computeTotalBytesFromIndexTotal(V3DLONG indexTotal) {
    V3DLONG result=ceil((indexTotal*1.0)/4.0);
    return result;
}

V3DLONG VolumePatternIndex::calculateIndexScore(unsigned char* queryIndex, unsigned char* subjectIndex, V3DLONG indexTotal)
{
    V3DLONG score=0L;
    V3DLONG initialTotal=indexTotal/4;

    unsigned char oooooo11 = 3;
    unsigned char s1=0;
    unsigned char q1=0;
    int scorePosition=0;

    for (V3DLONG i=0;i<initialTotal;i=i+4) {
        unsigned char s=*subjectIndex;
        unsigned char q=*queryIndex;

        for (int p=0;p<4;p++) {
            s1= s&oooooo11;
            q1= q&oooooo11;

            scorePosition = q1*4+s1;
            score+=matrix[scorePosition];

            s >>= 2;
            q >>= 2;
        }
        subjectIndex++;
        queryIndex++;
    }

    for (V3DLONG i=initialTotal;i<indexTotal;) {
        unsigned char s=*subjectIndex;
        unsigned char q=*queryIndex;

        for (int p=0;p<4;p++) {
            if (i<indexTotal) {
                s1= s&oooooo11;
                q1= q&oooooo11;

                scorePosition = q1*4+s1;
                score+=matrix[scorePosition];

                s >>= 2;
                q >>= 2;
            }
            i++;
        }
        subjectIndex++;
        queryIndex++;
    }

    return score;
}

unsigned char* VolumePatternIndex::indexImage(My4DImage* image, int channel, V3DLONG* subregion, unsigned char* indexData)
{
    My4DImage* cubifiedImage=AnalysisTools::cubifyImageByChannel(image, channel, unitSize, AnalysisTools::CUBIFY_TYPE_AVERAGE, subregion);
    if (iXmax==-1) {
        iXmax=cubifiedImage->getXDim();
        iYmax=cubifiedImage->getYDim();
        iZmax=cubifiedImage->getZDim();
    } else {
        if (cubifiedImage->getXDim()!=iXmax ||
                cubifiedImage->getYDim()!=iYmax ||
                cubifiedImage->getZDim()!=iZmax) {
            qDebug() << "Cubified image dimensions do not match";
            return false;
        }
    }
    // Next, we will walk through the image and gradually populate a binary structure with the scores.
    V3DLONG cubifiedTotal=iXmax*iYmax*iZmax;
    // We can store 4 2-bit values per 8-bit byte, so these are the number of bytes we need:
    V3DLONG tempIndexTotalBytes=computeTotalBytesFromIndexTotal(cubifiedTotal);
    if (indexTotalBytes==-1L) {
        indexTotalBytes=tempIndexTotalBytes;
    } else {
        if (indexTotalBytes!=tempIndexTotalBytes) {
            qDebug() << "New value for indexTotalBytes does not match: " << tempIndexTotalBytes << " vs original=" << indexTotalBytes;
            return 0;
        }
    }
    if (indexData==0L) {
        indexData=new unsigned char[indexTotalBytes];
    }
    for (int j=0;j<indexTotalBytes;j++) {
        indexData[j]=0;
    }
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
    return indexData;
}

bool VolumePatternIndex::openIndexAndReadHeader()
{
    fid=fopen(indexFilePath.toAscii().data(), "rb");
    if (!fid) {
        qDebug() << "Could not open file=" << indexFilePath << " to read";
        return false;
    }

    // 1. Image Size
    fread(&x0, sizeof(int), 1, fid);
    fread(&x1, sizeof(int), 1, fid);

    fread(&y0, sizeof(int), 1, fid);
    fread(&y1, sizeof(int), 1, fid);

    fread(&z0, sizeof(int), 1, fid);
    fread(&z1, sizeof(int), 1, fid);

    // 2. Unit Size
    fread(&unitSize, sizeof(int), 1, fid);

    // 3. Threshold
    int thresholdNumber=0;
    fread(&thresholdNumber, sizeof(int), 1, fid);
    if (thresholdNumber!=3) {
        qDebug() << "Only 3 threshold levels currently supported";
        fclose(fid);
        return false;
    }
    threshold=new int[3];
    for (int i=0;i<3;i++) {
        fread(threshold+i, sizeof(int), 1, fid);
    }

    // 4. Cube Size
    fread(&iXmax, sizeof(int), 1, fid);
    fread(&iYmax, sizeof(int), 1, fid);
    fread(&iZmax, sizeof(int), 1, fid);

    V3DLONG indexTotal=iXmax*iYmax*iZmax;
    indexTotalBytes=computeTotalBytesFromIndexTotal(indexTotal);

    // 5. Num Index Files
    int numIndexFiles;
    fread(&numIndexFiles, sizeof(int), 1, fid);

    // 6. Index File Paths
    char* filenameBuffer = new char[FILENAME_BUFFER_SIZE];
    for (int i=0;i<numIndexFiles;i++) {
        int filenameSize;
        fread(&filenameSize, sizeof(int), 1, fid);
        if (filenameSize>FILENAME_BUFFER_SIZE) {
            qDebug() << "Exceeded max filename buffer size";
            fclose(fid);
            return false;
        }
        fread(filenameBuffer, sizeof(char), filenameSize, fid);
        QString filename(filenameBuffer);
        indexFileList.append(filename);
        int channelIndex;
        fread(&channelIndex, sizeof(int), 1, fid);
        indexChannelList.append(channelIndex);
    }
    return true;
}

bool VolumePatternIndex::calculateImageScore(My4DImage* queryImage, My4DImage* subjectImage, int subjectChannel, V3DLONG* score)
{
    V3DLONG xSize=qx1-qx0;
    if (xSize!=(x1-x0)) {
        qDebug() << "q and s xSize not compatible: " << qx0 << " " << qx1 << " " << x0 << " " << x1;
        return false;
    }
    V3DLONG ySize=qy1-qy0;
    if (ySize!=(y1-y0)) {
        qDebug() << "q and s ySize not compatible: " << qy0 << " " << qy1 << " " << y0 << " " << y1;
        return false;
    }
    V3DLONG zSize=qz1-qz0;
    if (zSize!=(z1-z0)) {
        qDebug() << "q and s zSize not compatible: " << qz0 << " " << qz1 << " " << z0 << " " << z1;
        return false;
    }

    V3DLONG ox=qx1-x1;
    V3DLONG oy=qy1-y1;
    V3DLONG oz=qz1-z1;

    V3DLONG localScore=0L;

    v3d_uint8* subjectData=subjectImage->getRawDataAtChannel(subjectChannel);
    v3d_uint8* queryData=queryImage->getRawDataAtChannel(queryChannel);

    for (V3DLONG z=qz0;z<qz1;z++) {
        V3DLONG q_zoffset=z*ySize*xSize;
        V3DLONG s_zoffset=(z-oz)*ySize*xSize;
        for (V3DLONG y=qy0;y<qy1;y++) {
            V3DLONG q_yoffset=q_zoffset+y*xSize;
            V3DLONG s_yoffset=s_zoffset+(y-oy)*xSize;
            for (V3DLONG x=qx0;x<qx1;x++) {
                V3DLONG q_offset=q_yoffset+x;
                V3DLONG s_offset=s_yoffset+(x-ox);

                v3d_uint8 queryValue=queryData[q_offset];
                v3d_uint8 subjectValue=subjectData[s_offset];

                int sPosition=3;
                if (subjectValue<threshold[0]) {
                    sPosition=0;
                } else if (subjectValue<threshold[1]) {
                    sPosition=1;
                } else if (subjectValue<threshold[2]) {
                    sPosition=2;
                } // then implicitly=3

                int qPosition=3;
                if (queryValue<threshold[0]) {
                    qPosition=0;
                } else if (queryValue<threshold[1]) {
                    qPosition=1;
                } else if (queryValue<threshold[2]) {
                    qPosition=2;
                } // then implicitly=3

                int matrixPosition=qPosition*4+sPosition;
                localScore+=matrix[matrixPosition];
            }
        }
    }
    *score=localScore;
    return true;
}





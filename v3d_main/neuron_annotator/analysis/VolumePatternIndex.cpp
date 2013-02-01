
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


       empty        0        -1        -4      -16

Qry    low         -1         1        -1       -8

       medium      -4        -1         8        4

       high       -16        -8         4       32


 */



#include "VolumePatternIndex.h"
#include "../utility/ImageLoader.h"

const int VolumePatternIndex::FILENAME_BUFFER_SIZE = 2000;

const int VolumePatternIndex::DEFAULT_UNIT_SIZE = 10;
const int VolumePatternIndex::DEFAULT_THRESHOLD_A = 7;
const int VolumePatternIndex::DEFAULT_THRESHOLD_B = 20;
const int VolumePatternIndex::DEFAULT_THRESHOLD_C = 50;
const int VolumePatternIndex::DEFAULT_MAX_HITS = 100;
const QString VolumePatternIndex::DEFAULT_MATRIX_STRING("      0 -1 -4 -16       -1 1 -1 -8       -4 -1 8 4     -16 -8 4 32 ");
const QString VolumePatternIndex::DEFAULT_FULL_MATRIX_STRING(" 0  0 -4 -16        0 0 -1 -8       -8 -1 8 4     -32 -8 4 32 ");

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
    queryIndex=0L;
    indexTotalBytes=-1L;
    matrix=0L;
    fullmatrix=0L;
    unitSize=DEFAULT_UNIT_SIZE;
    threshold=0L;
    maxHits=DEFAULT_MAX_HITS;
    DEBUG_FLAG=false;
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
                return 1;
            }
        } else if (arg=="-unitSize") {
            QString unitSizeString=(*argList)[++i];
            unitSize=unitSizeString.toInt();
        } else if (arg=="-threshold") {
            QString thresholdString=(*argList)[++i];
            if (!parseThresholdString(thresholdString)) {
                qDebug() << "Could not parse threshold string=" << thresholdString;
                return 1;
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
        } else if (arg=="-fullmatrix") {
            QString fullmatrixString=(*argList)[++i];
            if (!parseFullMatrixString(fullmatrixString)) {
                qDebug() << "Could not parse full matrix string=" << fullmatrixString;
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
                return 1;
            }
        }
    }
    if (matrix==0L) {
        if (!parseMatrixString(DEFAULT_MATRIX_STRING)) {
            qDebug() << "Could not parse default matrix string=" << DEFAULT_MATRIX_STRING;
            return 1;
        }
    }
    if (fullmatrix==0L) {
        if (!parseFullMatrixString(DEFAULT_FULL_MATRIX_STRING)) {
            qDebug() << "Could not parse default full matrix string=" << DEFAULT_FULL_MATRIX_STRING;
            return 1;
        }
    }
    if (threshold==0L) {
        threshold=new int[3];
        threshold[0]=DEFAULT_THRESHOLD_A;
        threshold[1]=DEFAULT_THRESHOLD_B;
        threshold[2]=DEFAULT_THRESHOLD_C;
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
    return true;
}

bool VolumePatternIndex::parseMatrixString(QString matrixString) {
    QRegExp splitRegex("\\s+");
    QStringList mList=matrixString.trimmed().split(splitRegex);
    if (mList.size()!=16) {
        for (int i=0;i<mList.size();i++) {
            qDebug() << "i=" << i << " string=" << mList[i];
        }
        return false;
    }
    matrix=new int[16];
    for (int i=0;i<16;i++) {
        matrix[i]=mList[i].toInt();
        qDebug() << "Using matrix " << i << " = " << matrix[i];
    }
    return true;
}

bool VolumePatternIndex::parseFullMatrixString(QString matrixString) {
    QRegExp splitRegex("\\s+");
    QStringList mList=matrixString.trimmed().split(splitRegex);
    if (mList.size()!=16) {
        for (int i=0;i<mList.size();i++) {
            qDebug() << "i=" << i << " string=" << mList[i];
        }
        return false;
    }
    fullmatrix=new int[16];
    for (int i=0;i<16;i++) {
        fullmatrix[i]=mList[i].toInt();
        qDebug() << "Using full matrix " << i << " = " << fullmatrix[i];
    }
    return true;
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

void VolumePatternIndex::formatQuerySubregion(V3DLONG* subregion)
{
    subregion[0]=qx0;
    subregion[1]=qx1;
    subregion[2]=qy0;
    subregion[3]=qy1;
    subregion[4]=qz0;
    subregion[5]=qz1;
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
    indexData=0L;
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
        indexImage(&sourceImage, indexChannelList[fileIndex], subregion);
        if (indexData==0L) {
            qDebug() << "Error with indexImage";
            return false;
        }
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

    qDebug() << "Writing header file...";

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

    qDebug() << "x0=" << x0 << " x1=" << x1 << " y0=" << y0 << " y1=" << y1 << " z0=" << z0 << " z1=" << z1;

    fwrite(&x0, sizeof(V3DLONG), 1, fid);
    fwrite(&x1, sizeof(V3DLONG), 1, fid);

    fwrite(&y0, sizeof(V3DLONG), 1, fid);
    fwrite(&y1, sizeof(V3DLONG), 1, fid);

    fwrite(&z0, sizeof(V3DLONG), 1, fid);
    fwrite(&z1, sizeof(V3DLONG), 1, fid);

    // 2. Unit Size

    fwrite(&unitSize, sizeof(int), 1, fid);

    // 3. Threshold

    int thresholdNumber=3;
    fwrite(&thresholdNumber, sizeof(int), 1, fid);
    for (int i=0;i<3;i++) {
        fwrite(threshold+i, sizeof(int), 1, fid);
    }

    // 4. Cube Size

    fwrite(&iXmax, sizeof(V3DLONG), 1, fid);
    fwrite(&iYmax, sizeof(V3DLONG), 1, fid);
    fwrite(&iZmax, sizeof(V3DLONG), 1, fid);

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
    queryImage=new My4DImage();
    if (!queryLoader.loadImage(queryImage, queryImageFilePath)) {
        qDebug() << "Could not load query image file=" << queryImageFilePath;
        return false;
    }

    qDebug() << "Convert query to index format...";
    V3DLONG* subregion=0L;
    if (qx0==-1L) {
        qx0=0;
        qx1=queryImage->getXDim();
        qy0=0;
        qy1=queryImage->getYDim();
        qz0=0;
        qz1=queryImage->getZDim();
    }
    subregion=new V3DLONG[6];
    formatQuerySubregion(subregion);
    indexData=0L;
    indexImage(queryImage, queryChannel, subregion);
    if (indexData==0) {
        qDebug() << "Error with indexImage";
        return false;
    }

    V3DLONG nonZeroCount=0L;
    for (int j=0;j<indexTotalBytes;j++) {
        if (indexData[j]>0) {
            nonZeroCount++;
        }
    }
    qDebug() << "Non-zero query indexData count=" << nonZeroCount << " of total=" << indexTotalBytes;

    queryIndex=new unsigned char [indexTotalBytes];

    for (int m=0;m<indexTotalBytes;m++) {
        queryIndex[m]=indexData[m];
        indexData[m]=0;
    }

//    for (int i=0;i<indexTotalBytes;i++) {
//        qDebug() << "queryCheck " << i << " : " << queryIndex[i];
//    }

    qDebug() << "Begin scoring (index phase)...";
    V3DLONG indexTotal=iXmax*iYmax*iZmax;
    for (int i=0;i<indexFileList.size();i++) {
        size_t readSize=0;
        if ( (readSize=fread(indexData, 1, indexTotalBytes, fid))!=indexTotalBytes ) {
            qDebug() << "Could not read full block of " << indexTotalBytes << " , read " << readSize << " instead";
            return false;
        }
        qDebug() << "Calculating index score for " << indexFileList[i];
        QString filename=indexFileList[i];
        if (filename.contains("20111102102342562")) {
            DEBUG_FLAG=true;
        } else {
            DEBUG_FLAG=false;
        }
        V3DLONG score=calculateIndexScore(queryIndex, indexData, indexTotal);
        //qDebug() << i << ". score=" << score;
        indexScoreList.append(score);
    }

    qDebug() << "Begin sorting...";

    QList< QPair<V3DLONG, int> > pairList;

    qDebug() << "indexScoreList size=" << indexScoreList.size();

    for (int i=0;i<indexScoreList.size();i++) {
        pairList.append(qMakePair(indexScoreList[i], i));
    }

    qSort(pairList.begin(), pairList.end(), VolumePatternIndex::compareScores);

    qDebug() << "pairList size=" << pairList.size();

    int finalResultSize=maxHits;
    if (finalResultSize>pairList.size()) {
        finalResultSize=pairList.size();
    }

    QList< QPair<V3DLONG, int> > finalResultList;

    if (fullSearch) {

        qDebug() << "Doing full search";

        for (int i=0;i<finalResultSize;i++) {
            int p = pairList[i].second;
            qDebug() << "Using " << i << ". " << indexFileList[p];
        }


        QList< QPair<V3DLONG, int> > fullScoreList;

        // Take the top maxHits and compute the full-resolution scores
        for (int i=0;i<finalResultSize;i++) {
            QPair<V3DLONG, int> p=pairList[i];
            int index=p.second;
            QString filename=indexFileList[index];
            ImageLoader loader;
            My4DImage* currentImage=new My4DImage();
            qDebug() << "Full search - loading " << filename << "...";
            loader.loadImage(currentImage, filename);
            qDebug() << "Scoring...";
            V3DLONG score;
            int subjectChannel=indexChannelList[index];
            if (!calculateImageScore(queryImage, currentImage, subjectChannel, &score)) {
                qDebug() << "Error calculating score for " << filename;
                return false;
            }
            fullScoreList.append(qMakePair(score, index));
            delete currentImage;
        }

        qSort(fullScoreList.begin(), fullScoreList.end(), VolumePatternIndex::compareScores);

        for (int i=0;i<finalResultSize;i++) {
            finalResultList.append(fullScoreList[i]);
        }

    } else {

        qDebug() << "Using initial results, finalResultSize=" << finalResultSize;

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

    if (indexData!=0L) { delete [] indexData; indexData=0L; }
    if (queryIndex!=0L) { delete [] queryIndex; queryIndex=0L; }

    return true;
}



V3DLONG VolumePatternIndex::computeTotalBytesFromIndexTotal(V3DLONG indexTotal) {
    V3DLONG result=ceil((indexTotal*1.0)/4.0);
    return result;
}

V3DLONG VolumePatternIndex::calculateIndexScore(unsigned char* queryIndex, unsigned char* subjectIndex, V3DLONG indexTotal)
{
    V3DLONG score=0L;
    V3DLONG byteTotalFloor=indexTotal/4;
    V3DLONG byteTotal=computeTotalBytesFromIndexTotal(indexTotal);

    unsigned char oooooo11 = 3;
    unsigned char s1=0;
    unsigned char q1=0;
    int scorePosition=0;

    V3DLONG* matrixBins = new V3DLONG[16];
    V3DLONG* matrixScores = new V3DLONG[16];

    for (int i=0;i<16;i++) {
        matrixBins[i]=0L;
        matrixScores[i]=0L;
    }

//    qDebug() << "calculateIndexScore start";

    V3DLONG position=0L;
    for (V3DLONG i=0;i<byteTotalFloor;i++) {
        unsigned char s=subjectIndex[i];
        unsigned char q=queryIndex[i];

 //       qDebug() << "i=" << i;

        for (int p=0;p<4;p++) {

 //           qDebug() << "p=" << p;

            s1= s&oooooo11;
            q1= q&oooooo11;

            scorePosition = q1*4+s1;

            if (DEBUG_FLAG) {
               qDebug() << " i=" << i << " p=" << p << " s=" << s << " q=" << q << " s1=" << s1 << " q1=" << q1 << " scorePosition=" << scorePosition;
            }

//            int scoreIncrement=matrix[scorePosition];

//            if (scoreIncrement>2) {
//                qDebug() << "score=" << scoreIncrement << " position=" << scorePosition;
//            }

            score+=matrix[scorePosition];

            matrixBins[scorePosition] += 1;
            matrixScores[scorePosition] += matrix[scorePosition];

            s >>= 2;
            q >>= 2;

            position+=4;
        }
    }

    for (V3DLONG i=byteTotalFloor;i<byteTotal;i++) {
        unsigned char s=subjectIndex[i];
        unsigned char q=queryIndex[i];

        for (int p=0;p<4;p++) {
            if (position<indexTotal) {
                s1= s&oooooo11;
                q1= q&oooooo11;

                scorePosition = q1*4+s1;

                if (DEBUG_FLAG) {
                   qDebug() << " i=" << i << " p=" << p << " s=" << s << " q=" << q << " s1=" << s1 << " q1=" << q1 << " scorePosition=" << scorePosition;
                }

                score+=matrix[scorePosition];

                matrixBins[scorePosition] += 1;
                matrixScores[scorePosition] += matrix[scorePosition];

                s >>= 2;
                q >>= 2;

            }
            position++;
        }
    }

//    qDebug() << "Returning score=" << score;

    qDebug() << "Bins:";

    qDebug() << matrixBins[0] << " " << matrixBins[1] << " " << matrixBins[2] << " " << matrixBins[3];
    qDebug() << matrixBins[4] << " " << matrixBins[5] << " " << matrixBins[6] << " " << matrixBins[7];
    qDebug() << matrixBins[8] << " " << matrixBins[9] << " " << matrixBins[10] << " " << matrixBins[11];
    qDebug() << matrixBins[12] << " " << matrixBins[13] << " " << matrixBins[14] << " " << matrixBins[15];

    qDebug() << "Scores:";

    qDebug() << matrixScores[0] << " " << matrixScores[1] << " " << matrixScores[2] << " " << matrixScores[3];
    qDebug() << matrixScores[4] << " " << matrixScores[5] << " " << matrixScores[6] << " " << matrixScores[7];
    qDebug() << matrixScores[8] << " " << matrixScores[9] << " " << matrixScores[10] << " " << matrixScores[11];
    qDebug() << matrixScores[12] << " " << matrixScores[13] << " " << matrixScores[14] << " " << matrixScores[15];

    delete [] matrixBins;
    delete [] matrixScores;

    return score;
}

void VolumePatternIndex::indexImage(My4DImage* image, int channel, V3DLONG* subregion)
{

    qDebug() << "indexImage : using threshold values " << threshold[0] << " " << threshold[1] << " " << threshold[2];

    V3DLONG count0=0L;
    V3DLONG count1=0L;
    V3DLONG count2=0L;
    V3DLONG count3=0L;

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
            indexData=0L;
            return;
        }
    }

//    ImageLoader debugLoader;
//    debugLoader.saveImage(cubifiedImage, "cubifiedDebugImage.v3dpbd");

    // Next, we will walk through the image and gradually populate a binary structure with the scores.
    V3DLONG cubifiedTotal=iXmax*iYmax*iZmax;
    // We can store 4 2-bit values per 8-bit byte, so these are the number of bytes we need:
    V3DLONG tempIndexTotalBytes=computeTotalBytesFromIndexTotal(cubifiedTotal);
    if (indexTotalBytes==-1L) {
        indexTotalBytes=tempIndexTotalBytes;
    } else {
        if (indexTotalBytes!=tempIndexTotalBytes) {
            qDebug() << "New value for indexTotalBytes does not match: " << tempIndexTotalBytes << " vs original=" << indexTotalBytes;
            indexData=0L;
        }
    }
    if (indexData==0L) {
        qDebug() << "Allocating indexData of size indexTotalBytes=" << indexTotalBytes;
        indexData=new unsigned char[indexTotalBytes];
    }
    for (int j=0;j<indexTotalBytes;j++) {
        indexData[j]=0;
    }
    V3DLONG indexPosition=0;
    int bytePosition=0;
    unsigned char cvalue=0;

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
                    indexData[indexPosition]=cvalue;
                    cvalue=0;
                    indexPosition++;
                    if (indexPosition>=indexTotalBytes) {
                        qDebug() << "Exceeded indexTotalBytes at z=" << z << " y=" << y << " x=" << x;
                        indexData=0L;
                        return;
                    }
                }

                // 0=00000000
                // 1=00000001
                // 2=00000010
                // 3=00000011
                if (value<threshold[0]) {
                    // leave as zero
                    count0++;
                } else if (value<threshold[1]) {
                    cvalue |= 1;
                    count1++;
                } else if (value<threshold[2]) {
                    cvalue |= 2;
                    count2++;
                } else {
                    cvalue |= 3;
                    count3++;
                }
                if (bytePosition<3) {
                    cvalue <<= 2;
                }
                bytePosition++;
            }
        }
    }

    // Finish last case
    if (bytePosition!=4) {
        if (bytePosition==1) {
            cvalue <<=6;
        } else if (bytePosition==2) {
            cvalue <<=4;
        } else if (bytePosition==3) {
            cvalue <<=2;
        }
        indexData[indexPosition]=cvalue;
    }

    qDebug() << "Voxel counts: 0=" << count0 << " 1=" << count1 << " 2=" << count2 << " 3=" << count3;
}

bool VolumePatternIndex::openIndexAndReadHeader()
{
    qDebug() << "Reading header file...";
    fid=fopen(indexFilePath.toAscii().data(), "rb");
    if (!fid) {
        qDebug() << "Could not open file=" << indexFilePath << " to read";
        return false;
    }

    // 1. Image Size
    fread(&x0, sizeof(V3DLONG), 1, fid);
    fread(&x1, sizeof(V3DLONG), 1, fid);

    fread(&y0, sizeof(V3DLONG), 1, fid);
    fread(&y1, sizeof(V3DLONG), 1, fid);

    fread(&z0, sizeof(V3DLONG), 1, fid);
    fread(&z1, sizeof(V3DLONG), 1, fid);

    qDebug() << "x0=" << x0 << " x1=" << x1 << " y0=" << y0 << " y1=" << y1 << " z0=" << z0 << " z1=" << z1;

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
    fread(&iXmax, sizeof(V3DLONG), 1, fid);
    fread(&iYmax, sizeof(V3DLONG), 1, fid);
    fread(&iZmax, sizeof(V3DLONG), 1, fid);

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

//    qDebug() << "calculateImageScore : qx0=" << qx0 << " qx1=" << qx1 << " qy0=" << qy0 << " qy1= " << qy1 << " qz0=" << qz0 << " qz1=" << qz1;
//    qDebug() << "   xSize=" << xSize << " ySize= " << ySize << " zSize= " << zSize;

    V3DLONG localScore=0L;

    V3DLONG* matrixBins=new V3DLONG[16];
    V3DLONG* matrixScores=new V3DLONG[16];
    for (int m=0;m<16;m++) {
        matrixBins[m]=0L;
        matrixScores[m]=0L;
    }

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
                matrixBins[matrixPosition] += 1;
                int mScore=fullmatrix[matrixPosition];
                matrixScores[matrixPosition] += mScore;
                localScore += mScore;
            }
        }
    }
    *score=localScore;
    qDebug() << "Local score=" << localScore;
    for (int m=0;m<16;m++) {
        qDebug() << m << "=" << matrixBins[m] << "  " << matrixScores[m];
    }
    delete [] matrixBins;
    delete [] matrixScores;
    return true;
}





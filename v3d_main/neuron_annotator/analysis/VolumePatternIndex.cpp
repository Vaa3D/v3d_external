
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

  The input 3D mask is sampled into search dimensions, and scored against each
  index entry.

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
const int VolumePatternIndex::ID_BUFFER_SIZE = 1000;

const int VolumePatternIndex::DEFAULT_UNIT_SIZE = 10;
const int VolumePatternIndex::DEFAULT_THRESHOLD_A = 7;
const int VolumePatternIndex::DEFAULT_THRESHOLD_B = 20;
const int VolumePatternIndex::DEFAULT_THRESHOLD_C = 50;
const double VolumePatternIndex::DEFAULT_MIN_SCORE = -1000000.0;
const int VolumePatternIndex::DEFAULT_MAX_HITS = 100;
const int VolumePatternIndex::DEFAULT_BINARY_PROXY_VALUE = 255;
const QString VolumePatternIndex::DEFAULT_MATRIX_STRING("      0 -1 -4 -16       -1 1 -1 -8       -4 -1 8 4     -16 -8 4 32 ");

const int VolumePatternIndex::MODE_UNDEFINED=-1;
const int VolumePatternIndex::MODE_INDEX=0;
const int VolumePatternIndex::MODE_SEARCH=1;
const int VolumePatternIndex::MODE_APPEND=2;

const int VolumePatternIndex::INDEX_STATE_CREATE=0;
const int VolumePatternIndex::INDEX_STATE_APPEND=1;

VolumePatternIndex::VolumePatternIndex()
{
    fid=0;
    mode=MODE_UNDEFINED;
    defaultChannelToIndex=-1;
    queryChannel=-1;
    iXmax=-1;
    iYmax=-1;
    iZmax=-1;
    x0=x1=y0=y1=z0=z1=-1;
    qx0=qx1=qy0=qy1=qz0=qz1=-1;
    indexData=0L;
    queryIndex=0L;
    queryIndexSkipPositions=0L;
    indexTotalBytes=-1L;
    matrix=0L;
    unitSize=DEFAULT_UNIT_SIZE;
    threshold=0L;
    minScore=DEFAULT_MIN_SCORE;
    maxHits=DEFAULT_MAX_HITS;
    DEBUG_FLAG=false;
    skipzeros=false;
    indexState=INDEX_STATE_CREATE;
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
    } else if (mode==MODE_APPEND) {
      return appendIndex();
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
        } else if (arg=="-outputFile") {
            outputFilePath=(*argList)[++i];
        } else if (arg=="-maxHits") {
            QString maxHitsString=(*argList)[++i];
            maxHits=maxHitsString.toInt();
        } else if (arg=="-minScore") {
            QString minScoreString=(*argList)[++i];
            minScore=minScoreString.toDouble();
        } else if (arg=="-skipzeros") {
            skipzeros=true;
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
        } else if (modeString=="append") {
	  mode=MODE_APPEND;
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
            if (outputFilePath.length()<1) {
                qDebug() << "outputFile must be defined";
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
    if (threshold==0L) {
        threshold=new int[3];
        threshold[0]=DEFAULT_THRESHOLD_A;
        threshold[1]=DEFAULT_THRESHOLD_B;
        threshold[2]=DEFAULT_THRESHOLD_C;
    }
    return 0;
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

bool VolumePatternIndex::populateIndexFileList() {
    QFile indexFileListFile(inputFileListPath);
    if (!indexFileListFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open file=" << inputFileListPath << " to read";
        return false;
    }
    while(!indexFileListFile.atEnd()) {
        QString line=indexFileListFile.readLine();
	QStringList indexInfoList=parseIndexFileLine(line);
	if (indexInfoList.size()==3) {

	  qDebug() << "indexInfoList 0=" << indexInfoList[0] << " 1=" << indexInfoList[1] << " 2=" << indexInfoList[2];

	  // Check for repeats
	  QString idQString=indexInfoList[0];
	  if (idPathHash.contains(idQString)) {
	    qDebug() << "Encountered repeat of hash entry=" << idQString;
	    return false;
	  }
	  indexIdList.append(idQString);
	  indexChannelList.append(indexInfoList[2].toInt());
	  QString filePath=indexInfoList[1];
	  QFileInfo fileInfo(filePath);
	  if (!fileInfo.exists()) {
	    qDebug() << "Could not verify that file=" << filePath << " exists";
	    return false;
	  }
	  qDebug() << "Verified " << filePath << " exists";
	  indexFileList.append(filePath);
	  idPathHash[idQString]=filePath;
	} else if (indexInfoList.size() > 0) {
	  qDebug() << "Could not process information from line=" << line;
	  return false;
	}
    }
    indexFileListFile.close();
    return true;
}

// This method returns a list with 3 values { id, path, channel }, or an empty list if the line is empty
QStringList VolumePatternIndex::parseIndexFileLine(QString line)
{
  QStringList resultList;
  qDebug() << "Parsing index file line=" << line;
  line=line.trimmed();
  if (line.length()==0) {
    return resultList; // empty
  }
  int channel=-1;
  QRegExp splitRegex("\\s+");
  QStringList fList=line.split(splitRegex);
  for (int i=0;i<fList.size();i++) {
    qDebug() << "line " << i << "=" << fList[i];
  }
  QString id;
  QString filePath;
  if (fList.size()==1) {
    return resultList; // empty
  }
  id=fList[0];
  filePath=fList[1];
  if (fList.size()>2) {
    channel=fList[2].toInt();
    if (channel<0 || channel>1000) {
      channel=defaultChannelToIndex;
    }
  } else {
    channel=defaultChannelToIndex;
  }
  resultList.append(id);
  resultList.append(filePath);
  QString channelString=QString::number(channel);
  resultList.append(channelString);
  return resultList;
}

bool VolumePatternIndex::appendIndex()
{

  // Validate index file path - these are the new files to be added
  QFileInfo indexFileInfo(indexFilePath);
  if (!indexFileInfo.exists()) {
    qDebug() << "Could not find pre-existing index file to append, " << indexFilePath;
    return false;
  }

  // Get dimensional info from current index, and then move to end to add new entries
  if (!openIndexAndReadHeader()) {
    qDebug() << "Could not open and read header of file=" << indexFilePath;
    return false;
  }

  // Skip through index file and populate id/path hash
  int idLength=0;
  int pathLength=0;
  char* filePathBuffer=new char[FILENAME_BUFFER_SIZE];
  char* idBuffer=new char[ID_BUFFER_SIZE];

  size_t readSize=0;
  while( (readSize=fread(&idLength, 1, sizeof(int), fid))==sizeof(int) ) {
    if (readSize>=ID_BUFFER_SIZE) {
      qDebug() << "idLength exceeds ID_BUFFER_SIZE";
      return false;
    }
    if ( (readSize=fread(idBuffer, 1, idLength, fid))!=idLength) {
      fclose(fid);
      qDebug() << "Unexpectedly could not read path from index file";
      return false;
    }
    if ( !(readSize=fread(&pathLength, 1, sizeof(int), fid))==sizeof(int) ) {
      qDebug() << "pathLength could not be read";
      return false;
    }

    if (pathLength>=FILENAME_BUFFER_SIZE) {
      qDebug() << "pathLength exceeds FILENAME_BUFFER_SIZE";
      return false;
    }
    if ( (readSize=fread(filePathBuffer, 1, pathLength, fid))!=pathLength) {
      fclose(fid);
      qDebug() << "Unexpectedly could not read path from index file";
      return false;
    }
    idBuffer[idLength]='\0';
    QString idQString=QString(idBuffer);
    filePathBuffer[pathLength]='\0';
    QString pathQString=QString(filePathBuffer);
    idPathHash[idQString]=pathQString;

    // Read the channel size
    int channelSize;
    if ( (readSize=fread(&channelSize, 1, sizeof(int), fid))!=sizeof(int) ) {
      fclose(fid);
      qDebug() << "Unexpectedly could not channel size for entry";
      return false;
    }

    // Next, advance the file pointer past the index data
    if (fseek(fid, indexTotalBytes, SEEK_CUR)!=0) {
      qDebug() << "Could not advance file pointer past index data";
      return false;
    }
  }

  // Now, close the index file for reading, and re-open for writing, in preparation
  // for adding new entries.

  fclose(fid);
  fid=0L;

  QByteArray ba = indexFilePath.toUtf8();
  const char* filepath = ba.constData();

  fid=fopen(filepath, "ab");
  if (!fid) {
    qDebug() << "Could not open file=" << indexFilePath << " to append";
    return false;
  }

  indexState=INDEX_STATE_APPEND;
  return addEntriesToIndex();

}

bool VolumePatternIndex::createIndex()
{
  indexState=INDEX_STATE_CREATE;
  return addEntriesToIndex();
}


bool VolumePatternIndex::addEntriesToIndex()
{
    qDebug() << "addEntriesToIndex() start";

    if ( !populateIndexFileList() )
    {
        qDebug() << "populateIndexFileList failed";
        return false;
    }
    if ( indexFileList.size() < 1 )
    {
        qDebug() << "No files to index";
        return false;
    }
    int fileIndex = 0;
    indexData = 0L;
    char* filePathBuffer = new char[FILENAME_BUFFER_SIZE];
    char* idBuffer = new char[ID_BUFFER_SIZE];
    for ( fileIndex = 0; fileIndex < indexFileList.size(); fileIndex++ )
    {
        qDebug() << "Processing file " << fileIndex;
        My4DImage* sourceImage = 0L;
        ImageLoader loader;
        QString filePathToLoad = indexFileList[fileIndex];
        qDebug() << "Starting with file=" << filePathToLoad;
        if ( filePathToLoad.endsWith( ".mask" ) )
        {
            MaskChan mc;
            QStringList ql;
            ql.append( filePathToLoad );
            sourceImage = mc.createImageFromMaskFiles( ql );
        }
        else
        {
            sourceImage = new My4DImage();
            if ( !loader.loadImage( sourceImage, filePathToLoad ) )
            {
                qDebug() << "Could not load file=" << filePathToLoad;
                return false;
            }
        }
        qDebug() << "Indexing " << fileIndex << " of " << indexFileList.size() << " : " << filePathToLoad << "...";
        // Note: if indexState==INDEX_STATE_APPEND, then we assume the dimensions have been read by the index read step previously
        if ( indexState == INDEX_STATE_CREATE && ( fileIndex == 0 && x0 == -1 ) )
        {
            // Then assume we are to use the first image to set the selection region
            x0 = 0; x1 = sourceImage->getXDim();
            y0 = 0; y1 = sourceImage->getYDim();
            z0 = 0; z1 = sourceImage->getZDim();
        }
        indexImage( sourceImage, indexChannelList[fileIndex], false );
        if ( indexData == 0L )
        {
            qDebug() << "Error with indexImage";
            return false;
        }
        if ( fileIndex == 0 && indexState == INDEX_STATE_CREATE )
        {
            if ( !openIndexAndWriteHeader() )
            {
                qDebug() << "Could not open index file and write header";
                return false;
            }
        }

        // Write id, path, and channel for this entry before index data
        QString indexId = indexIdList[fileIndex];
        int length = indexId.length();

        if ( length > ID_BUFFER_SIZE )
        {
            qDebug() << "Id buffer is not large enough for=" << indexId;
            return false;
        }
        fwrite( &length, sizeof( int ), 1, fid );
        QByteArray ba = indexId.toUtf8();
        const char* index_id = ba.constData();

        strcpy( idBuffer, index_id );
        fwrite( idBuffer, sizeof( char ), length, fid );

        length = filePathToLoad.length();
        if ( length > FILENAME_BUFFER_SIZE )
        {
            qDebug() << "File path length name buffer not large enough for=" << filePathToLoad;
            return false;
        }
        fwrite( &length, sizeof( int ), 1, fid );
        ba = filePathToLoad.toUtf8();
        const char* filepath = ba.constData();
        strcpy( filePathBuffer, filepath );
        fwrite( filePathBuffer, sizeof( char ), length, fid );
        int channelIndex = indexChannelList[fileIndex];
        fwrite( &channelIndex, sizeof( int ), 1, fid );

        // Next, write the index data
        fwrite( indexData, sizeof( unsigned char ), indexTotalBytes, fid );

        qDebug() << "Cleaning up";
        delete sourceImage;
    }
    if ( indexData != 0 )
    {
        delete [] indexData;
    }
    delete [] filePathBuffer;
    fclose( fid );
    fid = 0;
    return true;
}

bool VolumePatternIndex::openIndexAndWriteHeader() {

    qDebug() << "Writing header file...";

    QByteArray ba = indexFilePath.toUtf8();
    const char* filepath = ba.constData();

    fid=fopen(filepath, "wb");
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

    return true;
}

bool VolumePatternIndex::doSearch()
{
    qDebug() << "doSearch() start - reading index header...";
    if (!openIndexAndReadHeader()) {
        qDebug() << "Could not open and read header of file=" << indexFilePath;
        return false;
    }
    V3DLONG indexTotal=iXmax*iYmax*iZmax;

    qDebug() << "Read query...";
    queryImage=0L;
    if (queryImageFilePath.endsWith(".mask")) {
      MaskChan qmc;
      QStringList qmcList;
      qmcList.append(queryImageFilePath);
      queryImage=qmc.createImageFromMaskFiles(qmcList);
    } else {
      ImageLoader queryLoader;
      queryImage=new My4DImage();
      if (!queryLoader.loadImage(queryImage, queryImageFilePath)) {
	qDebug() << "Could not load query image file=" << queryImageFilePath;
	return false;
      }
    }

    // Binary check
    V3DLONG zeroCount=0L;
    V3DLONG oneCount=0L;
    V3DLONG otherCount=0L;
    unsigned char* qData = queryImage->getRawDataAtChannel(queryChannel);
    V3DLONG units=queryImage->getTotalUnitNumber();
    for (int i=0;i<units;i++) {
        if (qData[i]==0) {
            zeroCount++;
        } else if (qData[i]==1) {
            oneCount++;
        } else {
            otherCount++;
        }
    }
    if (otherCount==0 && oneCount>0) {
        V3DLONG changeCount=0L;
        qDebug() << "Re-normalizing binary image for search";
        for (int i=0;i<units;i++) {
            if (qData[i]==1) {
                qData[i]=DEFAULT_BINARY_PROXY_VALUE;
                changeCount++;
            }
        }
        qDebug() << "Changed " << changeCount << " values from binary image";
    }


    qDebug() << "Convert query to index format...";
    if (qx0==-1L) {
        qx0=0;
        qx1=queryImage->getXDim();
        qy0=0;
        qy1=queryImage->getYDim();
        qz0=0;
        qz1=queryImage->getZDim();
    }
    indexData=0L;
    if (skipzeros) {
        indexImage(queryImage, queryChannel, true);
        V3DLONG nonskipCount=0L;
        V3DLONG skipCount=0L;
        V3DLONG cubifiedTotal=iXmax*iYmax*iZmax;
        for (int q=0;q<cubifiedTotal;q++) {
            if (queryIndexSkipPositions[q]==0) {
                nonskipCount++;
            } else {
                skipCount++;
            }
        }
        qDebug() << "Total non-skip count after indexImage=" << nonskipCount << " whereas skipCount=" << skipCount;
    } else {
        indexImage(queryImage, queryChannel, false);

    }
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

  // Skip through index file and populate id/path hash
  int idLength=0;
  int pathLength=0;
  char* filePathBuffer=new char[FILENAME_BUFFER_SIZE];
  char* idBuffer=new char[ID_BUFFER_SIZE];

  size_t readSize=0;

  qDebug() << "Start index scoring loop...";

  while( (readSize=fread(&idLength, 1, sizeof(int), fid))==sizeof(int) ) {
    if (readSize>=ID_BUFFER_SIZE) {
      qDebug() << "idLength exceeds ID_BUFFER_SIZE";
      return false;
    }
    if ( (readSize=fread(idBuffer, 1, idLength, fid))!=idLength) {
      fclose(fid);
      qDebug() << "Unexpectedly could not read path from index file";
      return false;
    }
    if ( !(readSize=fread(&pathLength, 1, sizeof(int), fid))==sizeof(int) ) {
      qDebug() << "pathLength could not be read";
      return false;
    }

    if (pathLength>=FILENAME_BUFFER_SIZE) {
      qDebug() << "pathLength exceeds FILENAME_BUFFER_SIZE";
      return false;
    }
    if ( (readSize=fread(filePathBuffer, 1, pathLength, fid))!=pathLength) {
      fclose(fid);
      qDebug() << "Unexpectedly could not read path from index file";
      return false;
    }
    idBuffer[idLength]='\0';
    QString idQString=QString(idBuffer);
    filePathBuffer[pathLength]='\0';
    QString pathQString=QString(filePathBuffer);
    idPathHash[idQString]=pathQString;

    indexIdList.append(idQString);
    indexFileList.append(pathQString);

    qDebug() << "id=" << idQString << " path=" << pathQString;

    int channelSize;
    if ( (readSize=fread(&channelSize, 1, sizeof(int), fid))!=sizeof(int) ) {
      qDebug() << "Unexpectedly could not channel size for entry";
      return false;
    }

    if ( (readSize=fread(indexData, 1, indexTotalBytes, fid))!=indexTotalBytes) {
      qDebug() << "Could not read full block of " << indexTotalBytes << " , read " << readSize << " instead";
      return false;
    }

    qDebug() << "Calculating index score for " << pathQString;

    V3DLONG score=calculateIndexScore(queryIndex, indexData, indexTotal, queryIndexSkipPositions);
    qDebug() << "score=" << score;
    indexScoreList.append(score);
  }

  qDebug() << "Final readSize=" << readSize;

  // Now, close the index file for reading, and re-open for writing, in preparation
  // for adding new entries.

  fclose(fid);
  fid=0L;

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

  qDebug() << "Using initial results, finalResultSize=" << finalResultSize;

  for (int i=0;i<finalResultSize;i++) {
    finalResultList.append(pairList[i]);
  }

  qDebug() << "==============================================================";

  qDebug() << "Writing to output file=" << outputFilePath;

  QFile outputFile(outputFilePath);
  if (!outputFile.open(QIODevice::WriteOnly)) {
    qDebug() << "Could not open file=" << outputFilePath;
    return false;
  }
  QTextStream scoreOutput(&outputFile);


  qDebug()  << "Starting sort...";

  for (int i=0;i<finalResultSize;i++) {
    int position=i+1;
    QPair<V3DLONG, int> p=finalResultList[i];
    V3DLONG score=p.first;
    int index=p.second;
    qDebug() << "score=" << score << " index=" << index;
    QString filename=indexFileList[index];
    QString id=indexIdList[index];
    qDebug() << "filename=" << filename << " id=" << id;
    double dScore = 1.0L * score;
    if (dScore >= minScore) {
      scoreOutput << position << ". " << score << " : " << id << " : " << filename << "\n";
    }
  }

  qDebug() << "Flushing and closing files";

  scoreOutput.flush();
  outputFile.close();

  qDebug() << "Done closing files";

  if (indexData!=0L) { delete [] indexData; indexData=0L; }
  if (queryIndex!=0L) { delete [] queryIndex; queryIndex=0L; }

  return true;
}

V3DLONG VolumePatternIndex::computeTotalBytesFromIndexTotal(V3DLONG indexTotal) {
    V3DLONG result=ceil((indexTotal*1.0)/4.0);
    return result;
}

V3DLONG VolumePatternIndex::calculateIndexScore(unsigned char* queryIndex, unsigned char* subjectIndex, V3DLONG indexTotal, unsigned char* skipPositions=0L)
{
    V3DLONG score=0L;
    V3DLONG byteTotalFloor=indexTotal/4;
    V3DLONG byteTotal=computeTotalBytesFromIndexTotal(indexTotal);

    unsigned char oooooo11 = 3;
    unsigned char s1=0;
    unsigned char q1=0;
    int scorePosition=0;

    V3DLONG* matrixBins = 0L;
    V3DLONG* matrixScores = 0L;

    if (DEBUG_FLAG) {

        matrixBins = new V3DLONG[16];
        matrixScores = new V3DLONG[16];

        for (int i=0;i<16;i++) {
            matrixBins[i]=0L;
            matrixScores[i]=0L;
        }

    }


    V3DLONG position=0L;

    if (skipPositions==0) {

        if (DEBUG_FLAG) {
            qDebug() << "Starting scoring loop";
        }

        for (V3DLONG i=0;i<byteTotalFloor;i++) {
            unsigned char s=subjectIndex[i];
            unsigned char q=queryIndex[i];
            if (DEBUG_FLAG) {
                qDebug() << "i=" << i << " s=" << s << " q=" << q;
            }
            for (int p=0;p<4;p++) {
                s1= s&oooooo11;
                q1= q&oooooo11;
                scorePosition = q1*4+s1;
                if (DEBUG_FLAG) {
                    qDebug() << "   p=" << p << " s1=" << s1 << " q1=" << q1 << " scorePosition=" << scorePosition;
                }
                score+=matrix[scorePosition];
                if (DEBUG_FLAG) {
                    matrixBins[scorePosition] += 1;
                    matrixScores[scorePosition] += matrix[scorePosition];
                }
                s >>= 2;
                q >>= 2;
                position++;
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
                    score+=matrix[scorePosition];
                    if (DEBUG_FLAG) {
                        matrixBins[scorePosition] += 1;
                        matrixScores[scorePosition] += matrix[scorePosition];
                    }
                    s >>= 2;
                    q >>= 2;
                }
                position++;
            }
        }

    } else {

        if (DEBUG_FLAG) {
            qDebug() << " Running under skipPositions mode";
        }

        for (V3DLONG i=0;i<byteTotalFloor;i++) {
            unsigned char s=subjectIndex[i];
            unsigned char q=queryIndex[i];
            for (int p=0;p<4;p++) {
                if (skipPositions[position]==0) {
                    s1= s&oooooo11;
                    q1= q&oooooo11;
                    scorePosition = q1*4+s1;
                    score+=matrix[scorePosition];
                    if (DEBUG_FLAG) {
                        matrixBins[scorePosition] += 1;
                        matrixScores[scorePosition] += matrix[scorePosition];
                    }
                }
                s >>= 2;
                q >>= 2;
                position++;
            }
        }

        for (V3DLONG i=byteTotalFloor;i<byteTotal;i++) {
            unsigned char s=subjectIndex[i];
            unsigned char q=queryIndex[i];
            for (int p=0;p<4;p++) {
                if (position<indexTotal) {
                    if (skipPositions[position]==0) {
                        s1= s&oooooo11;
                        q1= q&oooooo11;
                        scorePosition = q1*4+s1;
                        score+=matrix[scorePosition];
                        if (DEBUG_FLAG) {
                            matrixBins[scorePosition] += 1;
                            matrixScores[scorePosition] += matrix[scorePosition];
                        }
                    }
                    s >>= 2;
                    q >>= 2;
                }
                position++;
            }
        }

    }

    if (DEBUG_FLAG) {

        qDebug() << "Returning score=" << score;

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

    }

    return score;
}

void VolumePatternIndex::indexImage(My4DImage* image, int channel, bool skipzeros=false)
{

    qDebug() << "indexImage : using threshold values " << threshold[0] << " " << threshold[1] << " " << threshold[2];

    V3DLONG count0=0L;
    V3DLONG count1=0L;
    V3DLONG count2=0L;
    V3DLONG count3=0L;

    My4DImage* cubifiedImage=0L;

    V3DLONG subregion[6];

    subregion[0]=x0;
    subregion[1]=x1;
    subregion[2]=y0;
    subregion[3]=y1;
    subregion[4]=z0;
    subregion[5]=z1;

    if (skipzeros) {
        qDebug() << "indexImage - skipzeros is true - populating queryIndexSkipPositions";
        cubifiedImage=AnalysisTools::cubifyImageByChannel(image, channel, unitSize, AnalysisTools::CUBIFY_TYPE_AVERAGE, subregion, true, &queryIndexSkipPositions);
    } else {
        unsigned char** zeroPointer=0L;
        qDebug() << "indexImage - skipzeros is false - calling cubifyImageByChannel with zeroPointer";
        cubifiedImage=AnalysisTools::cubifyImageByChannel(image, channel, unitSize, AnalysisTools::CUBIFY_TYPE_AVERAGE, subregion, false, zeroPointer);
    }
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

    ImageLoader debugLoader;
    debugLoader.saveImage(cubifiedImage, "cubifiedDebugImage.v3dpbd");

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
    QByteArray ba = indexFilePath.toUtf8();
    const char* filepath = ba.constData();
    fid=fopen(filepath, "rb");
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
    return true;

    // Note: fid is intentionally not closed
}


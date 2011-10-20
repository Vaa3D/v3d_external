
#include <QString>
#include <QtCore>
#include <QDir>
#include "../../v3d/v3d_core.h"
#include "ImageLoader.h"

using namespace std;

const int ImageLoader::MODE_UNDEFINED=0;
const int ImageLoader::MODE_LOAD_TEST=1;
const int ImageLoader::MODE_CONVERT=2;

class SleepThread : QThread {
public:
    SleepThread() {}
    void msleep(int miliseconds) {
        QThread::msleep(miliseconds);
    }
};

ImageLoader::ImageLoader()
{
    mode=MODE_UNDEFINED;
    inputFilepath="";
    targetFilepath="";
    compressionBuffer=0;
    fid=0;
    keyread=0;
    image=0;
    decompressionThread=0;
    compressionPosition=0;
    decompressionPosition=0;
    decompressionPrior=0;
}

ImageLoader::~ImageLoader()
{
    if (compressionBuffer!=0)
        delete [] compressionBuffer;
    if (keyread!=0)
        delete [] keyread;
    // Note we do not delete image because we do this explicitly only if error
}

int ImageLoader::processArgs(vector<char*> *argList) {
    for (int i=0;i<argList->size();i++) {
        QString arg=(*argList)[i];
        bool done=false;
        if (arg=="-loadtest") {
            mode=MODE_LOAD_TEST;
            do {
                QString possibleFile=(*argList)[++i];
                if (!possibleFile.startsWith("-")) {
                    inputFilepath=possibleFile;
                } else {
                    done=true;
                    i--; // rewind
                }
            } while(!done && i<(argList->size()-1));
        } else if (arg=="-convert") {
            mode=MODE_CONVERT;
            bool haveInput=false;
            do {
                QString possibleFile=(*argList)[++i];
                if (!possibleFile.startsWith("-") && !haveInput) {
                    inputFilepath=possibleFile;
                    haveInput=true;
                } else if (!possibleFile.startsWith("-") && haveInput) {
                    targetFilepath=possibleFile;
                } else {
                    done=true;
                    i--; // rewind
                }
            } while(!done && i<(argList->size()-1));
        }
    }
    if (inputFilepath.length()<1) {
        return 1;
    }
    return 0;
}

bool ImageLoader::execute() {
    if (!validateFile())
        return false;
    QTime stopwatch;
    stopwatch.start();

    if (mode==MODE_UNDEFINED) {
        qDebug() << "ImageLoader::execute() - no mode defined - doing nothing";
        return true;
    } else if (mode==MODE_LOAD_TEST) {
        stopwatch.start();
        image=loadImage(inputFilepath);
        if (image!=0) {
            qDebug() << "Loading time is " << stopwatch.elapsed() / 1000.0 << " seconds";
            return true;
        }
        return false;
    } else if (mode==MODE_CONVERT) {
        if (inputFilepath.compare(targetFilepath)==0) {
            qDebug() << "ImageLoader::execute() - can not convert a file to itself";
            return false;
        }
        image=loadImage(inputFilepath);
        qDebug() << "Loading time is " << stopwatch.elapsed() / 1000.0 << " seconds";
        stopwatch.restart();
        qDebug() << "Saving to file " << targetFilepath;
        if (targetFilepath.endsWith(".v3dpbd")) {
            V3DLONG sz[4];
            sz[0] = image->getXDim();
            sz[1] = image->getYDim();
            sz[2] = image->getZDim();
            sz[3] = image->getCDim();
            unsigned char* data = 0;
            if (image->getDatatype()==1) {
                data = image->getRawData();
            } else if (image->getDatatype()==2) {
                data = convertType2Type1(sz, image);
                if (data==0) {
                    qDebug() << "ImageLoader::execute - problem allocating memory for conversion of type 2 to type 1";
                    return false;
                }
                delete image;
                image=0;
            } else {
                qDebug() << "ImageLoader::execute - do not support source data other than type 1 or type 2";
                return false;
            }
            saveStack2RawPBD(targetFilepath.toAscii().data(), data, sz);
            if (image!=0) {
                delete image;
            } else {
                delete data;
            }
        } else {
            image->saveImage(targetFilepath.toAscii().data());
            delete image;
        }
        qDebug() << "Saving time is " << stopwatch.elapsed() / 1000.0 << " seconds";
        return true;
    }
    return false; // should not get here
}

unsigned char * ImageLoader::convertType2Type1(const V3DLONG * sz, My4DImage *image) {
    Image4DProxy<My4DImage> proxy(image);
    V3DLONG totalSize=sz[0]*sz[1]*sz[2]*sz[3];
    unsigned char * data = new unsigned char [totalSize];
    if (data==0) {
        return data;
    }
    for (V3DLONG s3=0;s3<sz[3];s3++) {
        for (V3DLONG s2=0;s2<sz[2];s2++) {
            for (V3DLONG s1=0;s1<sz[1];s1++) {
                for (V3DLONG s0=0;s0<sz[0];s0++) {
                    unsigned int v = (*proxy.at_uint16(s0,s1,s2,s3)) / 16;
                    if (v>255) {
                        v=255;
                    }
                    data[s3*sz[2]*sz[1]*sz[0] + s2*sz[1]*sz[0] + s1*sz[0] + s0] = v;
                }
            }
        }
    }
    return data;
}


bool ImageLoader::validateFile() {
    qDebug() << "Input file = " << inputFilepath;
    QFileInfo fileInfo(inputFilepath);
    if (fileInfo.exists()) {
        qDebug() << " verified this file exists with size=" << fileInfo.size();
    } else {
        qDebug() << " file does not exist";
        return false;
    }
    return true;
}

My4DImage* ImageLoader::loadImage(QString filepath) {
    qDebug() << "Starting to load file " << filepath;
    My4DImage* image=new My4DImage();
    if (filepath.endsWith(".tif") || filepath.endsWith(".lsm") || filepath.endsWith(".v3draw") || filepath.endsWith(".raw")) {
        image->loadImage(filepath.toAscii().data());
    } else if (filepath.endsWith(".v3dpbd")) {
        if (loadRaw2StackPBD(filepath.toAscii().data(), image)!=0) {
            qDebug() << "Error with loadRaw2StackPBD";
            return 0;
        }
    }
    return image;
}

QString ImageLoader::getFilePrefix(QString filepath) {
    QStringList list=filepath.split(QRegExp("\\."));
    if (list.length()==0) {
        return filepath;
    } else {
        return list.at(0);
    }
}


/*

 This function implements a hybrid of PackBits with difference-encoding, as follows:

 <header byte>

 0 to 32   : Implies the following (n+1) bytes are literal
 33 to 127 : Implies the prior byte is to be followed by (n-32) 2-bit (4-value) differences, to be applied accumulatively.
             Any extra space to the remaining byte boundary is to be ignored. The 4 values are interpreted as as a progessive
             4D array incrementing the following values in order { 0, 1, 2, -1 }, such that,

             Value   Difference Sequence

             0    =  0,  0,  0,  0
             1    =  0,  0,  0,  1
             2    =  0,  0,  0,  2
             3    =  0,  0,  0, -1
             4    =  0,  0,  1,  0
             5    =  0,  0,  1,  1
             6    =  0,  0,  1,  2
             7    =  0,  0,  1, -1
             8    =  0,  0,  2,  0
            ...
            123   =  1, -1,  2, -1
            124   =  1, -1, -1,  0
            125   =  1, -1, -1,  1
            126   =  1, -1, -1,  2
            127   =  1, -1, -1, -1
            128   =  2,  0,  0,  0
            129   =  2,  0,  0,  1
            130   =  2,  0,  0,  2
            131   =  2,  0,  0, -1
            132   =  2,  0,  1,  0
            133   =  2,  0,  1,  1
            ...
            188   =  2, -1, -1,  0
            189   =  2, -1, -1,  1
            190   =  2, -1, -1,  2
            191   =  2, -1, -1, -1
            192   = -1,  0,  0,  0
            193   = -1,  0,  0,  1
            194   = -1,  0,  0,  2
            195   = -1,  0,  0, -1
            ...
            249   = -1, -1,  2,  1
            250   = -1, -1,  2,  2
            251   = -1, -1,  2, -1
            252   = -1, -1, -1,  0
            253   = -1, -1, -1,  1
            254   = -1, -1, -1,  2
            255   = -1, -1, -1, -1

128 to 255 : Implies the following single byte is to be repeated (n-127) times

*/

int ImageLoader::saveStack2RawPBD(const char * filename, unsigned char* data, const V3DLONG * sz)
{
    int berror=0;
    int datatype=1; // PBD only supports type=1 as output

        /* This function save a data stack to raw file */
                printf("size of [V3DLONG]=[%ld], [V3DLONG]=[%ld] [int]=[%ld], [short int]=[%ld], [double]=[%ld], [float]=[%ld]\n",
                       sizeof(V3DLONG), sizeof(V3DLONG), sizeof(int), sizeof(short int), sizeof(double), sizeof(float));
        V3DLONG i;

        fid = fopen(filename, "wb");
        if (!fid)
        {
            return exitWithError("Fail to open file for writing");
        }

        /* Write header */
                         // raw_image_stack_by_hpeng
        char formatkey[] = "v3d_volume_pkbitdf_encod";
        int lenkey = strlen(formatkey);

        V3DLONG nwrite = fwrite(formatkey, 1, lenkey, fid);
        if (nwrite!=lenkey)
        {
            return exitWithError("File write error");
        }

        char endianCodeMachine = checkMachineEndian();
        if (endianCodeMachine!='B' && endianCodeMachine!='L')
        {
                return exitWithError("This program only supports big- or little- endian but not other format. Cannot save data on this machine.");
        }

        nwrite = fwrite(&endianCodeMachine, 1, 1, fid);
        if (nwrite!=1)
        {
                return exitWithError("Error happened in file writing.");
        }

        //int b_swap = (endianCodeMachine==endianCodeData)?0:1;
        //int b_swap = 0; //for this machine itself, should not swap data.

        short int dcode = (short int)datatype;
        if (dcode!=1 && dcode!=2 && dcode!=4)
        {
            QString errorMsg = QString("Unrecognized data type code = [%1]. This code is not supported in this version.").arg(dcode);
            return exitWithError(errorMsg);
        }

        //if (b_swap) swap2bytes((void *)&dcode);
        nwrite=fwrite(&dcode, 2, 1, fid); /* because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read. */
        if (nwrite!=1)
        {
                return exitWithError("Writing file error.");
        }

        V3DLONG unitSize = datatype; /* temporarily I use the same number, which indicates the number of bytes for each data point (pixel). This can be extended in the future. */

        //short int mysz[4];
        BIT32_UNIT mysz[4];//060806
                                           //if (b_swap)  {
                                           //for (i=0;i<4;i++) mysz[i] = (short int) sz[i];
                for (i=0;i<4;i++) mysz[i] = (BIT32_UNIT) sz[i];
                //swap2bytes((void *)(mysz+i));
                //}
                nwrite = fwrite(mysz, 4, 4, fid); /* because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read. */
        if (nwrite!=4)
        {
                return exitWithError("Writing file error.");
        }

        V3DLONG totalUnit = 1;
        for (i=0;i<4;i++)
        {
                totalUnit *= sz[i];
        }

        V3DLONG maxSize = totalUnit*unitSize;
        unsigned char * imgRe = new unsigned char [totalUnit];

        printf("Allocated compression target with maxSize=%d\n", maxSize);

        V3DLONG compressionSize = compressPBD(imgRe, data, maxSize, maxSize);

        if (compressionSize==0) {
            return exitWithError("Error during compressPBD");
        }

        double finalCompressionRatio = (maxSize*1.0)/compressionSize;

        printf("Total original size=%d  post-compression size=%d  ratio=%f\n", totalUnit, compressionSize, finalCompressionRatio);

        printf("Writing file...");

        nwrite = fwrite(imgRe, unitSize, compressionSize, fid);
        if (nwrite!=compressionSize)
        {
                QString errorMsg = QString("Something wrong in file writing. The program wrote %1 data points but the file says there should be %2 data points.")
                                   .arg(nwrite).arg(totalUnit);
                return exitWithError(errorMsg);
        }

        /* clean and return */
        fclose(fid);
        delete [] imgRe;
        printf("done.\n");
        return berror;
}


// We assume here that preBuffer is
V3DLONG ImageLoader::compressPBD(unsigned char * imgRe, unsigned char * preBuffer, V3DLONG bufferLength, V3DLONG spaceLeft) {
    bool debug=false;
    V3DLONG p=0;

    if (bufferLength==0) {
        printf("ImageLoader::compressPBD - unexpectedly received buffer of zero size\n");
        return 0;
    }

    unsigned char currentValue=0;
    int dbuffer[95];
    V3DLONG activeLiteralIndex=-1; // if -1 this means there is no literal mode started
    for (int i=0;i<bufferLength;i++) {

        if (p>=spaceLeft) {
            printf("ImageLoader::compressPBD ran out of space p=%d\n", p);
            return 0;
        }

        // From this point we assume the result has been accumulating in imgRe, and at this moment
        // we are searching for the best approach for the next segment. First, we will try reading
        // the next value, and testing what the runlength encoding efficiency would be. The
        // efficiency is simply the (number of bytes encoded / actual bytes). Next, we will test
        // the different encoding, and (depending on its reach), see what its efficiency is.
        // The minimum number of unencoded bytes to use run-length encoding for is 3 (code, value).
        // The minimum number of unencoded bytes to use difference encoding is 3 also (code, value).

        int reTest=1;
        currentValue=preBuffer[i];
        int currentPosition=i+1;
        while(currentPosition<bufferLength && reTest<128) { // 128 is the max number of repeats supported
            if (preBuffer[currentPosition++]==currentValue) {
                reTest++;
            } else {
                break;
            }
        }
        double reEfficiency = reTest*1.0 / 2.0; // 2-bytes are used for the encoding

        if (reEfficiency>=4.0) { // 4.0 is the max efficiency from the difference encoding
            // Then use RE
            imgRe[p++]=reTest+127; // The code for number of repeats
            imgRe[p++]=currentValue; // The repeated value
            i+=(reTest-1); // because will increment one more time at top of loop
            activeLiteralIndex=-1;
        } else {
            double dfEfficiency=0.0;
            int c=i;
            if (i>0) { // I.e., if not first since we can't start with a difference encoding
                // We need to evaluate difference encoding starting with the prior value
                unsigned int priorValue=preBuffer[i-1];
                int unitsToCheck=bufferLength-i;
                if (unitsToCheck>95) {
                    unitsToCheck=95; // 95 is max supported number of differences
                }
                for (int j=0;j<95;j++) {
                    dbuffer[j]=0; // clear the difference buffer
                }
                for (;c<i+unitsToCheck;c++) {
                    int d=preBuffer[c] - priorValue;
                    if (d>2 || d<-1) {
                        break;
                    }
                    priorValue=preBuffer[c]; // since by definition the diff encoding is cumulative
                    if (d==-1) {
                        d=3; // we use this to represent -1 in the dbuffer for key lookup later
                    }
                    dbuffer[c-i]=d;
                }
                dfEfficiency = ((c-i)*1.0)/(((c-i)/4)+2); // The denominator includes the coding byte and the encoding bytes
            }
            // Now we can decide between RE and DF based on efficiency
            if (reEfficiency>dfEfficiency && reEfficiency>1.0) {
                // Then use RE
                imgRe[p++]=reTest+127; // The code for number of repeats
                imgRe[p++]=currentValue; // The repeated value
                i+=(reTest-1); // because will increment one more time at top of loop
                activeLiteralIndex=-1;
            } else if (dfEfficiency>1.0) {
                // First, encode the number of units we expect
                imgRe[p++]=c-i+32;

                // Then use DF. We want to move forward in units of 4, and pick the correct encoding.
                // Note that is doesn't matter if we pad extra 0s because we know what the correct
                // length is from above.
                int cp=i;
                unsigned char d0,d1,d2,d3;
                d0=d1=d2=d3=0;
                while(cp<c) {
                    int start=cp-i;
                    d0=dbuffer[start];
                    if (cp+1<c) {
                        d1=dbuffer[start+1];
                        if (cp+2<c) {
                            d2=dbuffer[start+2];
                            if (cp+3<c) {
                                d3=dbuffer[start+3];
                            }
                        }
                    }
                    // In decompression, we want FIFO for speed, so we put last first.
                    unsigned char v=0;
                    v |= d3;
                    v <<= 2;
                    v |= d2;
                    v <<= 2;
                    v |= d1;
                    v <<= 2;
                    v |= d0;

                    imgRe[p++]=v;
                    cp+=4; // Move ahead 4 steps at a time
                }

                activeLiteralIndex=-1;
                i=c-1; // will increment at top
            } else { // This will catch the case where dfEfficiency is 0.0 due to i==0
                // We need to add this value as a literal. If there is already a literal mode, then simply
                // add it. Otherwise, start a new one.
                if (activeLiteralIndex<0 || imgRe[activeLiteralIndex]>=32) {
                    // We need a new index
                    imgRe[p++]=0;
                    activeLiteralIndex=p-1; // Our new literal index
                    imgRe[p++]=currentValue; // Add the current value onto current sequence of literals
                } else {
                    imgRe[activeLiteralIndex] += 1; // Increment existing literal count
                    imgRe[p++]=currentValue; // Add the current value onto current sequence of literals
                }
            }
        }
    }
    return p;
}

int ImageLoader::loadRaw2StackPBD(char * filename, My4DImage * & image) {
    {
            int berror = 0;
            decompressionPrior = 0;

            QTime stopwatch;
            stopwatch.start();

            int datatype;

            fid = fopen(filename, "rb");
            if (!fid)
            {
                    return exitWithError("Fail to open file for reading.");
            }

            fseek (fid, 0, SEEK_END);
            V3DLONG fileSize = ftell(fid);
            rewind(fid);

            /* Read header */

            char formatkey[] = "v3d_volume_pkbitdf_encod";
            V3DLONG lenkey = strlen(formatkey);

    #ifndef _MSC_VER //added by PHC, 2010-05-21
            if (fileSize<lenkey+2+4*4+1) // datatype has 2 bytes, and sz has 4*4 bytes and endian flag has 1 byte.
            {
                    QString errorMessage =
                            QString("The size of your input file is too small and is not correct, -- it is too small to contain the legal header.\n");
                    errorMessage.append(QString("The fseek-ftell produces a file size = %1.").arg(fileSize));
                   return exitWithError(errorMessage);
            }
    #endif

            keyread = new char [lenkey+1];
            if (!keyread)
            {
                    return exitWithError("Fail to allocate memory.");
            }
            V3DLONG nread = fread(keyread, 1, lenkey, fid);
            if (nread!=lenkey)
            {
                    return exitWithError("File unrecognized or corrupted file.");
            }
            keyread[lenkey] = '\0';

            V3DLONG i;
            if (strcmp(formatkey, keyread)) /* is non-zero then the two strings are different */
            {
                    return exitWithError("Unrecognized file format.");
            }

            char endianCodeData;
            fread(&endianCodeData, 1, 1, fid);
            if (endianCodeData!='B' && endianCodeData!='L')
            {
                    return exitWithError("This program only supports big- or little- endian but not other format. Check your data endian.");
            }

            char endianCodeMachine;
            endianCodeMachine = checkMachineEndian();
            if (endianCodeMachine!='B' && endianCodeMachine!='L')
            {
                    return exitWithError("This program only supports big- or little- endian but not other format. Check your data endian.");
            }

            int b_swap = (endianCodeMachine==endianCodeData)?0:1;

            short int dcode = 0;
            fread(&dcode, 2, 1, fid); /* because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read. */
            if (b_swap)
                    swap2bytes((void *)&dcode);

            switch (dcode)
            {
                    case 1:
                            datatype = 1; /* temporarily I use the same number, which indicates the number of bytes for each data point (pixel). This can be extended in the future. */
                            break;

                    case 2:
                            datatype = 2;
                            break;

                    case 4:
                            datatype = 4;
                            break;

                    default:
                            QString errorMessage = QString("Unrecognized data type code [%1]. The file type is incorrect or this code is not supported in this version.").arg(dcode);
                            return exitWithError(errorMessage);
            }

            if (datatype!=1) {
                return exitWithError("ImageLoader::loadRaw2StackPBD : only datatype=1 supported");
            }

            V3DLONG unitSize = datatype; // temporarily I use the same number, which indicates the number of bytes for each data point (pixel). This can be extended in the future.

            BIT32_UNIT mysz[4];
            mysz[0]=mysz[1]=mysz[2]=mysz[3]=0;
            int tmpn=fread(mysz, 4, 4, fid); // because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read.
            if (tmpn!=4)
            {
                    QString errorMessage = QString("This program only reads [%1] units.").arg(tmpn);
                    return exitWithError(errorMessage);
            }

            if (b_swap && (unitSize==2 || unitSize==4)) {
                QString errorMessage = "b_swap true and unitSize > 1 - this is not implemented in current code";
                return exitWithError(errorMessage);
            }

            if (b_swap)
            {
                    for (i=0;i<4;i++)
                    {
                        //swap2bytes((void *)(mysz+i));
                        printf("mysz raw read unit[%ld]: [%d] ", i, mysz[i]);
                        swap4bytes((void *)(mysz+i));
                        printf("swap unit: [%d][%0x] \n", mysz[i], mysz[i]);
                    }
            }

            V3DLONG * sz = new V3DLONG [4]; // reallocate the memory if the input parameter is non-null. Note that this requests the input is also an NULL point, the same to img.
            if (!sz)
            {
                    return exitWithError("Fail to allocate memory.");
            }

            V3DLONG totalUnit = 1;
            for (i=0;i<4;i++)
            {
                    sz[i] = (V3DLONG)mysz[i];
                    totalUnit *= sz[i];
            }

            //mexPrintf("The input file has a size [%ld bytes], different from what specified in the header [%ld bytes]. Exit.\n", fileSize, totalUnit*unitSize+4*4+2+1+lenkey);
            //mexPrintf("The read sizes are: %ld %ld %ld %ld\n", sz[0], sz[1], sz[2], sz[3]);

            V3DLONG headerSize=4*4+2+1+lenkey;
            V3DLONG compressedBytes=fileSize-headerSize;
            maxDecompressionSize=totalUnit*unitSize;

            if (image) delete image;
            image = new My4DImage();

            compressionBuffer = new unsigned char [compressedBytes];

            V3DLONG remainingBytes = compressedBytes;
            //V3DLONG nBytes2G = V3DLONG(1024)*V3DLONG(1024)*V3DLONG(1024)*V3DLONG(2);
            V3DLONG readStepSizeBytes = V3DLONG(1024)*20000;
            V3DLONG totalReadBytes = 0;
            V3DLONG cntBuf = 0;

            // Transfer data to My4DImage
            image->loadImage(sz[0], sz[1], sz[2], sz[3], 1);
            decompressionBuffer = image->getRawData();

            const int MAX_CHECKS_PER_DECOMPRESSION=2000;
            const int CYCLE_WAIT_MS=5;
            int decompChecks=0;

            while (remainingBytes>0)
            {
                    V3DLONG curReadBytes = (remainingBytes<readStepSizeBytes) ? remainingBytes : readStepSizeBytes;
                    V3DLONG curReadUnits = curReadBytes/unitSize;
                    nread = fread(compressionBuffer+cntBuf*readStepSizeBytes, unitSize, curReadUnits, fid);
                    totalReadBytes+=nread;
                    if (nread!=curReadUnits)
                    {
                        QString errorMessage = QString("Something wrong in file reading. The program reads [%1 data points] but the file says there should be [%2 data points].")
                                               .arg(nread).arg(totalUnit);
                            return exitWithError(errorMessage);
                    }
                    remainingBytes -= nread;
                    cntBuf++;
                    //printf("r1\n");
                    while (decompressionThread!=0 && !decompressionThread->isFinished()) {
                        if (decompChecks>MAX_CHECKS_PER_DECOMPRESSION) {
                            exitWithError("Error in decompressor : exceeded time limit for decompression thread");
                        }
                        SleepThread st;
                        st.msleep(CYCLE_WAIT_MS);
                        decompChecks++;
                    }
                    decompressionThread=&(QtConcurrent::run(this, &ImageLoader::updateCompressionBuffer, compressionBuffer+totalReadBytes));
                    //printf("r2\n");
            }
            qDebug() << "Total time elapsed after all reads is " << stopwatch.elapsed() / 1000.0 << " seconds";
            stopwatch.restart();
            decompChecks=0;
            while (decompressionThread!=0 && !decompressionThread->isFinished()) {
                if (decompChecks>MAX_CHECKS_PER_DECOMPRESSION) {
                    exitWithError("Error in decompressor : exceeded time limit for decompression thread");
                }
                SleepThread st;
                st.msleep(CYCLE_WAIT_MS);
                decompChecks++;
            }
            qDebug() << "Time to complete final decompression thread is " << stopwatch.elapsed() / 1000.0 << " seconds";

            // Success - can delete compressedData
            delete [] compressionBuffer; compressionBuffer=0;

            // clean and return
            if (keyread) {delete [] keyread; keyread = 0;}
            fclose(fid); //bug fix on 060412
            return berror;
    }
}

int ImageLoader::exitWithError(QString errorMessage) {
    qDebug() << errorMessage;
//    if (decompressionThread!=0)
//        decompressionThread->~QFuture();
    if (compressionBuffer!=0)
        delete [] compressionBuffer;
    if (keyread!=0)
        delete keyread;
    if (fid!=0)
        fclose(fid);
    int berror=1;
    return berror;
}

V3DLONG ImageLoader::decompressPBD(unsigned char * sourceData, unsigned char * targetData, V3DLONG sourceLength) {

    // Decompress data
    V3DLONG cp=0;
    V3DLONG dp=0;
    const unsigned char mask=0x0003;
    unsigned char p0,p1,p2,p3;
    unsigned char value=0;
    unsigned char pva=0;
    unsigned char pvb=0;
    int leftToFill=0;
    int fillNumber=0;
    unsigned char * toFill=0;
    unsigned char sourceChar=0;
    while(cp<sourceLength) {

        value=sourceData[cp];

        if (value<33) {
            // Literal 0-32
            unsigned char count=value+1;
            for (int j=cp+1;j<cp+1+count;j++) {
                targetData[dp++]=sourceData[j];
            }
            cp+=(count+1);
            decompressionPrior=targetData[dp-1];
        } else if (value<128) {
            // Difference 33-127
            leftToFill=value-32;
            while(leftToFill>0) {
                fillNumber=(leftToFill<4 ? leftToFill : 4);
                sourceChar=sourceData[++cp];
                toFill = targetData+dp;
                p0=sourceChar & mask;
                sourceChar >>= 2;
                p1=sourceChar & mask;
                sourceChar >>= 2;
                p2=sourceChar & mask;
                sourceChar >>= 2;
                p3=sourceChar & mask;
                pva=(p0==3?-1:p0)+decompressionPrior;

                *toFill=pva;
                if (fillNumber>1) {
                    toFill++;
                    pvb=pva+(p1==3?-1:p1);
                    *toFill=pvb;
                    if (fillNumber>2) {
                        toFill++;
                        pva=(p2==3?-1:p2)+pvb;
                        *toFill=pva;
                        if (fillNumber>3) {
                            toFill++;
                            *toFill=(p3==3?-1:p3)+pva;
                        }
                    }
                }

                decompressionPrior = *toFill;
                dp+=fillNumber;
                leftToFill-=fillNumber;
            }
            cp++;
        } else {
            // Repeat 128-255
            unsigned char repeatCount=value-127;
            unsigned char repeatValue=sourceData[++cp];

            for (int j=0;j<repeatCount;j++) {
                targetData[dp++]=repeatValue;
            }
            decompressionPrior=repeatValue;
            cp++;
        }

    }
    return dp;
}

// This is the main decompression function, which is basically a wrapper for the decompression function
// of ImageLoader, except it determines the boundary of acceptable processing given the contents of the
// most recently read block. This function assumes it is called sequentially in order by the signal/slot
// system, and so doesn't have to check that the prior processing step is finished. updatedCompressionBuffer
// points to the first invalid data position, i.e., all previous positions starting with compressionBuffer
// are valid.
void ImageLoader::updateCompressionBuffer(unsigned char * updatedCompressionBuffer) {
    //printf("d1\n");
    if (compressionPosition==0) {
        // Just starting
        compressionPosition=compressionBuffer;
    }
    unsigned char * lookAhead=compressionPosition;
    while(lookAhead<updatedCompressionBuffer) {
        unsigned char lav=*lookAhead;
        // We will keep going until we find nonsense or reach the end of the block
        if (lav<33) {
            // Literal values - the actual number of following literal values
            // is equal to the lav+1, so that if lav==32, there are 33 following
            // literal values.
            if ( lookAhead+lav+1 < updatedCompressionBuffer ) {
                // Then we can process the whole literal section - we can move to
                // the next position
                lookAhead += (lav+2);
            } else {
                break; // leave lookAhead in current maximum position
            }
        } else if (lav<128) {
            // Difference section. The number of difference entries is equal to lav-32, so that
            // if lav==33, the minimum, there will be 1 difference entry. For a given number of
            // difference entries, there are a factor of 4 fewer compressed entries. With the
            // equation below, lav-33 will be 4 when lav==37, which is 5 entries, requiring 2 bytes, etc.
            unsigned char compressedDiffEntries=(lav-33)/4 + 1;
            if ( lookAhead+compressedDiffEntries < updatedCompressionBuffer ) {
                // We can process this section, so advance to next position to evaluate
                lookAhead += (compressedDiffEntries+1);
            } else {
                break; // leave in current max position
            }
        } else {
            // Repeat section. Number of repeats is equal to lav-127, but the very first
            // value is the value to be repeated. The total number of compressed positions
            // is always == 2
            if ( lookAhead+1 < updatedCompressionBuffer ) {
                lookAhead += 2;
            } else {
                break; // leave in current max position
            }
        }
    }
    // At this point, lookAhead is in an invalid position, which if equal to updatedCompressionBuffer
    // means the entire compressed update can be processed.
    V3DLONG compressionLength=lookAhead-compressionPosition;
    if (decompressionPosition==0) {
        // Needs to be initialized
        decompressionPosition=decompressionBuffer;
    }
    //qDebug() << "updateCompressionBuffer calling decompressPBD compressionPosition=" << compressionPosition << " decompressionPosition=" << decompressionPosition
    //        << " size=" << compressionLength << " previousTotalDecompSize=" << getDecompressionSize() << " maxDecompSize=" << maxDecompressionSize;
    V3DLONG dlength=decompressPBD(compressionPosition, decompressionPosition, compressionLength);
    compressionPosition=lookAhead;
    decompressionPosition+=dlength;
    //printf("d2\n");
}







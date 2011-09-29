
#include <QString>
#include <QtCore>
#include <QDir>
#include "../../v3d/v3d_core.h"
#include "ImageLoader.h"

using namespace std;

const unsigned char RE_TRIGGER_CODE=249;
const int RE_CUBE_SIZE=6;

ImageLoader::ImageLoader()
{
}

ImageLoader::~ImageLoader()
{
    for (int i=0;i<imageList.size();i++) {
        My4DImage* image=imageList.at(i);
        delete image;
    }
}

int ImageLoader::processArgs(vector<char*> *argList) {
    for (int i=0;i<argList->size();i++) {
        QString arg=(*argList)[i];
        if (arg=="-load") {
            bool done=false;
            do {
                QString possibleFile=(*argList)[++i];
                if (!possibleFile.startsWith("-")) {
                    inputFileList.append(possibleFile);
                } else {
                    done=true;
                    i--; // rewind
                }
            } while(!done && i<(argList->size()-1));
        }
    }
    if (inputFileList.length()<1) {
        return 1;
    }
    return 0;
}

bool ImageLoader::execute() {
    if (!validateFiles())
        return false;
    QTime stopwatch;
    stopwatch.start();

    for (int i=0;i<inputFileList.size();i++) {
        My4DImage* image=0;
        if (inputFileList.at(i).endsWith(".v3dpbd")) {
            if (loadRaw2StackPBD(inputFileList.at(i).toAscii().data(), image)!=0) {
                qDebug() << "Error with loadRaw2StackPBD";
                return false;
            }
        } else {
            stopwatch.restart();
            image=loadImage(inputFileList.at(i));
            qDebug() << "Loading total time is " << stopwatch.elapsed() / 1000.0 << " seconds";

            QString filePrefix=getFilePrefix(inputFileList.at(i));
            QString saveFilepath=filePrefix.append(".v3dpbd");
            V3DLONG sz[4];
            sz[0] = image->getXDim();
            sz[1] = image->getYDim();
            sz[2] = image->getZDim();
            sz[3] = image->getCDim();
            unsigned char**** data = (unsigned char****)image->getData();
            saveStack2RawPBD(saveFilepath.toAscii().data(), data, sz, image->getDatatype());
        }
        if (image!=0) {
            imageList.append(image);
        }

//        if (!inputFileList.at(i).endsWith(".v3draw")) {
//            QString filePrefix=getFilePrefix(inputFileList.at(i));
//            QString saveFilepath=filePrefix.append(".v3draw");
//            qDebug() << "Saving to file " << saveFilepath;
//            image->saveImage(saveFilepath.toAscii().data());
//            qDebug() << "Done.";
//        }
    }

    //qDebug() << "Loading total time elapsed is " << stopwatch.elapsed() / 1000.0 << " seconds";
    return true;
}

bool ImageLoader::validateFiles() {
    qDebug() << "Input files:";
    for (int i=0;i<inputFileList.size();i++) {
        QString filepath=inputFileList.at(i);
        qDebug() << "Filepath=" << filepath;
        QFileInfo fileInfo(filepath);
        if (fileInfo.exists()) {
            qDebug() << " verified this file exists with size=" << fileInfo.size();
        } else {
            qDebug() << " file does not exist";
            return false;
        }
    }
    return true;
}

My4DImage* ImageLoader::loadImage(QString filepath) {
    qDebug() << "Starting to load file " << filepath;
    My4DImage* image=new My4DImage();
    if (filepath.endsWith(".tif") || filepath.endsWith(".lsm") || filepath.endsWith(".v3draw")) {
        image->loadImage(filepath.toAscii().data());
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

/* This is a modified version of Hanchuan Peng's raw format in which a simple volumetric runlength is applied.
   8x8x8 (512) voxel cubes are iterated through the volume. Initially, only datatype=1 (8-bit) is supported.
   The cube is unpacked into a one-dimensional array. Lengths of the array in which values are repeated are
   condensed as 3-mers <trigger-code><number-of-occurences><value>. If a value occurs once, twice, or three times
   in a row, then its value is simply used without invoking the runlength trigger code. If the code itself is
   present in the data as a n-mer (where n can be 1, 2, etc.) then it is encoded as <trigger-code><n><trigger-code> */

int ImageLoader::saveStack2RawRE(const char * filename, unsigned char**** data, const V3DLONG * sz, int datatype)
{
    int berror=0;


    if (datatype!=1) {
        printf("saveStack2RawRE : Only datatype 1 is supported\n");
        berror=1;
        return berror;
    }

        /* This function save a data stack to raw file */
                printf("size of [V3DLONG]=[%ld], [V3DLONG]=[%ld] [int]=[%ld], [short int]=[%ld], [double]=[%ld], [float]=[%ld]\n",
                       sizeof(V3DLONG), sizeof(V3DLONG), sizeof(int), sizeof(short int), sizeof(double), sizeof(float));
        V3DLONG i;

        FILE * fid = fopen(filename, "wb");
        if (!fid)
        {
                printf("Fail to open file for writing.\n");
                berror = 1;
                return berror;
        }

        /* Write header */
                         // raw_image_stack_by_hpeng
        char formatkey[] = "v3d_volume_runleng_encod";
        int lenkey = strlen(formatkey);

        V3DLONG nwrite = fwrite(formatkey, 1, lenkey, fid);
        if (nwrite!=lenkey)
        {
                printf("Fi le write error.\n");
                berror = 1;
                return berror;
        }

        char endianCodeMachine = checkMachineEndian();
        if (endianCodeMachine!='B' && endianCodeMachine!='L')
        {
                printf("This program only supports big- or little- endian but not other format. Cannot save data on this machine.\n");
                berror = 1;
                return berror;
        }

        nwrite = fwrite(&endianCodeMachine, 1, 1, fid);
        if (nwrite!=1)
        {
                printf("Error happened in file writing.\n");
                berror = 1;
                return berror;
        }

        //int b_swap = (endianCodeMachine==endianCodeData)?0:1;
        //int b_swap = 0; //for this machine itself, should not swap data.

        short int dcode = (short int)datatype;
        if (dcode!=1 && dcode!=2 && dcode!=4)
        {
                printf("Unrecognized data type code [%d]. This code is not supported in this version.\n", dcode);
                berror = 1;
                return berror;
        }

        //if (b_swap) swap2bytes((void *)&dcode);
        nwrite=fwrite(&dcode, 2, 1, fid); /* because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read. */
        if (nwrite!=1)
        {
                printf("Writing file error.\n");
                berror = 1;
                return berror;
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
                printf("Writing file error.\n");
                berror = 1;
                return berror;
        }

        V3DLONG totalUnit = 1;
        for (i=0;i<4;i++)
        {
                totalUnit *= sz[i];
        }

        V3DLONG maxSize = totalUnit;
        V3DLONG actualSize=0;
        V3DLONG sourceVoxelCount=0;
        unsigned char * imgRe = new unsigned char [totalUnit];

        printf("Allocated imgRe with maxSize=%d\n", totalUnit);

        V3DLONG xSize = sz[0];
        V3DLONG ySize = sz[1];
        V3DLONG zSize = sz[2];
        V3DLONG cSize = sz[3];

        V3DLONG xCube = (xSize/RE_CUBE_SIZE) + 1;
        V3DLONG yCube = (ySize/RE_CUBE_SIZE) + 1;
        V3DLONG zCube = (zSize/RE_CUBE_SIZE) + 1;

        V3DLONG cubeSize3=RE_CUBE_SIZE*RE_CUBE_SIZE*RE_CUBE_SIZE;

        unsigned char * cubeBuffer = new unsigned char [cubeSize3];
        //int cubeSize=0;
        V3DLONG channelSize = xSize*ySize*zSize;
        V3DLONG planeSize = xSize*ySize;

        for (V3DLONG cc=0;cc<cSize;cc++) {
            for (V3DLONG zc=0;zc<zCube;zc++) {
                for (V3DLONG yc=0;yc<yCube;yc++) {
                    for (V3DLONG xc=0;xc<xCube;xc++) {

                        // Reset buffer
                        //cubeSize=0;
                        for (i=0;i<cubeSize3;i++) {
                            cubeBuffer[i]=0;
                        }

                        // Populate buffer for current cube
                        V3DLONG zStart=zc*RE_CUBE_SIZE;
                        V3DLONG yStart=yc*RE_CUBE_SIZE;
                        V3DLONG xStart=xc*RE_CUBE_SIZE;

                        i=0L;

                        ////printf("Inner loop start : zStart=%d  yStart=%d  xStart=%d\n", zStart, yStart, xStart);
                        fflush(stdout);

                        for (V3DLONG z=zStart;z<zStart+RE_CUBE_SIZE;z++) {
                            for (V3DLONG y=yStart;y<yStart+RE_CUBE_SIZE;y++) {
                                for (V3DLONG x=xStart;x<xStart+RE_CUBE_SIZE;x++) {
                                    if (z<zSize && y<ySize && x<xSize && i<cubeSize3) {
                                        //printf("Adding cubeBuffer entry i=%d  z=%d  y=%d  x=%d\n", i, z, y, x);
                                        cubeBuffer[i++] = data[cc][z][y][x];
                                    } else {
                                        // printf("Inner loop skip : z=%d zSize=%d  y=%d ySize=%d  x=%d xSize=%d  i=%d\n",z,zSize,y,ySize,x,xSize,i);
                                    }
                                }
                            }
                        }

                        //printf("After inner loop\n");
                        //fflush(stdout);

                        if (i>0) {

                            sourceVoxelCount+=i;
                            V3DLONG compressionSize = compressCubeBuffer(imgRe+actualSize, cubeBuffer, i, maxSize-actualSize);
                            double compressionRatio=(sourceVoxelCount*1.0)/actualSize;
                            //printf("Compression-ratio=%f  sourceVoxels=%d  actualSize=%d  pre-size=%d   post-size=%d\n", compressionRatio, sourceVoxelCount, actualSize, i, compressionSize);
                            if (compressionSize==0) {
                                printf("Error during compressCubeBuffer\n");
                                berror=1;
                                return berror;
                            }
                            actualSize += compressionSize;

                        }
                    }
                }
            }
        }

        double finalCompressionRatio = (sourceVoxelCount*1.0)/actualSize;

        printf("Total original size=%d  sourceVoxelCount=%d  post-compression size=%d  ratio=%f\n", totalUnit, sourceVoxelCount, actualSize, finalCompressionRatio);

        printf("Writing file...");

        nwrite = fwrite(imgRe, unitSize, actualSize, fid);
        if (nwrite!=actualSize)
        {
                printf("Something wrong in file writing. The program wrote [%ld data points] but the file says there should be [%ld data points].\n", nwrite, totalUnit);
                berror = 1;
                return berror;
        }

        /* clean and return */

        fclose(fid);

        delete [] cubeBuffer;

        delete [] imgRe;

        printf("done.\n");

        return berror;
}

// We assume here that preBuffer is
V3DLONG ImageLoader::compressCubeBuffer(unsigned char * imgRe, unsigned char * preBuffer, V3DLONG bufferLength, V3DLONG spaceLeft) {
    bool debug=false;
    V3DLONG p=0;

    if (bufferLength==0) {
        printf("ImageLoader::compressCubeBuffer - unexpectedly received buffer of zero size\n");
        return 0;
    }

    //printf("\nStart compression, length=%d\n", bufferLength);

    unsigned char currentValue=0;
    unsigned char count=0;
    for (int i=0;i<bufferLength;i++) {
        if (p>=spaceLeft) {
            printf("ImageLoader::compressCubeBuffer ran out of space p=%d\n", p);
            return 0;
        }
        bool last=(i==(bufferLength-1));
        if (i==0 && !last) {
            // Initialize
            if (debug) printf("%d\t---\n",preBuffer[i]);
            currentValue=preBuffer[i];
            count=1;
        } else if (i==0 && last) {
            if (currentValue==RE_TRIGGER_CODE) {
                // We use this expanded form even if count is low in this case
                imgRe[p++]=RE_TRIGGER_CODE;
                imgRe[p++]=1;
                imgRe[p++]=RE_TRIGGER_CODE;
                if (debug) printf("%d\t%d\n",preBuffer[i],imgRe[p-3]);
                if (debug) printf("---\t%d\n",imgRe[p-2]);
                if (debug) printf("---\t%d\n",imgRe[p-1]);

            } else {
                if (debug) printf("%d\t%d\n",preBuffer[i],imgRe[p-1]);
                imgRe[p++]=currentValue;
            }
        } else if (!last) {
            if (count<255 && preBuffer[i]==currentValue) {
                if (debug) printf("%d\t---\n",preBuffer[i]);
                count++; // This is the easy case
            } else if (count==255 && preBuffer[i]==currentValue) {
                // We must flush because we are at max count
                imgRe[p++]=RE_TRIGGER_CODE;
                imgRe[p++]=count;
                imgRe[p++]=currentValue;
                if (debug) printf("%d\t%d\n",preBuffer[i],imgRe[p-3]);
                if (debug) printf("---\t%d\n",imgRe[p-2]);
                if (debug) printf("---\t%d\n",imgRe[p-1]);
                count=1; // currentValue stays the same
            } else if (preBuffer[i]!=currentValue) {
                // We don't have to check 255 because we are flushing anyway
                if (currentValue==RE_TRIGGER_CODE) {
                    // We use this expanded form even if count is low in this case
                    imgRe[p++]=RE_TRIGGER_CODE;
                    imgRe[p++]=count;
                    imgRe[p++]=RE_TRIGGER_CODE;
                    if (debug) printf("---\t%d\n",imgRe[p-3]);
                    if (debug) printf("---\t%d\n",imgRe[p-2]);
                    if (debug) printf("---\t%d\n",imgRe[p-1]);
                } else {
                    if (count<4) {
                        for (int j=0;j<count;j++) {
                            imgRe[p++]=currentValue;
                            if (debug) printf("---\t%d\n",imgRe[p-1]);
                        }
                    } else {
                        // Use compression
                        imgRe[p++]=RE_TRIGGER_CODE;
                        imgRe[p++]=count;
                        imgRe[p++]=currentValue;
                        if (debug) printf("---\t%d\n",imgRe[p-3]);
                        if (debug) printf("---\t%d\n",imgRe[p-2]);
                        if (debug) printf("---\t%d\n",imgRe[p-1]);
                    }
                }
                currentValue=preBuffer[i];
                if (debug) printf("%d\t---\n",currentValue);
                count=1;
            } else {
                printf("Should not get to this state in not-last possibilities\n");
                return 0;
            }
        } else if (last) {
            if (count<255 && preBuffer[i]==currentValue) {
                count++;
                if (currentValue==RE_TRIGGER_CODE) {
                    // We use this expanded form even if count is low in this case
                    imgRe[p++]=RE_TRIGGER_CODE;
                    imgRe[p++]=count;
                    imgRe[p++]=RE_TRIGGER_CODE;
                    if (debug) printf("%d\t%d\n",preBuffer[i],imgRe[p-3]);
                    if (debug) printf("---\t%d\n",imgRe[p-2]);
                    if (debug) printf("---\t%d\n",imgRe[p-1]);
                } else {
                    if (count<4) {
                        if (debug) printf("%d\t---\n",preBuffer[i]);
                        for (int j=0;j<count;j++) {
                            imgRe[p++]=currentValue;
                            if (debug) printf("---\t%d\n",imgRe[p-1]);
                        }
                    } else {
                        // Use compression
                        if (debug) printf("%d\t---\n",preBuffer[i]);
                        imgRe[p++]=RE_TRIGGER_CODE;
                        imgRe[p++]=count;
                        imgRe[p++]=currentValue;
                        if (debug) printf("---\t%d\n",imgRe[p-3]);
                        if (debug) printf("---\t%d\n",imgRe[p-2]);
                        if (debug) printf("---\t%d\n",imgRe[p-1]);
                    }
                }
            } else if (count==255 && preBuffer[i]==currentValue) {
                // We must flush because we are at max count
                imgRe[p++]=RE_TRIGGER_CODE;
                imgRe[p++]=count;
                imgRe[p++]=currentValue;
                if (debug) printf("---\t%d\n",imgRe[p-3]);
                if (debug) printf("---\t%d\n",imgRe[p-2]);
                if (debug) printf("---\t%d\n",imgRe[p-1]);
                count=1; // currentValue stays the same
                if (debug) printf("%d\t---\n",preBuffer[i]);
                if (currentValue==RE_TRIGGER_CODE) {
                    imgRe[p++]=RE_TRIGGER_CODE;
                    imgRe[p++]=count;
                    imgRe[p++]=RE_TRIGGER_CODE;
                    if (debug) printf("---\t%d\n",imgRe[p-3]);
                    if (debug) printf("---\t%d\n",imgRe[p-2]);
                    if (debug) printf("---\t%d\n",imgRe[p-1]);
                } else {
                    imgRe[p++]=currentValue;
                    if (debug) printf("---\t%d\n",imgRe[p-1]);
                }
            } else if (preBuffer[i]!=currentValue) {
                // We don't have to check 255 because we are flushing anyway
                if (currentValue==RE_TRIGGER_CODE) {
                    // We use this expanded form even if count is low in this case
                    imgRe[p++]=RE_TRIGGER_CODE;
                    imgRe[p++]=count;
                    imgRe[p++]=RE_TRIGGER_CODE;
                    if (debug) printf("---\t%d\n",imgRe[p-3]);
                    if (debug) printf("---\t%d\n",imgRe[p-2]);
                    if (debug) printf("---\t%d\n",imgRe[p-1]);
                } else {
                    if (count<4) {
                        for (int j=0;j<count;j++) {
                            imgRe[p++]=currentValue;
                            if (debug) printf("---\t%d\n",imgRe[p-1]);
                        }
                    } else {
                        // Use compression
                        imgRe[p++]=RE_TRIGGER_CODE;
                        imgRe[p++]=count;
                        imgRe[p++]=currentValue;
                        if (debug) printf("---\t%d\n",imgRe[p-3]);
                        if (debug) printf("---\t%d\n",imgRe[p-2]);
                        if (debug) printf("---\t%d\n",imgRe[p-1]);
                    }
                }
                currentValue=preBuffer[i];
                count=1;
                if (debug) printf("%d\t---\n",preBuffer[i]);
                if (currentValue==RE_TRIGGER_CODE) {
                    imgRe[p++]=RE_TRIGGER_CODE;
                    imgRe[p++]=count;
                    imgRe[p++]=RE_TRIGGER_CODE;
                    if (debug) printf("---\t%d\n",imgRe[p-3]);
                    if (debug) printf("---\t%d\n",imgRe[p-2]);
                    if (debug) printf("---\t%d\n",imgRe[p-1]);
                } else {
                    imgRe[p++]=currentValue;
                    if (debug) printf("---\t%d\n",imgRe[p-1]);
                }
            } else {
                printf("Should not get to this state in last possibilities\n");
                return 0;
            }
        }

    }
    return p;
}


int ImageLoader::loadRaw2StackRE(char * filename, My4DImage * & image)
{
        int berror = 0;

        QTime stopwatch;
        stopwatch.start();

        int datatype;

        FILE * fid = fopen(filename, "rb");
        if (!fid)
        {
                printf("Fail to open file for reading.\n");
                berror = 1;
                return berror;
        }

        fseek (fid, 0, SEEK_END);
        V3DLONG fileSize = ftell(fid);
        rewind(fid);

        /* Read header */

        char formatkey[] = "v3d_volume_runleng_encod";
        V3DLONG lenkey = strlen(formatkey);

#ifndef _MSC_VER //added by PHC, 2010-05-21
        if (fileSize<lenkey+2+4*4+1) // datatype has 2 bytes, and sz has 4*4 bytes and endian flag has 1 byte.
        {
                printf("The size of your input file is too small and is not correct, -- it is too small to contain the legal header.\n");
                printf("The fseek-ftell produces a file size = %ld.", fileSize);
                berror = 1;
                return berror;
        }
#endif

        char * keyread = new char [lenkey+1];
        if (!keyread)
        {
                printf("Fail to allocate memory.\n");
                berror = 1;
                return berror;
        }
        V3DLONG nread = fread(keyread, 1, lenkey, fid);
        if (nread!=lenkey)
        {
                printf("File unrecognized or corrupted file.\n");
                berror = 1;
                return berror;
        }
        keyread[lenkey] = '\0';

        V3DLONG i;
        if (strcmp(formatkey, keyread)) /* is non-zero then the two strings are different */
        {
                printf("Unrecognized file format.\n");
                if (keyread) {delete []keyread; keyread=0;}
                berror = 1;
                return berror;
        }

        char endianCodeData;
        fread(&endianCodeData, 1, 1, fid);
        if (endianCodeData!='B' && endianCodeData!='L')
        {
                printf("This program only supports big- or little- endian but not other format. Check your data endian.\n");
                berror = 1;
                if (keyread) {delete []keyread; keyread=0;}
                return berror;
        }

        char endianCodeMachine;
        endianCodeMachine = checkMachineEndian();
        if (endianCodeMachine!='B' && endianCodeMachine!='L')
        {
                printf("This program only supports big- or little- endian but not other format. Check your data endian.\n");
                berror = 1;
                if (keyread) {delete []keyread; keyread=0;}
                return berror;
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
                        printf("Unrecognized data type code [%d]. The file type is incorrect or this code is not supported in this version.\n", dcode);
                        if (keyread) {delete []keyread; keyread=0;}
                                berror = 1;
                        return berror;
        }

        if (datatype!=1) {
            printf("ImageLoader::loadRaw2StackRE : only datatype=1 supported\n");
            berror=1;
            return berror;
        }

        V3DLONG unitSize = datatype; // temporarily I use the same number, which indicates the number of bytes for each data point (pixel). This can be extended in the future.

        BIT32_UNIT mysz[4];
        mysz[0]=mysz[1]=mysz[2]=mysz[3]=0;
        int tmpn=fread(mysz, 4, 4, fid); // because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read.
        if (tmpn!=4)
        {
                printf("This program only reads [%d] units.\n", tmpn);
                berror=1;
                return berror;
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
                printf("Fail to allocate memory.\n");
                if (keyread) {delete []keyread; keyread=0;}
                berror = 1;
                return berror;
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
        V3DLONG decompressedBytes=totalUnit*unitSize;

        if (image) delete image;
        image = new My4DImage();

        unsigned char * compressedData = new unsigned char [compressedBytes];
        unsigned char * decompressedData = new unsigned char [decompressedBytes];

        V3DLONG remainingBytes = compressedBytes;
        V3DLONG nBytes2G = V3DLONG(1024)*V3DLONG(1024)*V3DLONG(1024)*V3DLONG(2);
        V3DLONG cntBuf = 0;
        while (remainingBytes>0)
        {
                V3DLONG curReadBytes = (remainingBytes<nBytes2G) ? remainingBytes : nBytes2G;
                V3DLONG curReadUnits = curReadBytes/unitSize;
                nread = fread(compressedData+cntBuf*nBytes2G, unitSize, curReadUnits, fid);
                if (nread!=curReadUnits)
                {
                        printf("Something wrong in file reading. The program reads [%ld data points] but the file says there should be [%ld data points].\n", nread, totalUnit);
                        delete [] compressedData;
                        delete image;
                        delete [] keyread;
                        berror = 1;
                        return berror;
                }

                remainingBytes -= nBytes2G;
                cntBuf++;
        }

        // swap the data bytes if necessary

        if (b_swap==1)
        {
                if (unitSize==2)
                {
                        for (i=0;i<totalUnit; i++)
                        {
                                swap2bytes((void *)(compressedData+i*unitSize));
                        }
                }
                else if (unitSize==4)
                {
                        for (i=0;i<totalUnit; i++)
                        {
                                swap4bytes((void *)(compressedData+i*unitSize));
                        }
                }
        }

        qDebug() << "Loading total time elapsed is " << stopwatch.elapsed() / 1000.0 << " seconds";
        stopwatch.restart();

        // Decompress data
        V3DLONG cp=0;
        V3DLONG dp=0;
        while(cp<compressedBytes) {
            if (compressedData[cp]==RE_TRIGGER_CODE) {
                cp++;
                for (int cpi=0;cpi<compressedData[cp];cpi++) {
                    decompressedData[dp++]=compressedData[cp+1];
                }
                cp++;
            } else {
                decompressedData[dp++]=compressedData[cp];
            }
            cp++;
        }
        if (dp!=decompressedBytes) {
            printf("Error - expected decompressed byte count=%ld but only found %ld\n",decompressedBytes, dp);
            delete [] compressedData;
            delete [] decompressedData;
            delete [] keyread;
            fclose(fid);
            berror=1;
            return berror;
        }

        qDebug() << "Decompression total time elapsed is " << stopwatch.elapsed() / 1000.0 << " seconds";
        stopwatch.restart();

        // Success - can delete compressedData
        delete [] compressedData; compressedData=0;

        // Transfer data to My4DImage
        image->loadImage(sz[0], sz[1], sz[2], sz[3], 1);
        unsigned char**** imageData = (unsigned char ****)image->getData();

        V3DLONG xSize = sz[0];
        V3DLONG ySize = sz[1];
        V3DLONG zSize = sz[2];
        V3DLONG cSize = sz[3];

        V3DLONG xCube = (xSize/RE_CUBE_SIZE) + 1;
        V3DLONG yCube = (ySize/RE_CUBE_SIZE) + 1;
        V3DLONG zCube = (zSize/RE_CUBE_SIZE) + 1;

        dp=0;

        for (V3DLONG cc=0;cc<cSize;cc++) {
            for (V3DLONG zc=0;zc<zCube;zc++) {
                for (V3DLONG yc=0;yc<yCube;yc++) {
                    for (V3DLONG xc=0;xc<xCube;xc++) {

                        V3DLONG zStart=zc*RE_CUBE_SIZE;
                        V3DLONG yStart=yc*RE_CUBE_SIZE;
                        V3DLONG xStart=xc*RE_CUBE_SIZE;

                        V3DLONG zl=zStart+RE_CUBE_SIZE;
                        V3DLONG yl=yStart+RE_CUBE_SIZE;
                        V3DLONG xl=xStart+RE_CUBE_SIZE;

                        for (V3DLONG z=zStart;z<zl;z++) {
                            if (z<zSize) {
                                unsigned char ** zp=imageData[cc][z];
                                for (V3DLONG y=yStart;y<yl;y++) {
                                    if (y<ySize) {
                                        unsigned char * yp=zp[y];
                                        for (V3DLONG x=xStart;x<xl;x++) {
                                            if (x<xSize) {
                                                yp[x]=decompressedData[dp++];
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        if (dp!=decompressedBytes) {
            printf("Error after transer to My4DImage - expected dp to equal total decompressed bytes=%ld but only=%ld\n", decompressedBytes, dp);
            delete [] decompressedData;
            delete image;
            delete [] keyread;
            berror=1;
            fclose(fid);
            return berror;
        }

        qDebug() << "Transfer to My4DImage total time elapsed is " << stopwatch.elapsed() / 1000.0 << " seconds";

        // clean and return
        if (keyread) {delete [] keyread; keyread = 0;}
        delete [] decompressedData;
        fclose(fid); //bug fix on 060412
        return berror;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

int ImageLoader::createDfValueByKeyMap(unsigned char *dfValueByKey) {
    // First clear the map
    for (int i=0;i<4000;i++) {
        dfValueByKey[i]=0;
    }
    // This assumes dfValueByKey is an array of unsigned int size=4000
    unsigned char vlist [4];
    vlist[0]=0;
    vlist[1]=1;
    vlist[2]=2;
    vlist[3]=3;
    unsigned char value=0;
    for (int l0=0;l0<4;l0++) {
        for (int l1=0;l1<4;l1++) {
            for (int l2=0;l2<4;l2++) {
                for (int l3=0;l3<4;l3++) {
                    int key=vlist[l0]*1000 + vlist[l1]*100 + vlist[l2]*10 + vlist[l3];
                    if (key>=4000) {
                        printf("ImageLoader::createDfValueByKeyMap key index unexpectedly greater than 3999\n");
                        return 1;
                    } else if (value==256) {
                        printf("ImageLoader::createDfValueByKeyMap unexpectedly ran out of values\n");
                        return 1;
                    }
                    dfValueByKey[key]=value++;
                }
            }
        }
    }
    return 0;
}

int ImageLoader::saveStack2RawPBD(const char * filename, unsigned char**** data, const V3DLONG * sz, int datatype)
{
    int berror=0;

    unsigned char dfValueByKey [4000];

    createDfValueByKeyMap(dfValueByKey);

    if (datatype!=1) {
        printf("saveStack2RawPBD : Only datatype 1 is supported\n");
        berror=1;
        return berror;
    }

        /* This function save a data stack to raw file */
                printf("size of [V3DLONG]=[%ld], [V3DLONG]=[%ld] [int]=[%ld], [short int]=[%ld], [double]=[%ld], [float]=[%ld]\n",
                       sizeof(V3DLONG), sizeof(V3DLONG), sizeof(int), sizeof(short int), sizeof(double), sizeof(float));
        V3DLONG i;

        FILE * fid = fopen(filename, "wb");
        if (!fid)
        {
                printf("Fail to open file for writing.\n");
                berror = 1;
                return berror;
        }

        /* Write header */
                         // raw_image_stack_by_hpeng
        char formatkey[] = "v3d_volume_pkbitdf_encod";
        int lenkey = strlen(formatkey);

        V3DLONG nwrite = fwrite(formatkey, 1, lenkey, fid);
        if (nwrite!=lenkey)
        {
                printf("File write error.\n");
                berror = 1;
                return berror;
        }

        char endianCodeMachine = checkMachineEndian();
        if (endianCodeMachine!='B' && endianCodeMachine!='L')
        {
                printf("This program only supports big- or little- endian but not other format. Cannot save data on this machine.\n");
                berror = 1;
                return berror;
        }

        nwrite = fwrite(&endianCodeMachine, 1, 1, fid);
        if (nwrite!=1)
        {
                printf("Error happened in file writing.\n");
                berror = 1;
                return berror;
        }

        //int b_swap = (endianCodeMachine==endianCodeData)?0:1;
        //int b_swap = 0; //for this machine itself, should not swap data.

        short int dcode = (short int)datatype;
        if (dcode!=1 && dcode!=2 && dcode!=4)
        {
                printf("Unrecognized data type code [%d]. This code is not supported in this version.\n", dcode);
                berror = 1;
                return berror;
        }

        //if (b_swap) swap2bytes((void *)&dcode);
        nwrite=fwrite(&dcode, 2, 1, fid); /* because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read. */
        if (nwrite!=1)
        {
                printf("Writing file error.\n");
                berror = 1;
                return berror;
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
                printf("Writing file error.\n");
                berror = 1;
                return berror;
        }

        V3DLONG totalUnit = 1;
        for (i=0;i<4;i++)
        {
                totalUnit *= sz[i];
        }

        V3DLONG maxSize = totalUnit;
        V3DLONG actualSize=0;
        V3DLONG sourceVoxelCount=0;
        unsigned char * imgRe = new unsigned char [totalUnit];

        printf("Allocated imgRe with maxSize=%d\n", totalUnit);

        V3DLONG xSize = sz[0];
        V3DLONG ySize = sz[1];
        V3DLONG zSize = sz[2];
        V3DLONG cSize = sz[3];

        V3DLONG xCube = (xSize/RE_CUBE_SIZE) + 1;
        V3DLONG yCube = (ySize/RE_CUBE_SIZE) + 1;
        V3DLONG zCube = (zSize/RE_CUBE_SIZE) + 1;

        V3DLONG cubeSize3=RE_CUBE_SIZE*RE_CUBE_SIZE*RE_CUBE_SIZE;

        unsigned char * cubeBuffer = new unsigned char [cubeSize3];
        //int cubeSize=0;
        V3DLONG channelSize = xSize*ySize*zSize;
        V3DLONG planeSize = xSize*ySize;

        for (V3DLONG cc=0;cc<cSize;cc++) {
            for (V3DLONG zc=0;zc<zCube;zc++) {
                for (V3DLONG yc=0;yc<yCube;yc++) {
                    for (V3DLONG xc=0;xc<xCube;xc++) {

                        // Reset buffer
                        //cubeSize=0;
                        for (i=0;i<cubeSize3;i++) {
                            cubeBuffer[i]=0;
                        }

                        // Populate buffer for current cube
                        V3DLONG zStart=zc*RE_CUBE_SIZE;
                        V3DLONG yStart=yc*RE_CUBE_SIZE;
                        V3DLONG xStart=xc*RE_CUBE_SIZE;

                        i=0L;

                        ////printf("Inner loop start : zStart=%d  yStart=%d  xStart=%d\n", zStart, yStart, xStart);
                        fflush(stdout);

                        for (V3DLONG z=zStart;z<zStart+RE_CUBE_SIZE;z++) {
                            for (V3DLONG y=yStart;y<yStart+RE_CUBE_SIZE;y++) {
                                for (V3DLONG x=xStart;x<xStart+RE_CUBE_SIZE;x++) {
                                    if (z<zSize && y<ySize && x<xSize && i<cubeSize3) {
                                        //printf("Adding cubeBuffer entry i=%d  z=%d  y=%d  x=%d\n", i, z, y, x);
                                        cubeBuffer[i++] = data[cc][z][y][x];
                                    } else {
                                        // printf("Inner loop skip : z=%d zSize=%d  y=%d ySize=%d  x=%d xSize=%d  i=%d\n",z,zSize,y,ySize,x,xSize,i);
                                    }
                                }
                            }
                        }

                        //printf("After inner loop\n");
                        //fflush(stdout);

                        if (i>0) {

                            sourceVoxelCount+=i;
                            V3DLONG compressionSize = compressCubeBufferPBD(imgRe+actualSize, cubeBuffer, i, maxSize-actualSize, dfValueByKey);
                            double compressionRatio=(sourceVoxelCount*1.0)/actualSize;
                            printf("Compression-ratio=%f  sourceVoxels=%d  actualSize=%d  pre-size=%d   post-size=%d\n", compressionRatio, sourceVoxelCount, actualSize, i, compressionSize);
                            if (compressionSize==0) {
                                printf("Error during compressCubeBuffer\n");
                                berror=1;
                                return berror;
                            }
                            actualSize += compressionSize;

                        }
                    }
                }
            }
        }

        double finalCompressionRatio = (sourceVoxelCount*1.0)/actualSize;

        printf("Total original size=%d  sourceVoxelCount=%d  post-compression size=%d  ratio=%f\n", totalUnit, sourceVoxelCount, actualSize, finalCompressionRatio);

        printf("Writing file...");

        nwrite = fwrite(imgRe, unitSize, actualSize, fid);
        if (nwrite!=actualSize)
        {
                printf("Something wrong in file writing. The program wrote [%ld data points] but the file says there should be [%ld data points].\n", nwrite, totalUnit);
                berror = 1;
                return berror;
        }

        /* clean and return */

        fclose(fid);

        delete [] cubeBuffer;

        delete [] imgRe;

        printf("done.\n");

        return berror;
}


// We assume here that preBuffer is
V3DLONG ImageLoader::compressCubeBufferPBD(unsigned char * imgRe, unsigned char * preBuffer, V3DLONG bufferLength, V3DLONG spaceLeft, unsigned char * dfmap) {
    bool debug=false;
    V3DLONG p=0;

    if (debug) {
        printf("\n");
    }

    if (bufferLength==0) {
        printf("ImageLoader::compressCubeBufferPBD - unexpectedly received buffer of zero size\n");
        return 0;
    }

    //printf("\nStart compression, length=%d\n", bufferLength);

    unsigned char currentValue=0; // This is important - by definition current value starts at zero
    int dbuffer[95];
    V3DLONG activeLiteralIndex=-1; // if -1 this means there is no literal mode started
    for (int i=0;i<bufferLength;i++) {
        if (p>=spaceLeft) {
            printf("ImageLoader::compressCubeBufferPBD ran out of space p=%d\n", p);
            return 0;
        }
        bool last=(i==(bufferLength-1));

        // From this point we assume the result has been accumulating in imgRe, and at this moment
        // we are searching for the best approach for the next segment. First, we will try reading
        // the next value, and testing what the runlength encoding efficiency would be. The
        // efficiency is simply the (number of bytes encoded / actual bytes). Next, we will test
        // the different encoding, and (depending on its reach), see what its efficiency is.
        // The minimum number of unencoded bytes to use run-length encoding for is 3 (code, value).
        // The minimum number of unencoded bytes to use difference encoding is 3 also (code, value).

        int reTest=1;
        int currentPosition=i;
        currentValue=preBuffer[i];
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
            if (debug) {
                for (int d=0;d<reTest;d++) {
                    printf("r");
                }
            }
        } else {
            // We need to evaluate difference encoding starting with the prior value
            unsigned char priorValue=0;
            if (i>0) {
                priorValue=preBuffer[i-1];
            }
            int unitsToCheck=bufferLength-i;
            if (unitsToCheck>95) {
                unitsToCheck=95; // 95 is max supported number of differences
            }
            int c=i;
            for (int j=0;j<95;j++) {
                dbuffer[j]=0; // clear the difference buffer
            }
            for (;c<i+unitsToCheck;c++) {
                int d=preBuffer[c] - priorValue;
                if (d>2 || d<-1) {
                    break;
                }
                priorValue+=d; // since by definition the diff encoding is cumulative
                if (d==-1) {
                    d=3; // we use this to represent -1 in the dbuffer for key lookup later
                }
                dbuffer[c-i]=d;
            }
            double dfEfficiency = ((c-i)*1.0)/(((c-i)/4)+2); // The denominator includes the coding byte and the encoding bytes
            if (reEfficiency>dfEfficiency && reEfficiency>1.0) {
                // Then use RE
                imgRe[p++]=reTest+127; // The code for number of repeats
                imgRe[p++]=currentValue; // The repeated value
                i+=(reTest-1); // because will increment one more time at top of loop
                activeLiteralIndex=-1;
                if (debug) {
                    for (int d=0;d<reTest;d++) {
                        printf("r");
                    }
                }
            } else if (dfEfficiency>1.0) {
                // First, encode the number of units we expect
                imgRe[p++]=c-i+32;
                // Then use DF. We want to move forward in units of 4, and pick the correct encoding.
                // Note that is doesn't matter if we pad extra 0s because we know what the correct
                // length is from above.
                int cp=i;
                int d0,d1,d2,d3;
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
                    int lookupKey=1000*d0+100*d1+10*d2+d3;
                    unsigned char dc=dfmap[lookupKey];
                    //printf("\ndebug: i=%d  c=%d  cp=%d  lookupKey=%d  p=%ld\n", i, c, cp, lookupKey, p);
                    fflush(stdout);
                    imgRe[p++]=dc;
                    cp+=4; // Move ahead 4 steps at a time
                }
                activeLiteralIndex=-1;
                if (debug) {
                    for (int d=0;d<(c-i);d++) {
                        printf("d");
                    }
                }
                i=c-1; // will increment at top
            } else {
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
                if (debug) {
                    printf("l");
                }
            }
        }
    }
    printf("\n");
    return p;
}

int ImageLoader::loadRaw2StackPBD(char * filename, My4DImage * & image) {
    return 1;
}






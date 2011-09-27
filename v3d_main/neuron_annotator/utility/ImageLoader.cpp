
#include <QString>
#include <QtCore>
#include <QDir>
#include "../../v3d/v3d_core.h"
#include "ImageLoader.h"

using namespace std;

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
        My4DImage* image=loadImage(inputFileList.at(i));
        if (image!=0) {
            imageList.append(image);
        }
        QString filePrefix=getFilePrefix(inputFileList.at(i));
        QString saveFilepath=filePrefix.append(".v3dvre");
        V3DLONG sz[4];
        sz[0] = image->getXDim();
        sz[1] = image->getYDim();
        sz[2] = image->getZDim();
        sz[3] = image->getCDim();
        unsigned char**** data = (unsigned char****)image->getData();
        saveStack2RawRE(saveFilepath.toAscii().data(), data, sz, image->getDatatype());
        //image->saveImage(saveFilepath.toAscii().data());
    }

    qDebug() << "Loading total time elapsed is " << stopwatch.elapsed() / 1000.0 << " seconds";
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

    const int CUBE_SIZE=6;

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

        V3DLONG xCube = (xSize/CUBE_SIZE) + 1;
        V3DLONG yCube = (ySize/CUBE_SIZE) + 1;
        V3DLONG zCube = (zSize/CUBE_SIZE) + 1;

        V3DLONG cubeSize3=CUBE_SIZE*CUBE_SIZE*CUBE_SIZE;

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
                        V3DLONG zStart=zc*CUBE_SIZE;
                        V3DLONG yStart=yc*CUBE_SIZE;
                        V3DLONG xStart=xc*CUBE_SIZE;

                        i=0L;

                        ////printf("Inner loop start : zStart=%d  yStart=%d  xStart=%d\n", zStart, yStart, xStart);
                        fflush(stdout);

                        for (V3DLONG z=zStart;z<zStart+CUBE_SIZE;z++) {
                            for (V3DLONG y=yStart;y<yStart+CUBE_SIZE;y++) {
                                for (V3DLONG x=xStart;x<xStart+CUBE_SIZE;x++) {
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
V3DLONG ImageLoader::compressCubeBuffer(unsigned char * imgRe, unsigned char * preBuffer, V3DLONG bufferLength, V3DLONG spaceLeft) {
    const unsigned char RE_TRIGGER_CODE=249;
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





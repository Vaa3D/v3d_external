
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
        QString saveFilepath=filePrefix.append(".v3drawre");
        V3DLONG sz[4];
        sz[0] = image->getXDim();
        sz[1] = image->getYDim();
        sz[2] = image->getZDim();
        sz[3] = image->getCDim();
        saveStack2RawRE(saveFilepath.toAscii().data(), (const unsigned char *)image->getData(), sz, image->getDatatype());
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

int ImageLoader::saveStack2RawRE(const char * filename, const unsigned char * img, const V3DLONG * sz, int datatype)
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
        char formatkey[] = "raw_hpeng_w_runlen_encod";
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

        V3DLONG maxSize = totalUnit*2;
        V3DLONG actualSize=0;
        V3DLONG sourceVoxelCount=0;
        unsigned char * imgRe = new unsigned char [maxSize];

        V3DLONG xSize = sz[0];
        V3DLONG ySize = sz[1];
        V3DLONG zSize = sz[2];
        V3DLONG cSize = sz[3];

        V3DLONG xCube = (xSize/8) + 1;
        V3DLONG yCube = (ySize/8) + 1;
        V3DLONG zCube = (zSize/8) + 1;

        unsigned char cubeBuffer [512];
        int cubeSize=0;
        V3DLONG channelSize = xSize*ySize*zSize;
        V3DLONG planeSize = xSize*ySize;

        for (V3DLONG cc=0;cc<cSize;cc++) {
            for (V3DLONG zc=0;zc<zCube;zc++) {
                for (V3DLONG yc=0;yc<yCube;yc++) {
                    for (V3DLONG xc=0;xc<xCube;xc++) {
                        // Reset buffer
                        cubeSize=0;
                        for (i=0;i<512;i++) {
                            cubeBuffer[i]=0;
                        }
                        // Populate buffer for current cube
                        V3DLONG zStart=zc*8;
                        V3DLONG yStart=yc*8;
                        V3DLONG xStart=xc*8;

                        i=0;

                        for (V3DLONG z=zStart;z<zStart+8;z++) {
                            for (V3DLONG y=yStart;y<yStart+8;y++) {
                                for (V3DLONG x=xStart;x<xStart+8;x++) {
                                    if (z<zSize && y<ySize && x<xSize && i<512) {
                                        cubeBuffer[i++] = *(img + channelSize*cc + planeSize*z + xSize*y + x);
                                    }
                                }
                            }
                        }

                        sourceVoxelCount+=i;
                        int compressionSize = compressCubeBuffer(imgRe+actualSize, cubeBuffer, i, maxSize-actualSize);
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

        printf("Total original size=%d  sourceVoxelCount=%d  post-compression size=%d\n", totalUnit, sourceVoxelCount, actualSize);

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

        printf("done.\n");

        return berror;
}

// We assume here that preBuffer is
V3DLONG ImageLoader::compressCubeBuffer(unsigned char * imgRe, unsigned char * preBuffer, V3DLONG bufferLength, V3DLONG spaceLeft) {
    unsigned char postBuffer[5000];
    const unsigned char RE_TRIGGER_CODE=249;
    V3DLONG p=0;

    if (bufferLength==0) {
        printf("ImageLoader::compressCubeBuffer - unexpectedly received buffer of zero size\n");
        return 0;
    }

    unsigned char currentValue=0;
    unsigned char count=0;
    for (int i=0;i<bufferLength;i++) {
        if (p>=spaceLeft) {
            printf("ImageLoader::compressCubeBuffer ran out of space p=%d\n", p);
            return 0;
        }
        bool last=(i==(bufferLength-1));
        if (i==0) {
            // Initialize
            currentValue=preBuffer[i];
            count=1;
        } else {
            if ( count<255 && preBuffer[i]==currentValue && !last ) {
                // We are free to continue counting
                count++;
            } else if (count==255 && preBuffer[i]==currentValue && !last) {
                // We must flush because we are at max count
                imgRe[p++]=RE_TRIGGER_CODE;
                imgRe[p++]=count;
                imgRe[p++]=currentValue;
                count=1;
            } else if (preBuffer[i]!=currentValue || last) {
                // We have changed currentValues
                if (currentValue==RE_TRIGGER_CODE) {
                    imgRe[p++]=RE_TRIGGER_CODE;
                    imgRe[p++]=count;
                    imgRe[p++]=RE_TRIGGER_CODE;
                } else {
                    if (count<4) {
                        for (int j=0;j<count;j++) {
                            imgRe[p++]=currentValue;
                        }
                    } else {
                        imgRe[p++]=RE_TRIGGER_CODE;
                        imgRe[p++]=count;
                        imgRe[p++]=currentValue;
                    }
                    currentValue=preBuffer[i];
                    count=1;
                }
            } else {
                printf("ImageLoader::compressCubeBuffer - we should never get to this point i=%d count=%d currentValue=%d\n", i, count, currentValue);
                return 0;
            }
        }
    }
    return p;
}





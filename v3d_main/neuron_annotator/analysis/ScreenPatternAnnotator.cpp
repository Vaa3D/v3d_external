#include "ScreenPatternAnnotator.h"
#include "../utility/ImageLoader.h"
#include "../../v3d/histogramsimple.h"

ScreenPatternAnnotator::ScreenPatternAnnotator()
{
    inputImage=0;
    patternChannelIndex=-1;
    lut16Color=0;
    imageGlobal16ColorImage=0;
}

ScreenPatternAnnotator::~ScreenPatternAnnotator()
{
    if (inputImage!=0) {
        delete inputImage;
    }
    if (lut16Color!=0) {
        delete [] lut16Color;
    }
    if (imageGlobal16ColorImage!=0) {
        delete imageGlobal16ColorImage;
    }
}

bool ScreenPatternAnnotator::execute()
{
    inputImage=new My4DImage();
    ImageLoader imageLoader;
    imageLoader.loadImage(inputImage, inputStackFilepath);
    if (inputImage->getDatatype()!=V3D_UINT8) {
        qDebug() << "ScreenPatternGenerator currently only supports 8-bit input data";
        return false;
    }
    // Compute Global 256-bin Histogram
    V3DLONG xSize=inputImage->getXDim();
    V3DLONG ySize=inputImage->getYDim();
    V3DLONG zSize=inputImage->getZDim();
    V3DLONG channelSize=xSize*ySize*zSize;
    global256BinHistogram.compute(inputImage->getRawData() + channelSize*patternChannelIndex ,channelSize);
    V3DLONG* global256BinHistogramArray=global256BinHistogram.getHistogram();

    // Create Global 16-Color Image
    lut16Color=create16Color8BitLUT();
    imageGlobal16ColorImage=create3DHeatmapFromChannel(inputImage, patternChannelIndex, lut16Color);
    ImageLoader imageLoaderForSave;
    QString filepathToSave("heatmap16Color.v3dpbd");
    bool saveHeatmapStatus=imageLoaderForSave.saveImage(imageGlobal16ColorImage, filepathToSave);
    if (!saveHeatmapStatus) {
        qDebug() << "ScreenPatternAnnotator::execute() Error during save of heatmap file";
        return false;
    }

    return true;
}

int ScreenPatternAnnotator::processArgs(vector<char*> *argList)
{
    for (int i=0;i<argList->size();i++) {
        QString arg=(*argList)[i];
        bool done=false;
        if (arg=="-input") {
            do {
                QString possibleFile=(*argList)[++i];
                if (!possibleFile.startsWith("-")) {
                    inputStackFilepath=possibleFile;
                } else {
                    done=true;
                    i--; // rewind
                }
            } while(!done && i<(argList->size()-1));
        } else if (arg=="-pattern_channel") {
            QString patternChannelIndexString=(*argList)[++i];
            patternChannelIndex=patternChannelIndexString.toInt();
        } else if (arg=="-prefix") {
            outputPrefix=(*argList)[++i];
        } else if (arg=="-dir") {
            outputDirectoryPath=(*argList)[++i];
        }
    }
    if (inputStackFilepath.length()<1) {
        qDebug() << "inputStackFilepath has invalid length";
        return 1;
    } else if (patternChannelIndex<0) {
        qDebug() << "patternChannelIndex must be greater than zero";
        return 1;
    } else if (outputPrefix.length()<1) {
        qDebug() << "outputPrefix has invalid length";
        return 1;
    } else if (outputDirectoryPath.length()<1) {
        qDebug() << "outputDirectoryPath has invalid length";
        return 1;
    }
    return 0;
}

v3d_uint8 * ScreenPatternAnnotator::create16Color8BitLUT()
{
    v3d_uint8 * lut16 = new v3d_uint8[256*3];

    for (int i=0;i<16;i++) {
        for (int j=0;j<16;j++) {
            int index=i*16+j;
            if (i==0) {
                lut16[index]    = 0;
                lut16[index+256]= 0;
                lut16[index+512]= 0;
            } else if (i==1) {
                lut16[index]    = 1;
                lut16[index+256]= 1;
                lut16[index+512]= 171;
            } else if (i==2) {
                lut16[index]    = 1;
                lut16[index+256]= 1;
                lut16[index+512]= 224;
            } else if (i==3) {
                lut16[index]    = 0;
                lut16[index+256]= 110;
                lut16[index+512]= 255;
            } else if (i==4) {
                lut16[index]    = 1;
                lut16[index+256]= 171;
                lut16[index+512]= 254;
            } else if (i==5) {
                lut16[index]    = 1;
                lut16[index+256]= 224;
                lut16[index+512]= 254;
            } else if (i==6) {
                lut16[index]    = 1;
                lut16[index+256]= 254;
                lut16[index+512]= 1;
            } else if (i==7) {
                lut16[index]    = 190;
                lut16[index+256]= 255;
                lut16[index+512]= 0;
            } else if (i==8) {
                lut16[index]    = 255;
                lut16[index+256]= 255;
                lut16[index+512]= 0;
            } else if (i==9) {
                lut16[index]    = 255;
                lut16[index+256]= 224;
                lut16[index+512]= 0;
            } else if (i==10) {
                lut16[index]    = 255;
                lut16[index+256]= 141;
                lut16[index+512]= 0;
            } else if (i==11) {
                lut16[index]    = 250;
                lut16[index+256]= 94;
                lut16[index+512]= 0;
            } else if (i==12) {
                lut16[index]    = 245;
                lut16[index+256]= 0;
                lut16[index+512]= 0;
            } else if (i==13) {
                lut16[index]    = 245;
                lut16[index+256]= 0;
                lut16[index+512]= 185;
            } else if (i==14) {
                lut16[index]    = 222;
                lut16[index+256]= 180;
                lut16[index+512]= 222;
            } else if (i==15) {
                lut16[index]    = 255;
                lut16[index+256]= 255;
                lut16[index+512]= 255;
            }
        }
    }
    return lut16;
}

// This method below assumes the heatmap has 256 entries in the order 256 entries for red, then 256 entries for green, then 256 entries for blue.
My4DImage * ScreenPatternAnnotator::create3DHeatmapFromChannel(My4DImage * sourceImage, V3DLONG sourceChannel, v3d_uint8 * lookupTable)
{
    if (sourceImage->getDatatype()!=V3D_UINT8) {
        qDebug() << "ScreenPatternAnnotator::create3DHeatmapFromChannel() only supports datatype=1";
        return 0;
    }
    My4DImage * targetImage = new My4DImage();
    targetImage->loadImage(sourceImage->getXDim(), sourceImage->getYDim(), sourceImage->getZDim(), 3 /* R, G, B */, sourceImage->getDatatype());
    v3d_uint8 * sourceData = sourceImage->getRawDataAtChannel(sourceChannel);
    v3d_uint8 * targetDataR = targetImage->getRawDataAtChannel(0L);
    v3d_uint8 * targetDataG = targetImage->getRawDataAtChannel(1L);
    v3d_uint8 * targetDataB = targetImage->getRawDataAtChannel(2L);
    V3DLONG totalVoxels=sourceImage->getXDim()*sourceImage->getYDim()*sourceImage->getZDim();
    for (V3DLONG i=0;i<totalVoxels;i++) {
        v3d_uint8 value=sourceData[i];
        targetDataR[i]=lookupTable[value];
        targetDataG[i]=lookupTable[value+256];
        targetDataB[i]=lookupTable[value+512];
    }
    return targetImage;
}



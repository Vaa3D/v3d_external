#include <qtconcurrentrun.h>
#include <cmath>
#include <vector>
#include "CellCounter3D.h"
#include "../v3d/v3d_core.h"

#define DIALATE 0
#define ERODE 1

#define CELL 0
#define BACKGROUND 1

int CellCounter3D::ARG_STATUS_USAGE=0;
int CellCounter3D::ARG_STATUS_OK=1;
int CellCounter3D::ARG_STATUS_HELP=2;

class SleepThread : QThread {
public:
    SleepThread() {}
    void msleep(int miliseconds) {
        QThread::msleep(miliseconds);
    }
};

CellCounter3D::CellCounter3D()
{
    image=0;
    errorStatus=0;
    loadedFromInputFile=false;

    // Command-line parameters
    INITIAL_SIGNAL_THRESHOLD=10;
    INITIAL_BACKGROUND_THRESHOLD=10;
    SIGMA_NORMALIZATION=2.0;
    NORMALIZATION_THRESHOLD=20;
    EROSION_CYCLES=3;
    EROSION_ELEMENT_SIZE=2;
    EROSION_THRESHOLD=44;
    DIALATION_CYCLES=3;
    DIALATION_ELEMENT_SIZE=2;
    DIALATION_THRESHOLD=44;
    CS_CENTER_RADIUS=11.0;
    CS_CENTER_VALUE=1.0;
    CS_SURROUND_RADIUS=2.0;
    CS_SURROUND_VALUE=-2.0;
    CS_THRESHOLD_START=70;
    CS_THRESHOLD_INCREMENT=5;
    CS_THRESHOLD_MAX=180;
    MARK_SIZE=2;
    MARK_RADIUS=10;
    MARK_COLOR[0]=1000;
    MARK_COLOR[1]=0;
    MARK_COLOR[2]=1000;
    SIGNAL_COLOR[0]=255;
    SIGNAL_COLOR[1]=255;
    SIGNAL_COLOR[2]=255;
    MIN_REGION_VOXELS=100;
    MAX_REGION_VOXELS=40000;
}

CellCounter3D::~CellCounter3D() {
    if (image!=0 && loadedFromInputFile) {
        delete image;
    }
}

void CellCounter3D::loadMy4DImage(My4DImage* image) {
    this->image=image;
    xDim=image->getXDim();
    yDim=image->getYDim();
    zDim=image->getZDim();
}

void CellCounter3D::loadInputFile() {
    loadedFromInputFile=true;
    My4DImage tmpImage;
    image = new My4DImage();
    tmpImage.loadImage(inputFilePath.toLocal8Bit().data());
    CellCounter3D::convertMy4DImage2channelToRGB(tmpImage, *image);
    this->loadMy4DImage(image);
}


bool CellCounter3D::findCells() {
    if (image==0) {
        qDebug() << "CellCounter3D::findCells() - no image loaded";
        return false;
    }

    unsigned char**** data = (unsigned char****)image->getData();

    long dataChannel0Count=countNonZero(data[CELL]);
    long dataChannel1Count=countNonZero(data[BACKGROUND]);

    qDebug() << "data channel 0 voxel count = " << dataChannel0Count;
    qDebug() << "data channel 1 voxel count = " << dataChannel1Count;

    // Step 1: create buffers
    //
    // We will preserve the original data until the last step in which we
    // mark the location of the cells. We will need two working copies of
    // each channel, one to serve as the source and other the target.
    qDebug() << "Allocating working space";
    workingData = new unsigned char****[2];
    for (int w=0;w<2;w++) {
        workingData[w] = new unsigned char***[1]; // for now we are only using 1 channel (CELL) so conserve memory
        for (int c=0;c<2;c++) {
            workingData[w][c] = new unsigned char**[zDim];
            for (int z=0;z<zDim;z++) {
                workingData[w][c][z] = new unsigned char*[yDim];
                for (int y=0;y<yDim;y++) {
                    workingData[w][c][z][y] = new unsigned char[xDim];
                    for (int x=0;x<xDim;x++) {
                        workingData[w][c][z][y][x]=0;
                    }
                }
            }
        }
    }

    int w=0;

    // Step 2: initial threshold
    qDebug() << "Performing initial threshold";
    for (int z=0;z<zDim;z++) {
        for (int y=0;y<yDim;y++) {
            for (int x=0;x<xDim;x++) {
                unsigned char red=data[CELL][z][y][x];
                if (red < INITIAL_SIGNAL_THRESHOLD) {
                    red = 0;
                }
                unsigned char green=data[BACKGROUND][z][y][x];
                if (green < INITIAL_BACKGROUND_THRESHOLD) {
                    green = 0;
                }
                if (red > green) {
                    workingData[w][CELL][z][y][x]=red;
                } else {
                    workingData[w][CELL][z][y][x]=0;
                }
            }
        }
    }

    normalizeNonZero(workingData[w][CELL], SIGMA_NORMALIZATION);

    clearChannel(BACKGROUND, data);

    long initialCellVoxelCount=countNonZero(workingData[w][CELL]);

    qDebug() << "Initial cell voxel count = " << initialCellVoxelCount;

    int w1 = w;
    int w2 = (w1==0 ? 1 : 0);

    applyBinaryThreshold(workingData[w1][CELL], workingData[w2][CELL], NORMALIZATION_THRESHOLD);
    w1 = (w1==0 ? 1 : 0);
    w2 = (w2==0 ? 1 : 0);

    // Debug - let's have a look
    //copyToImage(CELL, workingData[w1], data, false /* non-zero only */, false /* markAllChannels */);
    //return true;

    //gammaFilter(workingData[w1][CELL], GAMMA_CORRECTION);

    // Step 3: Dialtion/Erosion

   qDebug() << "Starting dialation-erosion";

   long voxelCount=initialCellVoxelCount;

    // Debug - let's have a look
    //copyToImage(CELL, workingData[w1], data, false /* non-zero only */, false /* markAllChannels */);
    //return;

    // Erode CELL to better define borders
    int lastVoxelCount=voxelCount;
    int i=0;
    for (i=0;i<EROSION_CYCLES;i++) {
        qDebug() << "Beginning erosion iteration = " << i;
        dialateOrErode(ERODE, workingData[w1][CELL], workingData[w2][CELL], EROSION_ELEMENT_SIZE, EROSION_THRESHOLD);
        voxelCount=countNonZero(workingData[w2][CELL]);
        if (voxelCount==lastVoxelCount) {
            qDebug() << "Converged after " << i << " erosion cycles";
            break;
        } else {
            lastVoxelCount=voxelCount;
        }
        qDebug() << "           voxel count = " << lastVoxelCount;
        w1 = (w1==0 ? 1 : 0);
        w2 = (w2==0 ? 1 : 0);
    }

    // Dialate CELL signal to fill nucleus and gaps.
    for (int i=0;i<DIALATION_CYCLES;i++) {
         qDebug() << "Beginning dialation iteration = " << i;
         dialateOrErode(DIALATE, workingData[w1][CELL], workingData[w2][CELL], DIALATION_ELEMENT_SIZE, DIALATION_THRESHOLD);
         voxelCount=countNonZero(workingData[w2][CELL]);
         qDebug() << "Updated voxel count = " << voxelCount;
         w1 = (w1==0 ? 1 : 0);
         w2 = (w2==0 ? 1 : 0);
     }

    // Debug - let's have a look
    //copyToImage(CELL, workingData[w1], data, false /* non-zero only */, false /* markAllChannels */);
    //return true;

    centerSurroundFilter(workingData[w1][CELL], workingData[w2][CELL], CS_CENTER_RADIUS, CS_CENTER_VALUE, CS_SURROUND_RADIUS, CS_SURROUND_VALUE);
    w1 = (w1==0 ? 1 : 0);
    w2 = (w2==0 ? 1 : 0);

    // Center-Surround Search Loop
    bool csSuccess=false;
    int csThreshold=CS_THRESHOLD_START;
    while(!csSuccess && (csThreshold <= CS_THRESHOLD_MAX)) {
        qDebug() << "Center-Surround search, threshold=" << csThreshold << " of max=" << CS_THRESHOLD_MAX;
        applyBinaryThreshold(workingData[w1][CELL], workingData[w2][CELL], csThreshold);
        if (findConnectedRegions(workingData[w2][CELL]) && errorStatus==0) {
            qDebug() << "Center-Surround search successful";
            csSuccess=true;
            w1 = (w1==0 ? 1 : 0);
            w2 = (w2==0 ? 1 : 0);
            lastTargetIndex=w1;
        } else {
            errorStatus=0;
            csThreshold+=CS_THRESHOLD_INCREMENT;
        }
    }
    if (!csSuccess) {
        qDebug() << "Center-Surround search failed";
        qDebug() << "CellCounter3D::findCells() : Error - findConnectedRegions() failed";
        if (!errorStatus) {
            errorStatus=1;
        }
        return false;
    }

    // Debug
    //copyToImage(CELL, workingData[w1], data, false /* non-zero only */, false /* markAllChannels */);

    return true;
}

void CellCounter3D::clearChannel(int channel, unsigned char**** d) {
    for (int z=0;z<zDim;z++) {
        for (int y=0;y<yDim;y++) {
            for (int x=0;x<xDim;x++) {
                d[channel][z][y][x]=0;
            }
        }
    }
}

void CellCounter3D::markImage() {
    //return;
    if (errorStatus) {
        qDebug() << "markImage() skipping execution due to error state";
        return;
    }
    unsigned char**** data = (unsigned char****)image->getData();

    // Allocate mask
    unsigned char**** mask = new unsigned char*** [3];
    for (int c=0;c<3;c++) {
        mask[c]=new unsigned char** [zDim];
        for (int z=0;z<zDim;z++) {
            mask[c][z]=new unsigned char* [yDim];
            for (int y=0;y<yDim;y++) {
                mask[c][z][y]=new unsigned char[xDim];
                for (int x=0;x<xDim;x++) {
                    mask[c][z][y][x]=0;
                }
            }
        }
    }

    // Phase 1: Iterate and build outer mark
    for (int i=0;i<regionCoordinates.size();i+=3) {
        int z=regionCoordinates.at(i);
        int y=regionCoordinates.at(i+1);
        int x=regionCoordinates.at(i+2);
        for (int ez=z-MARK_RADIUS;ez<z+MARK_RADIUS;ez++) {
            int sz=ez;
            if (sz<0) {
                sz=0;
            } else if (ez>=zDim) {
                sz=zDim-1;
            }
            for (int ey=y-MARK_RADIUS;ey<y+MARK_RADIUS;ey++) {
                int sy=ey;
                if (ey<0) {
                    sy=0;
                } else if (ey>=yDim) {
                    sy=yDim-1;
                }
                for (int ex=x-MARK_RADIUS;ex<x+MARK_RADIUS;ex++) {
                    int sx=ex;
                    if (ex<0) {
                        sx=0;
                    } else if (ex>=xDim) {
                        sx=xDim-1;
                    }
                    double distance = std::sqrt((double)((sz-z)*(sz-z)+(sy-y)*(sy-y)+(sx-x)*(sx-x)));
                    if (distance<=MARK_RADIUS) {
                        unsigned char signal=data[0][sz][sy][sx];
                        int r=(MARK_COLOR[0]*signal)/255;
                        int g=(MARK_COLOR[1]*signal)/255;
                        int b=(MARK_COLOR[2]*signal)/255;
                        mask[0][sz][sy][sx]=(r>255 ? 255 : r);
                        mask[1][sz][sy][sx]=(g>255 ? 255 : g);
                        mask[2][sz][sy][sx]=(b>255 ? 255 : b);
                    }
                }
            }
        }
    }

    // Phase 2: Iterate and build inner mark
    for (int i=0;i<regionCoordinates.size();i+=3) {
        int z=regionCoordinates.at(i);
        int y=regionCoordinates.at(i+1);
        int x=regionCoordinates.at(i+2);
        for (int ez=z-MARK_SIZE;ez<z+MARK_SIZE;ez++) {
            int sz=ez;
            if (sz<0) {
                sz=0;
            } else if (ez>=zDim) {
                sz=zDim-1;
            }
            for (int ey=y-MARK_SIZE;ey<y+MARK_SIZE;ey++) {
                int sy=ey;
                if (ey<0) {
                    sy=0;
                } else if (ey>=yDim) {
                    sy=yDim-1;
                }
                for (int ex=x-MARK_SIZE;ex<x+MARK_SIZE;ex++) {
                    int sx=ex;
                    if (ex<0) {
                        sx=0;
                    } else if (ex>=xDim) {
                        sx=xDim-1;
                    }
                    mask[0][sz][sy][sx]=255;
                    mask[1][sz][sy][sx]=255;
                    mask[2][sz][sy][sx]=255;
                }
            }
        }
    }

    // Phase 3: Remap data to RGB according to requested color
    for (int z=0;z<zDim;z++) {
        for (int y=0;y<yDim;y++) {
            for (int x=0;x<xDim;x++) {
                unsigned char signal=data[0][z][y][x];
                int r=(SIGNAL_COLOR[0]*signal)/255;
                int g=(SIGNAL_COLOR[1]*signal)/255;
                int b=(SIGNAL_COLOR[2]*signal)/255;
                data[0][z][y][x]=(r>255 ? 255 : r);
                data[1][z][y][x]=(g>255 ? 255 : g);
                data[2][z][y][x]=(b>255 ? 255 : b);
            }
        }
    }

    copyToImage(0, mask, data, true /* non-zero only */, false /* markAllChannels */);
    copyToImage(1, mask, data, true /* non-zero only */, false /* markAllChannels */);
    copyToImage(2, mask, data, true /* non-zero only */, false /* markAllChannels */);

    // Delete mask
    for (int c=0;c<2;c++) {
        for (int z=0;z<zDim;z++) {
            for (int y=0;y<yDim;y++) {
                delete [] mask[c][z][y];
            }
            delete [] mask[c][z];
        }
        delete [] mask[c];
    }
    delete [] mask;
}

void CellCounter3D::dialateOrErode(int type, unsigned char*** s, unsigned char*** t, int elementSize, int neighborsForThreshold) {
    qDebug() << "CellCounter3D::dialateOrErode() start";
    currentSource=s;
    currentTarget=t;
    QList< QFuture<void> > deList;
    for (int z=0;z<zDim;z++) {
        QFuture<void> qf = QtConcurrent::run(this, &CellCounter3D::dialateOrErodeZslice, type, z, elementSize, neighborsForThreshold);
        deList.append(qf);
    }
    while(1) {
        SleepThread st;
        st.msleep(5000);
        int doneCount=0;
        for (int i=0;i<deList.size();i++) {
            QFuture<void> qf=deList.at(i);
            if (qf.isFinished()) {
                doneCount++;
            }
        }
        int stillActive=deList.size()-doneCount;
        if (stillActive==0) {
            break;
        } else {
            qDebug() << "Waiting on " << stillActive << " z-slices";
        }
    }
}

long CellCounter3D::countNonZero(unsigned char*** d) {
    long count=0;
    for (int z=0;z<zDim;z++) {
        for (int y=0;y<yDim;y++) {
            for (int x=0;x<xDim;x++) {
                if (d[z][y][x]>0) {
                    count++;
                }
            }
        }
    }
    return count;
}


void CellCounter3D::dialateOrErodeZslice(int type, int z, int elementSize, int neighborsForThreshold) {
    unsigned char*** s=currentSource;
    unsigned char*** t=currentTarget;
    long dialateCount=0;
    long erosionCount=0;
    for (int y=0;y<yDim;y++) {
        for (int x=0;x<xDim;x++) {

            int currentValue=s[z][y][x];

            if (type==ERODE) {
                if (currentValue==0) {
                    // nothing to do
                    t[z][y][x]=0;
                    continue;
                }
            } else if (type==DIALATE) {
                if (currentValue>0) {
                    t[z][y][x]=currentValue;
                    continue;
                }
            }

            int neighborCount=0;
            for (int ez=z-elementSize;ez<z+elementSize;ez++) {
                int sz=ez;
                if (sz<0) {
                    sz=0;
                } else if (ez>=zDim) {
                    sz=zDim-1;
                }
                for (int ey=y-elementSize;ey<y+elementSize;ey++) {
                    int sy=ey;
                    if (ey<0) {
                        sy=0;
                    } else if (ey>=yDim) {
                        sy=yDim-1;
                    }
                    for (int ex=x-elementSize;ex<x+elementSize;ex++) {
                        int sx=ex;
                        if (ex<0) {
                            sx=0;
                        } else if (ex>=xDim) {
                            sx=xDim-1;
                        }
                        // DEBUG
                        unsigned char neighborValue=s[sz][sy][sx];
                        if (neighborValue>0) {
                            neighborCount++;
                        }
                    }
                }
            }

            if (type==DIALATE) {
                if (neighborCount>=neighborsForThreshold) {
                    dialateCount++;
                    t[z][y][x] = 255;
                } else {
                    t[z][y][x] = 0;
                }
            } else if (type==ERODE) {
                if (neighborCount<=neighborsForThreshold) {
                    erosionCount++;
                    t[z][y][x] = 0;
                } else {
                    t[z][y][x] = s[z][y][x];
                }
            }

        }
    }
    if (type==DIALATE) {
        qDebug() << "Dialated " << dialateCount << " voxels";
    } else if (type==ERODE) {
        qDebug() << "Eroded " << erosionCount << " voxels";
    }
}


void CellCounter3D::copyToImage(int sourceChannel, unsigned char**** d, unsigned char**** data, bool nonZeroOnly, bool markAllChannels) {
    for (int z=0;z<zDim;z++) {
        for (int y=0;y<yDim;y++) {
            for (int x=0;x<xDim;x++) {
                unsigned char value=d[sourceChannel][z][y][x];
                if (!nonZeroOnly || (nonZeroOnly && value>0) ) {
                    if (markAllChannels) {
                        data[0][z][y][x]=d[sourceChannel][z][y][x];
                        data[1][z][y][x]=d[sourceChannel][z][y][x];
                        data[2][z][y][x]=d[sourceChannel][z][y][x];
                    } else {
                        data[sourceChannel][z][y][x]=d[sourceChannel][z][y][x];
                    }
                }
            }
        }
    }
}

void CellCounter3D::addCrossMark(unsigned char**** d, int z, int y, int x, int size) {
    for (int mz=z-size;mz<z+size;mz++) {
        int pz=mz;
        if (pz<0) {
            pz=0;
        } else if (pz>=zDim) {
            pz=zDim-1;
        }
        d[0][pz][y][x]=255;
        d[1][pz][y][x]=255;
        d[2][pz][y][x]=255;
    }
    for (int my=y-size;my<y+size;my++) {
        int py=my;
        if (py<0) {
            py=0;
        } else if (py>=yDim) {
            py=yDim-1;
        }
        d[0][z][py][x]=255;
        d[1][z][py][x]=255;
        d[2][z][py][x]=255;
    }
    for (int mx=x-size;mx<x+size;mx++) {
        int px=mx;
        if (px<0) {
            px=0;
        } else if (px>=xDim) {
            px=xDim-1;
        }
        d[0][z][y][px]=255;
        d[1][z][y][px]=255;
        d[2][z][y][px]=255;
    }
}

void CellCounter3D::gammaFilter(unsigned char*** d, double gamma) {
    unsigned char gammaLookup[256];
    for (int i = 0; i < 256; i++) {
        gammaLookup[i] = (unsigned char)(std::pow(i/255.0, gamma) * 255.0 + 0.499);
    }
    for (int z=0;z<zDim;z++) {
        for (int y=0;y<yDim;y++) {
            for (int x=0;x<xDim;x++) {
                int rg=gammaLookup[d[z][y][x]];
                d[z][y][x]=rg;
            }
        }
    }
}

void CellCounter3D::normalizeNonZero(unsigned char*** d, double sigma) {
    // First calc std dev
    double total=0.0;
    int count=0;
    for (int z=0;z<zDim;z++) {
        for (int y=0;y<yDim;y++) {
            for (int x=0;x<xDim;x++) {
                unsigned char value=d[z][y][x];
                if (value>0) {
                    total+=d[z][y][x];
                    count++;
                }
            }
        }
    }
    double average = total / count;
    double a2=0.0;
    for (int z=0;z<zDim;z++) {
        for (int y=0;y<yDim;y++) {
            for (int x=0;x<xDim;x++) {
                unsigned char value=d[z][y][x];
                if (value>0) {
                    a2+=((double)value-average)*((double)value-average);
                }
            }
        }
    }
    double std=std::sqrt(a2/count);
    double dev=std*sigma;
    for (int z=0;z<zDim;z++) {
        for (int y=0;y<yDim;y++) {
            for (int x=0;x<xDim;x++) {
                unsigned char value=d[z][y][x];
                if (value>0) {
                    double v=((double)value-average)/dev;
                    if (v>1.0) {
                        v=1.0;
                    } else if (v<-1.0) {
                        v=-1.0;
                    }
                    unsigned char newValue=((v+1.0)/2.0)*255.0;
                    d[z][y][x]=newValue;
                }
            }
        }
    }
}

void CellCounter3D::applyBinaryThreshold(unsigned char*** source, unsigned char*** target, unsigned char threshold) {
    for (int z=0;z<zDim;z++) {
        for (int y=0;y<yDim;y++) {
            for (int x=0;x<xDim;x++) {
                if (source[z][y][x]>threshold) {
                    target[z][y][x]=255;
                } else {
                    target[z][y][x]=0;
                }
            }
        }
    }
}

void CellCounter3D::centerSurroundFilter(unsigned char*** source, unsigned char*** output, double centerSize, double centerValue, double surroundSize, double surroundValue) {
    // Create target workspace
    double*** target = new double** [zDim];
    for (int z=0;z<zDim;z++) {
        target[z] = new double* [yDim];
        for (int y=0;y<yDim;y++) {
            target[z][y] = new double [xDim];
        }
    }
    // First create filter
    int filterRadius=centerSize+surroundSize;
    int filterDiameter=filterRadius*2;
    double*** filterMatrix = new double** [filterDiameter];
    for (int z=0;z<filterDiameter;z++) {
        filterMatrix[z] = new double* [filterDiameter];
        for (int y=0;y<filterDiameter;y++) {
            filterMatrix[z][y] = new double [filterDiameter];
            for (int x=0;x<filterDiameter;x++) {
                double pz = (double)(z-filterRadius);
                double py = (double)(y-filterRadius);
                double px = (double)(x-filterRadius);
                double distance=std::sqrt(pz*pz+py*py+px*px);
                if (distance <= centerSize) {
                    filterMatrix[z][y][x] = centerValue;
                } else if (distance <= (centerSize+surroundSize)) {
                    filterMatrix[z][y][x] = surroundValue;
                } else {
                    filterMatrix[z][y][x] = 0.0;
                }
            }
        }
    }

    // Shared for center-surround threads
    csSource = source;
    csOutput = output;
    csFilterMatrix = filterMatrix;
    csTarget = target;
    csCenterSize = centerSize;
    csCenterValue = centerValue;
    csSurroundSize = surroundSize;
    csSurroundValue = surroundValue;
    csFilterMin=0.0;
    csFilterMax=0.0;

    QList< QFuture<void> > csList;
    for (int z=0;z<zDim;z++) {
        QFuture<void> qf = QtConcurrent::run(this, &CellCounter3D::centerSurroundFilterZSlice, z);
        csList.append(qf);
    }
    while(1) {
        SleepThread st;
        st.msleep(5000);
        int doneCount=0;
        for (int i=0;i<csList.size();i++) {
            QFuture<void> qf=csList.at(i);
            if (qf.isFinished()) {
                doneCount++;
            }
        }
        int stillActive=csList.size()-doneCount;
        if (stillActive==0) {
            break;
        } else {
            qDebug() << "Waiting on " << stillActive << " z-slices";
        }
    }

    for (int z=0;z<zDim;z++) {
        for (int y=0;y<yDim;y++) {
            for (int x=0;x<xDim;x++) {
                output[z][y][x]=255.0*( (target[z][y][x]-csFilterMin)/(csFilterMax-csFilterMin) );
            }
        }
    }
    for (int z=0;z<zDim;z++) {
        for (int y=0;y<yDim;y++) {
            delete [] target[z][y];
        }
        delete [] target[z];
    }
    delete [] target;
    for (int z=0;z<filterDiameter;z++) {
        for (int y=0;y<filterDiameter;y++) {
            delete [] filterMatrix[z][y];
        }
        delete [] filterMatrix[z];
    }
    delete [] filterMatrix;

}

void CellCounter3D::centerSurroundFilterZSlice(int z) {
    int filterRadius=csCenterSize + csSurroundSize;
    // Now apply
    qDebug() << "Computing center-surround for z=" << z;
    for (int y=0;y<yDim;y++) {
        for (int x=0;x<xDim;x++) {

            unsigned char originValue=csSource[z][y][x];

            if (originValue==0) {
                csTarget[z][y][x]=0.0;
                continue;
            }

            double filterOutput=0.0;
            for (int fz=z+filterRadius;fz>z-filterRadius;fz--) {
                int pz=fz;
                if (pz<0) {
                    pz=0;
                } else if (pz>=zDim) {
                    pz=zDim-1;
                }
                for (int fy=y+filterRadius;fy>y-filterRadius;fy--) {
                    int py=fy;
                    if (py<0) {
                        py=0;
                    } else if (py>=yDim) {
                        py=yDim-1;
                    }
                    for (int fx=x+filterRadius;fx>x-filterRadius;fx--) {
                        int px=fx;
                        if (px<0) {
                            px=0;
                        } else if (px>=xDim) {
                            px=xDim-1;
                        }
                        unsigned char sourceValue=csSource[pz][py][px];
                        double s=0.0;
                        if (sourceValue>0) {
                            s=1.0;
                        } else {
                            s=-1.0;
                        }
                        filterOutput += s*csFilterMatrix[fz - z + filterRadius - 1][fy - y + filterRadius - 1][fx - x + filterRadius - 1];
                    }
                }
            }
            csTarget[z][y][x]=filterOutput;
            csLock.lock();
            if (filterOutput>csFilterMax) {
                csFilterMax=filterOutput;
            }
            if (filterOutput<csFilterMin) {
                csFilterMin=filterOutput;
            }
            csLock.unlock();
        }
    }
}


// In this function, we will iterate through the data and find all isolated
// structures which consist of neighboring voxels. For each such isolated
// structure (or connected region) we will then compute its center point,
// and add these coordinates to the 'connectedRegions' list.

bool CellCounter3D::findConnectedRegions(unsigned char*** d) {
    // To do this, we need a mask to keep track of which voxels we have
    // already included in a group
    unsigned char*** mask=new unsigned char** [zDim];
    for (int z=0;z<zDim;z++) {
        mask[z] = new unsigned char * [yDim];
        for (int y=0;y<yDim;y++) {
            mask[z][y] = new unsigned char [xDim];
            for (int x=0;x<xDim;x++) {
                mask[z][y][x]=0;
            }
        }
    }
    // Now we walk through the space and find neighboring groups
    int r=0;
    for (int z=0;z<zDim;z++) {
        for (int y=0;y<yDim;y++) {
            for (int x=0;x<xDim;x++) {
                if (d[z][y][x]>0 && mask[z][y][x]==0) {
                    // We found a group
                    QList<int> groupList;
                    groupList.append(z);
                    groupList.append(y);
                    groupList.append(x);
                    mask[z][y][x]=1;
                    findNeighbors(x,y,z,d,mask,groupList);
                    if (errorStatus) {
                        qDebug() << "CellCounter3D::findConnectedRegions() : Error status indicated after call to findNeighbors";
                        regionCoordinates.clear();
                        return false;
                    }
                    int zTotal=0;
                    int yTotal=0;
                    int xTotal=0;
                    int c=0;
                    for (int i=0;i<groupList.size();i+=3) {
                        int z=groupList.at(i);
                        int y=groupList.at(i+1);
                        int x=groupList.at(i+2);
                        zTotal+=z;
                        yTotal+=y;
                        xTotal+=x;
                        c++;
                    }
                    if (c>=MIN_REGION_VOXELS) {
                        int zAvg=zTotal/c;
                        int yAvg=yTotal/c;
                        int xAvg=xTotal/c;
                        regionCoordinates.append(zAvg);
                        regionCoordinates.append(yAvg);
                        regionCoordinates.append(xAvg);
                        qDebug() << "Added region " << r << "  z=" << zAvg << " y=" << yAvg << " x=" << xAvg << " c=" << c;
                        r++;
                    }
                }
            }
        }
    }
    for (int z=0;z<zDim;z++) {
        for (int y=0;y<yDim;y++) {
            delete [] mask[z][y];
        }
        delete [] mask[z];
    }
    delete [] mask;
    return true;
}

// Finds non-zero neighbors
void CellCounter3D::findNeighbors(int x, int y, int z, unsigned char*** d, unsigned char*** mask, QList<int> & neighborList) {
    for (int sz=z-1;sz<=z+1;sz++) {
        int pz=sz;
        if (pz<0) {
            pz=0;
        } else if (pz>=zDim) {
            pz=zDim-1;
        }
        for (int sy=y-1;sy<=y+1;sy++) {
            int py=sy;
            if (py<0) {
                py=0;
            } else if (py>=yDim) {
                py=yDim-1;
            }
            for (int sx=x-1;sx<=x+1;sx++) {
                int px=sx;
                if (px<0) {
                    px=0;
                } else if (px>=xDim) {
                    px=xDim-1;
                }
                if (!(pz==z && py==y && px==x)) {
                    unsigned char value=d[pz][py][px];
                    if (value>0) {
                        unsigned char maskValue=mask[pz][py][px];
                        if (maskValue==0) {
                            neighborList.append(pz);
                            neighborList.append(py);
                            neighborList.append(px);
                            mask[pz][py][px]=1;
                            findNeighbors(px,py,pz,d,mask, neighborList);
                            if (neighborList.size()>MAX_REGION_VOXELS) {
                                if (errorStatus!=1) {
                                    qDebug() << "CellCounter3D::findNeighbors() ERROR : exceeded MAX_REGION_VOXELS=" << MAX_REGION_VOXELS;
                                }
                                errorStatus=1;
                                neighborList.clear();
                                return;
                            }
                        }
                    }
                }
            }
        }
    }
    return;
}

void CellCounter3D::writeOutputImageFile() {
    if (errorStatus) {
        qDebug() << "writeOutputImageFile() skipping due to error state";
        return;
    }
    int lastPeriodIndex=inputFilePath.lastIndexOf(".");
    QString filePrefix=inputFilePath.left(lastPeriodIndex);
    QString outputFilePath=filePrefix + "_CellCounterImage.tif";
    image->saveImage(outputFilePath.toStdString().c_str());
}

void CellCounter3D::writeOutputReportFile() {
    if (errorStatus) {
        qDebug() << "writeOutputReportFile() skipping due to error state";
        return;
    }
    int lastPeriodIndex=inputFilePath.lastIndexOf(".");
    QString filePrefix=inputFilePath.left(lastPeriodIndex);
    QString outputFilePath=filePrefix + "_CellCounterReport.txt";
    QFile file(outputFilePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Could not open file " << outputFilePath << " to write";
    }
    QTextStream out(&file);
    for (int i=0;i<regionCoordinates.length();i=i+3) {
        int index=(i/3)+1;
        int z=regionCoordinates.at(i);
        int y=regionCoordinates.at(i+1);
        int x=regionCoordinates.at(i+2);
        out << index << "\t" << z << "\t" << y << "\t" << x << "\n";
    }
    file.close();
 }

int CellCounter3D::processArgs(vector<char*> *argList) {
    for (int i=0;i<argList->size();i++) {
        QString arg=(*argList)[i];
        if (arg=="-h") {
            return ARG_STATUS_HELP;
        }
        if (arg=="-i") {
            i++;
            inputFilePath=(*argList)[i];
        }
        else if (arg=="-ist") {
            i++;
            QString initialSignalThreshold=(*argList)[i];
            INITIAL_SIGNAL_THRESHOLD=initialSignalThreshold.toInt();
        }
        else if (arg=="-ibt") {
            i++;
            QString initialBackgroundThreshold=(*argList)[i];
            INITIAL_BACKGROUND_THRESHOLD=initialBackgroundThreshold.toInt();
        }
        else if (arg=="-sn") {
            i++;
            QString sigmaNormalization=(*argList)[i];
            SIGMA_NORMALIZATION=sigmaNormalization.toDouble();
        }
        else if (arg=="-nt") {
            i++;
            QString normalizationThreshold=(*argList)[i];
            NORMALIZATION_THRESHOLD=normalizationThreshold.toInt();
        }
        else if (arg=="-dc") {
            i++;
            QString dialationCycles=(*argList)[i];
            DIALATION_CYCLES=dialationCycles.toInt();
        }
        else if (arg=="-des") {
            i++;
            QString dialationElementSize=(*argList)[i];
            DIALATION_ELEMENT_SIZE=dialationElementSize.toInt();
        }
        else if (arg=="-dt") {
            i++;
            QString dialationThreshold=(*argList)[i];
            DIALATION_THRESHOLD=dialationThreshold.toInt();
        }
        else if (arg=="-ec") {
            i++;
            QString erosionCycles=(*argList)[i];
            EROSION_CYCLES=erosionCycles.toInt();
        }
        else if (arg=="-ees") {
            i++;
            QString erosionElementSize=(*argList)[i];
            EROSION_ELEMENT_SIZE=erosionElementSize.toInt();
        }
        else if (arg=="-et") {
            i++;
            QString erosionThreshold=(*argList)[i];
            EROSION_THRESHOLD=erosionThreshold.toInt();
        }
        else if (arg=="-cr") {
            i++;
            QString csCenterRadius=(*argList)[i];
            CS_CENTER_RADIUS=csCenterRadius.toDouble();
        }
        else if (arg=="-cv") {
            i++;
            QString csCenterValue=(*argList)[i];
            CS_CENTER_VALUE=csCenterValue.toDouble();
        }
        else if (arg=="-sr") {
            i++;
            QString csSurroundRadius=(*argList)[i];
            CS_SURROUND_RADIUS=csSurroundRadius.toDouble();
        }
        else if (arg=="-sv") {
            i++;
            QString csSurroundValue=(*argList)[i];
            CS_SURROUND_VALUE=csSurroundValue.toDouble();
        }
        else if (arg=="-cst") {
            i++;
            QString csThresholdStart=(*argList)[i];
            CS_THRESHOLD_START=csThresholdStart.toInt();
        }
        else if (arg=="-csi") {
            i++;
            QString csThresholdIncrement=(*argList)[i];
            CS_THRESHOLD_INCREMENT=csThresholdIncrement.toInt();
        }
        else if (arg=="-csm") {
            i++;
            QString csThresholdMax=(*argList)[i];
            CS_THRESHOLD_MAX=csThresholdMax.toInt();
        }
        else if (arg=="-ms") {
            i++;
            QString markSize=(*argList)[i];
            MARK_SIZE=markSize.toInt();
        }
        else if (arg=="-mc") {
            i++;
            QString markColor0=(*argList)[i++];
            QString markColor1=(*argList)[i++];
            QString markColor2=(*argList)[i];
            MARK_COLOR[0]=markColor0.toInt();
            MARK_COLOR[1]=markColor1.toInt();
            MARK_COLOR[2]=markColor2.toInt();
        }
        else if (arg=="-sc") {
            i++;
            QString signalColor0=(*argList)[i++];
            QString signalColor1=(*argList)[i++];
            QString signalColor2=(*argList)[i];
            SIGNAL_COLOR[0]=signalColor0.toInt();
            SIGNAL_COLOR[1]=signalColor1.toInt();
            SIGNAL_COLOR[2]=signalColor2.toInt();
        }
        else if (arg=="-mnr") {
            i++;
            QString minimumRegionVoxels=(*argList)[i];
            MIN_REGION_VOXELS=minimumRegionVoxels.toInt();
        }
        else if (arg=="-mxr") {
            i++;
            QString maximumRegionVoxels=(*argList)[i];
            MAX_REGION_VOXELS=maximumRegionVoxels.toInt();
        }
    }
    // Validation of mandatory information
    if (inputFilePath.length()>0) {
        return ARG_STATUS_OK;
    } else {
        return ARG_STATUS_USAGE;
    }
}


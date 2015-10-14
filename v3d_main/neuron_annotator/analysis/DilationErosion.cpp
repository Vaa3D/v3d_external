#include "DilationErosion.h"
#include "SleepThread.h"

#include "../terafly/src/presentation/theader.h"  //2015May PHC

const int DilationErosion::TYPE_DILATE=0;
const int DilationErosion::TYPE_ERODE=1;

DilationErosion::DilationErosion()
{
  currentSource=0L;
  currentTarget=0L;
  xDim=yDim=zDim=0;
}

void DilationErosion::dilateOrErode(int type, int xDim, int yDim, int zDim,
				    unsigned char*** s, unsigned char*** t, int elementSize, int neighborsForThreshold) {
    qDebug() << "CellCounter3D::dilateOrErode() start";
    currentSource=s;
    currentTarget=t;
    this->xDim=xDim;
    this->yDim=yDim;
    this->zDim=zDim;
    QList< QFuture<void> > deList;
    for (int z=0;z<zDim;z++) {
        QFuture<void> qf = QtConcurrent::run(this, &DilationErosion::dilateOrErodeZslice, type, z, elementSize, neighborsForThreshold);
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

void DilationErosion::dilateOrErodeZslice(int type, int z, int elementSize, int neighborsForThreshold) {
    unsigned char*** s=currentSource;
    unsigned char*** t=currentTarget;
    long dilateCount=0;
    long erosionCount=0;
    for (int y=0;y<yDim;y++) {
        for (int x=0;x<xDim;x++) {

            int currentValue=s[z][y][x];

            if (type==TYPE_ERODE) {
                if (currentValue==0) {
                    // nothing to do
                    t[z][y][x]=0;
                    continue;
                }
            } else if (type==TYPE_DILATE) {
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

            if (type==TYPE_DILATE) {
                if (neighborCount>=neighborsForThreshold) {
                    dilateCount++;
                    t[z][y][x] = 255;
                } else {
                    t[z][y][x] = 0;
                }
            } else if (type==TYPE_ERODE) {
                if (neighborCount<=neighborsForThreshold) {
                    erosionCount++;
                    t[z][y][x] = 0;
                } else {
                    t[z][y][x] = s[z][y][x];
                }
            }

        }
    }
    if (type==TYPE_DILATE) {
        qDebug() << "Dilated " << dilateCount << " voxels";
    } else if (type==TYPE_ERODE) {
        qDebug() << "Eroded " << erosionCount << " voxels";
    }
}


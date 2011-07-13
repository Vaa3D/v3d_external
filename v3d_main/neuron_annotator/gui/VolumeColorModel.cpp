#include "VolumeColorModel.h"
#include <QTime>
#include <iostream>

using namespace std;

/* explicit */
VolumeColorModel::VolumeColorModel(size_t xdim, size_t ydim, size_t zdim)
    : QObject(NULL), m_Data(NULL), m_xDim(xdim), m_yDim(ydim), m_zDim(zdim)
{
    m_Data = new ColorType[xdim * ydim * zdim];
}

/* explicit */
VolumeColorModel::VolumeColorModel(const VolumeColorModel& rhs)
    : QObject(NULL), m_Data(NULL), m_xDim(rhs.xDim()), m_yDim(rhs.yDim()), m_zDim(rhs.zDim())
{
    m_Data = new ColorType[rhs.numVoxels()];
    memcpy(m_Data, rhs.m_Data, rhs.numVoxels() * sizeof(ColorType));
}

/* virtual */
VolumeColorModel::~VolumeColorModel()
{
    if (m_Data) {
        delete [] m_Data;
        m_Data = NULL;
    }
}

VolumeColorModel& VolumeColorModel::operator=(const VolumeColorModel& rhs)
{
    if (this == &rhs) return *this;
    if (numVoxels() != rhs.numVoxels()) {
        if (m_Data) delete [] m_Data;
        m_Data = new ColorType[rhs.numVoxels()];
    }
    m_xDim = rhs.xDim();
    m_yDim = rhs.yDim();
    m_zDim = rhs.zDim();
    memcpy(m_Data, rhs.m_Data, rhs.numVoxels() * sizeof(ColorType));
    return *this;
}


void VolumeColorModel::benchmarkDataCopy()
{
    VolumeColorModel vol1(512, 512, 512);
    VolumeColorModel vol2(512, 512, 512);
    QTime time;
    time.start();
    vol2 = vol1;
    cout << "volume copy took " << time.elapsed() << " milliseconds for ";
    cout << vol1.numVoxels() << " voxels" << endl;
}

void VolumeColorModel::benchmarkDataIncrement()
{
    VolumeColorModel vol(512, 512, 512);
    QTime time;
    time.start();
    for(size_t z = 0; z < vol.zDim(); ++z) {
        VolumeColorModel::ZSlice zSlice = vol.zSlice(z);
        for (size_t y = 0; y < zSlice.yDim(); ++y) {
            VolumeColorModel::ScanLine scanLine = zSlice.scanLine(y);
            for (size_t x = 0; x < scanLine.xDim(); ++x) {
                scanLine[x] += 1;
            }
        }
    }
    cout << "volume increment took " << time.elapsed() << " milliseconds for ";
    cout << vol.numVoxels() << " voxels" << endl;
}

void VolumeColorModel::benchmarkDataMip() {}
void VolumeColorModel::benchmarkDataGamma() {}


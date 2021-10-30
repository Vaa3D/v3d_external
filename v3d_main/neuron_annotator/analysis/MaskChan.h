#ifndef MASKCHAN_H
#define MASKCHAN_H

#include <QtCore>
#include "../../v3d/v3d_core.h"

/*

  Format for mask and channel files.

  Mask files:

  long xsize; // space
  long ysize; // space
  long zsize; // space
  float xMicrons; // voxel size x
  float yMicrons; // voxel size y
  float zMicrons; // voxel size z
  long x0; // bounding box
  long x1; // bounding box, such that x0 is inclusive, x1 exclusive, etc
  long y0; // bb
  long y1; // bb
  long z0; // bb
  long z1; // bb
  long totalVoxels;
  unsigned char axis; // 0=yz(x), 1=xz(y), 2=xy(z)
  { // For each ray
    long skip;
    long pairs;
    { // For each pair
        long start;
        long end; // such that end-start is length, i.e., end is exclusive
    }
  }

  Channel files:

  long totalVoxels;
  unsigned char channels; // number of channels
  unsigned char recommendedRedChannel;
  unsigned char recommendedGreenChannel;
  unsigned char recommendedBlueChannel;
  unsigned char bytesPerChannel; // 1=8-bit, 2=16-bit
  { // For each channel
    { // For each voxel
        B value;
    }
  }

*/

class MaskRay
{
public:
    long skipCount;
    QList<long> startList;
    QList<long> endList;
};

class MaskChan
{
 public:
  MaskChan();
  bool setSourceImage(My4DImage* image);
  bool setLabelImage(My4DImage* labelImage);
  QList<int> getFragmentListFromLabelStack();
  bool createMaskChanForLabel(int label, const QString& maskFullPath, const QString& channelFullPath, QReadWriteLock* mutex);
  My4DImage* createImageFromMaskFiles(QStringList& maskFilePaths);

  static const int MAX_LABEL;

 private:
  void axisTracer(int direction, int label, QList<MaskRay*> * rayList, long& pairCount, long& voxelCount,
		  long& x0, long& x1, long& y0, long& y1, long& z0, long& z1, void* data=0L, long assumedVoxelCount=0L);

  void writeMaskList(QDataStream& dataOut, QList<MaskRay*>& list);

  My4DImage* sourceImage;
  My4DImage* labelImage;
  long* labelIndex;
  v3d_uint8* label8;
  v3d_uint16* label16;
  QList<int> labelList;

  long xdim;
  long ydim;
  long zdim;
  long cdim;

};

#endif // MASKCHAN_H


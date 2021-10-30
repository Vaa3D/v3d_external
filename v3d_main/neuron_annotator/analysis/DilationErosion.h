#ifndef DILATIONEROSION_H
#define DILATIONEROSION_H

#include <vector>
#include "../v3d/v3d_core.h"

#if defined (_MSC_VER)
#include "../basic_c_fun/vcdiff.h"
#else
#endif


class DilationErosion
{
 public:
  DilationErosion();
  ~DilationErosion() {}

  static const int TYPE_DILATE;
  static const int TYPE_ERODE;

 public:
  void dilateOrErode(int type, int xDim, int yDim, int zDim, unsigned char*** s, unsigned char*** t, int elementSize, int neighborsForThreshold);

 private:
  void dilateOrErodeZslice(int type, int z, int elementSize, int neighborsForThreshold);

  unsigned char*** currentSource;
  unsigned char*** currentTarget;

  int xDim;
  int yDim;
  int zDim;

};

#endif // DILATIONEROSION_H


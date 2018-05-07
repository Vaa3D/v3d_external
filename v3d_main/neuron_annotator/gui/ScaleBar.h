#ifndef NA_SCALE_BAR_H_
#define NA_SCALE_BAR_H_

#include <QPainter>

class ScaleBar 
{
public:
    void paint(float xVoxelSizeInMicrons, float screenPixelsPerImageVoxel, int windowWidth, int windowHeight, QPainter& painter);
};

#endif // NA_SCALE_BAR_H_

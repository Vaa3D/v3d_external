#ifndef COORDINATECONVERT_H
#define COORDINATECONVERT_H

#include "color_xyz.h"

/**
 * @brief The class to process coordinate conversion when communicate with server
 */
class CoordinateConvert
{
public:
    CoordinateConvert();
    ~CoordinateConvert();

    XYZ* convertGlobalToLocal(double x, double y, double z);
    XYZ* convertLocalToGlobal(double x, double y, double z);
    XYZ* convertMaxResToCurRes(double x, double y, double z);
    XYZ* convertCurResToMaxRes(double x, double y, double z);

    int getResIndex();
    void setResIndex(int resIndex);
    int getImgSize();
    void setImgSize(int imgSize);
    XYZ* getStartLocation();
    XYZ* getCenterLocation();
    void initLocation(XYZ* centerLoc);

private:
    XYZ *startLocation = nullptr;
    XYZ *centerLocation = nullptr;
    int resIndex; // default = 2
    int imageSize;
};

#endif // COORDINATECONVERT_H

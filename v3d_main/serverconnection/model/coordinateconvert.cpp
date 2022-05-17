#include "coordinateconvert.h"



CoordinateConvert::CoordinateConvert()
{

}

CoordinateConvert::~CoordinateConvert()
{
    delete startLocation;
    delete centerLocation;
}

/////////////////////////////////////////////////////////////////////////////////////
XYZ *CoordinateConvert::convertGlobalToLocal(double x, double y, double z)
{
    XYZ* node = convertMaxResToCurRes(x, y, z);
    node->x -= startLocation->x;
    node->y -= startLocation->y;
    node->z -= startLocation->z;
    return node;
}

XYZ *CoordinateConvert::convertLocalToGlobal(double x, double y, double z)
{
    x += startLocation->x;
    y += startLocation->y;
    z += startLocation->z;
    XYZ* node = convertCurResToMaxRes(x, y, z);
    return node;
}

XYZ *CoordinateConvert::convertMaxResToCurRes(double x, double y, double z)
{
    x /= pow(2, resIndex-1);
    y /= pow(2, resIndex-1);
    z /= pow(2, resIndex-1);
    return new XYZ((float) x, (float) y, (float) z);
}

XYZ *CoordinateConvert::convertCurResToMaxRes(double x, double y, double z)
{
    x *= pow(2, resIndex-1);
    y *= pow(2, resIndex-1);
    z *= pow(2, resIndex-1);
    return new XYZ((float) x, (float) y, (float) z);
}

int CoordinateConvert::getResIndex()
{
    return resIndex;
}

void CoordinateConvert::setResIndex(int resIndex)
{
    this->resIndex = resIndex;
}

int CoordinateConvert::getImgSize()
{
    return imageSize;
}

void CoordinateConvert::setImgSize(int imgSize)
{
    this->imageSize = imgSize;
}

XYZ *CoordinateConvert::getStartLocation()
{
    return startLocation;
}

XYZ *CoordinateConvert::getCenterLocation()
{
    return centerLocation;
}

void CoordinateConvert::initLocation(XYZ *centerLoc)
{
    centerLocation->x = (int)(centerLoc->x / pow(2, resIndex-1));
    centerLocation->y = (int)(centerLoc->y / pow(2, resIndex-1));
    centerLocation->z = (int)(centerLoc->z / pow(2, resIndex-1));

    startLocation->x = centerLocation->x - imageSize / 2;
    startLocation->y = centerLocation->y - imageSize / 2;
    startLocation->z = centerLocation->z - imageSize / 2;
}

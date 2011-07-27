#ifndef __MARK_IO_H_
#define __MARK_IO_H_
#include <string>
#include <list>
using namespace std;
struct MyImageMarker
{
    int type;           // 0-pxUnknown, 1-pxLocaNotUseful, 2-pxLocaUseful, 3-pxLocaUnsure, 4-pxTemp
    int shape;          // 0-pxUnset, 1-pxSphere, 2-pxCube, 3-pxCircleX, 4-pxCircleY, 5-pxCircleZ,
    // 6-pxSquareX, 7-pxSquareY, 8-pxSquareZ, 9-pxLineX, 10-pxLineY, 11-pxLineZ,
    // 12-pxTriangle, 13-pxDot;
    float x, y, z;      // point coordinates
    float radius;

    MyImageMarker() {type=shape=0; radius=x=y=z=0;}
	MyImageMarker(float xx, float yy, float zz): x(xx), y(yy), z(zz)
	{type=shape=0; radius=0;}
};

list <MyImageMarker> readMarker_file(const string & filename);
bool writeMarker_file(const string & filename, const list <MyImageMarker> & listMarker);
#endif

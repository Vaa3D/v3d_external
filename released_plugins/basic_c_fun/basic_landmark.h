//basic_landmark.h
//
// Copyright: Hanchuan Peng (Howard Hughes Medical Institute, Janelia Farm Research Campus).
// The License Information and User Agreement should be seen at http://penglab.janelia.org/proj/v3d .
//
// Last edit. 2009-Aug-21
//



#ifndef __PT_LOCATION_H__
#define __PT_LOCATION_H__

#include <time.h>

#include <string>
using std::string;

#include "color_xyz.h"
#include "v3d_basicdatatype.h"

#define VAL_INVALID -9999

enum PxLocationUsefulness
{
	pxUnknown, pxLocaNotUseful, pxLocaUseful, pxLocaUnsure, pxTemp
};
enum PxLocationMarkerShape
{
	pxUnset,
	pxSphere,
	pxCube,
	pxCircleX,
	pxCircleY,
	pxCircleZ,
	pxSquareX,
	pxSquareY,
	pxSquareZ,
	pxLineX,
	pxLineY,
	pxLineZ,
	pxTriangle,
	pxDot
};

struct LocationSimple
{
	float x, y, z;
	float radius;
	PxLocationUsefulness inputProperty;
	PxLocationMarkerShape shape;

	double pixval;
	double ave, sdev, skew, curt;
	double size, mass, pixmax;
	double ev_pc1, ev_pc2, ev_pc3; //the eigen values of principal components

	string name; //the name of a landmark
	string comments; //other info of the landmark
	int category; //the type of a particular landmark
	RGBA8 color;
	bool on;

	//public:
	void init()
	{
		x = 0;
		y = 0;
		z = 0;
		radius = 5;
		shape = pxSphere;
		inputProperty = pxLocaUseful;
		name = "unknown";
		comments = "";
		category = 0;
		//color.r = color.g = color.b = color.a = 255;

		srand(clock()); //time(NULL));
		color = random_rgba8(255);

		ave = sdev = skew = curt = 0;
		size = mass = 0;
		pixmax = 0;

		ev_pc1=ev_pc2=ev_pc3=VAL_INVALID; //set as invalid value

		on=true;
	}
	LocationSimple(int xx, int yy, int zz)
	{
		init();
		x = xx;
		y = yy;
		z = zz;
	}
	LocationSimple()
	{
		init();
	}
	void getCoord(int &xx, int &yy, int& zz)
	{
		xx = (int) x;
		yy = (int) y;
		zz = (int) z;
	}
	void getCoord(float &xx, float &yy, float& zz)
	{
		xx = x;
		yy = y;
		zz = z;
	}
	int getPixVal()
	{
		return pixval;
	}
	double getAve()
	{
		return ave;
	}
	double getSdev()
	{
		return sdev;
	}
	double getSkew()
	{
		return skew;
	}
	double getCurt()
	{
		return curt;
	}
	PxLocationUsefulness howUseful()
	{
		return inputProperty;
	}
};

struct PtIndexAndParents
{
	V3DLONG nodeInd;
	V3DLONG nodeParent;
	PtIndexAndParents()
	{
		nodeInd = -1;
		nodeParent = -1;
	}
	PtIndexAndParents(V3DLONG n, V3DLONG np)
	{
		nodeInd = n;
		nodeParent = np;
	}
};

#endif


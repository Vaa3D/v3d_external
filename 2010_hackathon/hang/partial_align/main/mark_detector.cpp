#include <iostream>
#include <list>
#include "v3d_basicdatatype.h"

#include "gaussian_blur.cpp"    // smooth
#include "../edge_detection.h"
#include "mark_detector.h"

#define SIGMA 1.0
#define THRESHOLD 20

using namespace std;
bool detect_mark(list<MyImageMarker> & markerlist, const unsigned char* inimg1d, V3DLONG sz[3])
{
	if(!inimg1d || sz[0] <=0 || sz[1] <=0 || sz[2]<=0) return false;
	double * outimg1d = new double [sz[0] * sz[1] * sz[2]];
	double * eimg1d = 0;
	//if(!smooth(outimg1d, inimg1d, sz, SIGMA)) return false;
	if(computeGaussian(outimg1d, inimg1d, sz, SIGMA, 3) == -1) return false;
	if(computeThreshold(outimg1d, sz, THRESHOLD) == -1) return false;
	if(computeGradience(eimg1d, outimg1d, sz) == -1) return false;

	V3DLONG gsz[3] = {10,10,10};
	V3DLONG * grids[3];
	V3DLONG gridnum;
	if(computeEdgeGrid(grids, gridnum, gsz, eimg1d, sz) == -1) return false;
	cout<<"gridnum = "<<gridnum<<" , gsz[0] = "<<gsz[0]<<" gsz[1] = "<<gsz[1]<<" gsz[2] = "<<gsz[2]<<endl;
	for(V3DLONG i = 0; i < gridnum; i++)
	{
		markerlist.push_back(MyImageMarker(grids[0][i], grids[1][i], grids[2][i]));
	}

	if(outimg1d) {delete outimg1d; outimg1d = 0;}
	if(eimg1d) {delete eimg1d; eimg1d = 0;}
}


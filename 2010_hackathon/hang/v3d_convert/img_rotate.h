#ifndef __IMG_ROTATE_H__
#define __IMG_ROTATE_H__

#include <iostream>
#include <cmath>
#include "v3d_basicdatatype.h"
#include "basic_memory.h"

using namespace std;
bool rotate_along_xaxis(double theta, unsigned char* inmg1d, V3DLONG * in_sz, unsigned char* & outimg1d, V3DLONG * & out_sz, bool keep_size = true);
bool rotate_along_yaxis(double theta, unsigned char* inmg1d, V3DLONG * in_sz, unsigned char* & outimg1d, V3DLONG * & out_sz, bool keep_size = true);
bool rotate_along_zaxis(double theta, unsigned char* inmg1d, V3DLONG * in_sz, unsigned char* & outimg1d, V3DLONG * & out_sz, bool keep_size = true);

#ifndef EPISILON 
#define EPISILON 0.000001
#endif

#define ABS(a)   ((a) > 0.0 ? (a) : -(a))
#define DIST(x1,y1,x2,y2) sqrt(((x1) - (x2))*((x1) - (x2)) + ((y1) - (y2))*((y1) - (y2)))

#define INIT_BKG_COLOR 255

/*
 * x1 = x0 , y1 = y0 * cos(theta) - z0 * sin(theta), z1 = y0 * sin(theta) + z0 * cos(theta)
 * */
bool rotate_along_xaxis(double theta, unsigned char* inimg1d, V3DLONG * in_sz, unsigned char* &outimg1d, V3DLONG * & out_sz, bool keep_size)
{
	if(inimg1d == 0 || outimg1d != 0 || in_sz[0] <=0 || in_sz[1] <= 0 || in_sz[2] <= 0 || in_sz[3] <= 0 ) return false;

	theta = - theta/180.0 * 3.1415926;

	if(!keep_size)
	{
		double max_h = 0, max_d = 0;
		double min_h = 1000000.0,  min_d = 1000000.0;
		theta = -theta;
		for(int k0 = 0; k0 < in_sz[2]; k0 += in_sz[2]-1)
		{
			double z0 = k0 - (in_sz[2]-1)/2.0;
			for(int j0 = 0; j0 < in_sz[1]; j0 += in_sz[1]-1)
			{
				double y0 = j0 - (in_sz[1]-1)/2.0;

				double j1 = y0 * cos(theta) - z0 * sin(theta) + (in_sz[1] - 1) / 2.0;
				double k1 = y0 * sin(theta) + z0 * cos(theta) + (in_sz[2] - 1) / 2.0;

				max_h = j1 > max_h ? j1 : max_h;
				max_d = k1 > max_d ? k1 : max_d;
				min_h = j1 < min_h ? j1 : min_h;
				min_d = k1 < min_d ? k1 : min_d;
			}
		}
		theta = -theta;

		out_sz = new V3DLONG[4];
		out_sz[0] = in_sz[0];
		out_sz[1] = (int)(max_h + 1) - (int)min_h + 1;
		out_sz[2] = (int)(max_d + 1) - (int)min_d + 1;
		out_sz[3] = in_sz[3];
	}
	else
	{

		out_sz = new V3DLONG[4];
		out_sz[0] = in_sz[0];
		out_sz[1] = in_sz[1];
		out_sz[2] = in_sz[2];
		out_sz[3] = in_sz[3];
	}

	cout<<"out_sz[0] = "<<out_sz[0]<<endl;
	cout<<"out_sz[1] = "<<out_sz[1]<<endl;
	cout<<"out_sz[2] = "<<out_sz[2]<<endl;
	cout<<"out_sz[3] = "<<out_sz[3]<<endl;

	unsigned char **** inimg4d = 0;
	unsigned char **** outimg4d = 0;
	try
	{
		outimg1d = new unsigned char[out_sz[0] * out_sz[1] * out_sz[2] * out_sz[3]];
		new4dpointer(inimg4d, in_sz[0], in_sz[1], in_sz[2], in_sz[3], inimg1d);
		new4dpointer(outimg4d, out_sz[0], out_sz[1], out_sz[2], out_sz[3], outimg1d);
	}
	catch(...)
	{
		cerr<<"Fail to allocate memory"<<endl;
		if(outimg1d) {delete [] outimg1d; outimg1d = 0;}
		if(inimg4d)delete4dpointer(inimg4d, in_sz[0], in_sz[1], in_sz[2], in_sz[3]);
		if(outimg4d)delete4dpointer(outimg4d, out_sz[0], out_sz[1], out_sz[2], out_sz[3]);
	}
	for(int i = 0; i < out_sz[0] * out_sz[1] * out_sz[2] * out_sz[3]; i++) outimg1d[i] = INIT_BKG_COLOR;

	for(int k0 = 0; k0 < out_sz[2]; k0++)
	{
		double z0 = k0 - (out_sz[2]-1)/2.0;
		for(int j0 = 0; j0 < out_sz[1]; j0++)
		{
			double y0 = j0 - (out_sz[1]-1)/2.0;

			int j1 = y0 * cos(theta) - z0 * sin(theta) + (in_sz[1] - 1) / 2.0 + 0.5;
			int k1 = y0 * sin(theta) + z0 * cos(theta) + (in_sz[2] - 1) / 2.0 + 0.5;

			if(k1 >= 0 && k1 < in_sz[2] && j1 >= 0 && j1 < in_sz[1])
			{
				for(int c = 0; c < in_sz[3]; c++)
					for(int i = 0; i < in_sz[0]; i++)
						outimg4d[c][k0][j0][i] = inimg4d[c][k1][j1][i];
			}
		}
	}

	if(inimg4d)delete4dpointer(inimg4d, in_sz[0], in_sz[1], in_sz[2], in_sz[3]);
	if(outimg4d)delete4dpointer(outimg4d, out_sz[0], out_sz[1], out_sz[2], out_sz[3]);

	return true;
}


/*
 * y1 = y0 , z1 = - x0 * sin(theta) + z0 * cos(theta), x1 = x0 * cos(theta) + z0 * sin(theta)
 * */
bool rotate_along_yaxis(double theta, unsigned char* inimg1d, V3DLONG * in_sz, unsigned char* &outimg1d, V3DLONG * & out_sz, bool keep_size)
{
	if(inimg1d == 0 || outimg1d != 0 || in_sz[0] <=0 || in_sz[1] <= 0 || in_sz[2] <= 0 || in_sz[3] <= 0 ) return false;

	theta = - theta/180.0 * 3.1415926;

	if(!keep_size)
	{
		double max_w = 0, max_d = 0;
		double min_w = 1000000.0,  min_d = 1000000.0;
		theta = -theta;
		for(int k0 = 0; k0 < in_sz[2]; k0 += in_sz[2]-1)
		{
			double z0 = k0 - (in_sz[2]-1)/2.0;
			for(int i0 = 0; i0 < in_sz[0]; i0 += in_sz[0]-1)
			{
				double x0 = i0 - (in_sz[0]-1)/2.0;

				double i1 = x0 * cos(theta) + z0 * sin(theta) + (in_sz[0] - 1) / 2.0;
				double k1 = - x0 * sin(theta) + z0 * cos(theta) + (in_sz[2] - 1) / 2.0;

				max_w = i1 > max_w ? i1 : max_w;
				max_d = k1 > max_d ? k1 : max_d;
				min_w = i1 < min_w ? i1 : min_w;
				min_d = k1 < min_d ? k1 : min_d;
			}
		}
		theta = -theta;

		out_sz = new V3DLONG[4];
		out_sz[0] = (int)(max_w + 1) - (int)min_w + 1;
		out_sz[1] = in_sz[1];
		out_sz[2] = (int)(max_d + 1) - (int)min_d + 1;
		out_sz[3] = in_sz[3];
	}
	else
	{

		out_sz = new V3DLONG[4];
		out_sz[0] = in_sz[0];
		out_sz[1] = in_sz[1];
		out_sz[2] = in_sz[2];
		out_sz[3] = in_sz[3];
	}

	cout<<"out_sz[0] = "<<out_sz[0]<<endl;
	cout<<"out_sz[1] = "<<out_sz[1]<<endl;
	cout<<"out_sz[2] = "<<out_sz[2]<<endl;
	cout<<"out_sz[3] = "<<out_sz[3]<<endl;

	unsigned char **** inimg4d = 0;
	unsigned char **** outimg4d = 0;
	try
	{
		outimg1d = new unsigned char[out_sz[0] * out_sz[1] * out_sz[2] * out_sz[3]];
		new4dpointer(inimg4d, in_sz[0], in_sz[1], in_sz[2], in_sz[3], inimg1d);
		new4dpointer(outimg4d, out_sz[0], out_sz[1], out_sz[2], out_sz[3], outimg1d);
	}
	catch(...)
	{
		cerr<<"Fail to allocate memory"<<endl;
		if(outimg1d) {delete [] outimg1d; outimg1d = 0;}
		if(inimg4d)delete4dpointer(inimg4d, in_sz[0], in_sz[1], in_sz[2], in_sz[3]);
		if(outimg4d)delete4dpointer(outimg4d, out_sz[0], out_sz[1], out_sz[2], out_sz[3]);
	}
	for(int i = 0; i < out_sz[0] * out_sz[1] * out_sz[2] * out_sz[3]; i++) outimg1d[i] = INIT_BKG_COLOR;

	for(int k0 = 0; k0 < out_sz[2]; k0++)
	{
		double z0 = k0 - (out_sz[2]-1)/2.0;
		for(int i0 = 0; i0 < out_sz[0]; i0++)
		{
			double x0 = i0 - (out_sz[0]-1)/2.0;

			int i1 = x0 * cos(theta) + z0 * sin(theta) + (in_sz[0] - 1) / 2.0 + 0.5;
			int k1 = -x0 * sin(theta) + z0 * cos(theta) + (in_sz[2] - 1) / 2.0 + 0.5;

			if(k1 >= 0 && k1 < in_sz[2] && i1 >= 0 && i1 < in_sz[0])
			{
				for(int c = 0; c < in_sz[3]; c++)
					for(int j = 0; j < in_sz[1]; j++)
						outimg4d[c][k0][j][i0] = inimg4d[c][k1][j][i1];
			}
		}
	}

	if(inimg4d)delete4dpointer(inimg4d, in_sz[0], in_sz[1], in_sz[2], in_sz[3]);
	if(outimg4d)delete4dpointer(outimg4d, out_sz[0], out_sz[1], out_sz[2], out_sz[3]);

	return true;
}

/*
 * z1 = z0, x1 = x0 * cos(theta) - y0 * sin(theta), y1 = x0 * sin(theta) + y0 * cos(theta)
 * */
bool rotate_along_zaxis(double theta, unsigned char* inimg1d, V3DLONG * in_sz, unsigned char* &outimg1d, V3DLONG * & out_sz, bool keep_size)
{
	if(inimg1d == 0 || outimg1d != 0 || in_sz[0] <=0 || in_sz[1] <= 0 || in_sz[2] <= 0 || in_sz[3] <= 0 ) return false;

	theta = - theta/180.0 * 3.1415926;

	if(!keep_size)
	{
		double max_w = 0, max_h = 0;
		double min_w = 1000000.0,  min_h = 1000000.0;
		theta = -theta;
		for(int j0 = 0; j0 < in_sz[1]; j0+= in_sz[1]-1)
		{
			double y0 = j0 - (in_sz[1]-1)/2.0;
			for(int i0 = 0; i0 < in_sz[0]; i0 += in_sz[0]-1)
			{
				double x0 = i0 - (in_sz[0]-1)/2.0;

				double i1 = x0 * cos(theta) - y0 * sin(theta) + (in_sz[0] - 1) / 2.0;
				double j1 = x0 * cos(theta) + y0 * sin(theta) + (in_sz[1] - 1) / 2.0;

				max_w = i1 > max_w ? i1 : max_w;
				max_h = j1 > max_h ? j1 : max_h;
				min_w = i1 < min_w ? i1 : min_w;
				min_h = j1 < min_h ? j1 : min_h;
			}
		}
		theta = -theta;

		out_sz = new V3DLONG[4];
		out_sz[0] = (int)(max_w + 1) - (int)min_w + 1;
		out_sz[1] = (int)(max_h + 1) - (int)min_h + 1;
		out_sz[2] = in_sz[2];
		out_sz[3] = in_sz[3];
	}
	else
	{

		out_sz = new V3DLONG[4];
		out_sz[0] = in_sz[0];
		out_sz[1] = in_sz[1];
		out_sz[2] = in_sz[2];
		out_sz[3] = in_sz[3];
	}

	cout<<"out_sz[0] = "<<out_sz[0]<<endl;
	cout<<"out_sz[1] = "<<out_sz[1]<<endl;
	cout<<"out_sz[2] = "<<out_sz[2]<<endl;
	cout<<"out_sz[3] = "<<out_sz[3]<<endl;

	unsigned char **** inimg4d = 0;
	unsigned char **** outimg4d = 0;
	try
	{
		outimg1d = new unsigned char[out_sz[0] * out_sz[1] * out_sz[2] * out_sz[3]];
		new4dpointer(inimg4d, in_sz[0], in_sz[1], in_sz[2], in_sz[3], inimg1d);
		new4dpointer(outimg4d, out_sz[0], out_sz[1], out_sz[2], out_sz[3], outimg1d);
	}
	catch(...)
	{
		cerr<<"Fail to allocate memory"<<endl;
		if(outimg1d) {delete [] outimg1d; outimg1d = 0;}
		if(inimg4d)delete4dpointer(inimg4d, in_sz[0], in_sz[1], in_sz[2], in_sz[3]);
		if(outimg4d)delete4dpointer(outimg4d, out_sz[0], out_sz[1], out_sz[2], out_sz[3]);
	}
	for(int i = 0; i < out_sz[0] * out_sz[1] * out_sz[2] * out_sz[3]; i++) outimg1d[i] = INIT_BKG_COLOR;

	for(int j0 = 0; j0 < out_sz[1]; j0++)
	{
		double y0 = j0 - (out_sz[1]-1)/2.0;
		for(int i0 = 0; i0 < out_sz[0]; i0++)
		{
			double x0 = i0 - (out_sz[0]-1)/2.0;

			int i1 = x0 * cos(theta) - y0 * sin(theta) + (in_sz[0] - 1) / 2.0 + 0.5;
			int j1 = x0 * sin(theta) + y0 * cos(theta) + (in_sz[1] - 1) / 2.0 + 0.5;
			if(i1 >= 0 && i1 < in_sz[0] && j1 >=0 && j1 < in_sz[1])
			{
				for(int c = 0; c < in_sz[3]; c++)
					for(int k = 0; k < in_sz[2]; k++)
						outimg4d[c][k][j0][i0] = inimg4d[c][k][j1][i1];
			}
		}
	}

	if(inimg4d)delete4dpointer(inimg4d, in_sz[0], in_sz[1], in_sz[2], in_sz[3]);
	if(outimg4d)delete4dpointer(outimg4d, out_sz[0], out_sz[1], out_sz[2], out_sz[3]);

	return true;
}

#endif

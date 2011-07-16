#include <iostream>
#include <cmath>
#include "v3d_basicdatatype.h"
#include "basic_memory.h"

using namespace std;

#ifndef EPISILON 
#define EPISILON 0.000001
#endif

#define ABS(a)   ((a) > 0.0 ? (a) : -(a))
#define DIST(x1,y1,x2,y2) sqrt(((x1) - (x2))*((x1) - (x2)) + ((y1) - (y2))*((y1) - (y2)))

bool rotate_along_xaxis(unsigned char* inimg1d, V3DLONG * in_sz, unsigned char* &outimg1d, V3DLONG * & out_sz, float theta, bool keep_size)
{
	if(inimg1d == 0 || outimg1d != 0 || in_sz[0] <=0 || in_sz[1] <= 0 || in_sz[2] <= 0 || in_sz[3] <= 0 ) return false;

	theta = - theta/180.0 * 3.1415926;

	if(!keep_size)
	{
		int max_h = 0; int max_d = 0;
		int min_h = 1000000; int min_d = 1000000;
		theta = -theta;
		for(int k = 0; k < in_sz[2]; k+= in_sz[2]-1)
		{
			float z = k - (in_sz[2]-1)/2.0;
			for(int j = 0; j < in_sz[1]; j+=in_sz[1]-1)
			{
				float y = j - (in_sz[1]-1)/2.0;
				float r = sqrt(y * y + z * z);
				int h = 0;
				int d = 0;

				float sin_phi = z / r;
				float cos_phi = y / r;
				float x2 = r * cos_phi * cos(theta) + r * sin_phi * sin(theta);
				float y2 = r * sin_phi * cos(theta) - r * cos_phi * sin(theta);

				h = y + (in_sz[1] - 1) / 2.0 + 0.5;
				d = z + (in_sz[2] - 1) / 2.0 + 0.5;
				max_h = h > max_h ? h : max_h;
				max_d = d > max_d ? d : max_d;
				min_h = h < min_h ? h : min_h;
				min_d = d < min_d ? d : min_d;
			}
		}
		theta = -theta;

		out_sz = new V3DLONG[4];
		out_sz[0] = in_sz[0];
		out_sz[1] = max_h - min_h + 1;
		out_sz[2] = max_d - min_d + 1;
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
		if(outimg1d) {delete outimg1d; outimg1d = 0;}
		if(inimg4d)delete4dpointer(inimg4d, in_sz[0], in_sz[1], in_sz[2], in_sz[3]);
		if(outimg4d)delete4dpointer(outimg4d, out_sz[0], out_sz[1], out_sz[2], out_sz[3]);
	}
	for(int i = 0; i < out_sz[0] * out_sz[1] * out_sz[2] * out_sz[3]; i++) outimg1d[i] = 255;

	for(int k = 0; k < out_sz[2]; k++)
	{
		float z = k - (out_sz[2]-1)/2.0;
		for(int j = 0; j < out_sz[1]; j++)
		{
			float y = j - (out_sz[1]-1)/2.0;
			float r = sqrt(y * y + z * z);
			int h = y + (in_sz[1] - 1) / 2.0 + 0.5;
			int d = z + (in_sz[2] - 1) / 2.0 + 0.5;
			if(r > EPISILON)
			{
				float sin_phi = z / r;
				float cos_phi = y / r;
				float y2 = r * cos_phi * cos(theta) + r * sin_phi * sin(theta);
				float z2 = r * sin_phi * cos(theta) - r * cos_phi * sin(theta);

				h = y2 + (in_sz[1] - 1) / 2.0 + 0.5;
				d = z2 + (in_sz[2] - 1) / 2.0 + 0.5;
			}	
			for(int c = 0; c < in_sz[3]; c++)
			{
				for(int i = 0; i < in_sz[0]; i++)
				{
					if(h >= 0 && h < in_sz[0] && d >=0 && d < in_sz[2])
						outimg4d[c][k][j][i] = inimg4d[c][d][h][i];
				}
			}
		}
	}

	if(inimg4d)delete4dpointer(inimg4d, in_sz[0], in_sz[1], in_sz[2], in_sz[3]);
	if(outimg4d)delete4dpointer(outimg4d, out_sz[0], out_sz[1], out_sz[2], out_sz[3]);

	return true;
}
bool rotate_along_yaxis(unsigned char* inimg1d, V3DLONG * in_sz, unsigned char* &outimg1d, V3DLONG * & out_sz, float theta, bool keep_size)
{
	if(inimg1d == 0 || outimg1d != 0 || in_sz[0] <=0 || in_sz[1] <= 0 || in_sz[2] <= 0 || in_sz[3] <= 0 ) return false;

	theta = - theta/180.0 * 3.1415926;

	if(!keep_size)
	{
		int max_w = 0; int max_d = 0;
		int min_w = 1000000; int min_d = 1000000;
		theta = -theta;
		for(int k = 0; k < in_sz[2]; k+= in_sz[2]-1)
		{
			float z = k - (in_sz[2]-1)/2.0;
			for(int i = 0; i < in_sz[0]; i+=in_sz[0]-1)
			{
				float x = i - (in_sz[0]-1)/2.0;
				float r = sqrt(x * x + z * z);
				int w = 0;
				int d = 0;

				float sin_phi = z / r;
				float cos_phi = x / r;
				float x2 = r * cos_phi * cos(theta) + r * sin_phi * sin(theta);
				float y2 = r * sin_phi * cos(theta) - r * cos_phi * sin(theta);

				w = x2 + (in_sz[0] - 1) / 2.0 + 0.5;
				d = y2 + (in_sz[1] - 1) / 2.0 + 0.5;
				max_w = w > max_w ? w : max_w;
				max_d = d > max_d ? d : max_d;
				min_w = w < min_w ? w : min_w;
				min_d = d < min_d ? d : min_d;
			}
		}
		theta = -theta;

		out_sz = new V3DLONG[4];
		out_sz[0] = max_w - min_w + 1;
		out_sz[1] = in_sz[1];
		out_sz[2] = max_d - min_d + 1;
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
		if(outimg1d) {delete outimg1d; outimg1d = 0;}
		if(inimg4d)delete4dpointer(inimg4d, in_sz[0], in_sz[1], in_sz[2], in_sz[3]);
		if(outimg4d)delete4dpointer(outimg4d, out_sz[0], out_sz[1], out_sz[2], out_sz[3]);
	}
	for(int i = 0; i < out_sz[0] * out_sz[1] * out_sz[2] * out_sz[3]; i++) outimg1d[i] = 255;

	for(int k = 0; k < out_sz[2]; k++)
	{
		float z = k - (out_sz[2]-1)/2.0;
		for(int i = 0; i < out_sz[0]; i++)
		{
			float x = i - (out_sz[0]-1)/2.0;
			float r = sqrt(x * x + z * z);
			int w = x + (in_sz[0] - 1) / 2.0 + 0.5;
			int d = z + (in_sz[2] - 1) / 2.0 + 0.5;
			if(r > EPISILON)
			{
				float sin_phi = z / r;
				float cos_phi = x / r;
				float x2 = r * cos_phi * cos(theta) + r * sin_phi * sin(theta);
				float z2 = r * sin_phi * cos(theta) - r * cos_phi * sin(theta);

				w = x2 + (in_sz[0] - 1) / 2.0 + 0.5;
				d = z2 + (in_sz[1] - 1) / 2.0 + 0.5;
			}	
			for(int c = 0; c < in_sz[3]; c++)
			{
				for(int j = 0; j < in_sz[1]; j++)
				{
					if(w >= 0 && w < in_sz[0] && d >=0 && d < in_sz[2])
						outimg4d[c][k][j][i] = inimg4d[c][d][j][w];
				}
			}
		}
	}

	if(inimg4d)delete4dpointer(inimg4d, in_sz[0], in_sz[1], in_sz[2], in_sz[3]);
	if(outimg4d)delete4dpointer(outimg4d, out_sz[0], out_sz[1], out_sz[2], out_sz[3]);

	return true;

}
bool rotate_along_zaxis(unsigned char* inimg1d, V3DLONG * in_sz, unsigned char* &outimg1d, V3DLONG * & out_sz, float theta, bool keep_size)
{
	if(inimg1d == 0 || outimg1d != 0 || in_sz[0] <=0 || in_sz[1] <= 0 || in_sz[2] <= 0 || in_sz[3] <= 0 ) return false;

	theta = - theta/180.0 * 3.1415926;

	if(!keep_size)
	{
		int max_w = 0; int max_h = 0;
		int min_w = 1000000; int min_h = 1000000;
		theta = -theta;
		for(int j = 0; j < in_sz[1]; j+= in_sz[1]-1)
		{
			float y = j - (in_sz[1]-1)/2.0;
			for(int i = 0; i < in_sz[0]; i+=in_sz[0]-1)
			{
				float x = i - (in_sz[0]-1)/2.0;
				float r = sqrt(x * x + y * y);
				int w = i;
				int h = j;

				float sin_phi = y / r;
				float cos_phi = x / r;
				float x2 = r * cos_phi * cos(theta) + r * sin_phi * sin(theta);
				float y2 = r * sin_phi * cos(theta) - r * cos_phi * sin(theta);

				w = x2 + (in_sz[0] - 1) / 2.0 + 0.5;
				h = y2 + (in_sz[1] - 1) / 2.0 + 0.5;
				max_w = w > max_w ? w : max_w;
				max_h = h > max_h ? h : max_h;
				min_w = w < min_w ? w : min_w;
				min_h = h < min_h ? h : min_h;
			}
		}
		theta = -theta;

		out_sz = new V3DLONG[4];
		out_sz[0] = max_w - min_w + 1;
		out_sz[1] = max_h - min_h + 1;
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
		if(outimg1d) {delete outimg1d; outimg1d = 0;}
		if(inimg4d)delete4dpointer(inimg4d, in_sz[0], in_sz[1], in_sz[2], in_sz[3]);
		if(outimg4d)delete4dpointer(outimg4d, out_sz[0], out_sz[1], out_sz[2], out_sz[3]);
	}
	for(int i = 0; i < out_sz[0] * out_sz[1] * out_sz[2] * out_sz[3]; i++) outimg1d[i] = 255;

	for(int j = 0; j < out_sz[1]; j++)
	{
		float y = j - (out_sz[1]-1)/2.0;
		for(int i = 0; i < out_sz[0]; i++)
		{
			float x = i - (out_sz[0]-1)/2.0;
			float r = sqrt(x * x + y * y);
			int w = x + (in_sz[0] - 1) / 2.0 + 0.5;
			int h = y + (in_sz[1] - 1) / 2.0 + 0.5;
			if(r > EPISILON)
			{
				float sin_phi = y / r;
				float cos_phi = x / r;
				float x2 = r * cos_phi * cos(theta) + r * sin_phi * sin(theta);
				float y2 = r * sin_phi * cos(theta) - r * cos_phi * sin(theta);

				w = x2 + (in_sz[0] - 1) / 2.0 + 0.5;
				h = y2 + (in_sz[1] - 1) / 2.0 + 0.5;
			}	
			for(int c = 0; c < in_sz[3]; c++)
			{
				for(int k = 0; k < in_sz[2]; k++)
				{
					if(w >= 0 && w < in_sz[0] && h >=0 && h < in_sz[1])
						outimg4d[c][k][j][i] = inimg4d[c][k][h][w];
				}
			}
		}
	}

	if(inimg4d)delete4dpointer(inimg4d, in_sz[0], in_sz[1], in_sz[2], in_sz[3]);
	if(outimg4d)delete4dpointer(outimg4d, out_sz[0], out_sz[1], out_sz[2], out_sz[3]);

	return true;
}


#include <iostream>
#include <cmath>
#include "v3d_basicdatatype.h"

using namespace std;

#ifndef EPISILON 
#define EPISILON 0.000001
#endif

#define ABS(a)   ((a) > 0.0 ? (a) : -(a))
#define DIST(x1,y1,x2,y2) sqrt(((x1) - (x2))*((x1) - (x2)) + ((y1) - (y2))*((y1) - (y2)))

bool rotate_along_zaxis(unsigned char* inimg1d, V3DLONG * in_sz, unsigned char* &outimg1d, float theta)
{
	if(inimg1d == 0 || outimg1d != 0 || in_sz[0] <=0 || in_sz[1] <= 0 || in_sz[2] <= 0 || in_sz[3] <= 0 ) return false;

	theta = theta/180.0 * 3.1415926;
	V3DLONG width = in_sz[0];
	V3DLONG height = in_sz[1];
	V3DLONG depth = in_sz[2];
	V3DLONG channels = in_sz[3];

	/*out_sz = new V3DLONG[4];
	out_sz[0] = width;
	out_sz[1] = height;
	out_sz[2] = depth;
	out_sz[3] = channels;
*/
	outimg1d = new unsigned char[width*height*depth*channels];
	for(int i = 0; i < width * height * depth * channels; i++) outimg1d[i] = 255;
	for(int k=0; k < depth; k++)
	{
		for(int j = 0; j < height; j++)
		{
			float y = j - (height-1)/2.0;
			for(int i = 0; i < width; i++)
			{
				float x = i - (width-1)/2.0;
				float r = sqrt(x * x + y * y);
				if(r > EPISILON)
				{
					float sin_phi = y / r;
					float cos_phi = x / r;
					float x2 = r * cos_phi * cos(theta) + r * sin_phi * sin(theta);
					float y2 = r * sin_phi * cos(theta) - r * cos_phi * sin(theta);

					int w = x2 + (width - 1) / 2.0 + 0.5;
					int h = y2 + (height - 1) / 2.0 + 0.5;
					if(w >= 0 && w < width && h >=0 && h < height)
					{
						int src_ind = (k * width * height + j * width + i) * channels;
						int dst_ind = (k * width * height + h * width + w) * channels;
						//cout<<"("<<i<<","<<j<<","<<k<<") -> ("<<w<<","<<h<<","<<k<<") "<<src_ind<<" -> "<<dst_ind<<endl;
						for(int c = 0; c < channels; c++)
						{
							outimg1d[dst_ind + c] = inimg1d[src_ind + c];
						}
					}

				}
				else 
				{
					int dst_ind = (k * width *height + j * width + i) * channels;
					int src_ind = dst_ind;
					//cout<<"src_ind = "<<src_ind<<endl;
					for(int c = 0; c < channels; c++)
					{
						outimg1d[dst_ind + c] = inimg1d[src_ind + c];
					}
				}
			}
		}
	}

	return true;
}

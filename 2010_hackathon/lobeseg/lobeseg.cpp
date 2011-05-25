//lobeseg.cpp
//separated from the main file on Aug 22, 2008
//last update: by Hanchuan Peng 090609. set the outputimage's in-chann's lobe rgn to be 0 after the segmentation

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>

//the folowing conditional compilation is added by PHC, 2010-05-20
#if defined (_MSC_VER) && (_WIN64)
#include "../../v3d_main/basic_c_fun/vcdiff.h"
#else
#endif

#include "../../v3d_main/basic_c_fun/stackutil.h"
#include "../../v3d_main/basic_c_fun/volimg_proc.h"

#include "lobeseg.h"

#include "../../v3d_main/worm_straighten_c/bdb_minus.h"

bool do_lobeseg_bdbminus(unsigned char *inimg1d, const V3DLONG sz[4], unsigned char *outimg1d, int in_channel_no, int out_channel_no, const BDB_Minus_ConfigParameter & mypara)
//note: assume the inimg1d and outimg1d have the same size, and normally out_channel_no should always be 2 (i.e. the third channel)
{
	if (!inimg1d || !outimg1d || !sz || sz[0]<0 || sz[1]<0 || sz[2]<0 || sz[3]<0 || in_channel_no<0 || in_channel_no>=sz[3] || out_channel_no<0)
	{
		printf("Invalid parameters to the function do_lobeseg_bdbminus(). \n");
		return false;
	}
	
	// Declare some necessary variables.
	vector<Coord2D> mCoord_Left;
	vector<Coord2D> mCoord_Right;
	vector<vector<Coord2D> > vl;
	vector<vector<Coord2D> > vr;
	
	//initialize control points

	int KK = ceil(double(sz[1])/10.0);

	float a_bottom_left[2];
	float a_bottom_right[2];

	float a_top_left[2];
	float a_top_right[2];

	a_bottom_left[0] = (sz[0] - 1)*2/5;	//x
	a_bottom_left[1] = sz[1] - 1;			//y

	a_bottom_right[0] = (sz[0] - 1)*3/5;
	a_bottom_right[1] = sz[1] - 1;

	a_top_left[0] = (sz[0] - 1)/10;
	a_top_left[1] = 0;

	a_top_right[0] = (sz[0] - 1)*9/10;
	a_top_right[1] = 0;

	Coord2D pt;
	int k, z, i, j;

	for (k=0; k <KK; k++)
	{
		pt.x = a_bottom_left[0] + k * (a_top_left[0]-a_bottom_left[0])/KK;
		pt.y = a_bottom_left[1] + k * (a_top_left[1]-a_bottom_left[1])/KK;
		mCoord_Left.push_back(pt);

		pt.x = a_bottom_right[0] + k * (a_top_right[0]-a_bottom_right[0])/KK;
		pt.y = a_bottom_right[1] + k * (a_top_right[1]-a_bottom_right[1])/KK;
		mCoord_Right.push_back(pt);
	}

	pt.x = a_top_left[0];
	pt.y = a_top_left[1];
	mCoord_Left.push_back(pt);

	pt.x = a_top_right[0];
	pt.y = a_top_right[1];
	mCoord_Right.push_back(pt);

	vl.push_back(mCoord_Left);
	vr.push_back(mCoord_Right);
	
	//do the computation
	{
		unsigned char ****inimg_4d = 0;
		new4dpointer(inimg_4d, sz[0], sz[1], sz[2], sz[3], inimg1d);

		unsigned char * tmp1d=0;
		try{
			tmp1d = new unsigned char [(V3DLONG)sz[0]*sz[1]];
		}
		catch(...){cerr << "Fail to allocate memory for temp image plane.\n" << endl;}

		unsigned char ** tmp2d=0;
		new2dpointer(tmp2d, sz[0], sz[1], tmp1d);

		cout << "start computing..." << endl;
		for (z=0;z<sz[2];z++)
		{
			for (int jj=0; jj<sz[1]; jj++)
			{
				for (int ii=0; ii<sz[0]; ii++)
					tmp2d[jj][ii] = 255 - inimg_4d[in_channel_no][z][jj][ii];
			}
			cout << z << " ";
			
			point_bdb_minus_2d_localwinmass_bl(tmp2d, sz[0], sz[1], vl.at(z), vl.at(z), mypara);
			vl.push_back( vl.at(z));
			
			point_bdb_minus_2d_localwinmass_bl(tmp2d, sz[0], sz[1], vr.at(z), vr.at(z), mypara);
			vr.push_back( vr.at(z));
		}
		cout << endl << "done computation." << endl << "Now saving file" <<endl;
		
		if (tmp2d) delete2dpointer(tmp2d, sz[0], sz[1]);
		if (tmp1d) {delete []tmp1d; tmp1d=0;}
		if (inimg_4d) delete4dpointer(inimg_4d, sz[0], sz[1], sz[2], sz[3]);
	}
	
	//now generate the separating surface
	int **left_bound=0, *left_bound1d=0;
	int **right_bound=0, *right_bound1d=0;
	
	try
	{
		left_bound1d = new int [sz[2]*sz[1]];
		new2dpointer(left_bound, sz[1], sz[2], left_bound1d);
		right_bound1d = new int [sz[2]*sz[1]];
		new2dpointer(right_bound, sz[1], sz[2], right_bound1d);
	}
	catch (...)
	{
		if (left_bound)	delete2dpointer(left_bound, sz[1], sz[2]);
		if (left_bound1d) {delete []left_bound1d; left_bound1d=0;}
		if (right_bound) delete2dpointer(right_bound, sz[1], sz[2]);
		if (right_bound1d) {delete []right_bound1d; right_bound1d=0;}
		
		printf("Fail to allocate momery.\n");
		return false;
	}
	

	for (z=0; z<sz[2]; z++)
	{
		vector<Coord2D> e_left;
		vector<Coord2D> e_right;

		e_left = vl[z+1];
		pt.x = vl[z+1][0].x;
		pt.y = sz[1]-1;
		e_left.insert(e_left.begin(), pt);
		e_left.insert(e_left.end(), vl[z+1].begin(), vl[z+1].end());
		pt.x = vl[z+1].end()->x;
		pt.y = 0;
		e_left.push_back(pt);

		e_right = vr[z+1];
		pt.x = vr[z+1][0].x;
		pt.y = sz[1]-1;
		e_right.insert(e_right.begin(), pt);
		e_right.insert(e_right.end(), vr[z+1].begin(), vr[z+1].end());
		pt.x = vr[z+1].end()->x;
		pt.y = 0;
		e_right.push_back(pt);

		int kk = e_left.size();
		
		for (k=0; k<kk-1; k++)
		{
			double i1 = e_left[k].x;	// "i" is "j" in the matlab code
			double j1 = e_left[k].y;
			double i2 = e_left[k+1].x;
			double j2 = e_left[k+1].y;
			
			if (j1>j2)
			{
				for (j=j1; j>j2; j--)
				{
					double w1 = j-j1;
					double w2 = j2-j;
					int tmpi = round((w2*i1 + w1*i2)/(w1+w2));
					left_bound[z][j] = tmpi;
				}				
			}
			else
			{
				for (j=j1; j<j2; j++)
				{
					double w1 = j-j1;
					double w2 = j2-j;
					int tmpi = round((w2*i1 + w1*i2)/(w1+w2));
					left_bound[z][j] = tmpi;
				}
			}
			
			i1 = e_right[k].x;	// "i" is "j" in the matlab code
			j1 = e_right[k].y;
			i2 = e_right[k+1].x;
			j2 = e_right[k+1].y;

			if (j1>j2)
			{
				for (j=j1; j>j2; j--)
				{
					double w1 = j-j1;
					double w2 = j2-j;
					int tmpi = round((w2*i1 + w1*i2)/(w1+w2));
					right_bound[z][j] = tmpi;
				}
			}
			else
			{
				for (j=j1; j<j2; j++)
				{
					double w1 = j-j1;
					double w2 = j2-j;
					int tmpi = round((w2*i1 + w1*i2)/(w1+w2));
					right_bound[z][j] = tmpi;
				}
			}
		}
		//printf("%d ", z);
	}
	//printf("\n");

	{
		unsigned char **** img_output_4d=0;
		new4dpointer(img_output_4d, sz[0], sz[1], sz[2], sz[3], outimg1d);

		for (z=0; z<sz[2]; z++)
		{
			for (j=0; j<sz[1]; j++)
			{
				for (i=0; i<sz[0]; i++)
				{

					if (i<left_bound[z][j])
					{
						img_output_4d[in_channel_no][z][j][i] = 0; //added by PHC,090609
						//img_output_4d[out_channel_no][z][j][i] = 254;
					}
					else if (i>right_bound[z][j])
					{
						img_output_4d[in_channel_no][z][j][i] = 0;  //added by PHC,090609
						//img_output_4d[out_channel_no][z][j][i] = 255;
					}
					else
					{
						//img_output_4d[out_channel_no][z][j][i] = 0;
					}
				}
			}

			if (sz[3]-1>=out_channel_no) //set up the mask if possible. 090730
			{
				for (j=0; j<sz[1]; j++)
				{
					for (i=0; i<sz[0]; i++)
					{
						if (i<left_bound[z][j])
						{
							//img_output_4d[in_channel_no][z][j][i] = 0; //added by PHC,090609
							img_output_4d[out_channel_no][z][j][i] = 254;
						}
						else if (i>right_bound[z][j])
						{
							//img_output_4d[in_channel_no][z][j][i] = 0;  //added by PHC,090609
							img_output_4d[out_channel_no][z][j][i] = 255;
						}
						else
						{
							img_output_4d[out_channel_no][z][j][i] = 0;
						}
					}
				}
			}

		}
		
		if (img_output_4d) {delete4dpointer(img_output_4d, sz[0], sz[1], sz[2], sz[3]); img_output_4d=0;}
	}

	// clean all workspace variables

	if (left_bound)	delete2dpointer(left_bound, sz[1], sz[2]);
	if (left_bound1d) {delete []left_bound1d; left_bound1d=0;}
	if (right_bound) delete2dpointer(right_bound, sz[1], sz[2]);
	if (right_bound1d) {delete []right_bound1d; right_bound1d=0;}

	return true;
}





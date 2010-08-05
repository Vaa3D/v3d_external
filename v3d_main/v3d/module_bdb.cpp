/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).  
 * All rights reserved.
 */


/************
                                            ********* LICENSE NOTICE ************

This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it. 

You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.

1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.

2. You agree to appropriately cite this work in your related studies and publications.

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/





/****************************************************************************
 **
 Copyright (C) 2006-2010 Hanchuan Peng. All rights reserved.

 module_bdb.cpp:
 separated from my4dimage.cpp on 2010-08-01
 */


#include <stdio.h>
#include <math.h>

#include <QtGui>

#include "v3d_core.h"
#include "mainwindow.h"

#include "../worm_straighten_c/bdb_minus.h"
#include "../worm_straighten_c/spline_cubic.h"

///////////////////////////////////////////////////////////////////////////////////////////////////



bool My4DImage::proj_worm_random_landmarking(int kch, double Kfactor, V3DLONG KK) //kch is the channel no
{
	//find the mean and std of the entire image
	if (datatype!=V3D_UINT8 || kch<0 || kch>=getCDim())
	{
		v3d_msg("Now only support UINT8 type of data. Your data is this type and thus do nothing.\n");
		return false;
	}

    ImagePixelType dtmp;
	int cur_z = int(floor(getZDim()/2));
	unsigned char ** inimg_data2d = ((unsigned char ****)getData(dtmp))[kch][cur_z];
	unsigned char * inimg_data1d = ((unsigned char ****)getData(dtmp))[kch][cur_z][0];

	double mean_val=0, std_val=0;
    data_array_mean_and_std(inimg_data1d, getXDim()*getYDim(), mean_val, std_val);
	double th_foreground = mean_val + Kfactor*std_val;
	printf("mean_val=%5.3f \t std_val=%5.3f\n", mean_val, std_val);

	//now compute

	V3DLONG i,j;
	vector <LocationSimple> xypos;
	LocationSimple cur_pos(-1,-1,-1);
	for (j=0;j<getYDim();j++)
	{
		for (i=0;i<getXDim();i++)
		{
			if (inimg_data2d[j][i]>th_foreground)
			{
				cur_pos.x = i;
				cur_pos.y = j;
				cur_pos.z = cur_z;
				cur_pos.inputProperty = pxLocaUnsure;
				xypos.push_back(cur_pos);
			}
		}
	}
	V3DLONG foreground_pixelnum = xypos.size();
	if (foreground_pixelnum<=0)
	{
		v3d_msg("You either choose an empty channel (all 0), or sth wrong. Do nothing.\n");
		return false;
	}

    //randomly initilize the control points

	listLandmarks.erase(listLandmarks.begin(), listLandmarks.end());
	listLocationRelationship.erase(listLocationRelationship.begin(), listLocationRelationship.end());

	for (i=0;i<KK;i++)
	{
#ifdef Q_OS_DARWIN
		V3DLONG tmppos=(random()%foreground_pixelnum);
#else
#ifdef Q_OS_WIN32
		V3DLONG tmppos=((V3DLONG(rand())*V3DLONG(rand()))%foreground_pixelnum); //080325, for compatability to Windows
#else

		V3DLONG tmppos=(random()%foreground_pixelnum); //081119. redhat

#endif
#endif
		listLandmarks.append(xypos.at(tmppos));
	}

	setFocusZ(cur_z+1);

	//update view
	updateViews();

	return true;
}

bool My4DImage::proj_worm_mst_diameter(bool b_keepDiameterOnly)
{
	if (listLandmarks.size()<=3)
	{
		v3d_msg("There are too few (<=3) defined landmark points. Do nothing to find the diameter of MST.\n");
		return false;
	}

	V3DLONG i;
	vector <LocationSimple> oldlist;
	for (i=0; i<listLandmarks.size();i++)
		oldlist.push_back(listLandmarks.at(i));
	vector <LocationSimple> diameterLocation;
	vector <PtIndexAndParents> mstParenthood;
	if (find_mst_diameter(oldlist, diameterLocation, mstParenthood)==false)
		return false;

	if (diameterLocation.size()<=0 ||
	    mstParenthood.size()<=0 ||
		V3DLONG(mstParenthood.size())!=listLandmarks.size()-1)
		return false;

	if (b_keepDiameterOnly) //in this case keep only the diameter graph
	{
		listLandmarks.clear();
		listLocationRelationship.clear();
		for (i=0;i<V3DLONG(diameterLocation.size());i++)
		{
			listLandmarks.append(diameterLocation.at(i));
			if (i>0)
				listLocationRelationship.append(PtIndexAndParents(i,i-1));
		}
		if (listLandmarks.count()>1)
			b_proj_worm_mst_diameter_set=true;
	}
	else //otherwise return the MST encoded in the mstParenthood
	{
		listLocationRelationship.clear();
		for (i=0;i<V3DLONG(mstParenthood.size());i++)
		{
			listLocationRelationship.append(mstParenthood.at(i));
		}
	}

	return true;
}

bool My4DImage::proj_worm_bdb_backbone()
{
	if (b_proj_worm_mst_diameter_set==false)
	{
		v3d_msg("The MST diameter has not been determined yet.\n");
		return false;
	}

	//first gen a maximum projection

	V3DLONG totalbytes = sz0*sz1*1*sz3;
	unsigned char * projvol1d = new unsigned char [totalbytes];
	if (!projvol1d)
	{
		v3d_msg("Fail to allocate memory in proj_worm_bdb_backbone() for the projection image.\n");
		return false;
	}
	XFormWidget * newwin = getXWidget()->getMainControlWindow()->createMdiChild();
	newwin->newProcessedImage("max projected", projvol1d, sz0, sz1, 1, sz3, V3D_UINT8); //only support UNIT8 now
	My4DImage * pProjDstImg = newwin->getImageData();

	unsigned char **** pProjDstImg_d4d = (unsigned char ****) pProjDstImg->getData();
	unsigned char **** pSrcImg_d4d = data4d_uint8;

	V3DLONG i,j,k,c;
	for (c=0; c<sz3; c++)
	{
		for (j=0;j<sz1;j++)
		{
			for (i=0;i<sz0;i++)
			{
				pProjDstImg_d4d[c][0][j][i] = pSrcImg_d4d[c][0][j][i];
				for (k=0; k<sz2; k++)
				{
					if (pSrcImg_d4d[c][k][j][i] > pProjDstImg_d4d[c][0][j][i])
						pProjDstImg_d4d[c][0][j][i] = pSrcImg_d4d[c][k][j][i];
				}
			}
		}
	}

	//
	/*
	 if(!point_bmeans_2d(const vector <Coord2D> & xypos, T ** inimg_data2d, vector <Coord2D> * mCoord_out, double TH, const BDB_Minus_ConfigParameter & sp_para))
	 {
	 v3d_msg("Fail in point_bmeans_2d().\n");
	 return false;
	 }
	 */

	//display the projection image and the backbone detection result
	newwin->show();
	newwin->getImageData()->updateViews();
	return true;
}

bool My4DImage::proj_worm_straightening(bool b_Restacking, int OutWid)
//if b_Restacking==true then do not show the interpolated backbone points and directly do straightening, otherwise just show the backbone location
{
	//this function will assume a set of well shaped points of the worm backbone has been detected, either for XY plane or XZ plane or YZ plane
	V3DLONG NPoints = listLandmarks.count();
//	printf("****QL:NPoints=%ld\n",NPoints);//090722-QL: for debug
	if (NPoints<=1)
	{
		v3d_msg("You have less than TWO control points for the backbone. Do nothing\n");
		return false;
	}
	if (OutWid<=0)
	{
		v3d_msg("The Width parameter is invalid, must > 0. You input [%d]. Do nothing.\n", OutWid);
		return false;
	}

	//set default of variables
	bool b_noerror=true;
	double *xpos = 0, *ypos = 0, *zpos = 0;
	V3DLONG i;
	LocationSimple tmp_pt(-1,-1,-1);
	const bool b_curveClosed = false; //note that for the worm project the curve is always open (i.e. natural but not peroidic)
	parameterCubicSpline ** cpara = 0;
	int cparaDim=0;
	bool b_3D_backbone=false;
	double *cp_x = 0, *cp_y = 0, *cp_z = 0, *cp_alpha = 0;
	V3DLONG cutPlaneNum = 0;

	V3DLONG insz[4]; insz[0]=getXDim(); insz[1]=getYDim(); insz[2]=getZDim(); insz[3]=getCDim();
	UINT8_TYPE * outvol1d = 0;
	V3DLONG * outdims = 0;

	// copy the control points location to the temp data structure for calling cubic-spline funcitons
	xpos = new double [NPoints];
	ypos = new double [NPoints];
	zpos = new double [NPoints];
	if (!xpos || !ypos || !zpos)
	{
		v3d_msg("Fail to allocate memory in proj_worm_cubic_spline_backbone(); Do nothing\n");
		b_noerror=false; goto Label_exit_proj_worm_cubic_spline_backbone;
	}

	for (i=0;i<NPoints;i++)
	{
		tmp_pt = listLandmarks.at(i);
		xpos[i] = tmp_pt.x;
		ypos[i] = tmp_pt.y;
		zpos[i] = tmp_pt.z;
	}

	//now estimate the cubic spline parameters
	if (b_3D_backbone)
	{
		cpara = est_cubic_spline_3d(xpos, ypos, zpos, NPoints, b_curveClosed);
		cparaDim = 3;
	}
	else
	{
		cpara = est_cubic_spline_2d(xpos, ypos, NPoints, b_curveClosed);
		cparaDim = 2;
	}

	//now find all the interpolated locations on the backbone (1-pixel spacing)
	if(!interpolate_cubic_spline(cpara, cparaDim, cp_x, cp_y, cp_z, cp_alpha, cutPlaneNum)) //080406: for debug
		//if(!interpolate_cubic_spline(cpara, cparaDim, 10, cp_x, cp_y, cp_z, cp_alpha, cutPlaneNum)) //080406: for debug
	{
		v3d_msg("Sth wrong in interpolate_cubic_spline(); \n");
		b_noerror=false; goto Label_exit_proj_worm_cubic_spline_backbone;
	}
//	printf("****QL:cutPlaneNum=%ld\n",cutPlaneNum);//090722-QL: for debug
//	for (i=0;i<cutPlaneNum;i++)//0907220-QL: for debug
//	{	//(cp_x[0],cp_y[0]) is wrong!
//		printf("\t(%ld):x=%.2f,y=%.2f\n",i,cp_x[i],cp_y[i]);
//	}

	//update(append to) the existing backbone points which will be displayed as different shapes
	if (b_Restacking==false)
	{
//		for (i=0;i<cutPlaneNum;i++)
		listLandmarks.clear();		//090722-QL: fix a bug in here, see below
		for (i=1;i<cutPlaneNum;i++)	//090722-QL: skip the first interpolated B spline point, since it is a wrong point!
		{
			tmp_pt.x = cp_x[i]; tmp_pt.y = cp_y[i];
			//tmp_pt.z = cp_z[i];
			tmp_pt.inputProperty = pxTemp;
			listLandmarks.append(tmp_pt);
		}
	}
	else
	{	//Now straighten via restacking
		if(!straight_nearestfill(getRawData(), insz, 4, cp_x, cp_y, cp_alpha, cutPlaneNum, OutWid, outvol1d, outdims))
		{
			v3d_msg("Fail to restack. \n");
			b_noerror=false; goto Label_exit_proj_worm_cubic_spline_backbone;
		}
		else
		{
			XFormWidget * newwin = getXWidget()->getMainControlWindow()->createMdiChild();
			newwin->newProcessedImage("straightened", outvol1d, outdims[0], outdims[1], outdims[2], outdims[3], datatype); //the datatype remain unchanged
			newwin->show();
			newwin->getImageData()->updateViews();
		}
	}

	//free space
Label_exit_proj_worm_cubic_spline_backbone:
	//note that outvol1d pointer will be re-used for the new image, thus do not delete here
	if (outdims) {delete []outdims; outdims=0;}

	if (cp_x) {delete []cp_x; cp_x=0;}
	if (cp_y) {delete []cp_y; cp_y=0;}
	if (cp_z) {delete []cp_z; cp_z=0;}
	if (cp_alpha) {delete []cp_alpha; cp_alpha=0;}

	if (cpara) //delete the cubic spline parameter data structure
	{
		for (i=0;i<cparaDim;i++) {if (cpara[i]) {delete cpara[i]; cpara[i]=0;}}
		delete []cpara; cpara=0;
	}

	if (xpos) {delete []xpos; xpos=0;}
	if (ypos) {delete []ypos; ypos=0;}
	if (zpos) {delete []zpos; zpos=0;}
	return b_noerror;
}


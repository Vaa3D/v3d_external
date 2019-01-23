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
 Copyright (C) 2006-2009 Hanchuan Peng. All rights reserved.

 my4dimage.cpp: some functions for the class my3dimage

 This is separated from v3d_core.cpp, as it becomes too big

 by Hanchuan Peng
 2008-12-07
 2009-07-18. replace many printf() as v3d_msg(), and some (not all yet) new() to try {} catch {}
 2009-07-18: update the converet-to-8bit function
 2009-09-11/12: automaker interface
 2009-11-14: separate the neuron tracing code to v3dimg_proj_neuron.cpp
 */

// avoid compile error from late load of windows.h
#ifdef _MSC_VER
#define NOMINMAX //added by PHC, 2010-05-20 to overcome VC min max macro
#include <windows.h>
#endif

#include <stdio.h>
#include <math.h>

#include "../3drenderer/v3dr_common.h"

#include <QLayout>
#include <QPainter>
#include <QPainterPath>
#include <QtGui>

#include <QString>

#include "v3d_core.h"
#include "mainwindow.h"

#include <fstream>
#include <iostream>
using namespace std;

#include "../worm_straighten_c/bdb_minus.h"
#include "../worm_straighten_c/spline_cubic.h"

#include "rotate_image.h"
#include "dialog_rotate.h"

#include "histogramsimple.h"

#include "atlas_viewer.h"

#include "../jba/c++/histeq.h"

#include "../basic_c_fun/volimg_proc.h"

#if defined(__APPLE__) //081124
#endif

#include "template_matching_cellseg_dialog.h"

#include "../gmm/fit_gmm.h"

#include "compute_win_pca.h"

#include "../3drenderer/barFigureDialog.h"
#include "../v3d/v3d_application.h"


bool compute_statistics_objects(Vol3DSimple<unsigned char> *grayimg, Vol3DSimple<unsigned short int> * maskimg, LocationSimple * & p_ano, V3DLONG & n_objects)
{
	if (!grayimg || !grayimg->valid() || !maskimg || !maskimg->valid() || p_ano) //p_ano MUST be 0 as this function need to alocate memory for it
	{
		v3d_msg("The inputs of compute_statistics_objects() are invalid.\n");
		return false;
	}

	if (!isSameSize(grayimg, maskimg))
	{
		v3d_msg("The sizes of the grayimg and maskimg do not match in compute_statistics_objects().\n");
		return false;
	}

	//first find the largest index
	V3DLONG i,j,k;

	unsigned short int *p_maskimg_1d = maskimg->getData1dHandle();
	unsigned short int ***p_maskimg_3d = maskimg->getData3dHandle();
	unsigned char *p_grayimg_1d = grayimg->getData1dHandle();
	unsigned char ***p_grayimg_3d = grayimg->getData3dHandle();

	n_objects = 0;
	for (i=0;i<maskimg->getTotalElementNumber();i++)
		n_objects = (p_maskimg_1d[i]>n_objects)?p_maskimg_1d[i]:n_objects;

	n_objects += 1; //always allocate one more, as the object index starts from 1. This will provide some convenience for later indexing (i.e. no need to minus 1)
	if (n_objects==1)
	{
		v3d_msg("The maskimg is all 0s. Nothing to generate!.\n");
		return false;
	}

	//then allocate memory and collect statistics

	try
	{
		p_ano = new LocationSimple [n_objects];
	}
	catch(...)
	{
		v3d_msg("Fail to allocate memory in compute_statistics_objects().\n");
		return false;
	}

	for (k=0;k<maskimg->sz2();k++)
		for (j=0;j<maskimg->sz1();j++)
			for (i=0;i<maskimg->sz0();i++)
			{
				V3DLONG cur_ind = p_maskimg_3d[k][j][i];
				if (p_grayimg_3d[k][j][i]==0) continue; //do not process 0 values, as it is background

				double cur_pix = double(p_grayimg_3d[k][j][i]);


				p_ano[cur_ind].size += 1;
				p_ano[cur_ind].mass += cur_pix;
				p_ano[cur_ind].sdev += cur_pix*cur_pix; //use the incremental formula
				if (cur_pix > p_ano[cur_ind].pixmax) p_ano[cur_ind].pixmax =  cur_pix;

				p_ano[cur_ind].x += i*cur_pix;
				p_ano[cur_ind].y += j*cur_pix;
				p_ano[cur_ind].z += k*cur_pix;
			}

	for (k=0;k<n_objects;k++)
	{
		if (p_ano[k].size>0)
		{
			p_ano[k].ave = p_ano[k].mass / p_ano[k].size;
			p_ano[k].sdev = sqrt(p_ano[k].sdev/p_ano[k].size - p_ano[k].ave*p_ano[k].ave); //use the incremental formula

			p_ano[k].x /= p_ano[k].mass;
			p_ano[k].y /= p_ano[k].mass;
			p_ano[k].z /= p_ano[k].mass;
		}
	}
	return true;
}

bool compute_statistics_objects(My4DImage *grayimg, V3DLONG c, My4DImage * maskimg, LocationSimple * & p_ano, V3DLONG & n_objects)
{
	if (!grayimg || !grayimg->valid() || !maskimg || !maskimg->valid() || p_ano) //p_ano MUST be 0 as this function need to alocate memory for it
	{
		v3d_msg("The inputs of compute_statistics_objects() are invalid.\n");
		return false;
	}

	if (grayimg->getDatatype()!=V3D_UINT8 || (maskimg->getDatatype()!=V3D_UINT16))
	{
		v3d_msg("The input datatypes are invalid. The grayimg must be UINT8 and the maskimg must be UINT16.\n");
		return false;
	}

	if (grayimg->getXDim()!=maskimg->getXDim() || grayimg->getYDim()!=maskimg->getYDim() || grayimg->getZDim()!=maskimg->getZDim())
	{
		v3d_msg("The sizes of the grayimg and maskimg do not match in compute_statistics_objects().\n");
		return false;
	}

	if (c<0 || c>=grayimg->getCDim() || maskimg->getCDim()!=1)
	{
		v3d_msg("The input channel of grayimg is invalid or the maskimg has more than 1 channel.\n");
		return false;
	}

	//first find the largest index
	V3DLONG i,j,k;

	unsigned short int *p_maskimg_1d = (unsigned short int *)(maskimg->getRawData());
	unsigned short int ***p_maskimg_3d = (unsigned short int ***)(((unsigned short int ****)maskimg->getData())[0]);
	unsigned char *p_grayimg_1d = (unsigned char *)(grayimg->getRawData()) + grayimg->getTotalUnitNumberPerChannel()*c;
	unsigned char ***p_grayimg_3d = (unsigned char ***)(((unsigned char ****)grayimg->getData())[c]);

	n_objects = 0;
	for (i=0;i<maskimg->getTotalUnitNumberPerChannel();i++)
		n_objects = (p_maskimg_1d[i]>n_objects)?p_maskimg_1d[i]:n_objects;

	n_objects += 1; //always allocate one more, as the object index starts from 1. This will provide some convenience for later indexing (i.e. no need to minus 1)
	if (n_objects==1)
	{
		v3d_msg("The maskimg is all 0s. Nothing to generate!.\n");
		return false;
	}

	//then allocate memory and collect statistics

	try
	{
		p_ano = new LocationSimple [n_objects];
	}
	catch(...)
	{
		v3d_msg("Fail to allocate memory in compute_statistics_objects().\n");
		return false;
	}

	for (k=0;k<maskimg->getZDim();k++)
		for (j=0;j<maskimg->getYDim();j++)
			for (i=0;i<maskimg->getXDim();i++)
			{
				V3DLONG cur_ind = p_maskimg_3d[k][j][i];
				if (p_grayimg_3d[k][j][i]==0) continue; //do not process 0 values, as it is background

				double cur_pix = double(p_grayimg_3d[k][j][i]);


				p_ano[cur_ind].size += 1;
				p_ano[cur_ind].mass += cur_pix;
				p_ano[cur_ind].sdev += cur_pix*cur_pix; //use the incremental formula
				if (cur_pix > p_ano[cur_ind].pixmax) p_ano[cur_ind].pixmax =  cur_pix;

				p_ano[cur_ind].x += i*cur_pix;
				p_ano[cur_ind].y += j*cur_pix;
				p_ano[cur_ind].z += k*cur_pix;
			}

	for (k=0;k<n_objects;k++)
	{
		if (p_ano[k].size>0)
		{
			p_ano[k].ave = p_ano[k].mass / p_ano[k].size;
			p_ano[k].sdev = sqrt(p_ano[k].sdev/p_ano[k].size - p_ano[k].ave*p_ano[k].ave); //use the incremental formula

			p_ano[k].x /= p_ano[k].mass;
			p_ano[k].y /= p_ano[k].mass;
			p_ano[k].z /= p_ano[k].mass;
		}
	}
	return true;
}


template <class T> int new3dpointer_v3d(T *** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, unsigned char * p1d)
{
	if (p!=0) {return 0;} //if the "p" is not empty initially, then do nothing and return un-successful

	p = new T ** [sz2];
	if (!p) {return 0;}

	for (V3DLONG i=0;i<sz2; i++)
	{
		p[i] = new T * [sz1];
		if (!p[i])
		{
			for (V3DLONG j=0;j<i;j++) {delete [] (p[i]);}
			delete []p;
			p=0;
			return 0;
		}
		else
		{
			for (V3DLONG j=0;j<sz1; j++)
				p[i][j] = (T *)(p1d + i*sz1*sz0*sizeof(T) + j*sz0*sizeof(T));
		}
	}

	return 1;
}

template <class T> void delete3dpointer_v3d(T *** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2)
{
	if (p==0) {return;} //if the "p" is empty initially, then do nothing

	for (V3DLONG i=0;i<sz2; i++)
	{
		if (p[i])
		{
			delete [] (p[i]);
			p[i] = 0;
		}
	}

	delete [] p;
	p = 0;

	//stupid method to remove stupid warnings
	sz0=sz0;
	sz1=sz1;

	return;
}

template <class T> int new4dpointer_v3d(T **** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, unsigned char * p1d)
{
	if (p!=0) {return 0;} //if the "p" is not empty initially, then do nothing and return un-successful

	p = new T *** [sz3];
	if (!p) {return 0;}

	for (V3DLONG i=0;i<sz3; i++)
	{
		p[i] = 0; //this sentence is very important to assure the function below knows this pointer is initialized as empty!!
		if (!new3dpointer_v3d(p[i], sz0, sz1, sz2, p1d+i*sz2*sz1*sz0*sizeof(T)))
		{
			v3d_msg("Problem happened in creating 3D pointers for channel-%ld.\n", i);
			for (V3DLONG j=0;j<i;j++) {delete [] (p[i]);}
			delete []p;
			p=0;
			return 0;
		}
	}

	return 1;
}

template <class T> void delete4dpointer_v3d(T **** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3)
{
	if (p==0) {return;} //if the "p" is empty initially, then do nothing

	for (V3DLONG i=0;i<sz3; i++)
	{
		delete3dpointer_v3d(p[i], sz0, sz1, sz2);
	}

	delete [] p;
	p = 0;

	//stupid method to remove stupid warnings
	sz0=sz0;
	sz1=sz1;
	sz2=sz2;

	return;
}

My4DImage::My4DImage()
{
	data4d_float32 = 0;
	data4d_uint16 = 0;
	data4d_uint8 = 0;
	data4d_virtual = 0; //a pointer used to link the external src data

	curFocusX = -1; //set as invalid values
	curFocusY = -1;
	curFocusZ = -1;

	p_xy_view = NULL;
	p_yz_view = NULL;
	p_zx_view = NULL;
	p_focusPointFeatureWidget = NULL;
	p_mainWidget = NULL;

	bLinkFocusViews = true;
	bDisplayFocusCross = true;
	//bImgValScaleDisplay = false;
	bLookingGlass = false;

	p_vmax = NULL;
	p_vmin = NULL;

	colorMap = new ColorMap(colorPseudoMaskColor, 256);

	b_proj_worm_mst_diameter_set=false;

	//global_setting.GPara_landmarkMatchingMethod = MATCH_MI; //080820

	atlasColorBlendChannel = 1; //081206
	bUseFirstImgAsMask=false; //081207
	curSearchText = "Enter search text here";

	last_hit_landmark=cur_hit_landmark=-1; //-1 is invalid for default. by PHC, 090119

	trace_bounding_box=NULL_BoundingBox; //090727 RZC: for trace from local view
	trace_z_thickness=1; //090727 RZC: weight for z-distance of graph

    b_triviewTimerON = false;

	connect(this, SIGNAL(focusFeatureViewTextUpdated()), this, SLOT(setText2FocusPointFeatureWidget()) );
}

My4DImage::~My4DImage()
{
	cleanExistData();
	if (colorMap) {delete colorMap; colorMap=0;} //080829: note that even the image data may get altered, I will not change the colormap unless the image is deleted
}

void My4DImage::createColorMap(int len, ImageDisplayColorType c)
{
	if (colorMap)
	{
		delete colorMap;
		colorMap=NULL;
	}

	//colorMap = new ColorMap(colorPseudoMaskColor, len);
	colorMap = new ColorMap(c, len); //070717
}

void My4DImage::switchColorMap(int len, ImageDisplayColorType c)
{
	if (colorMap && len>0 && (c==colorPseudoMaskColor || c==colorArnimFlyBrainColor || c==colorHanchuanFlyBrainColor)) //when switch colormap, must make sure the expected new colormap length/type are valid
	{
		delete colorMap;
		colorMap=NULL;
	}

	colorMap = new ColorMap(c, len); //
	//updateViews();//by PHC, 090211 //comment off by RZC 110804, no need
}

void My4DImage::getColorMapInfo(int & len, ImageDisplayColorType & c)
{
	if (colorMap)
	{
		len = colorMap->len;
		c = colorMap->ctype;
	}
}


void **** My4DImage::getData(ImagePixelType & dtype)
{
	dtype = this->getDatatype();
	if (dtype==V3D_UINT8 || dtype==V3D_UINT16 || dtype==V3D_FLOAT32)
		return data4d_virtual;
	else
		return NULL; //temnporarily allow only UINT8, UINT16, and FLOAT32 data
}

double My4DImage::at(int x, int y, int z, int c) const //return a double number because it can always be converted back to UINT8 and UINT16 without information loss
{ //return -1 in case error such as x,y,z,c are illegal values
	bool result =  (!data4d_virtual || x<0 || y<0 || z<0 || c<0 ||
					x >= this->getXDim() || y >= this->getYDim() || z>= this->getZDim() || c>=this->getCDim() );
	if ( result )
	{
		v3d_msg("error happened. Check the command line debuging info.");
		printf("error happened. p=%ld x=%d y=%d z=%d c=%d\n", (V3DLONG)data4d_virtual, x, y, z, c);
		return -1;
	}

	switch ( this->getDatatype() )
	{
		case V3D_UINT8:
			return double(data4d_uint8[c][z][y][x]);
			break;
		case V3D_UINT16:
			return double(data4d_uint16[c][z][y][x]);
			break;
		case V3D_FLOAT32:
			return double(data4d_float32[c][z][y][x]);
			break;
		default:
			return -1;
			break;
	}
}


void My4DImage::loadImage(const char* filename)
{
	cleanExistData();

	bool b_useMylib=false;


        bool lsmFlag = false;
        bool tiffFlag = false;
        QString qFilename = QString(filename);

        if (qFilename.endsWith("lsm") || qFilename.endsWith("LSM")) {
            lsmFlag = true;
        } else if (qFilename.endsWith("tif") || qFilename.endsWith("TIF") || qFilename.endsWith("tiff") || qFilename.endsWith("TIFF")) {
            tiffFlag = true;
        }

        if (lsmFlag) {
            b_useMylib = true;
        } else if (tiffFlag) {
            b_useMylib = false;
        } else if (V3dApplication::getMainWindow()) {
            b_useMylib = V3dApplication::getMainWindow()->global_setting.b_UseMylibTiff;
            qDebug() << "My4DImage::loadImage() set b_useMylib to value=" << b_useMylib << " based on global settings from MainWindow";
        }


        qDebug() << "My4DImage::loadImage() calling Image4DSimple::loadImage() with b_useMylib=" << b_useMylib;

	Image4DSimple::loadImage(filename, b_useMylib);

	setupData4D();
}

bool My4DImage::reshape(V3DLONG rsz0, V3DLONG rsz1, V3DLONG rsz2, V3DLONG rsz3)
{
	//if (!data4d_uint8) {v3d_msg("now only support unit8 in reshape().");  return false;}

	if (this->getXDim()==rsz0 && this->getYDim()==rsz1 && this->getZDim()==rsz2 && this->getCDim()==rsz3)
	{
		v3d_msg("The dimensions are the same. Do nothing.\n");
		return true;
	}
	if (this->getXDim()*this->getYDim()*this->getZDim()*this->getCDim() != rsz0*rsz1*rsz2*rsz3)
	{
		v3d_msg("The dimensions do not match. The total number of pixels are not the same. Do nothing.\n");
		return false;
	}

	cleanExistData_only4Dpointers();

	this->setXDim(rsz0);
	this->setYDim(rsz1);
	this->setZDim(rsz2);
	this->setCDim(rsz3);

	setupData4D();

	//update GUI

	curFocusX = this->getXDim()/2; //-= bpos_x+1; //begin from first slices
	curFocusY = this->getYDim()/2; //-= bpos_y+1;
	curFocusZ = this->getZDim()/2; //-= bpos_z+1;

	//update the color display mode, as the number of channels could change
	if (p_mainWidget->getColorType()!=colorPseudoMaskColor && p_mainWidget->getColorType()!=colorHanchuanFlyBrainColor && p_mainWidget->getColorType()!=colorArnimFlyBrainColor) //otherwise does not need to change
	{
		if (getCDim()>=3)
			p_mainWidget->setColorType(colorRGB);
		else if (getCDim()==2)
			p_mainWidget->setColorType(colorRG);
		else //==1
			p_mainWidget->setColorType(colorRed2Gray);
	}

	//update the landmarks
	listLandmarks.clear();
	listLocationRelationship.clear();

	//update view

	p_xy_view->deleteROI();
	p_yz_view->deleteROI();
	p_zx_view->deleteROI();

	p_mainWidget->updateDataRelatedGUI();

	updateViews();

}

bool My4DImage::permute(V3DLONG dimorder[4]) //081001: can also be impelemented using local swapping a pair of dimensions, and do multiple times; The serial pairs can be determined using quick sort algorithm.
{
	//first check the validity of the dimorder
	V3DLONG i,j,k,c;
	int dim_used_cnt[4]; for (i=0;i<4;i++) dim_used_cnt[i]=0; //initialized as 0
	for (i=0;i<4;i++)
	{
		switch(dimorder[i])
		{
			case 0: dim_used_cnt[dimorder[i]]++; break;
			case 1: dim_used_cnt[dimorder[i]]++; break;
			case 2: dim_used_cnt[dimorder[i]]++; break;
			case 3: dim_used_cnt[dimorder[i]]++; break;
			default:
				v3d_msg("dimorder contain invalid value. Must be interger between 0 to 3. No nothing.\n");
				return false;
		}
	}
	for (i=0;i<4;i++)
	{
		if (dim_used_cnt[i]!=1)
		{
			v3d_msg("dimorder contain invalid value. Every dimension index 0~3 must be used and only used once!. Do nothing.\n");
			return false;
		}
	}
	if (dimorder[0]==0 && dimorder[1]==1 && dimorder[2]==2 && dimorder[3]==3)
	{
		v3d_msg("The dimorder is in the same order of the original image. Nothing needs to be done. \n");
		return true;
	}

	//then generate a swap memory for data

	V3DLONG tmp_dim[4]; tmp_dim[0]=this->getXDim(); tmp_dim[1]=this->getYDim(); tmp_dim[2]=this->getZDim(); tmp_dim[3]=this->getCDim();
	My4DImage * tmp_img = 0;
	try
	{
		tmp_img = new My4DImage;
		tmp_img->loadImage(tmp_dim[dimorder[0]], tmp_dim[dimorder[1]], tmp_dim[dimorder[2]], tmp_dim[dimorder[3]], this->getDatatype() );
		if (!tmp_img->valid())
		{
			v3d_msg("Fail to produce a swap for image permutation. Do nothing.\n");
			return false;
		}
	}
	catch(...)
	{
		v3d_msg("Fail to allocate the swap memory. Do nothing.");
		if (tmp_img) {delete tmp_img; tmp_img=0;}
		return false;
	}

	float **** tmp_data4d_float32 = 0;
	USHORTINT16 **** tmp_data4d_uint16 = 0;
	unsigned char **** tmp_data4d_uint8 = 0;

	V3DLONG ind_array[4];
	switch ( this->getDatatype() )
	{
		case V3D_UINT8:
			tmp_data4d_uint8 = (unsigned char ****)tmp_img->getData();
			for (c=0;c<this->getCDim();c++)
			{
				ind_array[3] = c;
				for (k=0;k<this->getZDim();k++)
				{
					ind_array[2] = k;
					for (j=0;j<this->getYDim();j++)
					{
						ind_array[1] = j;
						for (i=0;i<this->getXDim();i++)
						{
							ind_array[0] = i;
							tmp_data4d_uint8[ind_array[dimorder[3]]][ind_array[dimorder[2]]][ind_array[dimorder[1]]][ind_array[dimorder[0]]] = data4d_uint8[c][k][j][i];
						}
					}
				}
			}
			break;
		case V3D_UINT16:
			tmp_data4d_uint16 = (USHORTINT16 ****)tmp_img->getData();
			for (c=0;c<this->getCDim();c++)
			{
				ind_array[3] = c;
				for (k=0;k<this->getZDim();k++)
				{
					ind_array[2] = k;
					for (j=0;j<this->getYDim();j++)
					{
						ind_array[1] = j;
						for (i=0;i<this->getXDim();i++)
						{
							ind_array[0] = i;
							tmp_data4d_uint16[ind_array[dimorder[3]]][ind_array[dimorder[2]]][ind_array[dimorder[1]]][ind_array[dimorder[0]]] = data4d_uint16[c][k][j][i];
						}
					}
				}
			}
			break;

		case V3D_FLOAT32:
			tmp_data4d_float32 = (float ****)tmp_img->getData();
			for (c=0;c<this->getCDim();c++)
			{
				ind_array[3] = c;
				for (k=0;k<this->getZDim();k++)
				{
					ind_array[2] = k;
					for (j=0;j<this->getYDim();j++)
					{
						ind_array[1] = j;
						for (i=0;i<this->getXDim();i++)
						{
							ind_array[0] = i;
							tmp_data4d_float32[ind_array[dimorder[3]]][ind_array[dimorder[2]]][ind_array[dimorder[1]]][ind_array[dimorder[0]]] = data4d_float32[c][k][j][i];
						}
					}
				}
			}
			break;


		default:
			v3d_msg("Should never be here in permute(). Check your program.\n");
			if (tmp_img) {delete tmp_img; tmp_img=0;}
			return false;
	}

	unsigned char *tmp_1d = tmp_img->getRawData();
	unsigned char *dst_1d = this->getRawData();
	for (i=0;i<getTotalBytes();i++)
    {
		dst_1d[i] = tmp_1d[i];
    }

	if (tmp_img) {delete tmp_img; tmp_img=0;}

	cleanExistData_only4Dpointers();

	this->setXDim( tmp_dim[dimorder[0]] );
	this->setYDim( tmp_dim[dimorder[1]] );
	this->setZDim( tmp_dim[dimorder[2]] );
	this->setCDim( tmp_dim[dimorder[3]] );

	setupData4D();

	//update GUI

	curFocusX = this->getXDim()/2; //-= bpos_x+1; //begin from first slices
	curFocusY = this->getYDim()/2; //-= bpos_y+1;
	curFocusZ = this->getZDim()/2; //-= bpos_z+1;

	//update the color display mode, as the number of channels could change
	if (p_mainWidget->getColorType()!=colorPseudoMaskColor && p_mainWidget->getColorType()!=colorHanchuanFlyBrainColor && p_mainWidget->getColorType()!=colorArnimFlyBrainColor) //otherwise does not need to change
	{
		if (getCDim()>=3)
			p_mainWidget->setColorType(colorRGB);
		else if (getCDim()==2)
			p_mainWidget->setColorType(colorRG);
		else //==1
			p_mainWidget->setColorType(colorRed2Gray);
	}

	//update the landmarks
	listLandmarks.clear();
	listLocationRelationship.clear();

	//update view

	p_xy_view->deleteROI();
	p_yz_view->deleteROI();
	p_zx_view->deleteROI();

	p_mainWidget->updateDataRelatedGUI();

	updateViews();
}

double My4DImage::getChannalMinIntensity(V3DLONG channo) //if channo <0 or out of range, then return the in of all channels
{
	if (!p_vmin) return 0;
	if (channo>=0 && channo<this->getCDim()) return p_vmin[channo];
	else {V3DLONG tmppos; return maxInVector(p_vmin, this->getCDim(), tmppos);}
}

double My4DImage::getChannalMaxIntensity(V3DLONG channo) //if channo <0 or out of range, then return the max of all channels
{
	if (!p_vmax) return 0;
	if (channo>=0 && channo<this->getCDim()) return p_vmax[channo];
	else {V3DLONG tmppos; return maxInVector(p_vmin, this->getCDim(), tmppos);}
}


void My4DImage::setupData4D()
{
	if (this->getError()==1 || ! this->getRawData() )
	{
		v3d_msg("Invalid input data for setting up 4D pointers setupData4D().\n", false);
		return;
	}

	if (!updateminmaxvalues())
	{
		v3d_msg("Fail to run successfully updateminmaxvalues(). Thus discontinue the setupData4D().\n", false);
		return;
	}

	switch ( this->getDatatype() )
	{
		case V3D_UINT8:
			if (!new4dpointer_v3d(data4d_uint8, this->getXDim(), this->getYDim(), this->getZDim(), this->getCDim(), this->getRawData() ))
			{
				this->setError( 1 );
				return;
			}
			data4d_virtual = (void ****)data4d_uint8;

			//if (sz3==1) //080829: as tere may be crop operation that keeps only one channel, thus always generate the colormap
			//createColorMap(256); //just use 256 for a safe pool for cell editing. No need to update the colormap again as in the constructor function the default colormap is already set
			break;

		case V3D_UINT16:
			if (!new4dpointer_v3d(data4d_uint16, this->getXDim(), this->getYDim(), this->getZDim(), this->getCDim(), this->getRawData() ))
			{
				this->setError( 1 );
				return;
			}
			data4d_virtual = (void ****)data4d_uint16;

			//080824: copied from wano project
			createColorMap(int(p_vmax[0])+1000); //add 1000 for a safe pool for cell editing.
			printf("set the color map max=%d\n", int(p_vmax[0]));

			break;

		case V3D_FLOAT32:
			if (!new4dpointer_v3d(data4d_float32, this->getXDim(), this->getYDim(), this->getZDim(), this->getCDim(), this->getRawData() ))
			{
				this->setError( 1 );
				return;
			}
			data4d_virtual = (void ****)data4d_float32;

			//FIXME: this may be a problem and need further test of float type for color indexing
			//createColorMap(256); //just use 256 for a safe pool for cell editing.
			break;

		default:
			this->setError( 1 );
			v3d_msg("Invalid data type found in setupData4D(). Should never happen, - check with V3D developers.");
			return;
			//break;
	}

	//set up the default color mapping table. 100824, PHC
	setupDefaultColorChannelMapping();

	//

	curFocusX = this->getXDim()>>1; //begin from mid location
	curFocusY = this->getYDim()>>1;
	curFocusZ = this->getZDim()>>1;
}

void My4DImage::setupDefaultColorChannelMapping() //20100824, PHC
{
	listChannels.clear();
	for (V3DLONG i=0; i<this->getCDim(); i++)
	{
		DataChannelColor dc;
		dc.n = i;
		dc.on  = true;
		V3DLONG tmp = i%7;
		dc.color.a = 255;
		dc.color.r  = dc.color.g = dc.color.b = 0;
		switch (tmp)
		{
			case 0: dc.color.r = 255; break;
			case 1: dc.color.g = 255; break;
			case 2: dc.color.b = 255; break;
			case 3: dc.color.r = 255; dc.color.g = 255; dc.color.b = 255; break;
			case 4: dc.color.r = 255; dc.color.g = 255; break;
			case 5: dc.color.r = 255; dc.color.b = 255; break;
			case 6: dc.color.g = 255; dc.color.b = 255; break;
			default:
				v3d_msg("Your should never see this msg. Check with V3D developer.");
		}

		listChannels.append(dc);
	}
}

bool My4DImage::updateminmaxvalues()
{
	if (this->getError() == 1 || !this->getRawData()  || this->getCDim()<=0 || this->getXDim()<=0 || this->getYDim()<=0 || this->getZDim()<=0)
	{
		v3d_msg("The image data is invalid.\n", false);
		return false;
	}

	//always delete the two pointers and recreate because if the image is altered in a plugin, the # of color channels may change
	if (p_vmax) {delete []p_vmax; p_vmax=0;}
	if (p_vmin) {delete []p_vmin; p_vmin=0;}

	try
	{
		p_vmax = new double [this->getCDim()];
		p_vmin = new double [this->getCDim()];
	}
	catch (...)
	{
		v3d_msg("Error happened in allocating memory in updateminmaxvalues().\n");
		this->setError(1);
		if (p_vmax) {delete []p_vmax; p_vmax=0;}
		if (p_vmin) {delete []p_vmin; p_vmin=0;}
		return false;
	}

	V3DLONG i, tmppos;
	V3DLONG channelPageSize = this->getTotalUnitNumberPerChannel();

	switch (this->getDatatype())
	{
		case V3D_UINT8:
			for(i=0;i<this->getCDim();i++)
			{
				unsigned char minvv,maxvv;
				V3DLONG tmppos_min, tmppos_max;
				unsigned char *datahead = (unsigned char *)getRawDataAtChannel(i);
				minMaxInVector(datahead, channelPageSize, tmppos_min, minvv, tmppos_max, maxvv);
				p_vmax[i] = maxvv; p_vmin[i] = minvv;
				v3d_msg(QString("channel %1 min=[%2] max=[%3]").arg(i).arg(p_vmin[i]).arg(p_vmax[i]),0);
			}
			break;

		case V3D_UINT16:
			for(i=0;i<this->getCDim();i++)
			{
				USHORTINT16 minvv,maxvv;
				V3DLONG tmppos_min, tmppos_max;
				USHORTINT16 *datahead = (USHORTINT16 *)getRawDataAtChannel(i);
				minMaxInVector(datahead, channelPageSize, tmppos_min, minvv, tmppos_max, maxvv);
				p_vmax[i] = maxvv; p_vmin[i] = minvv;
				v3d_msg(QString("channel %1 min=[%2] max=[%3]").arg(i).arg(p_vmin[i]).arg(p_vmax[i]),0);
			}
			break;

		case V3D_FLOAT32:
			for(i=0;i<this->getCDim();i++)
			{
                float minvv,maxvv;

				V3DLONG tmppos_min, tmppos_max;
				float *datahead = (float *)getRawDataAtChannel(i);

                if (0) //for debugging purpose. 2014-08-22
                {
                    minvv=datahead[0], maxvv=datahead[0];
                    for (V3DLONG myii=1; myii<channelPageSize;myii++)
                    {
                        if (minvv>datahead[myii]) minvv=datahead[myii];
                        else if (maxvv<datahead[myii]) maxvv=datahead[myii];
                    }

                    p_vmax[i] = maxvv; p_vmin[i] = minvv;
                    v3d_msg(QString("channel %1 min=[%2] max=[%3]").arg(i).arg(p_vmin[i]).arg(p_vmax[i]));
                }
                else
                {
                    if (minMaxInVector(datahead, channelPageSize, tmppos_min, minvv, tmppos_max, maxvv))
                    {
                        p_vmax[i] = maxvv; p_vmin[i] = minvv;
                        v3d_msg(QString("channel %1 min=[%2] max=[%3]").arg(i).arg(p_vmin[i]).arg(p_vmax[i]), 0);
                    }
                    else
                    {
                        v3d_msg("fail");
                    }
                }

			}
			break;

		default:
			this->setError(1);
			v3d_msg("Invalid data type found in updateminmaxvalues(). Should never happen, - check with V3D developers.");
			return false;
	}

	return true;
}

void My4DImage::loadImage(V3DLONG imgsz0, V3DLONG imgsz1, V3DLONG imgsz2, V3DLONG imgsz3, int imgdatatype) //an overloaded function to create a blank image
{
	Image4DSimple::createBlankImage(imgsz0, imgsz1, imgsz2, imgsz3, imgdatatype);
	if (this->getError()==1 || !this->getRawData() )
	{
		v3d_msg("Error happened in creating 1d data.\n");
		return;
	}

	try {
		p_vmax = new double [this->getCDim()];
		p_vmin = new double [this->getCDim()];
	}
	catch (...)
	{
		v3d_msg("Error happened in allocating memory.\n");
		this->setError(1);
		if (p_vmax) {delete []p_vmax; p_vmax=NULL;}
		if (p_vmin) {delete []p_vmin; p_vmin=NULL;}
		return;
	}

	V3DLONG i, tmppos;
	V3DLONG channelPageSize = V3DLONG(this->getXDim())*this->getYDim()*this->getZDim();

	switch (this->getDatatype())
	{
		case V3D_UINT8:
			if (!new4dpointer(data4d_uint8, this->getXDim(), this->getYDim(), this->getZDim(), this->getCDim(), this->getRawData() ))
			{
				this->setError(1);
				return;
			}
			data4d_virtual = (void ****)data4d_uint8;

			for(i=0;i<this->getCDim();i++)
			{
				p_vmax[i] = 0; //no need to compute as it is blank
				p_vmin[i] = 0;
			}

			//if (sz3==1)
			//createColorMap(256); //just use 256 for a safe pool for cell editing. 060501

			break;

		case V3D_UINT16:
			if (!new4dpointer(data4d_uint16, this->getXDim(), this->getYDim(), this->getZDim(), this->getCDim(), (USHORTINT16 *)this->getRawData() ))
			{
				this->setError(1);
				return;
			}
			data4d_virtual = (void ****)data4d_uint16;

			for(i=0;i<this->getCDim();i++)
			{
				p_vmax[i] = 0;
				p_vmin[i] = 0;
			}

			createColorMap(int(p_vmax[0])+1000); //add 1000 for a safe pool for cell editing. 060501
			printf("set the color map max=%d\n", int(p_vmax[0]));

			//printf("Warning: this data type UINT16 has not been supported in display yet.\n");

			break;

		case V3D_FLOAT32:
			if (!new4dpointer(data4d_float32, this->getXDim(), this->getYDim(), this->getZDim(), this->getCDim(), (float *)this->getRawData() ))
			{
				this->setError(1);
				return;
			}
			data4d_virtual = (void ****)data4d_float32;

			for(i=0;i<this->getCDim();i++)
			{
				p_vmax[i] = 0;
				p_vmin[i] = 0;
			}

			v3d_msg("Warning: this data type FLOAT32 has not been supported in display yet - create blank image.\n");

			break;

		default:
			this->setError(1);
			return;
			//break;
	}

	curFocusX = this->getXDim()>>1; //change to middle slide 090718. //begin from first slices. Original is 1, should be wrong. corrected to 0 on 060426
	curFocusY = this->getYDim()>>1;
	curFocusZ = this->getZDim()>>1;
}

bool My4DImage::saveVANO_data()
{
	// begin the save process
	QMessageBox::information(0, "Information about VANO/WANO file saving",
							 "To export to VANO/WANO file, the current selected image is assumed to be the ORIGINAL image used for segmentation. <br><br>"
							 "You will be asked to specify<br>"
							 "(1)the MASK image, which defines the image objects.<br>"
							 "(2)the channel of the ORIGINAL image that should be used to compute the statistics<br>"
							 "(3)the name of the linker file.<br><br>"
							 "The respective ORIGINAL image, MASK image, and the actual CSV format annotation file, will have the names [linkerfile].gray.tif, [linkerfile].mask.raw, and [linkerfile].apo.<br>"
							 );

	//select the grayimage and mask image
	if ( this->getDatatype() != V3D_UINT8 )
	{
		v3d_msg("Now the VANO exporting program only supports 8bit data for the ORIGINAL (grayscale) stack. Check your data first.");
		return false;
	}

	if (!p_mainWidget) return false;
	My4DImage * pMaskImg = p_mainWidget->selectSubjectImage();
	if (!pMaskImg)
	{
		v3d_msg("Now you have only opened one image. The mask image should also be opened. Do nothing.");
		return false;
	}

	//determine which channel the statistics should collected

	bool ok1;

#if defined(USE_Qt5)
	int ch_ind = QInputDialog::getInt(0, "channel index", "which channel you want to collect statitics and annotate?", 1, 1, getCDim(), 1, &ok1) - 1;
#else
	int ch_ind = QInputDialog::getInteger(0, "channel index", "which channel you want to collect statitics and annotate?", 1, 1, getCDim(), 1, &ok1) - 1;
#endif

	if (!ok1)
		return false;

	//now select the linker file name

	QString linkerFile = QFileDialog::getSaveFileName(0,
													  "Choose a filename to save under",
													  //"./",
													  QString(this->getFileName())+".ano",
													  "Save VANO linker file format (*.ano)");

	while (linkerFile.isEmpty()) //note that I used isEmpty() instead of isNull, although seems the Cancel operation will return a null string. phc 060422
	{
    	if(QMessageBox::Yes == QMessageBox::question (0, "", "Are you sure you do NOT want to save?", QMessageBox::Yes, QMessageBox::No))
	    {
		    return false;
		}
		linkerFile = QFileDialog::getSaveFileName(0,
												  "Choose a filename to save under",
												  "./",
												  //QString(this->getFileName())+".ano",
												  "Save VANO linker file format (*.ano)");
	}

	//now save to vano files
	QString anoFile = linkerFile + ".apo";
	QString maskFile = linkerFile + ".mask.raw";
	QString grayFile = linkerFile + ".gray.tif";

	//first save the text annotation apo files
	LocationSimple * p_ano = 0;
	V3DLONG n_objects = 0;
	if (!compute_statistics_objects(this, ch_ind, pMaskImg, p_ano, n_objects))
	{
		v3d_msg("Some errors happen during the computation of image objects' statistics. The annotation is not generated.");
		return false;
	}

	FILE * f_ano = fopen(qPrintable(anoFile), "wt");
	if (!f_ano)
	{
		v3d_msg("Fail to open the annotation file to write. The annotation is not generated.");
		if (p_ano) {delete []p_ano; p_ano=0;}
		return false;
	}

	for (V3DLONG i=1;i<n_objects;i++) //do not process 0 values, as it is background. Thus starts from 1
	{
		fprintf(f_ano, "%ld,%ld,%s,%s,%d,%d,%d,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,\n",
				i,i,"","", int(p_ano[i].z+0.5), int(p_ano[i].x+0.5), int(p_ano[i].y+0.5),
				p_ano[i].pixmax, p_ano[i].ave, p_ano[i].sdev, p_ano[i].size, p_ano[i].mass);
	}

	fclose(f_ano);

	//finally save to image and mask and linker file

	saveImage(qPrintable(grayFile));
	pMaskImg->saveImage(qPrintable(maskFile));

	FILE * f_linker = fopen(qPrintable(linkerFile), "wt");
	if (!f_ano)
	{
		v3d_msg("Fail to open the annotation linker file to write. The annotation is not generated.");
		return false;
	}
	fprintf(f_linker, "GRAYIMG=%s\n", qPrintable(grayFile));
	fprintf(f_linker, "MASKIMG=%s\n", qPrintable(maskFile));
	fprintf(f_linker, "ANOFILE=%s\n", qPrintable(anoFile));
	fclose(f_linker);

	if (p_ano) {delete []p_ano; p_ano=0;}

	printf("Current image is exported to VANO files with the linker file name [%s]\n", qPrintable(linkerFile));

	return true;
}

bool My4DImage::saveMovie()
{
	//	// begin the save process
	//
	//	QString outputFile = QFileDialog::getSaveFileName(0,
	//													  "Choose a filename to save under",
	//													  //"./",
	//													  QString(this->getFileName())+".cp.tif",
	//													  "Save file format (*.raw *.tif)");
	//
	//	while (outputFile.isEmpty()) //note that I used isEmpty() instead of isNull, although seems the Cancel operation will return a null string. phc 060422
	//	{
	//    	if(QMessageBox::Yes == QMessageBox::question (0, "", "Are you sure you do NOT want to save?", QMessageBox::Yes, QMessageBox::No))
	//	    {
	//		    return false;
	//		}
	//		outputFile = QFileDialog::getSaveFileName(0,
	//												  "Choose a filename to save under",
	//												  "./",
	//												  "Save file format (*.raw *.tif)");
	//	}
	//
	//	saveImage(qPrintable(outputFile));
	//
	//	printf("Current image is saved to the file %s\n", qPrintable(outputFile));
	//
	return true;
}


bool My4DImage::saveFile()
{
	// begin the save process

	QString outputFile = QFileDialog::getSaveFileName(0,
													  "Choose a filename to save under",
													  //"./",
													  QString(this->getFileName())+".cp.tif",
													  "Save file format (*.raw *.tif)");

	while (outputFile.isEmpty()) //note that I used isEmpty() instead of isNull, although seems the Cancel operation will return a null string. phc 060422
	{
    	if(QMessageBox::Yes == QMessageBox::question (0, "", "Are you sure you do NOT want to save?", QMessageBox::Yes, QMessageBox::No))
	    {
		    return false;
		}
		outputFile = QFileDialog::getSaveFileName(0,
												  "Choose a filename to save under",
												  "./",
												  "Save file format (*.raw *.tif)");
	}

	saveImage(qPrintable(outputFile));

	printf("Current image is saved to the file %s\n", qPrintable(outputFile));

	return true;
}

bool My4DImage::saveFile(char filename[]) {
    QString outputFile(filename);
    return saveFile(outputFile);
}

bool My4DImage::saveFile(QString outputFile)
{
	while (outputFile.isEmpty())
	{
    	if(QMessageBox::Yes == QMessageBox::question (0, "", "Are you sure you do NOT want to save?", QMessageBox::Yes, QMessageBox::No))
	    {
		    return false;
		}
		outputFile = QFileDialog::getSaveFileName(0,
												  "Choose a filename to save under",
												  "./",
												  "Save file format (*.raw *.tif)");
	}

	saveImage(qPrintable(outputFile));

	printf("Current image is saved to the file %s\n", qPrintable(outputFile));
	//if (p_mainWidget) {p_mainWidget->setCurrentFileName(filename);}

	return true;
}

void My4DImage::crop(int landmark_crop_opt)
{
	//get the bounding boxes

	if (!p_xy_view || !p_yz_view || !p_zx_view)
		return;

	QRect b_xy = p_xy_view->getRoiBoundingRect();
	QRect b_yz = p_yz_view->getRoiBoundingRect();
	QRect b_zx = p_zx_view->getRoiBoundingRect();

	V3DLONG bpos_x = qBound(V3DLONG(0), V3DLONG(qMax(b_xy.left(), b_zx.left())), this->getXDim()-1),
	bpos_y = qBound(V3DLONG(0), V3DLONG(qMax(b_xy.top(),  b_yz.top())), this->getYDim()-1),
	bpos_z = qBound(V3DLONG(0), V3DLONG(qMax(b_yz.left(), b_zx.top())), this->getZDim()-1),
	bpos_c = 0;
	V3DLONG epos_x = qBound(V3DLONG(0), V3DLONG(qMin(b_xy.right(), b_zx.right())), this->getXDim()-1),
	epos_y = qBound(V3DLONG(0), V3DLONG(qMin(b_xy.bottom(), b_yz.bottom())), this->getYDim()-1),
	epos_z = qBound(V3DLONG(0), V3DLONG(qMin(b_yz.right(), b_zx.bottom())), this->getZDim()-1),
	epos_c = this->getCDim()-1;

	if (bpos_x>epos_x || bpos_y>epos_y || bpos_z>epos_z)
	{
		v3d_msg("The roi polygons in three views are not intersecting! No crop is done!\n");
		return;
	}

	// create new data, copy over, and delete the original data

	crop(bpos_x, epos_x, bpos_y, epos_y, bpos_z, epos_z, bpos_c, epos_c, landmark_crop_opt);

	return;
}

void My4DImage::crop(V3DLONG bpos_x, V3DLONG epos_x, V3DLONG bpos_y, V3DLONG epos_y, V3DLONG bpos_z, V3DLONG epos_z, V3DLONG bpos_c, V3DLONG epos_c, int landmark_crop_opt)
{
	//get the bounding boxes

	if (!p_xy_view || !p_yz_view || !p_zx_view)
		return;

	bpos_x = qBound(V3DLONG(0), bpos_x, this->getXDim()-1);
	bpos_y = qBound(V3DLONG(0), bpos_y, this->getYDim()-1);
	bpos_z = qBound(V3DLONG(0), bpos_z, this->getZDim()-1);
	bpos_c = qBound(V3DLONG(0), bpos_c, this->getCDim()-1);

	epos_x = qBound(V3DLONG(0), epos_x, this->getXDim()-1);
	epos_y = qBound(V3DLONG(0), epos_y, this->getYDim()-1);
	epos_z = qBound(V3DLONG(0), epos_z, this->getZDim()-1);
	epos_c = qBound(V3DLONG(0), epos_c, this->getCDim()-1);

	if (bpos_x>epos_x || bpos_y>epos_y || bpos_z>epos_z)
	{
		v3d_msg("The parameters of crop() are invalid! No crop is done!\n");
		return;
	}

	// create new data, copy over, and delete the original data

	V3DLONG nsz0 = epos_x-bpos_x+1,
	nsz1 = epos_y-bpos_y+1,
	nsz2 = epos_z-bpos_z+1,
	nsz3 = epos_c-bpos_c+1;

    float **** ndata4d_float32 = 0;
    USHORTINT16 **** ndata4d_uint16 = 0;
    unsigned char **** ndata4d_uint8 = 0;

	unsigned char * ndata1d = 0;
	ImagePixelType ndatatype = this->getDatatype();

	V3DLONG i,j,k,c, i0,j0,k0,c0;

	try
	{
		switch ( this->getDatatype() )
		{
			case V3D_UINT8:
				ndata1d = new unsigned char [nsz0 * nsz1 * nsz2 * nsz3];
				if (!ndata1d)
				{
					v3d_msg("Cannot allocate memory for the cropped image. You may want to free memory by closing unnecessary programs.\n");
					return;
				}

				if (!new4dpointer_v3d(ndata4d_uint8, nsz0, nsz1, nsz2, nsz3, ndata1d))
				{
					this->setError(1);
					if (ndata1d) {delete ndata1d; ndata1d=0;}
					return;
				}
				for (c=bpos_c, c0=0;c<=epos_c;c++, c0++)
				{
					for (k=bpos_z, k0=0;k<=epos_z;k++, k0++)
					{
						for (j=bpos_y, j0=0;j<=epos_y;j++, j0++)
						{
							for (i=bpos_x, i0=0;i<=epos_x;i++, i0++)
							{
								ndata4d_uint8[c0][k0][j0][i0] = data4d_uint8[c][k][j][i];
							}
						}
					}
				}

				break;

			case V3D_UINT16:
				ndata1d = new unsigned char [nsz0 * nsz1 * nsz2 * nsz3 * sizeof(unsigned short int)];
				if (!ndata1d)
				{
					v3d_msg("Cannot allocate memory for the cropped image. You may want to free memory by closing unnecessary programs.\n");
					return;
				}

				if (!new4dpointer_v3d(ndata4d_uint16, nsz0, nsz1, nsz2, nsz3, ndata1d))
				{
					this->setError(1);
					if (ndata1d) {delete ndata1d; ndata1d=0;}
					return;
				}
				for (c=bpos_c, c0=0;c<=epos_c;c++, c0++)
				{
					for (k=bpos_z, k0=0;k<=epos_z;k++, k0++)
					{
						for (j=bpos_y, j0=0;j<=epos_y;j++, j0++)
						{
							for (i=bpos_x, i0=0;i<=epos_x;i++, i0++)
							{
								ndata4d_uint16[c0][k0][j0][i0] = data4d_uint16[c][k][j][i];
							}
						}
					}
				}

				break;

			case V3D_FLOAT32:
				ndata1d = new unsigned char [nsz0 * nsz1 * nsz2 * nsz3 * sizeof(float)];
				if (!ndata1d)
				{
					v3d_msg("Cannot allocate memory for the cropped image. You may want to free memory by closing unnecessary programs.\n");
					return;
				}

				if (!new4dpointer_v3d(ndata4d_float32, nsz0, nsz1, nsz2, nsz3, ndata1d))
				{
					this->setError(1);
					if (ndata1d) {delete ndata1d; ndata1d=0;}
					return;
				}
				for (c=bpos_c, c0=0;c<=epos_c;c++, c0++)
				{
					for (k=bpos_z, k0=0;k<=epos_z;k++, k0++)
					{
						for (j=bpos_y, j0=0;j<=epos_y;j++, j0++)
						{
							for (i=bpos_x, i0=0;i<=epos_x;i++, i0++)
							{
								ndata4d_float32[c0][k0][j0][i0] = data4d_float32[c][k][j][i];
							}
						}
					}
				}

				break;

			default:
				this->setError(1);
				if (ndata1d) {delete ndata1d; ndata1d=0;}
				return;
				//break;
		}
	}
	catch (...)
	{
		v3d_msg("Error happens in crop();\n");
		return;
	}

	cleanExistData_butKeepFileName();

	this->setRawDataPointer( ndata1d );

	data4d_float32 = ndata4d_float32;
	data4d_uint16 = ndata4d_uint16;
	data4d_uint8 = ndata4d_uint8;

	this->setDatatype( ndatatype );

	this->setXDim( nsz0 );
	this->setYDim( nsz1 );
	this->setZDim( nsz2 );
	this->setCDim( nsz3 );

	try
	{
		p_vmax = new double [this->getCDim()];
		p_vmin = new double [this->getCDim()];
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory.");
		this->setError(1);
		if (p_vmax) {delete []p_vmax; p_vmax=NULL;}
		if (p_vmin) {delete []p_vmin; p_vmin=NULL;}
		return;
	}

	switch ( this->getDatatype() )
	{
		case V3D_UINT8:
			data4d_virtual = (void ****)data4d_uint8;
			break;

		case V3D_UINT16:
			data4d_virtual = (void ****)data4d_uint16;
			break;

		case V3D_FLOAT32:
			data4d_virtual = (void ****)data4d_float32;
			break;

		default:
			this->setError(1);
			return;
			//break;
	}

	//update minmax

	if (!updateminmaxvalues())
	{
		v3d_msg("Fail to run successfully updateminmaxvalues() in proj_general_resampling()..\n", false);
		return;
	}

	//

	curFocusX = this->getXDim()/2; //-= bpos_x+1; //begin from first slices
	curFocusY = this->getYDim()/2; //-= bpos_y+1;
	curFocusZ = this->getZDim()/2; //-= bpos_z+1;

	//update the color display mode, as the number of channels could change
	if (p_mainWidget->getColorType()!=colorPseudoMaskColor && p_mainWidget->getColorType()!=colorHanchuanFlyBrainColor && p_mainWidget->getColorType()!=colorArnimFlyBrainColor) //otherwise does not need to change
	{
		if (getCDim()>=3)
			p_mainWidget->setColorType(colorRGB);
		else if (getCDim()==2)
			p_mainWidget->setColorType(colorRG);
		else //==1
			p_mainWidget->setColorType(colorRed2Gray);
	}

	//080828: update the landmarks
	if (landmark_crop_opt==1 || landmark_crop_opt==2)
	{
		LocationSimple tmp_pt(-1,-1,-1);
		for (V3DLONG i=listLandmarks.count()-1;i>=0;i--)
		{
			tmp_pt = listLandmarks.at(i);
			tmp_pt.x -= bpos_x;
			tmp_pt.y -= bpos_y;
			tmp_pt.z -= bpos_z;
			if (landmark_crop_opt==2) //in this case subtrast the min, but keep all landmarks
			{
				listLandmarks.replace(i, tmp_pt);
			}
			else //in this case, i.e. ==1, subtrast min and remove all that outside the bbox
			{
				if (tmp_pt.x<0 || tmp_pt.x>=(epos_x-bpos_x+1) ||
					tmp_pt.y<0 || tmp_pt.y>=(epos_y-bpos_y+1) ||
					tmp_pt.z<0 || tmp_pt.z>=(epos_z-bpos_z+1))
					listLandmarks.removeAt(i);
				else
					listLandmarks.replace(i, tmp_pt);
			}
		}
		listLocationRelationship.clear();
	}

	//update view

	p_xy_view->deleteROI();
	p_yz_view->deleteROI();
	p_zx_view->deleteROI();

	p_mainWidget->updateDataRelatedGUI();
    p_mainWidget->setWindowTitle_Suffix("_crop");

	return;
}

bool My4DImage::maskBW_roi_bbox(unsigned char tval, V3DLONG c_min, V3DLONG c_max, ImageMaskingCode my_maskcode, bool b_inside=true)
{
	if ( this->getDatatype() != V3D_UINT8 )
	{
		v3d_msg("only support UINT8 data in maskBW_roi_bbox();\n");
		return false;
	}

	//get the bounding boxes

	if (!p_xy_view || !p_yz_view || !p_zx_view)
		return false;

	QRect b_xy = p_xy_view->getRoiBoundingRect();
	QRect b_yz = p_yz_view->getRoiBoundingRect();
	QRect b_zx = p_zx_view->getRoiBoundingRect();

	V3DLONG bpos_x, bpos_y, bpos_z, bpos_c, epos_x, epos_y, epos_z, epos_c;

	bpos_x = qBound(V3DLONG(0), V3DLONG(qMax(b_xy.left(), b_zx.left())), this->getXDim()-1),
	bpos_y = qBound(V3DLONG(0), V3DLONG(qMax(b_xy.top(),  b_yz.top())), this->getYDim()-1),
	bpos_z = qBound(V3DLONG(0), V3DLONG(qMax(b_yz.left(), b_zx.top())), this->getZDim()-1),
	bpos_c = qBound(V3DLONG(0), c_min, this->getCDim()-1);

	epos_x = qBound(V3DLONG(0), V3DLONG(qMin(b_xy.right(), b_zx.right())), this->getXDim()-1),
	epos_y = qBound(V3DLONG(0), V3DLONG(qMin(b_xy.bottom(), b_yz.bottom())), this->getYDim()-1),
	epos_z = qBound(V3DLONG(0), V3DLONG(qMin(b_yz.right(), b_zx.bottom())), this->getZDim()-1),
	epos_c = qBound(V3DLONG(0), c_max, this->getCDim()-1);

	if (my_maskcode==IMC_XYZ_INTERSECT)
	{
		if (bpos_x>epos_x || bpos_y>epos_y || bpos_z>epos_z)
		{
			v3d_msg("The roi polygons in three views are not intersecting! No masking is done in maskBW_roi_bbox()!\n");
			return false;
		}
	}

	if (my_maskcode==IMC_XY) {bpos_z=0; epos_z=this->getZDim()-1;}
	else if (my_maskcode==IMC_YZ) {bpos_x=0; epos_x=this->getXDim()-1;}
	else if (my_maskcode==IMC_XZ) {bpos_y=0; epos_y=this->getYDim()-1;}

	//get the data 4d now
	unsigned char **** d4d = data4d_uint8;

	V3DLONG i,j,k,c;

	if (!b_inside) //add the b_inside part 090428
	{
		for (k=0;k<this->getZDim();k++)
			for (j=0;j<this->getYDim();j++)
				for (i=0;i<this->getXDim();i++)
				{
					if (k<bpos_z || k>epos_z || j<bpos_y || j>epos_y || i<bpos_x || i>epos_x)
					{
						for (c=bpos_c; c<=epos_c; c++)
						{
							d4d[c][k][j][i] = tval;
						}
					}
				}
	}
	else
	{
		if (my_maskcode==IMC_XYZ_INTERSECT || my_maskcode==IMC_XY || my_maskcode==IMC_YZ || my_maskcode==IMC_XZ)
		{
			for (c=bpos_c; c<=epos_c; c++)
				for (k=bpos_z; k<=epos_z; k++)
					for (j=bpos_y; j<=epos_y; j++)
						for (i=bpos_x; i<=epos_x; i++)
							d4d[c][k][j][i] = tval;
		}
		else if (my_maskcode==IMC_XYZ_UNION)
		{
			bool b_setFlag=false;
			for (k=0; k<this->getZDim(); k++)
			{
				b_setFlag = (k>=bpos_z && k<=epos_z) ? true : false;
				for (j=0; j<this->getYDim(); j++)
				{
					if (!b_setFlag) b_setFlag = (j>=bpos_y && j<=epos_y) ? true : false;
					for (i=0; i<this->getXDim(); i++)
					{
						if (!b_setFlag) b_setFlag = (i>=bpos_x && i<=epos_x) ? true : false;
						if (b_setFlag)
						{
							for (c=bpos_c; c<=epos_c; c++)
							{
								d4d[c][k][j][i] = tval;
							}
						}
					}
				}
			}
		}
		else
		{
			v3d_msg("Invalid ImageMaskingCode in maskBW_roi_bbox().\n");
			return false;
		}
	}

	updateViews();
	return true;
}

bool My4DImage::maskBW_roi(unsigned char tval, V3DLONG c_min, V3DLONG c_max, ImageMaskingCode my_maskcode, bool b_inside) //there is a bug in this program for seq masking which need to fixed later, phc 080424
{
	if ( this->getDatatype() != V3D_UINT8 )
	{
		v3d_msg("only support UINT8 data in maskBW_roi_bbox();\n");
		return false;
	}

	//get the bounding boxes

	if (!p_xy_view || !p_yz_view || !p_zx_view)
		return false;

	QPolygon pp_xy = p_xy_view->getRoi();
	QPolygon pp_yz = p_yz_view->getRoi();
	QPolygon pp_zx = p_zx_view->getRoi();

	QRect b_xy = p_xy_view->getRoiBoundingRect();
	QRect b_yz = p_yz_view->getRoiBoundingRect();
	QRect b_zx = p_zx_view->getRoiBoundingRect();

	V3DLONG bpos_x, bpos_y, bpos_z, bpos_c, epos_x, epos_y, epos_z, epos_c;

	bpos_x = qBound(V3DLONG(0), V3DLONG(qMax(b_xy.left(), b_zx.left())), this->getXDim()-1),
	bpos_y = qBound(V3DLONG(0), V3DLONG(qMax(b_xy.top(),  b_yz.top())), this->getYDim()-1),
	bpos_z = qBound(V3DLONG(0), V3DLONG(qMax(b_yz.left(), b_zx.top())), this->getZDim()-1),
	bpos_c = qBound(V3DLONG(0), c_min, this->getCDim()-1);

	epos_x = qBound(V3DLONG(0), V3DLONG(qMin(b_xy.right(), b_zx.right())), this->getXDim()-1),
	epos_y = qBound(V3DLONG(0), V3DLONG(qMin(b_xy.bottom(), b_yz.bottom())), this->getYDim()-1),
	epos_z = qBound(V3DLONG(0), V3DLONG(qMin(b_yz.right(), b_zx.bottom())), this->getZDim()-1),
	epos_c = qBound(V3DLONG(0), c_max, this->getCDim()-1);

	if (my_maskcode==IMC_XY) {bpos_z=0; epos_z=this->getZDim()-1;}
	else if (my_maskcode==IMC_YZ) {bpos_x=0; epos_x=this->getXDim()-1;}
	else if (my_maskcode==IMC_XZ) {bpos_y=0; epos_y=this->getYDim()-1;}

	//	if (bpos_x>epos_x) qSwap(bpos_x, epos_x);
	//	if (bpos_y>epos_y) qSwap(bpos_y, epos_y);
	//	if (bpos_z>epos_z) qSwap(bpos_z, epos_z);

	if (my_maskcode==IMC_XYZ_INTERSECT)
	{
		if (bpos_x>epos_x || bpos_y>epos_y || bpos_z>epos_z)
		{
			v3d_msg("The roi polygons in three views are not intersecting! No masking is done in maskBW_roi()!\n");
			return false;
		}
	}

	//get the data 4d now
	unsigned char **** d4d = data4d_uint8;

	V3DLONG i,j,k,c;
	//first handle rgn outside bounding box is necessary
	if (!b_inside)
	{
		for (k=0;k<this->getZDim();k++)
			for (j=0;j<this->getYDim();j++)
				for (i=0;i<this->getXDim();i++)
				{
					if (k<bpos_z || k>epos_z || j<bpos_y || j>epos_y || i<bpos_x || i>epos_x)
					{
						for (c=bpos_c; c<=epos_c; c++)
						{
							d4d[c][k][j][i] = tval;
						}
					}
				}
	}

	//now handle te rgn of bounding box
	if (my_maskcode==IMC_XY)
	{
		for (j=bpos_y; j<=epos_y; j++)
		{
			for (i=bpos_x; i<=epos_x; i++)
			{
				bool b_flag=pointInPolygon(i, j, pp_xy);
				b_flag = (b_inside)?b_flag:!b_flag;
				if (b_flag)
				{
					for (k=bpos_z; k<=epos_z; k++)
					{
						for (c=bpos_c; c<=epos_c; c++)
						{
							d4d[c][k][j][i] = tval;
						}
					}
				}
			}
		}
	}
	else if (my_maskcode==IMC_YZ)
	{
		for (k=bpos_z; k<=epos_z; k++)
		{
			for (j=bpos_y; j<=epos_y; j++)
			{
				bool b_flag=pointInPolygon(k, j, pp_yz);
				b_flag = (b_inside)?b_flag:!b_flag;
				if (b_flag)
				{
					for (i=bpos_x; i<=epos_x; i++)
					{
						for (c=bpos_c; c<=epos_c; c++)
						{
							d4d[c][k][j][i] = tval;
						}
					}
				}
			}
		}
	}
	else if (my_maskcode==IMC_XZ)
	{
		for (k=bpos_z; k<=epos_z; k++)
		{
			for (i=bpos_x; i<=epos_x; i++)
			{
				bool b_flag = pointInPolygon(i, k, pp_zx);
				b_flag = (b_inside)?b_flag:!b_flag;
				if (b_flag)
				{
					for (j=bpos_y; j<=epos_y; j++)
					{
						for (c=bpos_c; c<=epos_c; c++)
						{
							d4d[c][k][j][i] = tval;
						}
					}
				}
			}
		}
	}
	else if (my_maskcode==IMC_XYZ_INTERSECT)
	{
		for (k=bpos_z; k<=epos_z; k++)
		{
			for (j=bpos_y; j<=epos_y; j++)
			{
				for (i=bpos_x; i<=epos_x; i++)
				{
					bool b_flag = (pointInPolygon(i, j, pp_xy) && pointInPolygon(k, j, pp_yz) && pointInPolygon(i, k, pp_zx));
					b_flag = (b_inside)?b_flag:!b_flag;
					if (b_flag)
					{
						for (c=bpos_c; c<=epos_c; c++)
						{
							d4d[c][k][j][i] = tval;
						}
					}
				}
			}
		}
	}
	else if (my_maskcode==IMC_XYZ_UNION) //here the code is not correct, but I don't think correct it to the true union would make much sense for real processing
	{
		//it seems for union case I do not need to handle the b_inside case here, as that set will be empty if b_inside is false. 090428

		bool b_setFlag=false;
		for (k=0; k<this->getZDim(); k++)
		{
			b_setFlag = (k>=bpos_z && k<=epos_z) ? true : false;
			for (j=0; j<this->getYDim(); j++)
			{
				if (!b_setFlag) b_setFlag = (j>=bpos_y && j<=epos_y) ? true : false;
				for (i=0; i<this->getXDim(); i++)
				{
					if (!b_setFlag) b_setFlag = (i>=bpos_x && i<=epos_x) ? true : false;
					if (b_setFlag)
					{
						for (c=bpos_c; c<=epos_c; c++)
						{
							d4d[c][k][j][i] = tval;
						}
					}
				}
			}
		}
	}
	else
	{
		v3d_msg("Invalid ImageMaskingCode in maskBW_roi().\n");
		return false;
	}

	return true;
}

bool My4DImage::maskBW_channel(V3DLONG mask_channel_no)
{
	if ( this->getDatatype() != V3D_UINT8 )
	{
		v3d_msg("only support UINT8 data in maskBW_channel();\n");
		return false;
	}

	unsigned char tval=0;

	//get the bounding boxes

	V3DLONG bpos_x, bpos_y, bpos_z, bpos_c, epos_x, epos_y, epos_z, epos_c;

	bpos_x = 0,
	bpos_y = 0,
	bpos_z = 0,
	bpos_c = 0;

	epos_x = getXDim()-1;
	epos_y = getYDim()-1;
	epos_z = getZDim()-1;
	epos_c = getCDim()-1;

	//get the data 4d now
	unsigned char **** d4d = data4d_uint8;

	V3DLONG i,j,k,c;
	for (c=bpos_c; c<=epos_c; c++)
	{
		if (c==mask_channel_no)
			continue;

		for (j=bpos_y; j<=epos_y; j++)
		{
			for (i=bpos_x; i<=epos_x; i++)
			{
				for (k=bpos_z; k<=epos_z; k++)
				{
					if (d4d[mask_channel_no][k][j][i])
						d4d[c][k][j][i] = tval;
				}
			}
		}
	}

	updateViews();
	return true;
}



bool My4DImage::setNewImageData(unsigned char *ndata1d, V3DLONG nsz0, V3DLONG nsz1, V3DLONG nsz2, V3DLONG nsz3, ImagePixelType ndatatype, V3DLONG nszt, TimePackType tpk)
{
	if (!ndata1d || nsz0<=0 || nsz1<=0 || nsz2<=0 || nsz2<=0) return false;
	V3DLONG i;

	V3DLONG sz_time_old = this->getTDim();
	TimePackType timepacktype_old = this->getTimePackType();

	cleanExistData_butKeepFileName();

	this->setRawDataPointer( ndata1d );
	this->setDatatype( ndatatype );
	this->setXDim( nsz0 );
	this->setYDim( nsz1 );
	this->setZDim( nsz2 );
	this->setCDim( nsz3 );

	if (nszt<0)
	{
		this->setTDim( sz_time_old );
		this->setTimePackType( timepacktype_old );
	}
	else {
		this->setTDim( nszt );
		this->setTimePackType( tpk );
	}

	try
	{
		p_vmax = new double [this->getCDim()];
		p_vmin = new double [this->getCDim()];
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory.\n");
		this->setError(1);
		if (p_vmax) {delete []p_vmax; p_vmax=NULL;}
		if (p_vmin) {delete []p_vmin; p_vmin=NULL;}
		return false;
	}

	V3DLONG tmppos;
	V3DLONG channelPageSize = (V3DLONG)this->getXDim()*this->getYDim()*this->getZDim();

	if (!updateminmaxvalues())
	{
		v3d_msg("Fail to run successfully updateminmaxvalues() in setNewImageData()..\n", false);
		return false;
	}

	switch ( this->getDatatype() )
	{
		case V3D_UINT8:
			if (!new4dpointer_v3d(data4d_uint8, this->getXDim(), this->getYDim(), this->getZDim(), this->getCDim(), this->getRawData() ))
			{
				this->setError(1);
				this->deleteRawDataAndSetPointerToNull();
				return false;
			}
			data4d_virtual = (void ****)data4d_uint8;

			break;

		case V3D_UINT16:
			if (!new4dpointer_v3d(data4d_uint16, this->getXDim(), this->getYDim(), this->getZDim(), this->getCDim(), this->getRawData() ))
			{
				this->setError(1);
				this->deleteRawDataAndSetPointerToNull();
				return false;
			}
			data4d_virtual = (void ****)data4d_uint16;

			createColorMap(int(p_vmax[0])+1000); //add 1000 for a safe pool for cell editing.
			printf("set the color map max=%d\n", int(p_vmax[0]));

			break;

		case V3D_FLOAT32:
			if (!new4dpointer_v3d(data4d_float32, this->getXDim(), this->getYDim(), this->getZDim(), this->getCDim(), this->getRawData() ))
			{
				this->setError(1);
				this->deleteRawDataAndSetPointerToNull();
				return false;
			}
			data4d_virtual = (void ****)data4d_float32;

			break;

		default:
			this->setError(1);
			return false;
			//break;
	}

	setupDefaultColorChannelMapping(); //20100811. PHC

	curFocusX = this->getXDim()/2; //-= bpos_x+1; //begin from first slices
	curFocusY = this->getYDim()/2; //-= bpos_y+1;
	curFocusZ = this->getZDim()/2; //-= bpos_z+1;

	//update view

	if (!p_xy_view || !p_yz_view || !p_zx_view)
	{
		p_mainWidget->updateDataRelatedGUI();
	}
	else
	{
		p_xy_view->deleteROI();
		p_yz_view->deleteROI();
		p_zx_view->deleteROI();
	}

	p_mainWidget->setCTypeBasedOnImageData(); //the colormap info should be reset based on updated data. 2010-08-01

	p_mainWidget->updateDataRelatedGUI();
    p_mainWidget->setWindowTitle_Suffix("_processed");

	updateViews();
	return true;
}

bool My4DImage::rotate(ImagePlaneDisplayType ptype, const Options_Rotate & r_opt)
{
	if (!data4d_uint8 && !data4d_uint16 && !data4d_float32)
	{
		v3d_msg("None of the 4d data pointers is valid in rotate().");  return false;
	}

	V3DLONG insz[4]; insz[0]=this->getXDim(); insz[1]=this->getYDim(); insz[2]=this->getZDim(); insz[3]=this->getCDim();
	unsigned char * outvol1d=0;
	V3DLONG *outsz=0;
	bool b_res=false;

	switch (ptype)
	{
		case imgPlaneX:
			if (data4d_uint8)
				b_res = rotate_inPlaneX(this->getRawData(), insz, r_opt, outvol1d, outsz);
			else if (data4d_uint16)
			{
				unsigned short int * tmpout=0;
				b_res = rotate_inPlaneX((unsigned short int *)(this->getRawData()), insz, r_opt, tmpout, outsz);
				outvol1d = (unsigned char *)tmpout;
			}
			else if (data4d_float32)
			{
				float * tmpout=0;
				b_res = rotate_inPlaneX((float *)(this->getRawData()), insz, r_opt, tmpout, outsz);
				outvol1d = (unsigned char *)tmpout;
			}
			break;

		case imgPlaneY:
			if (data4d_uint8)
				b_res = rotate_inPlaneY(this->getRawData(), insz, r_opt, outvol1d, outsz);
			else if (data4d_uint16)
			{
				unsigned short int * tmpout=0;
				b_res = rotate_inPlaneY((unsigned short int *)(this->getRawData()), insz, r_opt, tmpout, outsz);
				outvol1d = (unsigned char *)tmpout;
			}
			else if (data4d_float32)
			{
				float * tmpout=0;
				b_res = rotate_inPlaneY((float *)(this->getRawData()), insz, r_opt, tmpout, outsz);
				outvol1d = (unsigned char *)tmpout;
			}
			break;

		case imgPlaneZ:
			if (data4d_uint8)
				b_res = rotate_inPlaneZ(this->getRawData(), insz, r_opt, outvol1d, outsz);
			else if (data4d_uint16)
			{
				unsigned short int * tmpout=0;
				b_res = rotate_inPlaneZ((unsigned short int *)(this->getRawData()), insz, r_opt, tmpout, outsz);
				outvol1d = (unsigned char *)tmpout;
			}
			else if (data4d_float32)
			{
				float * tmpout=0;
				b_res = rotate_inPlaneZ((float *)(this->getRawData()), insz, r_opt, tmpout, outsz);
				outvol1d = (unsigned char *)tmpout;
			}
			break;

		default:
			return false;
	}

	//assign the rotated image to the current image and update the pointers
	if (b_res)
    {
		setNewImageData(outvol1d, outsz[0], outsz[1], outsz[2], outsz[3], this->getDatatype());
    }
	else
	{
		v3d_msg("The rotate operation fails.\n");
		return false;
	}
	if (outsz) {delete []outsz; outsz=0;}

	//update view
	updateViews();

	return true;
}


bool My4DImage::flip(AxisCode my_axiscode)
{
	if (!valid()) {return false;}
	V3DLONG i,j,k,c;

	switch ( this->getDatatype() )
	{
		case V3D_UINT8:
			switch (my_axiscode)
		{
			case axis_x:
			{
				V3DLONG hsz0=floor((double)(this->getXDim()-1)/2.0); if (hsz0*2<this->getXDim()-1) hsz0+=1;
				for (c=0;c<this->getCDim();c++)
					for (k=0;k<this->getZDim();k++)
						for (j=0;j<this->getYDim();j++)
							for (i=0;i<hsz0;i++)
							{
								unsigned char tmpv = data4d_uint8[c][k][j][this->getXDim()-i-1];
								data4d_uint8[c][k][j][this->getXDim()-1-i] = data4d_uint8[c][k][j][i];
								data4d_uint8[c][k][j][i] = tmpv;
							}
			}
				break;
			case axis_y:
			{
				V3DLONG hsz1=floor((double)(this->getYDim()-1)/2.0); if (hsz1*2<this->getYDim()-1) hsz1+=1;
				qDebug("%d %d",this->getYDim(),hsz1);
				for (c=0;c<this->getCDim();c++)
					for (k=0;k<this->getZDim();k++)
						for (j=0;j<hsz1;j++)
							for (i=0;i<this->getXDim();i++)
							{
								unsigned char tmpv = data4d_uint8[c][k][this->getYDim()-j-1][i];
								data4d_uint8[c][k][this->getYDim()-1-j][i] = data4d_uint8[c][k][j][i];
								data4d_uint8[c][k][j][i] = tmpv;
							}
			}
				break;
			case axis_z:
			{
				V3DLONG hsz2=floor((double)(this->getZDim()-1)/2.0); if (hsz2*2<this->getZDim()-1) hsz2+=1;
				for (c=0;c<this->getCDim();c++)
					for (k=0;k<hsz2;k++)
						for (j=0;j<this->getYDim();j++)
							for (i=0;i<this->getXDim();i++)
							{
								unsigned char tmpv = data4d_uint8[c][this->getZDim()-k-1][j][i];
								data4d_uint8[c][this->getZDim()-1-k][j][i] = data4d_uint8[c][k][j][i];
								data4d_uint8[c][k][j][i] = tmpv;
							}
			}
				break;
			case axis_c:
			{
				V3DLONG hsz3=floor((double)(this->getCDim()-1)/2.0); if (hsz3*2<this->getCDim()-1) hsz3+=1;
				for (c=0;c<hsz3;c++)
				{
					for (k=0;k<this->getZDim();k++)
						for (j=0;j<this->getYDim();j++)
							for (i=0;i<this->getXDim();i++)
							{
								unsigned char tmpv = data4d_uint8[this->getCDim()-c-1][k][j][i];
								data4d_uint8[this->getCDim()-c-1][k][j][i] = data4d_uint8[c][k][j][i];
								data4d_uint8[c][k][j][i] = tmpv;
							}

					double tmpc;
					tmpc = p_vmax[this->getCDim()-c-1]; p_vmax[this->getCDim()-c-1] = p_vmax[c]; p_vmax[c] = tmpc;
					tmpc = p_vmin[this->getCDim()-c-1]; p_vmin[this->getCDim()-c-1] = p_vmin[c]; p_vmin[c] = tmpc;
				}
			}
				break;
			default:
				break;
		}
			break;

		case V3D_UINT16:
			switch (my_axiscode)
		{
			case axis_x:
			{
				V3DLONG hsz0=floor((double)(this->getXDim()-1)/2.0); if (hsz0*2<this->getXDim()-1) hsz0+=1;
				for (c=0;c<this->getCDim();c++)
					for (k=0;k<this->getZDim();k++)
						for (j=0;j<this->getYDim();j++)
							for (i=0;i<hsz0;i++)
							{
								unsigned short int tmpv = data4d_uint16[c][k][j][this->getXDim()-i-1];
								data4d_uint16[c][k][j][this->getXDim()-1-i] = data4d_uint16[c][k][j][i];
								data4d_uint16[c][k][j][i] = tmpv;
							}
			}
				break;
			case axis_y:
			{
				V3DLONG hsz1=floor((double)(this->getYDim()-1)/2.0); if (hsz1*2<this->getYDim()-1) hsz1+=1;
				for (c=0;c<this->getCDim();c++)
					for (k=0;k<this->getZDim();k++)
						for (j=0;j<hsz1;j++)
							for (i=0;i<this->getXDim();i++)
							{
								unsigned short int tmpv = data4d_uint16[c][k][this->getYDim()-j-1][i];
								data4d_uint16[c][k][this->getYDim()-1-j][i] = data4d_uint16[c][k][j][i];
								data4d_uint16[c][k][j][i] = tmpv;
							}
			}
				break;
			case axis_z:
			{
				V3DLONG hsz2=floor((double)(this->getZDim()-1)/2.0); if (hsz2*2<this->getZDim()-1) hsz2+=1;
				for (c=0;c<this->getCDim();c++)
					for (k=0;k<hsz2;k++)
						for (j=0;j<this->getYDim();j++)
							for (i=0;i<this->getXDim();i++)
							{
								unsigned short int tmpv = data4d_uint16[c][this->getZDim()-k-1][j][i];
								data4d_uint16[c][this->getZDim()-1-k][j][i] = data4d_uint16[c][k][j][i];
								data4d_uint16[c][k][j][i] = tmpv;
							}
			}
				break;
			case axis_c:
			{
				V3DLONG hsz3=floor((double)(this->getCDim()-1)/2.0); if (hsz3*2<this->getCDim()-1) hsz3+=1;
				for (c=0;c<hsz3;c++)
				{
					for (k=0;k<this->getZDim();k++)
						for (j=0;j<this->getYDim();j++)
							for (i=0;i<this->getXDim();i++)
							{
								unsigned short int tmpv = data4d_uint16[this->getCDim()-c-1][k][j][i];
								data4d_uint16[this->getCDim()-c-1][k][j][i] = data4d_uint16[c][k][j][i];
								data4d_uint16[c][k][j][i] = tmpv;
							}

					double tmpc;
					tmpc = p_vmax[this->getCDim()-c-1]; p_vmax[this->getCDim()-c-1] = p_vmax[c]; p_vmax[c] = tmpc;
					tmpc = p_vmin[this->getCDim()-c-1]; p_vmin[this->getCDim()-c-1] = p_vmin[c]; p_vmin[c] = tmpc;
				}
			}
				break;
			default:
				break;
		}
			break;


		case V3D_FLOAT32:
			switch (my_axiscode)
		{
			case axis_x:
			{
				V3DLONG hsz0=floor((double)(this->getXDim()-1)/2.0); if (hsz0*2<this->getXDim()-1) hsz0+=1;
				for (c=0;c<this->getCDim();c++)
					for (k=0;k<this->getZDim();k++)
						for (j=0;j<this->getYDim();j++)
							for (i=0;i<hsz0;i++)
							{
								float tmpv = data4d_float32[c][k][j][this->getXDim()-i-1];
								data4d_float32[c][k][j][this->getXDim()-1-i] = data4d_float32[c][k][j][i];
								data4d_float32[c][k][j][i] = tmpv;
							}
			}
				break;
			case axis_y:
			{
				V3DLONG hsz1=floor((double)(this->getYDim()-1)/2.0); if (hsz1*2<this->getYDim()-1) hsz1+=1;
				for (c=0;c<this->getCDim();c++)
					for (k=0;k<this->getZDim();k++)
						for (j=0;j<hsz1;j++)
							for (i=0;i<this->getXDim();i++)
							{
								float tmpv = data4d_float32[c][k][this->getYDim()-j-1][i];
								data4d_float32[c][k][this->getYDim()-1-j][i] = data4d_float32[c][k][j][i];
								data4d_float32[c][k][j][i] = tmpv;
							}
			}
				break;
			case axis_z:
			{
				V3DLONG hsz2=floor((double)(this->getZDim()-1)/2.0); if (hsz2*2<this->getZDim()-1) hsz2+=1;
				for (c=0;c<this->getCDim();c++)
					for (k=0;k<hsz2;k++)
						for (j=0;j<this->getYDim();j++)
							for (i=0;i<this->getXDim();i++)
							{
								float tmpv = data4d_float32[c][this->getZDim()-k-1][j][i];
								data4d_float32[c][this->getZDim()-1-k][j][i] = data4d_float32[c][k][j][i];
								data4d_float32[c][k][j][i] = tmpv;
							}
			}
				break;
			case axis_c:
			{
				V3DLONG hsz3=floor((double)(this->getCDim()-1)/2.0); if (hsz3*2<this->getCDim()-1) hsz3+=1;
				for (c=0;c<hsz3;c++)
				{
					for (k=0;k<this->getZDim();k++)
						for (j=0;j<this->getYDim();j++)
							for (i=0;i<this->getXDim();i++)
							{
								float tmpv = data4d_float32[this->getCDim()-c-1][k][j][i];
								data4d_float32[this->getCDim()-c-1][k][j][i] = data4d_float32[c][k][j][i];
								data4d_float32[c][k][j][i] = tmpv;
							}

					double tmpc;
					tmpc = p_vmax[this->getCDim()-c-1]; p_vmax[this->getCDim()-c-1] = p_vmax[c]; p_vmax[c] = tmpc;
					tmpc = p_vmin[this->getCDim()-c-1]; p_vmin[this->getCDim()-c-1] = p_vmin[c]; p_vmin[c] = tmpc;
				}
			}
				break;
			default:
				break;
		}
			break;

		default:
			this->setError(1);
			return false;
			//break;
	}

	//update view
	updateViews();
	return true;
}

bool My4DImage::invertcolor(int channo) //channo < 0 will invert all channels. Only works for uint8
{
	if ( this->getDatatype() !=V3D_UINT8)
	{
		v3d_msg("Now the color inversion program only supports 8bit data. Check your data first.\n");
		return false;
	}
	if (channo>=getCDim())
	{
		v3d_msg("Invalid chan parameter in invertcolor();\n");
		return false;
	}

	if (channo>=0)
	{
		V3DLONG chanbytes = getTotalUnitNumberPerChannel();
		unsigned char *p_end = getRawData()+(channo+1)*chanbytes, *p=0;
		for (p=getRawData()+channo*chanbytes;p<p_end; p++) {*p = 255- *p; }
	}
	else
	{
		V3DLONG chanbytes = getTotalUnitNumber();
		unsigned char *p_end = getRawData()+chanbytes, *p=0;
		for (p=getRawData();p<p_end; p++) {*p = 255- *p; }
	}

	updateViews();
	return true;
}

bool My4DImage::scaleintensity(int channo, double lower_th, double higher_th, double target_min, double target_max) //map the value linear from [lower_th, higher_th] to [target_min, target_max].
{
	if (channo>=getCDim())
	{
		v3d_msg("Invalid chan parameter in scaleintensity();\n");
		return false;
	}

	double t;
	if (lower_th>higher_th) {t=lower_th; lower_th=higher_th; higher_th=t;}
	if (target_min>target_max) {t=target_min; target_min=target_max; target_max=t;}

	double rate = (higher_th==lower_th) ? 1 : (target_max-target_min)/(higher_th-lower_th); //if the two th vals equal, then later-on t-lower_th will be 0 anyway

	V3DLONG i,j,k,c;

	V3DLONG channelPageSize = getTotalUnitNumberPerChannel();
	switch ( this->getDatatype() )
	{
		case V3D_UINT8:
			for (c=0;c<this->getCDim();c++)
			{
				if (channo>=0 && c!=channo)
					continue;
				for (k=0;k<this->getZDim();k++)
					for (j=0;j<this->getYDim();j++)
						for (i=0;i<this->getXDim();i++)
						{
							t = data4d_uint8[c][k][j][i];
							if (t>higher_th) t=higher_th;
							else if (t<lower_th) t=lower_th;
							data4d_uint8[c][k][j][i] = (unsigned char)((t - lower_th)*rate + target_min);
						}
			}

			break;

		case V3D_UINT16:
			for (c=0;c<this->getCDim();c++)
			{
				if (channo>=0 && c!=channo)
					continue;
				for (k=0;k<this->getZDim();k++)
					for (j=0;j<this->getYDim();j++)
						for (i=0;i<this->getXDim();i++)
						{
							t = data4d_uint16[c][k][j][i];
							if (t>higher_th) t=higher_th;
							else if (t<lower_th) t=lower_th;
							data4d_uint16[c][k][j][i] = (USHORTINT16)((t - lower_th)*rate + target_min);
						}
			}
			break;

		case V3D_FLOAT32:
			for (c=0;c<this->getCDim();c++)
			{
				if (channo>=0 && c!=channo)
					continue;
				for (k=0;k<this->getZDim();k++)
					for (j=0;j<this->getYDim();j++)
						for (i=0;i<this->getXDim();i++)
						{
							t = data4d_float32[c][k][j][i];
							if (t>higher_th) t=higher_th;
							else if (t<lower_th) t=lower_th;
							data4d_float32[c][k][j][i] = (t - lower_th)*rate + target_min;
						}
			}
			break;
		default:
			v3d_msg("invalid datatype in scaleintensity();\n");
			return false;
	}

	//update min and max
	if (!updateminmaxvalues())
	{
		v3d_msg("Fail to run successfully updateminmaxvalues() in scaleintensity()..\n", false);
		return false;
	}


	updateViews();
	return true;
}

bool My4DImage::getFlagImgValScaleDisplay()
{
	if (!p_mainWidget)
		return false;
	else
		return p_mainWidget->getFlagImgValScaleDisplay();
}

bool My4DImage::thresholdintensity(int channo, double th) //anything < th will be 0, others unchanged
{
	if (channo>=getCDim())
	{
		v3d_msg("Invalid chan parameter in thresholdintensity();\n");
		return false;
	}

	V3DLONG i,j,k,c;

	V3DLONG channelPageSize = getTotalUnitNumberPerChannel();
	switch ( this->getDatatype() )
	{
		case V3D_UINT8:
			for (c=0;c<this->getCDim();c++)
			{
				if (channo>=0 && c!=channo)
					continue;
				for (k=0;k<this->getZDim();k++)
					for (j=0;j<this->getYDim();j++)
						for (i=0;i<this->getXDim();i++)
						{
							if (data4d_uint8[c][k][j][i]<th) data4d_uint8[c][k][j][i]=0;
						}
			}

			break;

		case V3D_UINT16:
			for (c=0;c<this->getCDim();c++)
			{
				if (channo>=0 && c!=channo)
					continue;
				for (k=0;k<this->getZDim();k++)
					for (j=0;j<this->getYDim();j++)
						for (i=0;i<this->getXDim();i++)
						{
							if (data4d_uint16[c][k][j][i]<th) data4d_uint16[c][k][j][i]=0;
						}
			}
			break;

		case V3D_FLOAT32:
			for (c=0;c<this->getCDim();c++)
			{
				if (channo>=0 && c!=channo)
					continue;
				for (k=0;k<this->getZDim();k++)
					for (j=0;j<this->getYDim();j++)
						for (i=0;i<this->getXDim();i++)
						{
							if (data4d_float32[c][k][j][i]<th) data4d_float32[c][k][j][i]=p_vmin[c];
						}
			}
			break;
		default:
			v3d_msg("invalid datatype in scaleintensity();\n");
			return false;
	}

	//update min and max
	if (!updateminmaxvalues())
	{
		v3d_msg("Fail to run successfully updateminmaxvalues() in scaleintensity()..\n", false);
		return false;
	}


	updateViews();
	return true;
}

bool My4DImage::binarizeintensity(int channo, double th) //anything < th will be 0, others will be 1
{
	if (channo>=getCDim())
	{
		v3d_msg("Invalid chan parameter in invertcolor();\n");
		return false;
	}

	V3DLONG i,j,k,c;

	V3DLONG channelPageSize = getTotalUnitNumberPerChannel();
	switch ( this->getDatatype() )
	{
		case V3D_UINT8:
			for (c=0;c<this->getCDim();c++)
			{
				if (channo>=0 && c!=channo)
					continue;
				for (k=0;k<this->getZDim();k++)
					for (j=0;j<this->getYDim();j++)
						for (i=0;i<this->getXDim();i++)
						{
							data4d_uint8[c][k][j][i] = (data4d_uint8[c][k][j][i]<th)?0:1;
						}
			}

			break;

		case V3D_UINT16:
			for (c=0;c<this->getCDim();c++)
			{
				if (channo>=0 && c!=channo)
					continue;
				for (k=0;k<this->getZDim();k++)
					for (j=0;j<this->getYDim();j++)
						for (i=0;i<this->getXDim();i++)
						{
							data4d_uint16[c][k][j][i] = (data4d_uint16[c][k][j][i]<th)?0:1;
						}
			}
			break;

		case V3D_FLOAT32:
			for (c=0;c<this->getCDim();c++)
			{
				if (channo>=0 && c!=channo)
					continue;
				for (k=0;k<this->getZDim();k++)
					for (j=0;j<this->getYDim();j++)
						for (i=0;i<this->getXDim();i++)
						{
							data4d_float32[c][k][j][i] = (data4d_float32[c][k][j][i]<th)?0:1;
						}
			}
			break;
		default:
			v3d_msg("invalid datatype in scaleintensity();\n");
			return false;
	}

	//update min and max
	if (!updateminmaxvalues())
	{
		v3d_msg("Fail to run successfully updateminmaxvalues() in scaleintensity()..\n", false);
		return false;
	}


	updateViews();
	return true;
}

void My4DImage::setText2FocusPointFeatureWidget()
{
	p_focusPointFeatureWidget->setText2FocusPointFeatureWidget(); // added Dec 28, 2010 by YuY
}

QString My4DImage::setFocusFeatureViewText()
{
	if (p_focusPointFeatureWidget)
	{
		// set the focus pixel information

		QString tmps = "Voxel type: ";
		switch (getDatatype())
		{
			case V3D_UINT8: tmps.append("UINT8"); break;
			case V3D_UINT16: tmps.append("UINT16"); break;
			case V3D_FLOAT32: tmps.append("FLOAT32"); break;
			default: tmps.append("Unrecognized type. "); return tmps; break;
		}
		tmps.append("; ");
		if (p_mainWidget)
		{
			tmps.append(QString("Tri-view zoom: %1<br>").arg(p_mainWidget->disp_zoom));
		}

		tmps.append(QString("Focus: (%1, %2, %3)").arg(curFocusX+1).arg(curFocusY+1).arg(curFocusZ+1));

		QString v1,v2,v3;
		tmps.append(" RGB = (");
		switch ( this->getDatatype() )
		{
			case V3D_UINT8:
			{
				int tmpr=0, tmpg=0, tmpb=0;

				unsigned char ****ptmp = (unsigned char ****)data4d_virtual;
				if (getCDim()>=3)
					tmpb = int(ptmp[2][curFocusZ][curFocusY][curFocusX]);
				if (getCDim()>=2)
					tmpg = int(ptmp[1][curFocusZ][curFocusY][curFocusX]);
				if (getCDim()>=1)
					tmpr = int(ptmp[0][curFocusZ][curFocusY][curFocusX]);
				tmps.append(QString("%1,%2,%3)<br>").arg(tmpr).arg(tmpg).arg(tmpb));
			}
				break;

			case V3D_UINT16:
			{
				int tmpr=0, tmpg=0, tmpb=0;

				USHORTINT16 ****ptmp = (USHORTINT16 ****)data4d_virtual;
				if (getCDim()>=3)
					tmpb = int(ptmp[2][curFocusZ][curFocusY][curFocusX]);
				if (getCDim()>=2)
					tmpg = int(ptmp[1][curFocusZ][curFocusY][curFocusX]);
				if (getCDim()>=1)
					tmpr = int(ptmp[0][curFocusZ][curFocusY][curFocusX]);
				tmps.append(QString("%1,%2,%3)<br>").arg(tmpr).arg(tmpg).arg(tmpb));
			}
				break;

			case V3D_FLOAT32:
			{
				float tmpr=0, tmpg=0, tmpb=0;
				float ****ptmp = (float ****)data4d_virtual;
				if (getCDim()>=3)
					tmpb = (ptmp[2][curFocusZ][curFocusY][curFocusX]);
				if (getCDim()>=2)
					tmpg = (ptmp[1][curFocusZ][curFocusY][curFocusX]);
				if (getCDim()>=1)
					tmpr = (ptmp[0][curFocusZ][curFocusY][curFocusX]);
				tmps.append(QString("%1,%2,%3)<br>").arg(tmpr).arg(tmpg).arg(tmpb));
			}
				break;
		}

		//display the stack min/max info
		tmps.append("Channel min/max: ");
		V3DLONG i;
		for (i=0;i<getCDim();i++)
		{
			tmps.append(QString("C%1 [min=%2, max=%3]; ").arg(i+1).arg(p_vmin[i]).arg(p_vmax[i]));
		}

		//display the defined location info

		tmps.append("<br>Defined marker location: <br>");
		int tmpx,tmpy,tmpz;
		QString tmpc;
		LocationSimple tmpLocation(0,0,0);
		//printf("count: %d\n", listUsefulLocation.count());
		for (i=0;i<listLandmarks.count();i++)
		{
			tmpLocation = listLandmarks.at(i);
			tmpLocation.getCoord(tmpx,tmpy,tmpz);

			tmps.append("(");
			tmpc.setNum(i+1); tmpc.append(") ");
			tmps.append(tmpc);
			tmpc.setNum(tmpx); tmpc.append(",");
			tmps.append(tmpc);
			tmpc.setNum(tmpy); tmpc.append(",");
			tmps.append(tmpc);
			tmpc.setNum(tmpz);
			if ((i+1)%5==0) tmpc.append("<br>");
			else tmpc.append("; ");
			tmps.append(tmpc);


			////commented on 080604
			//	  tmpc.append(": [");
			//
			//	  tmpc.setNum(tmpLocation.getPixVal()); tmpc.append(", ");
			//      tmps.append(tmpc);
			//	  tmpc.setNum(tmpLocation.getAve()); tmpc.append(", ");
			//      tmps.append(tmpc);
			//	  tmpc.setNum(tmpLocation.getSdev()); tmpc.append(", ");
			//      tmps.append(tmpc);
			//	  tmpc.setNum(tmpLocation.getSkew()); tmpc.append(", ");
			//      tmps.append(tmpc);
			//	  tmpc.setNum(tmpLocation.getCurt()); tmpc.append("]\n");
			//      tmps.append(tmpc);
		}
		//p_focusPointFeatureWidget->setText(tmps); //can also be setPlainText() or SetHtml()
		p_focusPointFeatureWidget->setFocusFeatureViewTextContent(tmps);
		//qDebug()<<"emit focusFeatureViewTextUpdated ... ..."<<p_focusPointFeatureWidget->getFocusFeatureViewTextContent();
		emit focusFeatureViewTextUpdated();
		//p_focusPointFeatureWidget->append(tmps);

		//update
		//p_focusPointFeatureWidget->update();
		return tmps;
	}
	else
		return QString("The p_focusPointFeatureWidget pointer is invalid. You should NOT see this message at all. Please check with V3D development team.");
}

void My4DImage::cleanExistData_butKeepFileName()
{
	V3DLONG i;
	char oldFileName[1024];
	const char * srcFileName = this->getFileName();
	for (i=0;i<1024;i++)
	{
		oldFileName[i] = srcFileName[i];
		if (srcFileName[i]=='\0') break;
	}

	this->cleanExistData();

	this->setFileName( oldFileName );
}

void My4DImage::cleanExistData()
{
	cleanExistData_only4Dpointers();
	Image4DSimple::cleanExistData();
}

void My4DImage::cleanExistData_only4Dpointers()
{
	if (data4d_uint8 || data4d_uint16 || data4d_float32 || data4d_virtual) //080416. Only try to free space and set up b_error flag if applicable
	{
		switch ( this->getDatatype() )
		{
			case V3D_UINT8:
				delete4dpointer_v3d(data4d_uint8, this->getXDim(), this->getYDim(), this->getZDim(), this->getCDim());
				data4d_virtual = 0;
				break;

			case V3D_UINT16:
				delete4dpointer_v3d(data4d_uint16, this->getXDim(), this->getYDim(), this->getZDim(), this->getCDim());
				data4d_virtual = 0;
				break;

			case V3D_FLOAT32:
				delete4dpointer_v3d(data4d_float32, this->getXDim(), this->getYDim(), this->getZDim(), this->getCDim());
				data4d_virtual = 0;
				break;

			default:
				this->setError(1);
				return;
				//break;
		}
	}

	if (p_vmax) {delete []p_vmax; p_vmax = NULL;}
	if (p_vmin) {delete []p_vmin; p_vmin = NULL;}
}


void My4DImage::recordFocusProperty(PxLocationUsefulness t) //note that also need a function to delete a location (of course first retrieve it) from the list
{
	LocationSimple l = LocationSimple(curFocusX, curFocusY, curFocusZ);

	int b_exist=0;
	for (V3DLONG i=0;i<listLandmarks.count();i++)
	{
		int tmpx,tmpy,tmpz;
		LocationSimple ltmp = listLandmarks.at(i);
		ltmp.getCoord(tmpx,tmpy,tmpz);
		if (tmpx==curFocusX && tmpy==curFocusY && tmpz==curFocusZ)
		{
			b_exist=1;
			if (ltmp.howUseful()==t)
			{
				break; //do not add
			}
			else
			{
				l = listLandmarks.at(i);
				l.inputProperty = t;
				listLandmarks.replace(i, l);
			}
		}
	}

	if (b_exist==0)
	{
		computePointNeighborMoment(l, 1); //also compute the property
		l.inputProperty = t;
		listLandmarks.append(l);

		b_proj_worm_mst_diameter_set = false; //080318: whenever a new landmark is added, reset the flag of MST diameter existency
	}

	setFocusFeatureViewText();
}


void My4DImage::computePointNeighborMoment(int x, int y, int z, int c, double & curptval, double & ave, double & sdev, double & skew, double & curt)
{
	if (c<0 || c>= getCDim() || x<0 || x>=getXDim() || y<0 || y>=getYDim() || z<0 || z>=getZDim())
	{
		v3d_msg("Illegal operation: out of boundary.\n");
		return; //do nothing
	}
	double adev, var; //unused variables but needed to call moment()

	V3DLONG xr = 5, yr = 5, zr = 3;
	V3DLONG x0 = qMax(x-xr, V3DLONG(0)), x1 = qMin(x+xr, V3DLONG(getXDim()-1));
	V3DLONG y0 = qMax(y-yr, V3DLONG(0)), y1 = qMin(y+yr, V3DLONG(getYDim()-1));
	V3DLONG z0 = qMax(z-zr, V3DLONG(0)), z1 = qMin(z+zr, V3DLONG(getZDim()-1));

	V3DLONG i,j,k, nn;

	//  unsigned char *p1dtmp_uint8 = 0;
	//  USHORTINT16 *p1dtmp_uint16 = 0;
	float *p1dtmp_float32 = 0;

	switch( this->getDatatype() )
	{
		case V3D_UINT8:
			//	  p1dtmp_uint8 = new unsigned char [(x1-x0+1)*(y1-y0+1)*(z1-z0+1)];
			p1dtmp_float32 = new float [(x1-x0+1)*(y1-y0+1)*(z1-z0+1)];
			nn=0;
			for (k=z0;k<=z1;k++)
			{
				for (j=y0;j<=y1;j++)
				{
					for (i=x0;i<=x1;i++)
					{
						//		    p1dtmp_uint8[nn++] = data4d_uint8[c][k][j][i];
						p1dtmp_float32[nn] = float(data4d_uint8[c][k][j][i]);
						//			printf("%5.2f, ", p1dtmp_float32[nn]);
						nn++;
					}
				}
			}
			//	  moment(p1dtmp_uint8, nn, ave, adev, sdev, var, skew, curt);
			//	  delete []p1dtmp_uint8; p1dtmp_uint8=0;
			moment(p1dtmp_float32, nn, ave, adev, sdev, var, skew, curt);
			delete []p1dtmp_float32; p1dtmp_float32=0;
			curptval = double(data4d_uint8[c][z][y][x]);
			//	  printf("uint8 enter [%d] %d %d %5.3f, %5.3f, %5.3f, %5.3f\n", (x1-x0+1)*(y1-y0+1)*(z1-z0+1), nn, int(curptval), ave, sdev, skew, curt);
			break;

		case V3D_UINT16:
			//	  p1dtmp_uint16 = new USHORTINT16 [(x1-x0+1)*(y1-y0+1)*(z1-z0+1)];
			p1dtmp_float32 = new float [(x1-x0+1)*(y1-y0+1)*(z1-z0+1)];
			nn=0;
			for (k=z0;k<=z1;k++)
			{
				for (j=y0;j<=y1;j++)
					for (i=x0;i<=x1;i++)
						//		    p1dtmp_uint16[nn++] = data4d_uint16[c][k][j][i];
						p1dtmp_float32[nn++] = float(data4d_uint16[c][k][j][i]);
			}
			//	  moment(p1dtmp_uint16, nn, ave, adev, sdev, var, skew, curt);
			//	  delete []p1dtmp_uint16; p1dtmp_uint16=0;
			moment(p1dtmp_float32, nn, ave, adev, sdev, var, skew, curt);
			delete []p1dtmp_float32; p1dtmp_float32=0;
			curptval = data4d_uint16[c][z][y][x];
			break;

		case V3D_FLOAT32:
			p1dtmp_float32 = new float [(x1-x0+1)*(y1-y0+1)*(z1-z0+1)];
			nn=0;
			for (k=z0;k<=z1;k++)
			{
				for (j=y0;j<=y1;j++)
					for (i=x0;i<=x1;i++)
						p1dtmp_float32[nn++] = data4d_float32[c][k][j][i];
			}
			moment(p1dtmp_float32, nn, ave, adev, sdev, var, skew, curt);
			delete []p1dtmp_float32; p1dtmp_float32=0;
			curptval = data4d_float32[c][z][y][x];
			break;

		default:
			break;
	}
}

void My4DImage::computePointNeighborMoment(LocationSimple & L, int c) //overload for convenience
{
	computePointNeighborMoment(L.x, L.y, L.z, c, L.pixval, L.ave, L.sdev, L.skew, L.curt);
}



/////////////////////////////////////////////////////////////////////////////////////////////////


bool My4DImage::proj_general_blend_atlasfiles() //081124
{
	if (listAtlasFiles.size()<=0) return false;
	if ( this->getDatatype() !=V3D_UINT8 /*&& this->getDatatype() !=V3D_UINT16 && this->getDatatype() !=V3D_FLOAT32*/)
	{
//		v3d_msg("Now only support UINT8, UINT16, and FLOAT32.\n");
		v3d_msg("Now only support UINT8.\n");
		return false;
	}

	int chan_id_to_load;
	if (atlasColorBlendChannel<0) chan_id_to_load=0;
	else if (atlasColorBlendChannel>=this->getCDim()) chan_id_to_load=this->getCDim()-1;
	else chan_id_to_load =  atlasColorBlendChannel; //in this way, always assure the chan_to_load would be reasonable

	bool b_use_FirstImgAsMask_option=true;

	unsigned char * tmpdata_1d=0;
	V3DLONG *tmpdata_sz=0;
	int tmpdata_type=0;
	V3DLONG totalpagebytes = this->getXDim()*this->getYDim()*this->getZDim()*1;
	int k=0;
	for (int i=0;i<listAtlasFiles.size();i++)
	{
		if (!listAtlasFiles[i].on || !listAtlasFiles[i].exist)
			continue;
		//if (loadRaw2Stack((char *)qPrintable(listAtlasFiles[i].imgfile), tmpdata_1d, tmpdata_sz, tmpdata_type, chan_id_to_load)) //when return value is not 0, then there is an error
		//081204: thus can use tiff, raw (preferred as it is fast to load) and lsm
		if (!(::loadImage((char *)qPrintable(listAtlasFiles[i].imgfile), tmpdata_1d, tmpdata_sz, tmpdata_type, chan_id_to_load))) //when return value is not 0, then there is an error
		{
			printf("Fail to load the %dth file [%s] for atlas view.\n", i+1, qPrintable(listAtlasFiles[i].imgfile));
			continue;
		}
		if (tmpdata_type!=1 || tmpdata_sz[3]!=1)
		{
			printf("Ignore the %dth file [%s] for atlas view, as now only support 8-bit / 1 channel data.\n", i+1, qPrintable(listAtlasFiles[i].imgfile));
			continue;
		}
		if (tmpdata_sz[0]!=this->getXDim() || tmpdata_sz[1]!=this->getYDim() || tmpdata_sz[2]!=this->getZDim()) //this can be done much faster later-on, as I can revise the loadImage to get the size info only before loading the entire stack
		{
			printf("Ignore the %dth file [%s] for atlas view, as it has different size from the current one.\n", i+1, qPrintable(listAtlasFiles[i].imgfile));
			continue;
		}
		k++;
		if (k==1)
		{
			//first clear all old data
			if (this->getCDim()!=3)
			{
				printf("The current image must have THREE channels so that to blend, - but V3D will create a blank 3-channel image to help.\n");

				V3DLONG nsz0=this->getXDim(), nsz1=this->getYDim(), nsz2=this->getZDim(), nsz3=3;
				cleanExistData_butKeepFileName();

				loadImage(nsz0, nsz1, nsz2, nsz3, 1); //now create a blank image
				b_use_FirstImgAsMask_option = false; //in this case the first image is always 0, thus is not good for the masking
			}

			//then copy the new data over
			float rr,gg,bb;
			rr = float(listAtlasFiles[i].color.r)/255.0;
			gg = float(listAtlasFiles[i].color.g)/255.0;
			bb = float(listAtlasFiles[i].color.b)/255.0;

			qDebug("%d image: blending color weight = [%5.2f, %5.2f, %5.2f]\n", k, rr, gg, bb);

			unsigned char *data1d_c0 = (this->getRawData());
			unsigned char *data1d_c1 = (this->getRawData())+totalpagebytes;
			unsigned char *data1d_c2 = (this->getRawData())+totalpagebytes*2;

			for (V3DLONG j=0;j<totalpagebytes;j++)
			{
				data1d_c0[j] = rr*tmpdata_1d[j];
				data1d_c1[j] = gg*tmpdata_1d[j];
				data1d_c2[j] = bb*tmpdata_1d[j];
			}

			//update view
			//p_mainWidget->setColorAllType();
			p_mainWidget->updateDataRelatedGUI();
		}
		else
		{
			//then copy the new data over
			float rr,gg,bb;
			rr = float(listAtlasFiles[i].color.r)/255.0;
			gg = float(listAtlasFiles[i].color.g)/255.0;
			bb = float(listAtlasFiles[i].color.b)/255.0;

			qDebug("%d image: blending color weight = [%5.2f, %5.2f, %5.2f]\n", k, rr, gg, bb);

			unsigned char *data1d_c0 = this->getRawData();
			unsigned char *data1d_c1 = this->getRawData() + totalpagebytes;
			unsigned char *data1d_c2 = this->getRawData() +totalpagebytes*2;

			unsigned char c_r, c_g, c_b;

			if (b_use_FirstImgAsMask_option && bUseFirstImgAsMask)
			{
				for (V3DLONG j=0;j<totalpagebytes;j++)
				{
					if (data1d_c0[j] || data1d_c1[j] || data1d_c2[j])
					{
						c_r = rr*tmpdata_1d[j]; data1d_c0[j] = (c_r > data1d_c0[j]) ? c_r : data1d_c0[j];
						c_g = gg*tmpdata_1d[j]; data1d_c1[j] = (c_g > data1d_c1[j]) ? c_g : data1d_c1[j];
						c_b = bb*tmpdata_1d[j]; data1d_c2[j] = (c_b > data1d_c2[j]) ? c_b : data1d_c2[j];
					}
				}
			}
			else
			{
				for (V3DLONG j=0;j<totalpagebytes;j++)
				{
					c_r = rr*tmpdata_1d[j]; data1d_c0[j] = (c_r > data1d_c0[j]) ? c_r : data1d_c0[j];
					c_g = gg*tmpdata_1d[j]; data1d_c1[j] = (c_g > data1d_c1[j]) ? c_g : data1d_c1[j];
					c_b = bb*tmpdata_1d[j]; data1d_c2[j] = (c_b > data1d_c2[j]) ? c_b : data1d_c2[j];
				}
			}

			//update view
			//p_mainWidget->setColorAllType();
			p_mainWidget->updateDataRelatedGUI();
		}

		//now clean the data
		if (tmpdata_1d) {delete []tmpdata_1d; tmpdata_1d=0;}
		if (tmpdata_sz) {delete []tmpdata_sz; tmpdata_sz=0;}

	}

	//
	return (k==0) ? false : true;
}



QList <LocationSimple> readPosFile(const char * posFile) //last update 081209
{
	QList <LocationSimple> coordPos;
	LocationSimple c3d(-1,-1,-1);

	char curline[2000];
	ifstream file_op;
	file_op.open(posFile);
	if (!file_op)
	{
		fprintf(stderr, "Fail to open the pos file [%s]\n", posFile);
		return coordPos;
	}

	V3DLONG xpos, ypos, zpos;  xpos=ypos=zpos=-1;//set as default
	V3DLONG radius, shape;
	string tmp_name, tmp_comment;
	V3DLONG k=0;
	while(!file_op.eof())
	{
		file_op.getline(curline, 2000);
		cout<<curline<<endl;
		k++;
		//if (k>0) //ignore the first line
		{
			if (curline[0]=='#' || curline[0]=='x' || curline[0]=='X' || curline[0]=='\0') continue;

			QStringList qsl = QString(curline).split(",");
			int qsl_count=qsl.size();
			if (qsl_count<3)   continue;

			xpos = qsl[0].toInt();
			ypos = qsl[1].toInt();
			zpos = qsl[2].toInt();
			radius = (qsl_count>=4) ? qsl[3].toInt() : 0;
			shape = (qsl_count>=5) ? qsl[4].toInt() : 1;
			tmp_name = (qsl_count>=6) ? qPrintable(qsl[5].trimmed()) : "";
			tmp_comment = (qsl_count>=7) ? qPrintable(qsl[6].trimmed()) : "";

			if (xpos==-1 || ypos==-1 || zpos==-1)
			{
				continue;
			}
			else
			{
				c3d.x = xpos;
				c3d.y = ypos;
				c3d.z = zpos;
				c3d.radius = radius;
				c3d.shape = PxLocationMarkerShape(shape);
				c3d.name = tmp_name;
				c3d.comments = tmp_comment;

				c3d.inputProperty = pxLocaUseful;

				coordPos.append(c3d);

				cout<<"in"<<coordPos.last().x<<","<<coordPos.last().y<<","<<coordPos.last().z<<endl;
			}
			xpos=ypos=zpos=-1; //reset to default
		}
	}
	file_op.close();

	return coordPos;
}

QList <LocationSimple> readPosFile_usingMarkerCode(const char * posFile) //last update 090725
{
	QList <LocationSimple> coordPos;
	QList <ImageMarker> tmpList = readMarker_file(posFile);

	if (tmpList.count()<=0)
		return coordPos;

	coordPos.clear();
	for (int i=0;i<tmpList.count();i++)
	{
		LocationSimple pos;
		pos.x = tmpList.at(i).x;
		pos.y = tmpList.at(i).y;
		pos.z = tmpList.at(i).z;
		pos.radius = tmpList.at(i).radius;
		pos.shape = (PxLocationMarkerShape)(tmpList.at(i).shape);
		pos.name = (string)(qPrintable(tmpList.at(i).name));
		pos.comments = (string)(qPrintable(tmpList.at(i).comment));

		coordPos.append(pos);
	}

	return coordPos;
}

bool file_exist(const char * filename)
{
	bool exist;

	ifstream tmpf;
	tmpf.open(filename);
	if (!tmpf) exist = false; else exist = true;
	tmpf.close();

	return exist;
}

QList <InvidualAtlasFileInfo> readAtlasFormatFile(const char * filename)
{
	QList <InvidualAtlasFileInfo> mylist;
	InvidualAtlasFileInfo ifile;

	char curline[2000];
	ifstream file_op;
	file_op.open(filename);
	if (!file_op)
	{
		fprintf(stderr, "Fail to open the atlas file [%s]\n", filename);
		return mylist;
	}

	QString baseDir = filename;
	QString baseName = baseDir.section('/', -1);
	baseDir.chop(baseName.size());

	V3DLONG k=0;
	while(!file_op.eof())
	{
		file_op.getline(curline, 2000);
		cout<<curline<<endl;

		if (curline[0]=='#' || curline[0]=='\0') continue;
		k++;

		QStringList qsl = QString(curline).split(",");
		if (qsl.size()<6)   continue;

		ifile.n = qsl[0].toInt();
		ifile.category = qsl[1].trimmed();
		ifile.color.r = qsl[2].toInt();
		ifile.color.g = qsl[3].toInt();
		ifile.color.b = qsl[4].toInt();
		ifile.color.a = 255; //081211. This is very important so that the default color will be visible in the atlas viewer
		ifile.imgfile = qsl[5].trimmed();
		ifile.on = false;

		ifile.exist = file_exist(qPrintable(ifile.imgfile));
		if (!ifile.exist)
		{
			if (file_exist((qPrintable(baseDir + ifile.imgfile))))
			{
				ifile.imgfile = (baseDir + ifile.imgfile);
				ifile.exist = true;
			}
		}

		mylist.append(ifile);
	}
	file_op.close();

	return mylist;
}

void My4DImage::loadLandmarkFromFile()
{
    QString curFile = QFileDialog::getOpenFileName(0,
												   "Select a text file to load the coordinates of landmark points... ",
												   "",
												   "Landmark definition file (*.marker);;Landmark definition file (*.csv);;Landmark definition file (*.txt);;All Files (*)");
	if (curFile.isEmpty()) //note that I used isEmpty() instead of isNull
		return;

	FILE * fp = fopen(curFile.toUtf8().data(), "r");
	if (!fp)
	{
		QMessageBox::information(0, "Control point loading error",
								 "Could not open the file to load the landmark points.\n");
		return;
	}
	else
	{
		fclose(fp); //since I will open the file and close it in the function below, thus close it now
	}

	QList <LocationSimple> tmpList = readPosFile_usingMarkerCode(curFile.toUtf8().data()); //revised on 090725 to use the unique interface

	if (tmpList.count()<=0)
	{
		v3d_msg("Did not find any valid row/record of the markers. Thus do not overwrite the current landmarks if they exist.\n");
		return;
	}

	listLandmarks.clear();
	for (int i=0;i<tmpList.count();i++)
	{
		listLandmarks.append(tmpList.at(i));
	}
}

void My4DImage::saveLandmarkToFile()
{
	if (listLandmarks.count()<=0)
	{
		QMessageBox::information(0, "Control point saving error",
								 "You don't have any landmark defined yet. Do nothing.\n");
		return;
	}

    QString curFile = QFileDialog::getSaveFileName(0,
												   "Select a text file to save the coordinates of landmark points... ",
												   getXWidget()->getOpenFileNameLabel()+".marker","");
	//tr("(*.txt, *.csv);;All Files (*)")); //080619: This is a strange QT bug, which works without a problem in earlier QT version, but here always use the last para
	//to overwrite portion of the third parameter. Now I am using an empty string to get around this problem.
	//tr("Landmark definition file (*.txt, *.csv);;All Files (*)"));

	if (curFile.isEmpty()) //note that I used isEmpty() instead of isNull
		return;

	FILE * fp = fopen(curFile.toUtf8().data(), "w");
	if (!fp)
	{
		QMessageBox::information(0, "Control point saving error",
								 "Could not open the file to save the landmark points.\n");
		return;
	}

	fprintf(fp, "#x, y, z, radius, shape, name, comment\n"); //081209: change the first line as comment
	for (int i=0;i<listLandmarks.count(); i++)
	{
		fprintf(fp, "%ld,%ld,%ld,%ld,%ld,%s,%s\n",
				V3DLONG(listLandmarks.at(i).x), V3DLONG(listLandmarks.at(i).y), V3DLONG(listLandmarks.at(i).z),
				V3DLONG(listLandmarks.at(i).radius), V3DLONG(listLandmarks.at(i).shape),
				listLandmarks.at(i).name.c_str(), listLandmarks.at(i).comments.c_str());
	}

	fclose(fp);
}


bool My4DImage::compute_rgn_stat(LocationSimple & pt, int channo)
{
	if (!valid()) return false;

	V3DLONG xx = V3DLONG(pt.x+0.5);
	V3DLONG yy = V3DLONG(pt.y+0.5);
	V3DLONG zz = V3DLONG(pt.z+0.5);
	V3DLONG cc = channo; if (cc<0) cc=0; if (cc>=getCDim()) cc=getCDim()-1;
	V3DLONG rr = pt.radius; if (rr<0) rr=0;
	PxLocationMarkerShape ss = pt.shape;

	//now do the computation
	int res_peak=0;
	double res_mean=0, res_std=0;
	double res_size=0, res_mass=0;
	V3DLONG i,j,k;
     XYZ res_mcenter; //mass center
     double res_xI=0, res_yI=0,res_zI=0; // for mass center computation

	V3DLONG xs,xe,ys,ye,zs,ze;
	xs = xx-rr; if (xs<0) xs=0;
	xe = xx+rr; if (xe>=getXDim()) xe = getXDim()-1;
	ys = yy-rr; if (ys<0) ys=0;
	ye = yy+rr; if (ye>=getYDim()) ye = getYDim()-1;
	zs = zz-rr; if (zs<0) zs=0;
	ze = zz+rr; if (ze>=getZDim()) ze = getZDim()-1;
	double r2=double(rr+1)*(rr+1);

	switch (ss)
	{
		case pxSphere:
			for (k=zs;k<=ze;k++)
			{
				double cur_dk = (k-zz)*(k-zz);
				for (j=ys;j<=ye;j++)
				{
					double cur_dj = (j-yy)*(j-yy);
					double cur_d = cur_dk + cur_dj;
					if (cur_d>r2) continue;
					for (i=xs;i<=xe;i++)
					{
						double cur_di = (i-xx)*(i-xx);
						cur_d = cur_dk + cur_dj + cur_di;
						if (cur_d>r2) continue;

						double cur_v = this->at(i, j, k, cc);
						if (res_peak<cur_v) res_peak = cur_v;
						res_size++;
						res_mass += cur_v;
						res_std += cur_v*cur_v; //use the incremental formula
                              res_xI += i*cur_v;
                              res_yI += j*cur_v;
                              res_zI += k*cur_v;
					}
				}
			}
			res_mean = res_mass/res_size;
			res_std = sqrt(res_std/res_size - res_mean*res_mean);
               res_mcenter.x = res_xI/res_mass;
               res_mcenter.y = res_yI/res_mass;
               res_mcenter.z = res_zI/res_mass;
			break;

		case pxCube:
			for (k=zs;k<=ze;k++)
				for (j=ys;j<=ye;j++)
					for (i=xs;i<=xe;i++)
					{
						double cur_v = this->at(i, j, k, cc);
						if (res_peak<cur_v) res_peak = cur_v;
						res_size++;
						res_mass += cur_v;
						res_std += cur_v*cur_v; //use the incremental formula
                              res_xI += i*cur_v;
                              res_yI += j*cur_v;
                              res_zI += k*cur_v;
					}
			res_mean = res_mass/res_size;
			res_std = sqrt(res_std/res_size - res_mean*res_mean);
               res_mcenter.x = res_xI/res_mass;
               res_mcenter.y = res_yI/res_mass;
               res_mcenter.z = res_zI/res_mass;
			break;

		case pxCircleX:
			for (k=zs;k<=ze;k++)
			{
				double cur_dk = (k-zz)*(k-zz);
				for (j=ys;j<=ye;j++)
				{
					double cur_dj = (j-yy)*(j-yy);
					double cur_d = cur_dk + cur_dj;
					if (cur_d>r2) continue;
					for (i=xx;i<=xx;i++)
					{
						double cur_di = (i-xx)*(i-xx);
						cur_d = cur_dk + cur_dj + cur_di;
						if (cur_d>r2) continue;

						double cur_v = this->at(i, j, k, cc);
						if (res_peak<cur_v) res_peak = cur_v;
						res_size++;
						res_mass += cur_v;
						res_std += cur_v*cur_v; //use the incremental formula
                              res_xI += i*cur_v;
                              res_yI += j*cur_v;
                              res_zI += k*cur_v;
					}
				}
			}
			res_mean = res_mass/res_size;
			res_std = sqrt(res_std/res_size - res_mean*res_mean);
               res_mcenter.x = res_xI/res_mass;
               res_mcenter.y = res_yI/res_mass;
               res_mcenter.z = res_zI/res_mass;
			break;

		case pxCircleY:
			for (k=zs;k<=ze;k++)
			{
				double cur_dk = (k-zz)*(k-zz);
				for (j=yy;j<=yy;j++)
				{
					double cur_dj = (j-yy)*(j-yy);
					double cur_d = cur_dk + cur_dj;
					if (cur_d>r2) continue;
					for (i=xs;i<=xe;i++)
					{
						double cur_di = (i-xx)*(i-xx);
						cur_d = cur_dk + cur_dj + cur_di;
						if (cur_d>r2) continue;

						double cur_v = this->at(i, j, k, cc);
						if (res_peak<cur_v) res_peak = cur_v;
						res_size++;
						res_mass += cur_v;
						res_std += cur_v*cur_v; //use the incremental formula
                              res_xI += i*cur_v;
                              res_yI += j*cur_v;
                              res_zI += k*cur_v;
					}
				}
			}
			res_mean = res_mass/res_size;
			res_std = sqrt(res_std/res_size - res_mean*res_mean);
               res_mcenter.x = res_xI/res_mass;
               res_mcenter.y = res_yI/res_mass;
               res_mcenter.z = res_zI/res_mass;
			break;

		case pxCircleZ:
			for (k=zz;k<=zz;k++)
			{
				double cur_dk = (k-zz)*(k-zz);
				for (j=ys;j<=ye;j++)
				{
					double cur_dj = (j-yy)*(j-yy);
					double cur_d = cur_dk + cur_dj;
					if (cur_d>r2) continue;
					for (i=xs;i<=xe;i++)
					{
						double cur_di = (i-xx)*(i-xx);
						cur_d = cur_dk + cur_dj + cur_di;
						if (cur_d>r2) continue;

						double cur_v = this->at(i, j, k, cc);
						if (res_peak<cur_v) res_peak = cur_v;
						res_size++;
						res_mass += cur_v;
						res_std += cur_v*cur_v; //use the incremental formula
                              res_xI += i*cur_v;
                              res_yI += j*cur_v;
                              res_zI += k*cur_v;
					}
				}
			}
			res_mean = res_mass/res_size;
			res_std = sqrt(res_std/res_size - res_mean*res_mean);
               res_mcenter.x = res_xI/res_mass;
               res_mcenter.y = res_yI/res_mass;
               res_mcenter.z = res_zI/res_mass;
			break;

		case pxSquareX:
			for (k=zs;k<=ze;k++)
				for (j=ys;j<=ye;j++)
					for (i=xx;i<=xx;i++)
					{
						double cur_v = this->at(i, j, k, cc);
						if (res_peak<cur_v) res_peak = cur_v;
						res_size++;
						res_mass += cur_v;
						res_std += cur_v*cur_v; //use the incremental formula
                              res_xI += i*cur_v;
                              res_yI += j*cur_v;
                              res_zI += k*cur_v;
					}
			res_mean = res_mass/res_size;
			res_std = sqrt(res_std/res_size - res_mean*res_mean);
               res_mcenter.x = res_xI/res_mass;
               res_mcenter.y = res_yI/res_mass;
               res_mcenter.z = res_zI/res_mass;
			break;

		case pxSquareY:
			for (k=zs;k<=ze;k++)
				for (j=yy;j<=yy;j++)
					for (i=xs;i<=xe;i++)
					{
						double cur_v = this->at(i, j, k, cc);
						if (res_peak<cur_v) res_peak = cur_v;
						res_size++;
						res_mass += cur_v;
						res_std += cur_v*cur_v; //use the incremental formula
                              res_xI += i*cur_v;
                              res_yI += j*cur_v;
                              res_zI += k*cur_v;
					}
			res_mean = res_mass/res_size;
			res_std = sqrt(res_std/res_size - res_mean*res_mean);
               res_mcenter.x = res_xI/res_mass;
               res_mcenter.y = res_yI/res_mass;
               res_mcenter.z = res_zI/res_mass;
			break;

		case pxSquareZ:
			for (k=zz;k<=zz;k++)
				for (j=ys;j<=ye;j++)
					for (i=xs;i<=xe;i++)
					{
						double cur_v = this->at(i, j, k, cc);
						if (res_peak<cur_v) res_peak = cur_v;
						res_size++;
						res_mass += cur_v;
						res_std += cur_v*cur_v; //use the incremental formula
                              res_xI += i*cur_v;
                              res_yI += j*cur_v;
                              res_zI += k*cur_v;
					}
			res_mean = res_mass/res_size;
			res_std = sqrt(res_std/res_size - res_mean*res_mean);
               res_mcenter.x = res_xI/res_mass;
               res_mcenter.y = res_yI/res_mass;
               res_mcenter.z = res_zI/res_mass;
			break;

		case pxLineX:
			for (k=zz;k<=zz;k++)
				for (j=yy;j<=yy;j++)
					for (i=xs;i<=xe;i++)
					{
						double cur_v = this->at(i, j, k, cc);
						if (res_peak<cur_v) res_peak = cur_v;
						res_size++;
						res_mass += cur_v;
						res_std += cur_v*cur_v; //use the incremental formula
                              res_xI += i*cur_v;
                              res_yI += j*cur_v;
                              res_zI += k*cur_v;
					}
			res_mean = res_mass/res_size;
			res_std = sqrt(res_std/res_size - res_mean*res_mean);
               res_mcenter.x = res_xI/res_mass;
               res_mcenter.y = res_yI/res_mass;
               res_mcenter.z = res_zI/res_mass;
			break;

		case pxLineY:
			for (k=zz;k<=zz;k++)
				for (j=ys;j<=ye;j++)
					for (i=xx;i<=xx;i++)
					{
						double cur_v = this->at(i, j, k, cc);
						if (res_peak<cur_v) res_peak = cur_v;
						res_size++;
						res_mass += cur_v;
						res_std += cur_v*cur_v; //use the incremental formula
                              res_xI += i*cur_v;
                              res_yI += j*cur_v;
                              res_zI += k*cur_v;
					}
			res_mean = res_mass/res_size;
			res_std = sqrt(res_std/res_size - res_mean*res_mean);
               res_mcenter.x = res_xI/res_mass;
               res_mcenter.y = res_yI/res_mass;
               res_mcenter.z = res_zI/res_mass;
			break;

		case pxLineZ:
			for (k=zs;k<=ze;k++)
				for (j=yy;j<=yy;j++)
					for (i=xx;i<=xx;i++)
					{
						double cur_v = this->at(i, j, k, cc);
						if (res_peak<cur_v) res_peak = cur_v;
						res_size++;
						res_mass += cur_v;
						res_std += cur_v*cur_v; //use the incremental formula
                              res_xI += i*cur_v;
                              res_yI += j*cur_v;
                              res_zI += k*cur_v;
					}
			res_mean = res_mass/res_size;
			res_std = sqrt(res_std/res_size - res_mean*res_mean);
               res_mcenter.x = res_xI/res_mass;
               res_mcenter.y = res_yI/res_mass;
               res_mcenter.z = res_zI/res_mass;
			break;

		case pxDot:
			for (k=zz;k<=zz;k++)
				for (j=yy;j<=yy;j++)
					for (i=xx;i<=xx;i++)
					{
						double cur_v = this->at(i, j, k, cc);
						if (res_peak<cur_v) res_peak = cur_v;
						res_size++;
						res_mass += cur_v;
						res_std += cur_v*cur_v; //use the incremental formula
                              res_xI += i*cur_v;
                              res_yI += j*cur_v;
                              res_zI += k*cur_v;
					}
			res_mean = res_mass/res_size;
			res_std = sqrt(res_std/res_size - res_mean*res_mean);
               res_mcenter.x = res_xI/res_mass;
               res_mcenter.y = res_yI/res_mass;
               res_mcenter.z = res_zI/res_mass;
			break;

		case pxUnset:
		case pxTriangle:
		default:
			printf("Not supported shape. Not compute.\n");
			return false;
			break;
	}

	//now compute the eigen value info

	int b_win_shape=1; //0 for cube and 1 for sphere
	bool b_disp_CoM_etc=false; //if display center of mass and covariance info
	switch ( this->getDatatype() )
	{
		case V3D_UINT8:
			compute_win3d_pca(data4d_uint8[cc], this->getXDim(), this->getYDim(), this->getZDim(),
							  xx, yy, zz,
							  rr, rr, rr,
							  pt.ev_pc1, pt.ev_pc2, pt.ev_pc3, b_win_shape, b_disp_CoM_etc);
			break;

		case V3D_UINT16:
			compute_win3d_pca(data4d_uint16[cc], this->getXDim(), this->getYDim(), this->getZDim(),
							  xx, yy, zz,
							  rr, rr, rr,
							  pt.ev_pc1, pt.ev_pc2, pt.ev_pc3, b_win_shape, b_disp_CoM_etc);
			break;

		case V3D_FLOAT32:
			compute_win3d_pca(data4d_float32[cc], this->getXDim(), this->getYDim(), this->getZDim(),
							  xx, yy, zz,
							  rr, rr, rr,
							  pt.ev_pc1, pt.ev_pc2, pt.ev_pc3, b_win_shape, b_disp_CoM_etc);
			break;

		default:
			v3d_msg("Unsupported data type found in compute_rgn_stat(). \n");
			break;
	}

	bool b_compute_all_radius=false;
	if (b_compute_all_radius)
	{
		for (i=0;i<=rr;i++)
		{
			bool b_continue=true;
			switch ( this->getDatatype() )
			{
				case V3D_UINT8:
					compute_win3d_pca(data4d_uint8[cc], this->getXDim(), this->getYDim(), this->getZDim(),
									  xx, yy, zz,
									  i, i, i,
									  pt.ev_pc1, pt.ev_pc2, pt.ev_pc3, b_win_shape, b_disp_CoM_etc);
					break;
				case V3D_UINT16:
					compute_win3d_pca(data4d_uint16[cc], this->getXDim(), this->getYDim(), this->getZDim(),
									  xx, yy, zz,
									  i, i, i,
									  pt.ev_pc1, pt.ev_pc2, pt.ev_pc3, b_win_shape, b_disp_CoM_etc);
					break;

				case V3D_FLOAT32:
					compute_win3d_pca(data4d_float32[cc], this->getXDim(), this->getYDim(), this->getZDim(),
									  xx, yy, zz,
									  i, i, i,
									  pt.ev_pc1, pt.ev_pc2, pt.ev_pc3, b_win_shape, b_disp_CoM_etc);
					break;

				default:
					v3d_msg("Unsupported data type found in compute_rgn_stat(). \n");
					b_continue=false;
					break;
			}

			if (!b_continue)
				break;

			double Lscore = exp( -( (pt.ev_pc1-pt.ev_pc2)*(pt.ev_pc1-pt.ev_pc2) + (pt.ev_pc2-pt.ev_pc3)*(pt.ev_pc2-pt.ev_pc3) + (pt.ev_pc1-pt.ev_pc3)*(pt.ev_pc1-pt.ev_pc3) ) /
								(pt.ev_pc1*pt.ev_pc1 + pt.ev_pc2*pt.ev_pc2 + pt.ev_pc3*pt.ev_pc3) );
			double s_linear = (pt.ev_pc1-pt.ev_pc2)/(pt.ev_pc1+pt.ev_pc2+pt.ev_pc3);
			double s_planar = 2.0*(pt.ev_pc2-pt.ev_pc3)/(pt.ev_pc1+pt.ev_pc2+pt.ev_pc3);
			double s_sphere = 3.0*pt.ev_pc3/(pt.ev_pc1+pt.ev_pc2+pt.ev_pc3);
			printf("r=%d \t lamba1=%5.3f lamba2=%5.3f lamba3=%5.3f L_score=%5.3f linear_c=%5.3f planar_c=%5.3f spherical_c=%5.3f\n", i, pt.ev_pc1, pt.ev_pc2, pt.ev_pc3, Lscore, s_linear, s_planar, s_sphere);
		}
	}

	//now update the value of the respective

	QString tmp;
	pt.pixmax = res_peak;
	pt.ave = res_mean;
	pt.sdev = res_std;
	pt.size = res_size;
	pt.mass = res_mass;
     pt.mcenter = res_mcenter;

	//no need to update pt's ev_pc field as they have been updated

	return true;
}


void My4DImage::exportLandmarkToPointCloudAPOFile()
{
	if (listLandmarks.count()<=0)
	{
		QMessageBox::information(0, "landmark exporting error", "You don't have any landmark defined yet. Do nothing.\n");
		return;
	}

	bool ok1;

#if defined(USE_Qt5)
	int channo = QInputDialog::getInt(0, "select number of color channel", "which color channel you want to collect statistics?", 1, 1, getCDim(), 1, &ok1);
#else
	int channo = QInputDialog::getInteger(0, "select number of color channel", "which color channel you want to collect statistics?", 1, 1, getCDim(), 1, &ok1);
#endif
	if (!ok1)
		return;

    QString curFile = QFileDialog::getSaveFileName(0,
												   "Select a APO (text, csv format) file to export landmark points to point cloud... ",
												   getXWidget()->getOpenFileNameLabel()+".apo","");
	//tr("(*.txt, *.csv);;All Files (*)")); //080619: This is a strange QT bug, which works without a problem in earlier QT version, but here always use the last para
	//to overwrite portion of the third parameter. Now I am using an empty string to get around this problem.
	//tr("Landmark definition file (*.txt, *.csv);;All Files (*)"));

	if (curFile.isEmpty()) //note that I used isEmpty() instead of isNull
		return;

	FILE * fp = fopen(curFile.toUtf8().data(), "wt");
	if (!fp)
	{
		QMessageBox::information(0, "landmark exporting error", "Could not open the file to export the landmark points.\n");
		return;
	}

    LocationSimple *p_pt;
	for (int i=0;i<listLandmarks.count(); i++)
	{
		//compute the statistics
		compute_rgn_stat((LocationSimple &)(listLandmarks.at(i)), channo-1); //use 0, i.e. the first channel for convenience

		//then save
		p_pt = (LocationSimple *)(&(listLandmarks.at(i)));
		fprintf(fp, "%ld, %ld, %s,%s, %ld,%ld,%ld, %5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,\n",
				i, i, p_pt->name.c_str(), p_pt->comments.c_str(),
				V3DLONG(p_pt->z+0.5), V3DLONG(p_pt->x+0.5), V3DLONG(p_pt->y+0.5),
				p_pt->pixmax, p_pt->ave, p_pt->sdev, p_pt->size, p_pt->mass);
	}

	fclose(fp);
}

void My4DImage::exportLandmarkandRelationToSWCFile()
{
	v3d_msg("to be implemented.\n");
}

void My4DImage::exportNeuronToSWCFile()
{
	if (tracedNeuron.nsegs()<=0 || tracedNeuron.nrows()<=0)
	{
		v3d_msg("Neuron exporting error: You don't have a valid traced neuron yet. Do nothing.");
		return;
	}

    QString curFile = QFileDialog::getSaveFileName(0,
												   "Select a SWC file to save the traced neuron... ",
												   getXWidget()->getOpenFileNameLabel()+".swc","");

	if (curFile.isEmpty()) //note that I used isEmpty() instead of isNull
		return;

	FILE * fp = fopen(curFile.toUtf8().data(), "wt");
	if (!fp)
	{
		QMessageBox::information(0, "Neuron exporting error", "Could not open the file to export the neuron.\n");
		return;
	}

	fprintf(fp, "#%s\n", qPrintable(curFile));
	fprintf(fp, "#%s\n", tracedNeuron.name.c_str());

	V_NeuronSWC merged_neuron = merge_V_NeuronSWC_list(tracedNeuron);
	for (int i=0;i<merged_neuron.row.size(); i++)
	{
		fprintf(fp, "%ld %ld %5.3f %5.3f %5.3f %5.3f %ld\n",
				V3DLONG(merged_neuron.row[i].data[0]), V3DLONG(merged_neuron.row[i].data[1]), merged_neuron.row[i].data[2], merged_neuron.row[i].data[3], merged_neuron.row[i].data[4], merged_neuron.row[i].data[5], V3DLONG(merged_neuron.row[i].data[6]));
	}

	fclose(fp);
}


void My4DImage::updateViews()
{
	if (p_mainWidget)  p_mainWidget->show(); //090818 for V3D_PluginLoader

	//setUpdatesEnabled(true)

	if (p_xy_view) //seems update() will not really update , thus try the stupid brute force method
	{
		p_xy_view->updateViewPlane();
	}
	if (p_yz_view) {p_yz_view->updateViewPlane(); }
	if (p_zx_view)	{p_zx_view->updateViewPlane(); }
	if (p_focusPointFeatureWidget) {setFocusFeatureViewText();} // p_focusPointFeatureWidget->update();}
}

#define WANT_STREAM       // include iostream and iomanipulators
#include "../jba/newmat11/newmatap.h"
#include "../jba/newmat11/newmatio.h"
#ifdef use_namespace
using namespace RBD_LIBRARIES;
#endif

bool My4DImage::proj_general_principal_axis(ImagePlaneDisplayType ptype)
{
    //first generate the sum image of all planes for a particular axis code
    if (!data4d_uint8 && !data4d_uint16 && !data4d_float32)
    {
        v3d_msg("None of the 4D pointers is valid in proj_general_principal_axis().");  return false;
    }

    Options_Rotate tmp_opt;
    tmp_opt.b_keepSameSize = (QMessageBox::Yes == QMessageBox::question (0, "", "Keep rotated image the same size with the original image?", QMessageBox::Yes, QMessageBox::No)) ?
                true : false;

    tmp_opt.fillcolor=0;

    //

    float * sumdata1d = 0;
    float ** sumdata2d = 0;

    V3DLONG i,j,k,c;
    V3DLONG d0, d1;

    switch (ptype)
    {
    case imgPlaneX:
        d0 = this->getYDim(); d1 = this->getZDim();
        sumdata1d = new float [(V3DLONG)d0*d1];
        if (!sumdata1d) return false;
        if(!new2dpointer(sumdata2d, d0, d1, sumdata1d)) {if (sumdata1d) {delete []sumdata1d; sumdata1d=0;} return false;}

        if (data4d_uint8)
        {
            for (k=0;k<this->getZDim();k++)
            {
                for (j=0;j<this->getYDim();j++)
                {
                    double tmp=0;
                    for (i=0;i<this->getXDim();i++)
                        for (c=0;c<this->getCDim();c++)
                            tmp += float(data4d_uint8[c][k][j][i]);
                    sumdata2d[k][j]=tmp;
                }
            }
        }
        else if (data4d_uint16)
        {
            for (k=0;k<this->getZDim();k++)
            {
                for (j=0;j<this->getYDim();j++)
                {
                    double tmp=0;
                    for (i=0;i<this->getXDim();i++)
                        for (c=0;c<this->getCDim();c++)
                            tmp += float(data4d_uint16[c][k][j][i]);
                    sumdata2d[k][j]=tmp;
                }
            }
        }
        else if (data4d_float32)
        {
            for (k=0;k<this->getZDim();k++)
            {
                for (j=0;j<this->getYDim();j++)
                {
                    double tmp=0;
                    for (i=0;i<this->getXDim();i++)
                        for (c=0;c<this->getCDim();c++)
                            tmp += float(data4d_float32[c][k][j][i]);
                    sumdata2d[k][j]=tmp;
                }
            }
        }

        break;

    case imgPlaneY:
        d0 = this->getXDim(); d1 = this->getZDim();
        sumdata1d = new float [(V3DLONG)d0*d1];
        if (!sumdata1d) return false;
        if(!new2dpointer(sumdata2d, d0, d1, sumdata1d)) {if (sumdata1d) {delete []sumdata1d; sumdata1d=0;} return false;}

        if (data4d_uint8)
        {
            for (k=0;k<this->getZDim();k++)
            {
                for (i=0;i<this->getXDim(); i++)
                {
                    double tmp=0;
                    for (j=0;j<this->getYDim(); j++)
                        for (c=0;c<this->getCDim();c++)
                            tmp += float(data4d_uint8[c][k][j][i]);
                    sumdata2d[k][i]=tmp;
                }
            }
        }
        else if (data4d_uint16)
        {
            for (k=0;k<this->getZDim();k++)
            {
                for (i=0;i<this->getXDim(); i++)
                {
                    double tmp=0;
                    for (j=0;j<this->getYDim(); j++)
                        for (c=0;c<this->getCDim();c++)
                            tmp += float(data4d_uint16[c][k][j][i]);
                    sumdata2d[k][i]=tmp;
                }
            }
        }
        else if (data4d_float32)
        {
            for (k=0;k<this->getZDim();k++)
            {
                for (i=0;i<this->getXDim(); i++)
                {
                    double tmp=0;
                    for (j=0;j<this->getYDim(); j++)
                        for (c=0;c<this->getCDim();c++)
                            tmp += float(data4d_float32[c][k][j][i]);
                    sumdata2d[k][i]=tmp;
                }
            }
        }

        break;

    case imgPlaneZ:
        d0 = this->getXDim(); d1 = this->getYDim();
        sumdata1d = new float [(V3DLONG)d0*d1];
        if (!sumdata1d) return false;
        if(!new2dpointer(sumdata2d, d0, d1, sumdata1d)) {if (sumdata1d) {delete []sumdata1d; sumdata1d=0;} return false;}

        if (data4d_uint8)
        {
            for (j=0;j<this->getYDim();j++)
            {
                for (i=0;i<this->getXDim(); i++)
                {
                    double tmp=0;
                    for (k=0;k<this->getZDim(); k++)
                        for (c=0;c<this->getCDim();c++)
                            tmp += float(data4d_uint8[c][k][j][i]);
                    sumdata2d[j][i]=tmp;
                }
            }
        }
        else if (data4d_uint16)
        {
            for (j=0;j<this->getYDim();j++)
            {
                for (i=0;i<this->getXDim(); i++)
                {
                    double tmp=0;
                    for (k=0;k<this->getZDim(); k++)
                        for (c=0;c<this->getCDim();c++)
                            tmp += float(data4d_uint16[c][k][j][i]);
                    sumdata2d[j][i]=tmp;
                }
            }
        }
        else if (data4d_float32)
        {
            for (j=0;j<this->getYDim();j++)
            {
                for (i=0;i<this->getXDim(); i++)
                {
                    double tmp=0;
                    for (k=0;k<this->getZDim(); k++)
                        for (c=0;c<this->getCDim();c++)
                            tmp += float(data4d_float32[c][k][j][i]);
                    sumdata2d[j][i]=tmp;
                }
            }
        }
        break;

    default:
        return false;
    }

    //then find the major axis for this plane's sum image

    //first get the means
    double d0mean=0, d1mean=0, s=0;
    for (j=0;j<d1;j++)
    {
        for (i=0;i<d0;i++)
        {
            d1mean += sumdata2d[j][i]*j;
            d0mean += sumdata2d[j][i]*i;
            s += sumdata2d[j][i];
        }
    }
    d1mean /= s;
    d0mean /= s; //this is the intensity weighted mean values, thus the center of mass of the sum image

    //then get the covariance
    double p00=0, p11=0, p01=0, df0, df1, w;
    for (j=0;j<d1;j++)
    {
        df1 = (j-d1mean);
        for (i=0;i<d0;i++)
        {
            df0 = (i-d0mean);
            w = sumdata2d[j][i];
            p00 += w*df0*df0;
            p11 += w*df1*df1;
            p01 += w*df1*df0;
        }
    }
    p00 /= s;
    p01 /= s;
    p11 /= s;

    //then find the eigen vector
    SymmetricMatrix Cov_Matrix(2);
    Cov_Matrix.Row(1) << p00;
    Cov_Matrix.Row(2) << p01 << p11;

    DiagonalMatrix DD;
    Matrix VV;
    EigenValues(Cov_Matrix,DD,VV);

    double r_angle = acos(VV(2,1)/sqrt(VV(2,2)*VV(2,2)+VV(2,1)*VV(2,1)))/3.141592635*180.0;

    //commented 131029, by PHC
    /*
    if (1)
    {
        cout << "Cov_Matrix" << endl;
        cout << setw(12) << setprecision(3) << Cov_Matrix << endl <<endl;

        cout << "DD_Matrix" << endl;
        cout << setw(12) << setprecision(3) << DD << endl <<endl;

        cout << "VV_Matrix" << endl;
        cout << setw(12) << setprecision(3) << VV << endl <<endl;

        cout << setw(12) << setprecision(3) << r_angle << endl <<endl;
    }
    */

    //free space that no longer needed
    if (sumdata2d) {delete2dpointer(sumdata2d, d0, d1);}
    if (sumdata1d) {delete []sumdata1d, sumdata1d=0;}

    //finally rotate image in plane

    switch (ptype)
    {
    case imgPlaneX:
        tmp_opt.degree = -r_angle/180.0*3.141592635;
        tmp_opt.center_x = (this->getXDim()-1.0)/2;
        tmp_opt.center_y = d0mean;
        tmp_opt.center_z = d1mean;
        break;

    case imgPlaneY:
        tmp_opt.degree = -r_angle/180.0*3.141592635;
        tmp_opt.center_x = d0mean;
        tmp_opt.center_y = (this->getYDim()-1.0)/2;
        tmp_opt.center_z = d1mean;
        break;

    case imgPlaneZ:
        tmp_opt.degree = -r_angle/180.0*3.141592635;
        tmp_opt.center_x = d0mean;
        tmp_opt.center_y = d1mean;
        tmp_opt.center_z = (this->getZDim()-1.0)/2;
        break;

    default:
        return false;
    }

    if (!rotate(ptype, tmp_opt)) //this will update image, so remove the following code
        return false;

    //update view
    updateViews();
    return true;
}

bool My4DImage::proj_general_resampling(ImageResamplingCode mycode, double target_rez, double cur_rez, int interp_method)
{
	V3DLONG cur_sz[4];
	cur_sz[0] = this->getXDim(); cur_sz[1] = this->getYDim(); cur_sz[2] = this->getZDim(); cur_sz[3] = this->getCDim();
	double dfactor_x, dfactor_y, dfactor_z;

	if (mycode==PRS_Z_ONLY)
	{
		dfactor_x = 1; dfactor_y = 1; dfactor_z = target_rez/cur_rez;
		switch ( this->getDatatype() )
		{
			case V3D_UINT8:
				{
					if(!reslice_Z( data1d, cur_sz, target_rez, cur_rez, interp_method)) //z-resampling for 4D data
						return false;
				}
				break;

			case V3D_UINT16:
				{
					USHORTINT16 *curdata = (USHORTINT16 *)data1d;
					if(!reslice_Z( curdata, cur_sz, target_rez, cur_rez, interp_method)) //z-resampling for 4D data
						return false;
					data1d = (unsigned char *)curdata;
				}
				break;

			case V3D_FLOAT32:
				{
					float *curdata = (float *)data1d;
					if(!reslice_Z( curdata, cur_sz, target_rez, cur_rez, interp_method)) //z-resampling for 4D data
						return false;
					data1d = (unsigned char *)curdata;
				}
				break;

			default:
				v3d_msg("You should never see this error in proj_general_resampling(). Check with V3D developers.");
				return false;
		}
	}
	else
	{
		switch (mycode)
		{
			case PRS_X_ONLY:
				dfactor_x = target_rez/cur_rez; dfactor_y = 1; dfactor_z = 1;
				break;
			case PRS_Y_ONLY:
				dfactor_x = 1; dfactor_y = target_rez/cur_rez; dfactor_z = 1;
				break;
			case PRS_XY_SAME:
				dfactor_x = dfactor_y = target_rez/cur_rez; dfactor_z = 1;
				break;
			case PRS_XYZ_SAME:
				dfactor_x = dfactor_y = dfactor_z = target_rez/cur_rez;
				break;
			default:
				v3d_msg("Undefined/unsupported ImageResamplingCode found. Check the program.\n");
				return false;
		}

		switch ( this->getDatatype() )
		{
			case V3D_UINT8:
				{
					if (!resample3dimg_interp( data1d, cur_sz, dfactor_x, dfactor_y, dfactor_z, interp_method))
						return false;
				}
				break;

			case V3D_UINT16:
				{
					USHORTINT16 *curdata = (USHORTINT16 *)data1d;
					if (!resample3dimg_interp( curdata, cur_sz, dfactor_x, dfactor_y, dfactor_z, interp_method))
						return false;
					data1d = (unsigned char *)curdata;
				}
				break;

			case V3D_FLOAT32:
				{
					float *curdata = (float *)data1d;
					if (!resample3dimg_interp( curdata, cur_sz, dfactor_x, dfactor_y, dfactor_z, interp_method))
						return false;
					data1d = (unsigned char *)curdata;
				}
				break;

			default:
				v3d_msg("You should never see this error in proj_general_resampling(). Check with V3D developers.");
				return false;
		}
	}

	//remove the old 4D pointers, etc

	switch ( this->getDatatype() )
	{
		case V3D_UINT8:
			delete4dpointer_v3d(data4d_uint8, this->getXDim(), this->getYDim(), this->getZDim(), this->getCDim());
			data4d_virtual = 0;
			break;

		case V3D_UINT16:
			delete4dpointer_v3d(data4d_uint16, this->getXDim(), this->getYDim(), this->getZDim(), this->getCDim());
			data4d_virtual = 0;
			break;

		case V3D_FLOAT32:
			delete4dpointer_v3d(data4d_float32, this->getXDim(), this->getYDim(), this->getZDim(), this->getCDim());
			data4d_virtual = 0;
			break;

		default:
			this->setError(1);
			return false;
			//break;
	}

	//now create new 4D pointers
	this->setXDim( cur_sz[0] );
	this->setYDim( cur_sz[1] );
	this->setZDim( cur_sz[2] );
	this->setCDim( cur_sz[3] );

	//FIXME: a potential bug is the sz_time has not been set. by PHC, 100831?

	//update 4d pointers

	switch ( this->getDatatype() )
	{
		case V3D_UINT8:
			if (!new4dpointer_v3d(data4d_uint8, this->getXDim(), this->getYDim(), this->getZDim(), this->getCDim(), this->getRawData()))
			{
				this->setError(1);
				this->deleteRawDataAndSetPointerToNull();
				return false;
			}
			data4d_virtual = (void ****)data4d_uint8;
			break;

		case V3D_UINT16:
			if (!new4dpointer_v3d(data4d_uint16, this->getXDim(), this->getYDim(), this->getZDim(), this->getCDim(), (this->getRawData())))
			{
				this->setError(1);
				this->deleteRawDataAndSetPointerToNull();
				return false;
			}
			data4d_virtual = (void ****)data4d_uint16;
			break;

		case V3D_FLOAT32:
			if (!new4dpointer_v3d(data4d_float32, this->getXDim(), this->getYDim(), this->getZDim(), this->getCDim(), (this->getRawData())))
			{
				this->setError(1);
				this->deleteRawDataAndSetPointerToNull();
				return false;
			}
			data4d_virtual = (void ****)data4d_float32;
			break;

		default:
			this->setError(1);
			this->deleteRawDataAndSetPointerToNull();
			v3d_msg("Warning: unknown data type.\n");
			return false;
			//break;
	}

	//update minmax

	if (!updateminmaxvalues())
	{
		v3d_msg("Fail to run successfully updateminmaxvalues() in proj_general_resampling()..\n", false);
		return false;
	}

	//

	curFocusX = this->getXDim()/2; //-= bpos_x+1; //begin from mid slices
	curFocusY = this->getYDim()/2; //-= bpos_y+1;
	curFocusZ = this->getZDim()/2; //-= bpos_z+1;

	//update view

	p_xy_view->deleteROI();
	p_yz_view->deleteROI();
	p_zx_view->deleteROI();

	p_mainWidget->updateDataRelatedGUI();
    p_mainWidget->setWindowTitle_Suffix("_resampled");

	//update the landmark info if there is any
	LocationSimple tmp_pt(-1,-1,-1);
	for (V3DLONG i=0;i<listLandmarks.count();i++)
	{
		tmp_pt = listLandmarks.at(i);
		tmp_pt.x /= dfactor_x;
		tmp_pt.y /= dfactor_y;
		tmp_pt.z /= dfactor_z;
		listLandmarks.replace(i, tmp_pt);
	}

	return true;
}


bool My4DImage::proj_general_resampling_landmark_only(ImageResamplingCode mycode, double target_rez, double cur_rez)
{
	double dfactor_x, dfactor_y, dfactor_z;

	if (mycode==PRS_Z_ONLY)
	{
		dfactor_x = 1; dfactor_y = 1; dfactor_z = target_rez/cur_rez;
	}
	else
	{
		switch (mycode)
		{
			case PRS_X_ONLY:
				dfactor_x = target_rez/cur_rez; dfactor_y = 1; dfactor_z = 1;
				break;
			case PRS_Y_ONLY:
				dfactor_x = 1; dfactor_y = target_rez/cur_rez; dfactor_z = 1;
				break;
			case PRS_XY_SAME:
				dfactor_x = dfactor_y = target_rez/cur_rez; dfactor_z = 1;
				break;
			case PRS_XYZ_SAME:
				dfactor_x = dfactor_y = dfactor_z = target_rez/cur_rez;
				break;
			default:
				v3d_msg("Undefined/unsupported ImageResamplingCode found. Check the program.\n");
				return false;
		}
	}

	//update the landmark info if there is any
	LocationSimple tmp_pt(-1,-1,-1);
	for (V3DLONG i=0;i<listLandmarks.count();i++)
	{
		tmp_pt = listLandmarks.at(i);
		tmp_pt.x /= dfactor_x;
		tmp_pt.y /= dfactor_y;
		tmp_pt.z /= dfactor_z;
		listLandmarks.replace(i, tmp_pt);
	}

	return true;
}

bool My4DImage::proj_general_landmark_plusminus_constant(ImageResamplingCode mycode, double cval)
{
	double offval_x, offval_y, offval_z;
	switch (mycode)
	{
		case PRS_X_ONLY:
			offval_x = cval; offval_y = offval_z = 0;
			break;
		case PRS_Y_ONLY:
			offval_x = offval_z = 0; offval_y = cval;
			break;
		case PRS_Z_ONLY:
			offval_x = offval_y = 0; offval_z = cval;
			break;
		default:
			v3d_msg("Undefined/unsupported code in proj_general_landmark_plusminus_constant() found. Check the program.\n");
			return false;
	}

	//update the landmark info if there is any
	LocationSimple tmp_pt(-1,-1,-1);
	for (V3DLONG i=0;i<listLandmarks.count();i++)
	{
		tmp_pt = listLandmarks.at(i);
		tmp_pt.x += offval_x;
		tmp_pt.y += offval_y;
		tmp_pt.z += offval_z;
		listLandmarks.replace(i, tmp_pt);
	}

	return true;
}

bool My4DImage::proj_general_projection(AxisCode myaxis, V3DLONG mincoord, V3DLONG maxcoord)
{
	if (!p_mainWidget) return false;

	if ( this->getDatatype() !=V3D_UINT8)
	{
		v3d_msg("Now supports UINT8 only in proj_general_projection().\n");
		return false;
	}

	//create a new image for projection
	V3DLONG outsz0, outsz1, outsz2, outsz3;
	switch (myaxis)
	{
		case axis_x: outsz0 = 1; outsz1 = getYDim(); outsz2 = getZDim(); outsz3 = getCDim();
			if (mincoord<0) mincoord=0; if (mincoord>=getXDim()) mincoord=getXDim()-1;
			if (maxcoord<0) maxcoord=0; if (maxcoord>=getXDim()) maxcoord=getXDim()-1;
			break;
		case axis_y: outsz0 = getXDim(); outsz1 = 1; outsz2 = getZDim(); outsz3 = getCDim();
			if (mincoord<0) mincoord=0; if (mincoord>=getYDim()) mincoord=getYDim()-1;
			if (maxcoord<0) maxcoord=0; if (maxcoord>=getYDim()) maxcoord=getYDim()-1;
			break;
		case axis_z: outsz0 = getXDim(); outsz1 = getYDim(); outsz2 = 1; outsz3 = getCDim();
			if (mincoord<0) mincoord=0; if (mincoord>=getZDim()) mincoord=getXDim()-1;
			if (maxcoord<0) maxcoord=0; if (maxcoord>=getZDim()) maxcoord=getXDim()-1;
			break;
		default:
		{
			v3d_msg("Wrong axis code. check program in proj_general_projection().\n");
			return false;
		}
	}

	V3DLONG totalbytes = outsz0*outsz1*outsz2*outsz3;
	unsigned char * outvol1d = new unsigned char [totalbytes];
	if (!outvol1d)
	{
		v3d_msg("Fail to allocate memory in proj_general_projection() for the blended image.\n");
		return false;
	}
	XFormWidget * newwin = getXWidget()->getMainControlWindow()->createMdiChild();
	newwin->newProcessedImage("blended", outvol1d, outsz0, outsz1, outsz2, outsz3, V3D_UINT8); //note that I don't support other datatype yet
	My4DImage * pProjDstImg = newwin->getImageData();

	ImagePixelType projDataType;
	unsigned char **** pProjDstImg_d4d = (unsigned char ****) pProjDstImg->getData(projDataType);
	unsigned char **** pSrcImg_d4d = data4d_uint8;

	V3DLONG i,j,k,c;
	switch (myaxis)
	{
		case axis_x:
			for (c=0; c<outsz3; c++)
			{
				for (k=0;k<outsz2;k++)
				{
					for (j=0;j<outsz1;j++)
					{
						pProjDstImg_d4d[c][k][j][0] = pSrcImg_d4d[c][k][j][mincoord];
						for (i=mincoord+1; i<=maxcoord; i++)
						{
							if (pSrcImg_d4d[c][k][j][i] > pProjDstImg_d4d[c][k][j][0])
								pProjDstImg_d4d[c][k][j][0] = pSrcImg_d4d[c][k][j][i];
						}
					}
				}
			}
			break;

		case axis_y:
			for (c=0; c<outsz3; c++)
			{
				for (k=0;k<outsz2;k++)
				{
					for (i=0;i<outsz0;i++)
					{
						pProjDstImg_d4d[c][k][0][i] = pSrcImg_d4d[c][k][mincoord][i];
						for (j=mincoord+1; j<=maxcoord; j++)
						{
							if (pSrcImg_d4d[c][k][j][i] > pProjDstImg_d4d[c][k][0][i])
								pProjDstImg_d4d[c][k][0][i] = pSrcImg_d4d[c][k][j][i];
						}
					}
				}
			}
			break;

		case axis_z:
			for (c=0; c<outsz3; c++)
			{
				for (j=0;j<outsz1;j++)
				{
					for (i=0;i<outsz0;i++)
					{
						pProjDstImg_d4d[c][0][j][i] = pSrcImg_d4d[c][mincoord][j][i];
						for (k=mincoord+1; k<=maxcoord; k++)
						{
							if (pSrcImg_d4d[c][k][j][i] > pProjDstImg_d4d[c][0][j][i])
								pProjDstImg_d4d[c][0][j][i] = pSrcImg_d4d[c][k][j][i];
						}
					}
				}
			}
			break;
	}

	newwin->show();
	newwin->getImageData()->updateViews();
	return true;
}

bool My4DImage::proj_general_blend_channels()
{
	if (!p_mainWidget) return false;

	QList <BlendingImageInfo> bList = p_mainWidget->selectBlendingImages();
	if (bList.count()<=0)
		return false;

	//create a new image and blend it using the first one
	My4DImage * pimg = bList.at(0).pimg;
	V3DLONG totalbytes = pimg->getXDim()*pimg->getYDim()*pimg->getZDim()*3;
	unsigned char * outvol1d = new unsigned char [totalbytes];
	if (!outvol1d)
	{
		v3d_msg("Fail to allocate memory in proj_general_blend_channels() for the blended image.\n");
		return false;
	}
	XFormWidget * newwin = getXWidget()->getMainControlWindow()->createMdiChild();
	newwin->newProcessedImage("blended", outvol1d, pimg->getXDim(), pimg->getYDim(), pimg->getZDim(), 3, V3D_UINT8); //note that I don't support other datatype yet
	My4DImage * pBlendDstImg = newwin->getImageData();

	proj_general_blend_channel_real(pBlendDstImg, bList.at(0).pimg, bList.at(0).channo, bList.at(0).rr, bList.at(0).gg, bList.at(0).bb, true);

	//blend the rest
	for (V3DLONG i=1;i<bList.count(); i++)
	{
		proj_general_blend_channel_real(pBlendDstImg, bList.at(i).pimg, bList.at(i).channo, bList.at(i).rr, bList.at(i).gg, bList.at(i).bb, false);
	}

	newwin->show();
	newwin->getImageData()->updateViews();
	return true;
}

bool My4DImage::proj_general_blend_channel_real(My4DImage * pBlendDstImg, My4DImage * pBlendSrcImg, V3DLONG chnoBlendSrcImg, double rr, double gg, double bb, bool b_assignVal_NoComparison)
//rr, gg, bb are the three coefficients which represent the RGB color of the chnoBlendSrcImg'th channel of pBlendSrcImg in the pBlendDstImg image.
//when b_assignVal_NoComparison  is true, then directly use the blended pBlendSrcImg to assign the value of pBlendDstImg; This is used for initialization of pBlendDstImg.
{
	if (!pBlendDstImg || !pBlendDstImg->valid() || !pBlendSrcImg || !pBlendSrcImg->valid()
		|| pBlendDstImg->getCDim()!=3 || chnoBlendSrcImg<0 || chnoBlendSrcImg>=pBlendSrcImg->getCDim()
		|| pBlendDstImg->getXDim()!=pBlendSrcImg->getXDim()
		|| pBlendDstImg->getYDim()!=pBlendSrcImg->getYDim()
		|| pBlendDstImg->getZDim()!=pBlendSrcImg->getZDim()
		|| rr<0 || rr>1 || gg<0 || gg>1 || bb<0 || bb>1)
	{
		v3d_msg("Invalid parameters for proj_general_blend_channel_real(). Do nothing.\n");
		return false;
	}

	ImagePixelType blendDstDataType, blendSrcDataType;
	unsigned char **** pBlendDstImg_data4d = (unsigned char ****) pBlendDstImg->getData(blendDstDataType);
	unsigned char **** pBlendSrcImg_data4d = (unsigned char ****) pBlendSrcImg->getData(blendSrcDataType);

	if (blendDstDataType!=V3D_UINT8 || blendSrcDataType!=V3D_UINT8)
	{
		v3d_msg("In proj_general_blend_channel_real() the datatype must be UINT8. Your data to be blended has the wrong type. Do nothing.\n");
		return false;
	}

	//get data pointers
	unsigned char *** pBlendDstImg_d3d_red = pBlendDstImg_data4d[0];
	unsigned char *** pBlendDstImg_d3d_green = pBlendDstImg_data4d[1];
	unsigned char *** pBlendDstImg_d3d_blue = pBlendDstImg_data4d[2];

	unsigned char *** pBlendSrcImg_d3d = pBlendSrcImg_data4d[chnoBlendSrcImg];

	//now blending
	V3DLONG tsz0 = pBlendDstImg->getXDim(), tsz1 = pBlendDstImg->getYDim(), tsz2 = pBlendDstImg->getZDim();
	V3DLONG i,j,k;
	double curSrcVal, curSrcVal_r, curSrcVal_g, curSrcVal_b;
	if (b_assignVal_NoComparison) //the initialization case; no comparison of value
	{
		for (k=0; k<tsz2; k++)
		{
			for (j=0; j<tsz1; j++)
			{
				for (i=0; i<tsz0; i++)
				{
					curSrcVal = pBlendSrcImg_d3d[k][j][i];

					pBlendDstImg_d3d_red[k][j][i] = rr*curSrcVal;
					pBlendDstImg_d3d_green[k][j][i] = gg*curSrcVal;
					pBlendDstImg_d3d_blue[k][j][i] = bb*curSrcVal;
				}
			}
		}
	}
	else //the normal case should use maximum value to blend
	{
		for (k=0; k<tsz2; k++)
		{
			for (j=0; j<tsz1; j++)
			{
				for (i=0; i<tsz0; i++)
				{
					curSrcVal = pBlendSrcImg_d3d[k][j][i];
					curSrcVal_r = rr*curSrcVal;
					curSrcVal_g = gg*curSrcVal;
					curSrcVal_b = bb*curSrcVal;

					if (pBlendDstImg_d3d_red[k][j][i]<curSrcVal_r) pBlendDstImg_d3d_red[k][j][i] = curSrcVal_r;
					if (pBlendDstImg_d3d_green[k][j][i]<curSrcVal_g) pBlendDstImg_d3d_green[k][j][i] = curSrcVal_g;
					if (pBlendDstImg_d3d_blue[k][j][i]<curSrcVal_b) pBlendDstImg_d3d_blue[k][j][i] = curSrcVal_b;
				}
			}
		}
	}

	//
	return true;
}



bool My4DImage::proj_general_split_channels(bool b_keepallchannels, int chno)
{
	if (!p_mainWidget) return false;
	if ( this->getDatatype() !=V3D_UINT8)
	{
		v3d_msg("Now the slpit-channels only supports 8bit data. Check your data first.");
		return false;
	}
	if (b_keepallchannels==false)
	{
		if (chno<0 || chno>=getCDim())
		{
			v3d_msg("You have specified an invalid channel in proj_general_split_channels(). Check your data/program first.\n");
		}
	}

	//create new images for different channels
	for (int c=0;c<getCDim();c++)
	{
		if (b_keepallchannels==false)
		{
			if (chno!=c)
				continue; //just by pass
		}
		QString chno_name;
		V3DLONG totalbytes = getXDim()*getYDim()*getZDim()*1;
		try
		{
			unsigned char * outvol1d = new unsigned char [totalbytes];
			if (!outvol1d)
			{
				v3d_msg("Fail to allocate memory in proj_general_split_channels() for the blended image.\n");
				return false;
			}
			XFormWidget * newwin = getXWidget()->getMainControlWindow()->createMdiChild();
			newwin->newProcessedImage(QString("channel_")+chno_name.setNum(c+1), outvol1d, getXDim(), getYDim(), getZDim(), 1, V3D_UINT8); //note that I don't support other datatype yet
			My4DImage * pDstImg = newwin->getImageData();

			unsigned char *** pDstImg_d3d = ((unsigned char ****) pDstImg->getData())[0];
			unsigned char *** pSrcImg_d3d = ((unsigned char ****) getData())[c];

			//now assign values
			V3DLONG i,j,k;
			for (k=0; k<this->getZDim(); k++)
			{
				for (j=0; j<this->getYDim(); j++)
				{
					for (i=0; i<this->getXDim(); i++)
					{
						pDstImg_d3d[k][j][i] = pSrcImg_d3d[k][j][i];
					}
				}
			}
			pDstImg->p_vmax[0] = this->p_vmax[c];
			pDstImg->p_vmin[0] = this->p_vmin[c];

			//now show

			newwin->show();
			newwin->getImageData()->updateViews(); //090504
		}
		catch (...)
		{
			v3d_msg("Fail to split due to memory overflow in proj_general_split_channels().\n");
			return false;
		}
	}

	return true;
}

bool My4DImage::proj_general_hist_display()
{
	if (!valid()){v3d_msg("Invalid data");	return false;}
	if ( this->getDatatype() !=V3D_UINT8 &&  this->getDatatype() !=V3D_UINT16)
	{
		v3d_msg("Only support convert the indexed 8/16 bits image in histogram display.\n");
		return false;
	}

	//get hist disp range is needed

	bool ok1;
	V3DLONG dmin=-1, dmax=-1;
	if(QMessageBox::Yes == QMessageBox::question (0, "", "Do you want to specify a range of pixel intensity to compute the histogram?", QMessageBox::Yes, QMessageBox::No))
	{

#if defined(USE_Qt5)
		dmin = QInputDialog::getInt(0, QString("histogram range"), QString("min intensity of the histogram range"), 0, 0, 65535, 10, &ok1);
#else
		dmin = QInputDialog::getInteger(0, QString("histogram range"), QString("min intensity of the histogram range"), 0, 0, 65535, 10, &ok1);
#endif
		if (ok1)
		{

#if defined(USE_Qt5)
			dmax = QInputDialog::getInt(0, QString("histogram range"), QString("max intensity of the histogram range (max(8bits data)=255, max(16bit data)=65535)"), 255, 0, 65535, 10, &ok1);
#else
			dmax = QInputDialog::getInteger(0, QString("histogram range"), QString("max intensity of the histogram range (max(8bits data)=255, max(16bit data)=65535)"), 255, 0, 65535, 10, &ok1);
#endif
			if (!ok1)
				return false;
		}
		else
			return false;
	}
	if (dmin>dmax) qSwap(dmin, dmax); //{V3DLONG tmp=dmin; dmin=dmax; dmax=tmp;}

	//now compute

	QVector< QVector<int> > vvec;
	QStringList labelsLT;

	V3DLONG pagesz = getTotalUnitNumberPerChannel();
	for (int c=0;c<getCDim(); c++)
	{
		V3DLONG i;

		HistogramSimple hs;
		V3DLONG minv, maxv;
		V3DLONG pos_min, pos_max;

		if ( this->getDatatype() ==V3D_UINT8)
		{
			hs.compute((this->getRawData()) + c*pagesz, pagesz);
		}
		else if ( this->getDatatype() ==V3D_UINT16)
		{
			hs.compute((unsigned short int *)((this->getRawData()) + c*pagesz*2), pagesz);
		}
		else
		{
			v3d_msg("Only support convert the indexed 8/16 bits image in histogram display.\n");
			return false;
		}

		V3DLONG *hist = hs.getHistogram();
		V3DLONG NBIN = hs.getEffectiveLength();

		V3DLONG lb = (dmin<=maxv)?qMax(dmin,V3DLONG(0)):0, ub = (dmax>=0)?qMin(dmax, NBIN-1):(NBIN-1);
		if (lb>ub) qSwap(lb,ub);

		QVector<int> vec;
		double MM = hist[lb]; for (i=1;i<ub;i++) if (MM<hist[i]) MM=hist[i];
		if (MM<=0)
		{
			printf("The vector is all 0s.\n");

			for(i=lb; i<=ub; i++)
			{
				vec.push_back(0);
			}
		}
		else
		{
			if (dmin>dmax) {V3DLONG tmp=dmin; dmin=dmax; dmax=tmp;}

			for(i=lb; i<=ub; i++)
			{
				printf("%d [%ld]\n", i, hist[i]);
				vec.push_back(int(double(hist[i])/MM*10000));
			}
		}

		vvec.append( vec );
		labelsLT.append( QString("channel %1").arg(c+1) );

		//if (hist) {delete []hist; hist=0;}
	}
	QString labelRB = (dmax>=0) ? QString("%1").arg(dmax) : QString("%1").arg("255 or upper_bnd");

	barFigureDialog *dlg = new barFigureDialog(vvec, labelsLT, labelRB, 0, QSize(400, 150), QColor(50,50,50));
	dlg->show();

	return true;
}

bool My4DImage::proj_general_linear_adjustment()
{
	if (!valid()){v3d_msg("Invalid data");	return false;}

	V3DLONG pagesz = getTotalUnitNumberPerChannel();
	V3DLONG channels = getCDim();
	//output 8-bit data
	unsigned char * outvol1d = new unsigned char [channels*pagesz];
	if (!outvol1d)
	{
		v3d_msg("Fail to allocate memory for outvol1d!\n");
		return false;
	}

	if ( this->getDatatype() ==V3D_UINT8)
	{
		//TO DO Linear Adjustment
		for (V3DLONG c=0;c<channels; c++)
		{
			V3DLONG i;
			QVector<int> vec;

			unsigned char *curp = this->getRawData();
			V3DLONG icurp = c*pagesz;
			curp += icurp;

			unsigned char maxori = 0, minori = 0;
			for(i=0; i<pagesz; i++)
			{
				if(curp[i]>maxori)
					maxori = curp[i];
				if(curp[i]<minori)
					minori = curp[i];
			}

			V3DLONG NBIN = maxori+1;
			V3DLONG totalsLUT = (channels+1)*NBIN;

			//Look Up Table
			unsigned char *LUT = new unsigned char [totalsLUT];
			if(!LUT)
			{
				v3d_msg("Error allocate the memory!\n");
				return false;
			}
			else
			{
				for(V3DLONG i=0; i<totalsLUT; i++)
					LUT[i]=0;
			}

			//declare for histogram
			V3DLONG *hist = new V3DLONG [NBIN];
			if(!hist)
			{
				v3d_msg("Error allocate the memory!\n");
				return false;
			}
			else
			{
				for(i=0; i<NBIN; i++)
					hist[i]=0;
			}
			//declare for accumulated histogram
			V3DLONG *accuhist = new V3DLONG [NBIN];
			if(!accuhist)
			{
				v3d_msg("Error allocate the memory!\n");
				return false;
			}
			else
			{
				for(i=0; i<NBIN; i++)
					accuhist[i]=0;
			}

			V3DLONG curv;
			//compute histogram
			for(i=0; i<pagesz; i++)
			{
				curv = curp[i];
				hist[curv]++;
			}
			//statistic accumulated histogram
			for(i=0; i<NBIN; i++)
			{
				for(V3DLONG j=0; j<=i; j++)
				{
					accuhist[i] += hist[j];
				}
			}
			//printf("accuhist %ld pagesz %ld i %ld NBIN %ld \n", accuhist[i-1], pagesz, i-1, NBIN);

			//compute histogram satuate 98%
			unsigned char ileft = minori, iright = ileft;
			double sflag = 0;
			while(sflag<0.98)
			{
				sflag = double(accuhist[iright++])/double(pagesz);
			}
			iright--;
			//printf("ileft %ld iright %ld sflag %lf\n", ileft, iright, sflag);

			//compute LUT
			V3DLONG ic = c*NBIN;
			for (V3DLONG i=ileft; i<=iright; i++)
			{
				LUT[ic + i] = (unsigned char)(255*double(i-ileft)/double(iright-ileft));
			}

			//output
			for (V3DLONG i=0;i<pagesz;i++)
			{
				if(curp[i]<ileft) outvol1d[icurp + i] = 0;
				else if(curp[i]>iright) outvol1d[icurp + i] = 255;
				else
				{
					V3DLONG curv = curp[i];
					outvol1d[icurp + i] = LUT[ic + curv];
				}
			}

			if (hist) {delete []hist; hist=0;}
			if (accuhist) {delete []accuhist; accuhist=0;}
		}

	}
	else if ( this->getDatatype() ==V3D_UINT16)
	{
		//TO DO Linear Adjustment
		for (V3DLONG c=0;c<channels; c++)
		{
			V3DLONG i;
			QVector<int> vec;

			V3DLONG maxori = 0, minori = 0;
			USHORTINT16 *curp = (USHORTINT16 *)this->getRawData();
			V3DLONG icurp = c*pagesz;
			curp += icurp;

			for(i=0; i<pagesz; i++)
			{
				if(curp[i]>maxori)
					maxori = curp[i];
				if(curp[i]<minori)
					minori = curp[i];
			}

			V3DLONG NBIN=maxori+1;

			V3DLONG totalsLUT = (channels+1)*NBIN;

			//Look Up Table
			unsigned char *LUT = new unsigned char [totalsLUT];
			if(!LUT)
			{
				v3d_msg("Error allocate the memory!\n");
				return false;
			}
			else
			{
				for(V3DLONG i=0; i<totalsLUT; i++)
					LUT[i]=0;
			}

			//declare for histogram
			V3DLONG *hist = new V3DLONG [NBIN];
			if(!hist)
			{
				v3d_msg("Error allocate the memory!\n");
				return false;
			}
			else
			{
				for(i=0; i<NBIN; i++)
					hist[i]=0;
			}
			//declare for accumulated histogram
			V3DLONG *accuhist = new V3DLONG [NBIN];
			if(!accuhist)
			{
				v3d_msg("Error allocate the memory!\n");
				return false;
			}
			else
			{
				for(i=0; i<NBIN; i++)
					accuhist[i]=0;
			}

			printf("%ld \n",c);

			V3DLONG curv;

			//compute histogram
			for(i=0; i<pagesz; i++)
			{
				curv = curp[i];
				hist[curv]++;
			}
			//statistic accumulated histogram
			for(i=0; i<NBIN; i++)
			{
				for(V3DLONG j=0; j<=i; j++)
				{
					accuhist[i] += hist[j];
				}
			}

			//compute histogram satuate 98%
			V3DLONG ileft = minori, iright = ileft;

			double sflag = 0;
			while(sflag<0.98)
			{
				sflag = double(accuhist[iright++])/double(pagesz);
			}
			iright--;
			//printf("ileft %ld iright %ld sflag %lf\n", ileft, iright, sflag);

			//compute LUT
			V3DLONG ic = c*NBIN;
			for (V3DLONG i=ileft; i<=iright; i++)
			{
				LUT[ic + i] = (unsigned char)(255*double(i-ileft)/double(iright-ileft));
			}

			//output
			for (V3DLONG i=0;i<pagesz;i++)
			{
				if(curp[i]<ileft) outvol1d[icurp + i] = 0;
				else if(curp[i]>iright) outvol1d[icurp + i] = 255;
				else
				{
					V3DLONG curv = curp[i];
					outvol1d[icurp + i] = LUT[ic + curv];
				}
			}

			if (hist) {delete []hist; hist=0;}
			if (accuhist) {delete []accuhist; accuhist=0;}
		}
	}

	//show in a new widget
	V3DLONG sz0 = getXDim();
	V3DLONG sz1 = getYDim();
	V3DLONG sz2 = getZDim();
	V3DLONG sz3 = getCDim();

	XFormWidget * newwin = getXWidget()->getMainControlWindow()->createMdiChild();
	newwin->newProcessedImage("Linear Adjustment Image ", outvol1d,
							  this->getXDim(), this->getYDim(), this->getZDim(), this->getCDim(), V3D_UINT8); //RGB with similarity=1 Blue (255) & similarity=0 Red (0)

	newwin->show();
	newwin->getImageData()->updateViews();


	return true;
}

bool My4DImage::proj_general_hist_equalization(unsigned char lowerbound, unsigned char higherbound)
{
	V3DLONG pagesz = getTotalUnitNumberPerChannel();
	for (int c=0;c<getCDim(); c++)
	{
		//if (!hist_eq_uint8(data1d+c*pagesz, pagesz))
		if (!hist_eq_range_uint8(this->getRawData()+c*pagesz, pagesz, lowerbound, higherbound))
		{
			v3d_msg("Error happens in proj_general_hist_equalization();\n");
			return false;
		}
	}
	return true;
}

bool My4DImage::proj_general_convertIndexedImg2RGB()
{
	if (!valid() || getCDim()!=1)
	{
		v3d_msg("Right now can only convert an image with one single channel (so that it is a indexed image) to RGB.\n");
		return false;
	}
	if (!colorMap)
	{
		v3d_msg("The colormap of the current image is unset yet. Do nothing to convert to RGB.\n");
		return false;
	}
	if ( this->getDatatype() !=V3D_UINT8 &&  this->getDatatype() !=V3D_UINT16)
	{
		v3d_msg("Only support convert the indexed 8/16 bits image to RGB. You have a different datatype. Do nothing to convert to RGB.\n");
		return false;
	}

	V3DLONG tsz0 = getXDim(), tsz1 = getYDim(), tsz2 = getZDim(), tsz3 = 3;
	V3DLONG tbytes =tsz0*tsz1*tsz2*tsz3;
	unsigned char * outvol1d = 0;
	try
	{
		outvol1d = new unsigned char [tbytes];
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in proj_general_convertIndexedImg2RGB().\n");
		return false;
	}

	XFormWidget * newwin = getXWidget()->getMainControlWindow()->createMdiChild();
	newwin->newProcessedImage("RGB", outvol1d, tsz0, tsz1, tsz2, tsz3, V3D_UINT8); //note that I don't support other datatype yet
	unsigned char **** pDstImg4d = (unsigned char **** )(newwin->getImageData()->getData());

	//now produce the RGB image using the current colormap

	ColorMap *pc = colorMap;
	int clen = pc->len;

	if ( this->getDatatype() ==V3D_UINT16)
	{
		USHORTINT16 *** p3d = data4d_uint16[0];

		V3DLONG i,j,k;
		for (k=0;k<tsz2;k++)
			for (j=0;j<tsz1;j++)
				for (i=0;i<tsz0;i++)
				{
					V3DLONG ind = V3DLONG(p3d[k][j][i]);
					if (ind>=clen) ind = ind % clen;

					pDstImg4d[0][k][j][i] = pc->map2d[ind][0];
					pDstImg4d[1][k][j][i] = pc->map2d[ind][1];
					pDstImg4d[2][k][j][i] = pc->map2d[ind][2];
				}
	}
	else //in this case datatype must be V3D_UINT8, as checked in previous sentences
	{
		unsigned char *** p3d = data4d_uint8[0];

		V3DLONG i,j,k;
		for (k=0;k<tsz2;k++)
			for (j=0;j<tsz1;j++)
				for (i=0;i<tsz0;i++)
				{
					V3DLONG ind = V3DLONG(p3d[k][j][i]);
					if (ind>=clen) ind = ind % clen;

					pDstImg4d[0][k][j][i] = pc->map2d[ind][0];
					pDstImg4d[1][k][j][i] = pc->map2d[ind][1];
					pDstImg4d[2][k][j][i] = pc->map2d[ind][2];
				}
	}

	//show the window
	newwin->show();
	newwin->getImageData()->updateViews();
	return true;
}

bool My4DImage::proj_general_scaleandconvert28bit(int lb, int ub) //lb, ub: lower bound, upper bound
{
	if (!valid())
	{
		v3d_msg("Your data is invalid in proj_general_scaleandconvert28bit().\n");
		return false;
	}

	V3DLONG k;
	for (k=0;k<getCDim();k++)
		scaleintensity(k, p_vmin[k], p_vmax[k], double(lb), double(ub));

	V3DLONG tsz0 = getXDim(), tsz1 = getYDim(), tsz2 = getZDim(), tsz3 = getCDim();
	V3DLONG tunits =tsz0*tsz1*tsz2*tsz3;
	V3DLONG tbytes = tunits;

	unsigned char * outvol1d = 0;
	try
	{
		outvol1d = new unsigned char [tbytes];
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in proj_general_scaleandconvert28bit().\n");
		return false;
	}

	//now cpy data
	V3DLONG i;
	if ( this->getDatatype() ==V3D_UINT8)
	{
		unsigned char * cur_data1d = (unsigned char *)this->getRawData();
		for (i=0;i<tunits;i++)
			outvol1d[i] = cur_data1d[i];
	}
	else if ( this->getDatatype() ==V3D_UINT16)
	{
		unsigned short int * cur_data1d = (unsigned short int *)this->getRawData();
		for (i=0;i<tunits;i++)
			outvol1d[i] = (unsigned char)(cur_data1d[i]);
	}
	else if ( this->getDatatype() ==V3D_FLOAT32)
	{
		float * cur_data1d = (float *)this->getRawData();
		for (i=0;i<tunits;i++)
			outvol1d[i] = (unsigned char)(cur_data1d[i]);
	}
	else
	{
		v3d_msg("should not get here in proj_general_scaleandconvert28bit(). Check your code/data.");
		if (outvol1d) {delete []outvol1d;outvol1d=0;}
		return false;
	}

	setNewImageData(outvol1d, tsz0, tsz1, tsz2, tsz3, V3D_UINT8);

	getXWidget()->reset(); //to force reset the color etc
	return true;
}

bool My4DImage::proj_general_scaleandconvert28bit_1percentage(double apercent) //apercent is the percentage of signal that should be upper/lower saturation
{
	if (!valid())
	{
		v3d_msg("Your data is invalid in proj_general_scaleandconvert28bit_1percentage().\n");
		return false;
	}

	if (apercent<0 || apercent>=0.5)
	{
		v3d_msg(QString("Your percentage parameter [%1] is wrong. Must be bwteen 0 and 0.5.\n").arg(apercent));
		return false;
	}

	if ( this->getDatatype() ==V3D_FLOAT32) //for float data , first rescale it to [0, 4095]
	{
		for (V3DLONG k=0;k<getCDim();k++)
		{
			scaleintensity(k, p_vmin[k], p_vmax[k], double(0), double(4095));
		}
	}

	qDebug()<< "enter 1...";

	V3DLONG k;
	for (k=0;k<getCDim();k++)
	{
		V3DLONG maxvv = ceil(p_vmax[k]+1); //this should be safe now as the potential FLOAT32 data has been rescaled
		V3DLONG i;

		qDebug() << "ch k=" << k << " maxvv=" << maxvv;

		double *hist = 0;
		try
		{
			hist = new double [maxvv];
		}
		catch (...)
		{
			qDebug() << "fail to allocate"; return false;
			v3d_msg("Fail to allocate memory in proj_general_scaleandconvert28bit_1percentage().\n");
			return false;
		}

		for (i=0;i<maxvv;i++)
		{
			hist[i] = 0;
		}

		V3DLONG channelsz = getXDim()*getYDim()*getZDim();

		//find the histogram
		if ( this->getDatatype() ==V3D_UINT8)
		{
			unsigned char * cur_data1d = (unsigned char *)this->getRawData() + k*channelsz;
			for (i=0;i<channelsz;i++)
				hist[V3DLONG(cur_data1d[i])] += 1;
		}
		else if ( this->getDatatype() ==V3D_UINT16)
		{
			unsigned short int * cur_data1d = (unsigned short int *)this->getRawData() + k*channelsz;
			for (i=0;i<channelsz;i++)
				hist[cur_data1d[i]] += 1;
		}
		else if ( this->getDatatype() ==V3D_FLOAT32)
		{
			float * cur_data1d = (float *)this->getRawData() + k*channelsz;
			for (i=0;i<channelsz;i++)
				hist[V3DLONG(cur_data1d[i])] += 1;
		}

		qDebug() << "Histogram computed.";

		//compute the CDF
		for (i=1;i<maxvv;i++)
		{
			hist[i] += hist[i-1];
		}

		for (i=0;i<maxvv;i++)
		{
			hist[i] /= hist[maxvv-1];
		}

		//now search for the intensity thresholds
		double lowerth, upperth; lowerth = upperth = 0;
		for (i=0;i<maxvv-1;i++) //not the most efficient method, but the code should be readable
		{
			if (hist[i]<apercent && hist[i+1]>apercent)
				lowerth = i;
			if (hist[i]<1-apercent && hist[i+1]>1-apercent)
				upperth = i;
		}

		v3d_msg(QString("channel=%1 lower th=%2 upper th=%3").arg(k).arg(lowerth).arg(upperth), 0);

		//real rescale of intensity
		scaleintensity(k, lowerth, upperth, double(0), double(255));

		//free space
		if (hist) {delete []hist; hist=0;}
	}

	V3DLONG tsz0 = getXDim(), tsz1 = getYDim(), tsz2 = getZDim(), tsz3 = getCDim();
	V3DLONG tunits =tsz0*tsz1*tsz2*tsz3;
	V3DLONG tbytes = tunits;
	unsigned char * outvol1d = 0;
	try
	{
		outvol1d = new unsigned char [tbytes];
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in proj_general_scaleandconvert28bit().\n");
		return false;
	}

	//now cpy data
	V3DLONG i;
	if ( this->getDatatype() ==V3D_UINT8)
	{
		unsigned char * cur_data1d = (unsigned char *)this->getRawData();
		for (i=0;i<tunits;i++)
			outvol1d[i] = cur_data1d[i];
	}
	else if ( this->getDatatype() ==V3D_UINT16)
	{
		unsigned short int * cur_data1d = (unsigned short int *)this->getRawData();
		for (i=0;i<tunits;i++)
			outvol1d[i] = (unsigned char)(cur_data1d[i]);
	}
	else if ( this->getDatatype() ==V3D_FLOAT32)
	{
		float * cur_data1d = (float *)this->getRawData();
		for (i=0;i<tunits;i++)
			outvol1d[i] = (unsigned char)(cur_data1d[i]);
	}
	else
	{
		v3d_msg("should not get here in proj_general_scaleandconvert28bit(). Check your code/data.");
		if (outvol1d) {delete []outvol1d;outvol1d=0;}
		return false;
	}

	setNewImageData(outvol1d, tsz0, tsz1, tsz2, tsz3, V3D_UINT8);

	getXWidget()->reset(); //to force reset the color etc
	return true;
}


bool My4DImage::proj_general_convert16bit_to_8bit(int shiftnbits)
//shiftnbits will be 4 is the original data is 12-bit, or 8 if the original data uses all 16 bits. Should be 0 is just discard the upper 8-bits
{
	if (!valid() ||  this->getDatatype() !=V3D_UINT16)
	{
		v3d_msg("Your data is either invalid or not 16-bit image.\n");
		return false;
	}

	V3DLONG tsz0 = getXDim(), tsz1 = getYDim(), tsz2 = getZDim(), tsz3 = getCDim();
	V3DLONG tunits =tsz0*tsz1*tsz2*tsz3;
	V3DLONG tbytes = tunits;
	unsigned char * outvol1d = 0;
	try
	{
		outvol1d = new unsigned char [tbytes];
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in proj_general_convert16bit_to_8bit().\n");
		return false;
	}

	//now cpy data
	unsigned short int * cur_data1d = (unsigned short int *)this->getRawData();

	double dn = pow(2.0, double(shiftnbits));
	V3DLONG i;
	for (i=0;i<tunits;i++)
	{
		double tmp = (double)(cur_data1d[i]) / dn;
		if (tmp>255) outvol1d[i] = 255;
		else
			outvol1d[i] = (unsigned char)(tmp);
	}
	setNewImageData(outvol1d, tsz0, tsz1, tsz2, tsz3, V3D_UINT8);

	getXWidget()->reset(); //to force reset the color etc
	return true;
}

bool My4DImage::proj_general_convert32bit_to_8bit(int shiftnbits)
{
	if (!valid() ||  this->getDatatype() !=V3D_FLOAT32)
	{
		v3d_msg("Your data is either invalid or not 32-bit image.\n");
		return false;
	}

//	return proj_general_scaleandconvert28bit(0,255);

	V3DLONG tsz0 = getXDim(), tsz1 = getYDim(), tsz2 = getZDim(), tsz3 = getCDim();
	V3DLONG tunits =tsz0*tsz1*tsz2*tsz3;
	V3DLONG tbytes = tunits;
	unsigned char * outvol1d = 0;
	try
	{
		outvol1d = new unsigned char [tbytes];
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in proj_general_convert32bit_to_8bit().\n");
		return false;
	}

	//now cpy data
	float * cur_data1d = (float *)this->getRawData();

	double dn = pow(2.0, double(shiftnbits));
	V3DLONG i;
	for (i=0;i<tunits;i++)
	{
		double tmp = (double)(cur_data1d[i]) / dn;
		if (tmp>255) outvol1d[i] = 255;
		else
			outvol1d[i] = (unsigned char)(tmp);
	}
	setNewImageData(outvol1d, tsz0, tsz1, tsz2, tsz3, V3D_UINT8);

	getXWidget()->reset(); //to force reset the color etc
	return true;
}



V3DLONG My4DImage::find_closest_control_pt(int sx, int sy, int sz, double & dmin)
{
	LocationSimple tmpLocation(0,0,0);
	int tmpx,tmpy,tmpz;
	double dx,dy,dz, dd;
	V3DLONG ind_min=-1;
	for (unsigned V3DLONG i=0;i<listLandmarks.count();i++)
	{
		tmpLocation = listLandmarks.at(i);
		tmpLocation.getCoord(tmpx,tmpy,tmpz);
		dx=tmpx-sx; dy=tmpy-sy; dz=tmpz-sz;
		dd=dx*dx+dy*dy+dz*dz;
		if (i==0) {	dmin = dd; ind_min = 0;	}
		else { if (dd<dmin){ dmin = dd;	ind_min = i;}}
	}
	return ind_min;
}

V3DLONG My4DImage::find_closest_control_pt_thres(int sx, int sy, int sz, double rr, double & dmin)
{
	LocationSimple tmpLocation(0,0,0);
	int tmpx,tmpy,tmpz;
	double dx,dy,dz, dd;
	V3DLONG ind_min=-1;
	bool b_first=true;
	for (unsigned V3DLONG i=0;i<listLandmarks.count();i++)
	{
		tmpLocation = listLandmarks.at(i);
		tmpLocation.getCoord(tmpx,tmpy,tmpz);
		if ((dx=fabs(tmpx-sx))>rr || (dy=fabs(tmpy-sy))>rr || (dz=fabs(tmpz-sz))>rr)
			continue;
		dd=dx*dx+dy*dy+dz*dz;
		if (b_first) {	dmin = dd; ind_min = i;	b_first=false;}
		else { if (dd<dmin){ dmin = dd;	ind_min = i;}}
	}
	return ind_min;
}

/*
 //080411

 void RegistrationThread::do_registration(My4DImage *targetImg, My4DImage *subjectImg)
 {
 QMutexLocker locker(&mutex);

 this->targetImg = targetImg;
 this->subjectImg = subjectImg;

 if (!isRunning())
 {
 start(LowPriority);
 }
 else
 {
 restart = true;
 condition.wakeOne();
 }
 }

 void RegistrationThread::run()
 {
 if (!targetImg || !targetImg->valid() || !subjectImg || !subjectImg->valid())
 return;

 forever {
 mutex.lock();
 targetImg->proj_alignment_warp_using_landmarks_real(subjectImg);
 mutex.unlock();

 if (abort)
 return;

 if (!restart)
 emit done_registration();

 if (!restart)
 condition.wait(&mutex);
 restart = false;
 mutex.unlock();
 }

 return;
 }

 RegistrationThread::RegistrationThread(QObject *parent)
 : QThread(parent)
 {
 restart = false;
 abort = false;
 }

 RegistrationThread::~RegistrationThread()
 {
 mutex.lock();
 abort = true;
 condition.wakeOne();
 mutex.unlock();

 wait();
 }
 */

bool pointInPolygon(double x, double y, const QPolygon & pg) //use the algorithm at http://alienryderflex.com/polygon/
{
	int i, j=pg.count()-1;
	bool  oddNodes=false;

	for (i=0; i<pg.count(); i++)
	{
		if (pg.point(i).y()<y && pg.point(j).y()>=y || pg.point(j).y()<y && pg.point(i).y()>=y)
		{
			if (pg.point(i).x()+(y-pg.point(i).y())/(pg.point(j).y()-pg.point(i).y())*(pg.point(j).x()-pg.point(i).x())<x)
			{
				oddNodes=!oddNodes;
			}
		}
		j=i;
	}

	return oddNodes;
}


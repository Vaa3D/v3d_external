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

 module_flseg.cpp : the segmentation module of V3D
 
 separated from my4dimage.cpp on 2010-08-01
 */


//#include <stdio.h>
//#include <strings.h>
//#include <math.h>

//#include <QLayout>
//#include <QPainter>
//#include <QPainterPath>
#include <QtGui>

//#include <QString>

#include "v3d_core.h"
#include "mainwindow.h"

//#include <fstream> 
//#include <iostream> 
//using namespace std;

#include "dialog_watershed_para.h"
#include "FL_levelsetSegPara_dialog.h"

#include "../basic_c_fun/volimg_proc.h"

#include "../cellseg/FL_cellSegmentation3D.h"
#include "../cellseg/FL_main_brainseg.h"
#include "../cellseg/template_matching_seg.h"
#include "template_matching_cellseg_dialog.h"

#include "../gmm/fit_gmm.h"


bool My4DImage::proj_cellseg_templatematching()
{
	if (getDatatype()!=V3D_UINT8)
	{
		v3d_msg("Cell segmentation can only deal with Unsigned Char (8-bit) data at this moment. Do nothing.");
		return false;
	}

	//compute the correlation between every local window and a target template.

	para_template_matching_cellseg segpara;

	//use dialog to select seg parameters

	para_template_matching_cellseg_dialog *p_mydlg=0;
	if (!p_mydlg) p_mydlg = new para_template_matching_cellseg_dialog(getCDim(), &segpara);

	int res = p_mydlg->exec();
	if (res!=QDialog::Accepted)
		return false;
	else
		p_mydlg->fetchData(&segpara);
	if (p_mydlg) {delete p_mydlg; p_mydlg=0;}

	//prepare input and output data

	Vol3DSimple <unsigned char> * tmp_inimg = 0;
	Vol3DSimple <USHORTINT16> * tmp_outimg = 0;

	try
	{
		tmp_inimg = new Vol3DSimple <unsigned char> (this->getXDim(), this->getYDim(), this->getZDim());
		tmp_outimg = new Vol3DSimple <USHORTINT16> (this->getXDim(), this->getYDim(), this->getZDim());
	}
	catch (...)
	{
		v3d_msg("Unable to allocate memory", "Unable to allocate memory for processing. Do nothing.");
		if (tmp_inimg) {delete tmp_inimg; tmp_inimg=0;}
		if (tmp_outimg) {delete tmp_outimg; tmp_outimg=0;}
		return false;
	}

	//do computation
	unsigned char *** tmp_inimg3d = tmp_inimg->getData3dHandle();
	unsigned char *** pCur3d = ((unsigned char ****)getData())[segpara.channo];

	{
		for (V3DLONG k=0;k<getZDim();k++)
			for (V3DLONG j=0;j<getYDim();j++)
				for (V3DLONG i=0;i<getXDim();i++)
					tmp_inimg3d[k][j][i] = pCur3d[k][j][i];
	}

	bool b_res = template_matching_seg(tmp_inimg, tmp_outimg, segpara);
	if (tmp_inimg) {delete tmp_inimg; tmp_inimg=0;} //free the space immediately for better use of memory
	if (!b_res)
	{
		v3d_msg("Fail to do the cell segmentation().\n");
	}
	else
	{
		V3DLONG new_sz0 = tmp_outimg->sz0();
		V3DLONG new_sz1 = tmp_outimg->sz1();
		V3DLONG new_sz2 = tmp_outimg->sz2();
		V3DLONG new_sz3 = 1;
		V3DLONG tunits = new_sz0*new_sz1*new_sz2*new_sz3;
		V3DLONG tbytes = tunits*sizeof(USHORTINT16);
		unsigned char * outvol1d = new unsigned char [tbytes];

		XFormWidget * newwin = getXWidget()->getMainControlWindow()->createMdiChild();
		newwin->newProcessedImage("segmented", outvol1d, new_sz0, new_sz1, new_sz2, new_sz3, V3D_UINT16); //note that I don't support other datatype yet
		USHORTINT16 * p_d1d = (USHORTINT16 *)(newwin->getImageData()->getRawData());

		USHORTINT16 * tmpImg_d1d = (USHORTINT16 *)(tmp_outimg->getData1dHandle());

		//copy result data
		for (V3DLONG k=0;k<tunits;k++)
		{
			p_d1d[k] = tmpImg_d1d[k];
		}

		//also update the min and max values
		{
			USHORTINT16 minvv,maxvv;
			V3DLONG tmppos_min, tmppos_max;
			minMaxInVector(p_d1d, tunits, tmppos_min, minvv, tmppos_max, maxvv);
			newwin->getImageData()->p_vmax[0] = maxvv;
			newwin->getImageData()->p_vmin[0] = minvv;
			printf("channel info for the segmented image max=[%5.3f] min=[%5.3f]\n", newwin->getImageData()->p_vmax[0], newwin->getImageData()->p_vmin[0]);
		}


		//display new images
		newwin->show();
		newwin->getImageData()->updateViews();

	}

	//free unneeded variables
	if (tmp_inimg) {delete tmp_inimg; tmp_inimg=0;}
	if (tmp_outimg) {delete tmp_outimg; tmp_outimg=0;}
	return true;
}

bool My4DImage::proj_cellseg_cellcounting()
{
	v3d_msg("Yu Yang now learn how to add a funciton and menu");

	if (getDatatype()!=V3D_UINT8)
	{
		v3d_msg("Cell segmentation can only deal with Unsigned Char (8-bit) data at this moment. Do nothing.");
		return false;
	}

	//compute the correlation between every local window and a target template.

	para_template_matching_cellseg segpara;

	//use dialog to select seg parameters

	para_template_matching_cellseg_dialog *p_mydlg=0;
	if (!p_mydlg) p_mydlg = new para_template_matching_cellseg_dialog(getCDim(), &segpara);

	int res = p_mydlg->exec();
	if (res!=QDialog::Accepted)
		return false;
	else
		p_mydlg->fetchData(&segpara);
	if (p_mydlg) {delete p_mydlg; p_mydlg=0;}

	//prepare input and output data

	Vol3DSimple <unsigned char> * tmp_inimg = 0;
	Vol3DSimple <USHORTINT16> * tmp_outimg = 0;

	try
	{
		tmp_inimg = new Vol3DSimple <unsigned char> (this->getXDim(), this->getYDim(), this->getZDim());
		tmp_outimg = new Vol3DSimple <USHORTINT16> (this->getXDim(), this->getYDim(), this->getZDim());
	}
	catch (...)
	{
		v3d_msg("Unable to allocate memory for processing. Do nothing.");
		if (tmp_inimg) {delete tmp_inimg; tmp_inimg=0;}
		if (tmp_outimg) {delete tmp_outimg; tmp_outimg=0;}
		return false;
	}

	//do computation
	unsigned char *** tmp_inimg3d = tmp_inimg->getData3dHandle();
	unsigned char *** pCur3d = ((unsigned char ****)getData())[segpara.channo];

	{
		for (V3DLONG k=0;k<getZDim();k++)
			for (V3DLONG j=0;j<getYDim();j++)
				for (V3DLONG i=0;i<getXDim();i++)
					tmp_inimg3d[k][j][i] = pCur3d[k][j][i];
	}

	bool b_res = template_matching_seg(tmp_inimg, tmp_outimg, segpara);
	if (tmp_inimg) {delete tmp_inimg; tmp_inimg=0;} //free the space immediately for better use of memory
	if (!b_res)
	{
		v3d_msg("Fail to do the cell segmentation().\n");
	}
	else
	{
		V3DLONG new_sz0 = tmp_outimg->sz0();
		V3DLONG new_sz1 = tmp_outimg->sz1();
		V3DLONG new_sz2 = tmp_outimg->sz2();
		V3DLONG new_sz3 = 1;
		V3DLONG tunits = new_sz0*new_sz1*new_sz2*new_sz3;
		V3DLONG tbytes = tunits*sizeof(USHORTINT16);
		unsigned char * outvol1d = new unsigned char [tbytes];

		XFormWidget * newwin = getXWidget()->getMainControlWindow()->createMdiChild();
		newwin->newProcessedImage("segmented", outvol1d, new_sz0, new_sz1, new_sz2, new_sz3, V3D_UINT16); //note that I don't support other datatype yet
		USHORTINT16 * p_d1d = (USHORTINT16 *)(newwin->getImageData()->getRawData());

		USHORTINT16 * tmpImg_d1d = (USHORTINT16 *)(tmp_outimg->getData1dHandle());

		//copy result data
		for (V3DLONG k=0;k<tunits;k++)
		{
			p_d1d[k] = tmpImg_d1d[k];
		}

		//also update the min and max values
		{
			USHORTINT16 minvv,maxvv;
			V3DLONG tmppos_min, tmppos_max;
			minMaxInVector(p_d1d, tunits, tmppos_min, minvv, tmppos_max, maxvv);
			newwin->getImageData()->p_vmax[0] = maxvv;
			newwin->getImageData()->p_vmin[0] = minvv;
			printf("channel info for the segmented image max=[%5.3f] min=[%5.3f]\n", newwin->getImageData()->p_vmax[0], newwin->getImageData()->p_vmin[0]);
		}


		//display new images
		newwin->show();
		newwin->getImageData()->updateViews();

	}

	//free unneeded variables
	if (tmp_inimg) {delete tmp_inimg; tmp_inimg=0;}
	if (tmp_outimg) {delete tmp_outimg; tmp_outimg=0;}
	return true;
}


//bool My4DImage::proj_cellseg_watershed(int seg_channel_no, int segmethod) //Fuhui's convention is 0 for shape, 1 for intensity
bool My4DImage::proj_cellseg_watershed() //081211. remove parameters. //Fuhui's convention is 0 for shape, 1 for intensity
{
	if (getDatatype()!=V3D_UINT8)
	{
		v3d_msg("Cell segmentation can only deal with Unsigned Char (8-bit) data at this moment. Do nothing.");
		return false;
	}

	segParameter segpara;

	////081211 use dialog to select seg parameters

	dialog_watershed_para *p_mydlg=0;
	if (!p_mydlg) p_mydlg = new dialog_watershed_para(&segpara, this);
	int res = p_mydlg->exec();
	if (res!=QDialog::Accepted)
		return false;
	else
		p_mydlg->fetchData(&segpara);
	if (p_mydlg) {delete p_mydlg; p_mydlg=0;}

	//do the actual computation

	// set default values
	//	segpara.channelNo = 2;
	//	//segpara.backgndLevel = 15;
	//	segpara.medianFiltWid = 2;
	//	segpara.gaussinFiltWid = 1;
	//	segpara.adaptThreWid = 10;
	//	segpara.adaptThreStp = 5;
	//	segpara.segMethod = segmethod;

	//prepare input and output data

	Vol3DSimple <unsigned char> * tmp_inimg = 0;
	Vol3DSimple <USHORTINT16> * tmp_outimg = 0;

	try
	{
		tmp_inimg = new Vol3DSimple <unsigned char> (this->getXDim(), this->getYDim(), this->getZDim());
		tmp_outimg = new Vol3DSimple <USHORTINT16> (this->getXDim(), this->getYDim(), this->getZDim());
	}
	catch (...)
	{
		v3d_msg("Unable to allocate memory for processing. Do nothing.");
		if (tmp_inimg) {delete tmp_inimg; tmp_inimg=0;}
		if (tmp_outimg) {delete tmp_outimg; tmp_outimg=0;}
		return false;
	}

	//do computation
	unsigned char *** tmp_inimg3d = tmp_inimg->getData3dHandle();
	unsigned char *** pCur3d = ((unsigned char ****)getData())[segpara.channelNo];

	{
		for (V3DLONG k=0;k<getZDim();k++)
			for (V3DLONG j=0;j<getYDim();j++)
				for (V3DLONG i=0;i<getXDim();i++)
					tmp_inimg3d[k][j][i] = pCur3d[k][j][i];
	}

	bool b_res = FL_cellseg(tmp_inimg, tmp_outimg, segpara);
	if (tmp_inimg) {delete tmp_inimg; tmp_inimg=0;} //free the space immediately for better use of memory
	if (!b_res)
	{
		v3d_msg("Fail to do the cell segmentation using FL_cellseg().\n");
	}
	else
	{
		V3DLONG new_sz0 = tmp_outimg->sz0();
		V3DLONG new_sz1 = tmp_outimg->sz1();
		V3DLONG new_sz2 = tmp_outimg->sz2();
		V3DLONG new_sz3 = 1;
		V3DLONG tunits = new_sz0*new_sz1*new_sz2*new_sz3;
		V3DLONG tbytes = tunits*sizeof(USHORTINT16);
		unsigned char * outvol1d = new unsigned char [tbytes];

		XFormWidget * newwin = getXWidget()->getMainControlWindow()->createMdiChild();
		newwin->newProcessedImage("segmented", outvol1d, new_sz0, new_sz1, new_sz2, new_sz3, V3D_UINT16); //note that I don't support other datatype yet
		USHORTINT16 * p_d1d = (USHORTINT16 *)(newwin->getImageData()->getRawData());

		USHORTINT16 * tmpImg_d1d = (USHORTINT16 *)(tmp_outimg->getData1dHandle());

		for (V3DLONG k=0;k<tunits;k++)
		{
			p_d1d[k] = tmpImg_d1d[k];
		}

		//display new images
		newwin->show();
		newwin->getImageData()->updateViews();
	}

	//free unneeded variables
	if (tmp_inimg) {delete tmp_inimg; tmp_inimg=0;}
	if (tmp_outimg) {delete tmp_outimg; tmp_outimg=0;}
	return true;
}

bool My4DImage::proj_cellseg_levelset() //090121
{
	if (getDatatype()!=V3D_UINT8)
	{
		v3d_msg("Cell segmentation can only deal with Unsigned Char (8-bit) data at this moment. Do nothing.");
		return false;
	}

	levelset_segParameter segpara;
	//
	// use dialog to select seg parameters

	FL_levelsetSegPara para_exchange;
	para_exchange.channo = segpara.channelNo;
	para_exchange.dim = segpara.dimension;
	para_exchange.startslice = 1;
	para_exchange.endslice = 2;
	para_exchange.lambda = segpara.lamda;
	para_exchange.alpha = segpara.alf;
	para_exchange.epsilon = segpara.epsilon;
	para_exchange.delta = segpara.delt;
	para_exchange.rgns.push_back(12);
	para_exchange.b_useModel = true;
	//para_exchange.modelFilename;

	FL_levelsetSegPara_dialog *p_mydlg=0;
	if (!p_mydlg) p_mydlg = new FL_levelsetSegPara_dialog(getCDim(), getZDim(), &para_exchange);
	int res = p_mydlg->exec();
	if (res!=QDialog::Accepted)
		return false;
	else
		p_mydlg->fetchData(&para_exchange);
	if (p_mydlg) {delete p_mydlg; p_mydlg=0;}


	return true;
}



bool My4DImage::proj_cellseg_GaussianFit_pos(V3DLONG posx, V3DLONG posy, V3DLONG posz, V3DLONG posc, int nGauss, bool b_isotropic)
{
//	if (datatype!=V3D_UINT8)
//	{
//		printf("Invalid datatype. Only support UINT8 in proj_cellseg_GaussianFit_pos().\n");
//		return false;
//	}
	if (posx<0 || posx>=sz0 ||
	    posy<0 || posy>=sz1 ||
	    posz<0 || posz>=sz2 ||
	    posc<0 || posc>=sz3)
	{
		v3d_msg("Invalid para in proj_cellseg_GaussianFit_pos().\n");
		return false;
	}
	if (nGauss<=0)
	{
		v3d_msg("Invalid nGauss (<=0) in proj_cellseg_GaussianFit_pos().\n");
		return false;
	}

	//set default parameters
	double photonConversionFactor = 1.0;
	int winRadius = 20;
	int clusternum = nGauss;

	Vol3DSimple <MYFLOAT> * fitImg = new Vol3DSimple <MYFLOAT> (winRadius*2+1, winRadius*2+1, winRadius*2+1);
	if (!fitImg || !fitImg->valid()) {v3d_msg("Fail to alocate memory in proj_cellseg_GaussianFit_pos().\n"); return false;}

	//first copy data to fitImg
	MYFLOAT *** fitImg_data3d = fitImg->getData3dHandle();
	V3DLONG xb=posx-winRadius,xe=posx+winRadius,yb=posy-winRadius,ye=posy+winRadius,zb=posz-winRadius,ze=posz+winRadius;
	V3DLONG i,j,k,i1,j1,k1;
	for (k=zb,k1=0;k<=ze;k++,k1++)
	{
		for (j=yb,j1=0;j<=ye;j++,j1++)
		{
			for (i=xb,i1=0;i<=xe;i++,i1++)
			{
				if (k<0 || k>=sz2 ||
				    j<0 || j>=sz1 ||
					i<0 || i>=sz0)
					fitImg_data3d[k1][j1][i1] = 0;
				else
					fitImg_data3d[k1][j1][i1] = (MYFLOAT)(this->at(i,j,k,posc));
			}
		}
	}

	//estimate the Gaussian
	GMM3D_Est * p_est = fit_gmm(fitImg, photonConversionFactor, double(winRadius), clusternum);

	//set the output
	for (i=0;i<clusternum;i++)
	{
		LocationSimple tmp_location(posx-winRadius+p_est->mean[i].e1, posy-winRadius+p_est->mean[i].e2, posz-winRadius+p_est->mean[i].e3);
		tmp_location.radius=(p_est->std[i].e1+p_est->std[i].e2+p_est->std[i].e3)/3.0; //should extend to elipse later
		tmp_location.shape = pxSphere;
		tmp_location.inputProperty = pxLocaUseful;
		this->listLandmarks.append(tmp_location);
	}

	//free memory
	if (fitImg) {delete fitImg; fitImg=0;}
	if (p_est) {delete p_est; p_est=0;}
	return true;
}



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

 module_jba.cpp:
 separate from my4dimage.cpp on 2010-08-01
 */


#include <stdio.h>
#include <math.h>

#include <QtGui>

#include "v3d_core.h"
#include "mainwindow.h"

#include <fstream> 
#include <iostream> 
using namespace std;

#include "../jba/c++/jba_mainfunc.h"
#include "../jba/c++/jba_match_landmarks.h"
#include "../jba/c++/histeq.h"
#include "../jba/c++/jba_affine_xform.h"

#include "../lobeseg/lobeseg.h"

//#include "../basic_c_fun/volimg_proc.h"

#include "../cellseg/template_matching_seg.h"
#include "template_matching_cellseg_dialog.h"


///////////////////////////////////////////////////////////////////////////////////////////////////




bool My4DImage::proj_alignment_seed_grid(int kch)
{
	//find the mean and std of the entire image
	if (!valid() || datatype!=V3D_UINT8 || kch<0 || kch>=getCDim())
	{
		v3d_msg("Now only support UINT8 type of data. Your data is this type and thus do nothing.\n");
		return false;
	}

    ImagePixelType dtmp;
	unsigned char *** inimg_data3d = ((unsigned char ****)getData(dtmp))[kch];
	unsigned char * inimg_data1d = ((unsigned char ****)getData(dtmp))[kch][0][0];

	double mean_val=0, std_val=0;
    data_array_mean_and_std(inimg_data1d, getXDim()*getYDim()*getZDim(), mean_val, std_val);
	double Kfactor = 0.5;
	double th_foreground = mean_val + Kfactor*std_val;
	printf("mean_val=%5.3f \t std_val=%5.3f\n", mean_val, std_val);

	//now compute

	V3DLONG i,j,k;
	vector <LocationSimple> xypos;
	LocationSimple cur_pos(-1,-1,-1);

	double xstep=(double)getXDim()/8;
	double ystep=(double)getYDim()/8;
	double zstep=(double)getZDim()/8;

	listLandmarks.erase(listLandmarks.begin(), listLandmarks.end());
	listLocationRelationship.erase(listLocationRelationship.begin(), listLocationRelationship.end());
	for (k=0;k<V3DLONG(floor(getZDim()/zstep));k++)
	{
		for (j=0;j<V3DLONG(floor(getYDim()/ystep));j++)
		{
			for (i=0;i<V3DLONG(floor(getXDim()/xstep));i++)
			{
				if (inimg_data3d[V3DLONG(floor(k*zstep))][V3DLONG(floor(j*ystep))][V3DLONG(floor(i*xstep))]>th_foreground)
				{
					cur_pos.x = V3DLONG(floor(i*xstep));
					cur_pos.y = V3DLONG(floor(j*ystep));
					cur_pos.z = V3DLONG(floor(k*zstep));
					cur_pos.inputProperty = pxLocaUnsure;
					listLandmarks.push_back(cur_pos);
				}
			}
		}
	}

	//update view
	setFocusZ(getZDim()/2);
	setFocusX(getXDim()/2);
	setFocusY(getYDim()/2);

	updateViews();

	return true;
}



bool My4DImage::proj_alignment_seed_curvature(int kch)
{
	return true;
}

bool My4DImage::proj_alignment_seed_gradient(int kch)
{
	return true;
}

bool My4DImage::proj_alignment_seed_random(int kch, double Kfactor, V3DLONG KK)
{
	//find the mean and std of the entire image
	if (!valid() || datatype!=V3D_UINT8 || kch<0 || kch>=getCDim())
	{
		v3d_msg("Now only support UINT8 type of data. Your data is not this type and thus do nothing.\n");
		return false;
	}

    ImagePixelType dtmp;
	unsigned char *** inimg_data3d = ((unsigned char ****)getData(dtmp))[kch];
	unsigned char * inimg_data1d = ((unsigned char ****)getData(dtmp))[kch][0][0];

	double mean_val=0, std_val=0;
    data_array_mean_and_std(inimg_data1d, getXDim()*getYDim()*getZDim(), mean_val, std_val);
	double th_foreground = mean_val + Kfactor*std_val;
	printf("mean_val=%5.3f \t std_val=%5.3f\n", mean_val, std_val);

	//now compute

	V3DLONG i,j,k;
	vector <LocationSimple> xypos;
	LocationSimple cur_pos(-1,-1,-1);
	for (k=0;k<getZDim();k++)
	{
		for (j=0;j<getYDim();j++)
		{
			for (i=0;i<getXDim();i++)
			{
				if (inimg_data3d[k][j][i]>th_foreground)
				{
					cur_pos.x = i;
					cur_pos.y = j;
					cur_pos.z = k;
					cur_pos.inputProperty = pxLocaUnsure;
					xypos.push_back(cur_pos);
				}
			}
		}
	}
	V3DLONG foreground_pixelnum = xypos.size();
	if (foreground_pixelnum<=0)
	{
		v3d_msg("You either choose an empty channel (all 0), or sth wrong. Do nothing.\n");
		return false;
	}

	//

	//double TH=0.1;
	//V3DLONG KK=100;

    //randomly initilize the control points

	listLandmarks.erase(listLandmarks.begin(), listLandmarks.end());
	listLocationRelationship.erase(listLocationRelationship.begin(), listLocationRelationship.end());

	if (foreground_pixelnum<KK) KK=foreground_pixelnum/5;
	for (i=0;i<KK;i++)
	{
#ifdef Q_OS_DARWIN
		V3DLONG tmppos=(random()%foreground_pixelnum);
#else
#ifdef Q_OS_WIN32
		V3DLONG tmppos=((V3DLONG(rand())*V3DLONG(rand()))%foreground_pixelnum); //080325, for compatability to Windows
#else
		V3DLONG tmppos=(random()%foreground_pixelnum); //for redhat 081119
#endif
#endif
		listLandmarks.append(xypos.at(tmppos));
	}

	setFocusZ(getZDim()/2);
	setFocusX(getXDim()/2);
	setFocusY(getYDim()/2);

	//update view
	updateViews();

	return true;
}

bool My4DImage::proj_alignment_seed_file(QString curFile)
{
	if (!valid())
	{
		v3d_msg("You don't have the image data ready, - thus unable to import landmark. Do nothing.\n");
		return false;
	}

	if (curFile.isEmpty()) //note that I used isEmpty() instead of isNull
		return false;

	FILE * fp = fopen(curFile.toAscii(), "r");
	if (!fp)
	{
		v3d_msg("Could not open the file to load the landmark points.\n");
		return false;
	}
	else
	{
		fclose(fp); //since I will open the file and close it in the function below, thus close it now
	}

	QList <LocationSimple> tmpList = readPosFile_usingMarkerCode(curFile.toAscii());
	if (tmpList.count()<=0)
	{
		v3d_msg("Did not find any valid row/record of the coordinates. Thus do not overwrite the current landmarks if they exist.\n");
		return false;
	}

	listLandmarks.erase(listLandmarks.begin(), listLandmarks.end());
	listLocationRelationship.erase(listLocationRelationship.begin(), listLocationRelationship.end());

	for (int i=0;i<tmpList.count(); i++)
	{
		listLandmarks.append(tmpList.at(i));
	}

	//update the related views
	updateViews();

	return true;
}


bool My4DImage::proj_alignment_global()
{
	if (datatype!=V3D_UINT8)
	{
		v3d_msg("Now the alignment program only supports 8bit data. Check your data first.");
		return false;
	}
	if (!p_mainWidget) return false;
	My4DImage * pSubjectImg = p_mainWidget->selectSubjectImage();
	if (!pSubjectImg) return false;

	return proj_alignment_global_real(pSubjectImg);
}

bool My4DImage::proj_alignment_global_real(My4DImage * pSubjectImg)
{
	if (!pSubjectImg) return false;
	if (datatype!=V3D_UINT8 || pSubjectImg->getDatatype()!=V3D_UINT8)
	{
		v3d_msg("Now the alignment program only supports 8bit data. Check your data first.");
		return false;
	}

	//create the necessary paarmeters data structure for warping

	BasicWarpParameter bwp;
	Warp3D * my_warp = 0;
	V3DLONG sz_target[4], sz_subject[4];
	sz_target[0] = getXDim(); sz_target[1] = getYDim(); sz_target[2] = getZDim(); sz_target[3] = getCDim();
	sz_subject[0] = pSubjectImg->getXDim(); sz_subject[1] = pSubjectImg->getYDim(); sz_subject[2] = pSubjectImg->getZDim(); sz_subject[3] = pSubjectImg->getCDim();
	int channelNo_target=0, channelNo_subject=0;
	my_warp = new Warp3D((const unsigned char *)this->getRawData(), sz_target, channelNo_target,
						 (const unsigned char *)pSubjectImg->getRawData(), sz_subject, channelNo_subject, pSubjectImg->getDatatype(),
						 string(getFileName()), string(pSubjectImg->getFileName()), string("tmp_global.raw"));
	if (!my_warp)
	{
		v3d_msg("Fail to initialize the warping class in global alignment. Do nothing.\n");
		return false;
	}

	//now compute DF

	Vol3DSimple <DisplaceFieldF3D> * cur_df = 0;
	cur_df = my_warp->do_global_shift_transform(bwp);
	if (!cur_df || !cur_df->valid())
	{
		v3d_msg("Fail to generate the displacement field in global alignment. Do nothing.\n");
		return false;
	}

	//finally do warping
	bool res=my_warp->applyDFtoChannel((unsigned char *)pSubjectImg->getRawData(), sz_subject, -1, 1, my_warp->get_spos_subject(), cur_df);

	//free space
	if (cur_df) {delete cur_df; cur_df=0;}
	if (my_warp) {delete my_warp; my_warp=0;}
	return res;
}

bool My4DImage::proj_alignment_affine_matching_landmarks()
{
	if (listLandmarks.size()<4)
	{
		v3d_msg("The current image has less than 4 landmarks defined. To derive an 3D affine transform you will need at least 4 landmarks which are not on the same plane.");
		return false;
	}

	if (datatype!=V3D_UINT8)
	{
		v3d_msg("Now the alignment program only supports 8bit data. Check your data first.");
		return false;
	}
	if (!p_mainWidget) return false;
	My4DImage * pSubjectImg = p_mainWidget->selectSubjectImage();
	if (!pSubjectImg) return false;

	return proj_alignment_affine_matching_landmarks_real(pSubjectImg);
}

bool My4DImage::proj_alignment_affine_matching_landmarks_real(My4DImage * pSubjectImg)
{
	if (!pSubjectImg) return false;
	if (datatype!=V3D_UINT8 || pSubjectImg->getDatatype()!=V3D_UINT8)
	{
		v3d_msg("Now the alignment program only supports 8bit data. Check your data first.");
		return false;
	}

	//verify the landmarks pairs are valid
	if (pSubjectImg->listLandmarks.size()<4 || listLandmarks.size()<4)
	{
		v3d_msg("The subject or target image has less than 4 landmarks defined. To derive an 3D affine transform you will need at least 4 landmarks which are not on the same plane.");
		return false;
	}
	if (pSubjectImg->listLandmarks.size()!=listLandmarks.size())
	{
		v3d_msg("The subject and target image MUST have the same number of landmarks, and they don't. Check your data.");
		return false;
	}

	//produce the landmark data structures and find the affine transform matrix
	vector <Coord3D_JBA> matchTargetPos;
	vector <Coord3D_JBA> matchSubjectPos;
	int i;
	Coord3D_JBA tmp_pt;
	LocationSimple tmp_location(-1,-1,-1);
	for(i=0;i<listLandmarks.size();i++)
	{
		//for target
		tmp_location = listLandmarks.at(i);
		tmp_pt.x = tmp_location.x-1 - getXDim()/2; //note -1 should be needed
		tmp_pt.y = tmp_location.y-1 - getYDim()/2;
		tmp_pt.z = tmp_location.z-1 - getZDim()/2;

		matchTargetPos.push_back(tmp_pt);

		//for subject
		tmp_location = pSubjectImg->listLandmarks.at(i);
		tmp_pt.x = tmp_location.x-1 - pSubjectImg->getXDim()/2; //note -1 should be needed
		tmp_pt.y = tmp_location.y-1 - pSubjectImg->getYDim()/2;
		tmp_pt.z = tmp_location.z-1 - pSubjectImg->getZDim()/2;

		matchSubjectPos.push_back(tmp_pt);
	}

	WarpParameterAffine3D wpf_target2subject, wpf_subject2target;
	try
	{
		if (!compute_affine_xform_parameters_from_landmarks(matchSubjectPos, matchTargetPos, &wpf_target2subject, false)) //last para set to "false", - no automatic recenter
		{
			v3d_msg("Encounter a problem in computing the affine transform parameters. Do nothing.");
			return false;
		}
		if (!compute_affine_xform_parameters_from_landmarks(matchTargetPos, matchSubjectPos, &wpf_subject2target, false)) //last para set to "false", - no automatic recenter
		{
			v3d_msg("Encounter a problem in computing the affine transform parameters. Do nothing.");
			return false;
		}
	}
	catch(...)
	{
		v3d_msg("Detect an error in compute_affine_xform_parameters_from_landmarks()\n");
		return false;
	}

	printf("affine matrix:\n%5.3f\t%5.3f\t%5.3f\t%5.3f\n%5.3f\t%5.3f\t%5.3f\t%5.3f\n%5.3f\t%5.3f\t%5.3f\t%5.3f\n",
		   wpf_target2subject.mxx,wpf_target2subject.mxy,wpf_target2subject.mxz,wpf_target2subject.sx,
		   wpf_target2subject.myx,wpf_target2subject.myy,wpf_target2subject.myz,wpf_target2subject.sy,
		   wpf_target2subject.mzx,wpf_target2subject.mzy,wpf_target2subject.mzz,wpf_target2subject.sz);

	//create the necessary paarmeters data structure for warping

	V3DLONG img0_sz_subject[4], startpos_subject[3];
	img0_sz_subject[0] = pSubjectImg->getXDim();
	img0_sz_subject[1] = pSubjectImg->getYDim();
	img0_sz_subject[2] = pSubjectImg->getZDim();
	img0_sz_subject[3] = pSubjectImg->getCDim();
	startpos_subject[0] = startpos_subject[1] = startpos_subject[2] = 0;

	//now compute DF

	Vol3DSimple <DisplaceFieldF3D> * cur_df = 0;
	cur_df = get_DF_of_affine_warp(img0_sz_subject[0], img0_sz_subject[1], img0_sz_subject[2], &wpf_target2subject);
	if (!cur_df || !cur_df->valid())
	{
		v3d_msg("Fail to generate the displacement field in global affine alignment based on landmarks. Do nothing.\n");
		return false;
	}

	//finally do warping
	bool res = applyDFtoChannel_simple(pSubjectImg->getRawData(),
									   img0_sz_subject,
									   -1, // if channelNo0_ref<0 then apply to all channels
									   1, //datatype
									   startpos_subject,
									   cur_df);

	//now also transform the landmark
	for(i=0;i<listLandmarks.size();i++)
	{
		//for subject
		tmp_location = pSubjectImg->listLandmarks.at(i);
		tmp_pt.x = tmp_location.x-1 - pSubjectImg->getXDim()/2; //note -1 should be needed
		tmp_pt.y = tmp_location.y-1 - pSubjectImg->getYDim()/2;
		tmp_pt.z = tmp_location.z-1 - pSubjectImg->getZDim()/2;

		tmp_pt = matchSubjectPos.at(i);
		tmp_location.x = wpf_subject2target.mxx * tmp_pt.x + wpf_subject2target.mxy * tmp_pt.y + wpf_subject2target.mxz * tmp_pt.z + wpf_subject2target.sx + this->getXDim()/2 + 1;
		tmp_location.y = wpf_subject2target.myx * tmp_pt.x + wpf_subject2target.myy * tmp_pt.y + wpf_subject2target.myz * tmp_pt.z + wpf_subject2target.sy + this->getYDim()/2 + 1;
		tmp_location.z = wpf_subject2target.mzx * tmp_pt.x + wpf_subject2target.mzy * tmp_pt.y + wpf_subject2target.mzz * tmp_pt.z + wpf_subject2target.sz + this->getZDim()/2 + 1;
		pSubjectImg->listLandmarks.replace(i, tmp_location);
	}


	//free space
	if (cur_df) {delete cur_df; cur_df=0;}
	//if (my_warp) {delete my_warp; my_warp=0;}
	return res;
}

bool My4DImage::proj_alignment_flybrain_lobeseg()
{
	if (getCDim()<3)
	{
		v3d_msg("To do the Optical Lobe segmentation function, you must have a 3rd channel which will be used to store the segmentation mask. Check your data first.");
		return false;
	}
	if (datatype!=V3D_UINT8)
	{
		v3d_msg("Only UINT8 is supported for lobeseg function. Check your data first.\n");
		return false;
	}

	V3DLONG sz_target[4];
	sz_target[0] = getXDim(); sz_target[1] = getYDim(); sz_target[2] = getZDim(); sz_target[3] = getCDim();

	if(!do_lobeseg_bdbminus(getRawData(), sz_target, getRawData(), 0, 2, getXWidget()->getMainControlWindow()->flybrain_lobeseg_para))
	{
		v3d_msg("Fail to segment the optical lobes of the current image.\n");
		return false;
	}

	updateViews();
	getXWidget()->update();

	return true;
}

bool My4DImage::proj_general_maskBlue2Zero()
{
	if (getCDim()<3)
	{
		v3d_msg("To do the Optical Lobe segmentation function, you must have a 3rd channel which will be used to store the segmentation mask. Check your data first.");
		return false;
	}
	if (datatype!=V3D_UINT8)
	{
		v3d_msg("Only UINT8 is supported. Check your data first.");
		return false;
	}

	for (V3DLONG k=0;k<sz2;k++)
		for (V3DLONG j=0;j<sz1;j++)
			for (V3DLONG i=0;i<sz0;i++)
			{
				if (data4d_uint8[2][k][j][i]>0) //any nonzero is regarded as "blue"
				{
					for (int c=0;c<sz3;c++)
						data4d_uint8[c][k][j][i] = 0;
				}
			}

	updateViews();
	getXWidget()->update();

	return true;
}

bool My4DImage::proj_alignment_matching_point()
{
	if (datatype!=V3D_UINT8)
	{
		v3d_msg("Now the alignment program only supports 8bit data. Check your data first.");
		return false;
	}
	if (!p_mainWidget) return false;
	My4DImage * pSubjectImg = p_mainWidget->selectSubjectImage();
	if (!pSubjectImg) return false;

	return proj_alignment_matching_point_real(pSubjectImg);
}

bool My4DImage::proj_alignment_matching_point_real(My4DImage * pSubjectImg)
{
	if (!pSubjectImg) return false;
	if (datatype!=V3D_UINT8 || pSubjectImg->getDatatype()!=V3D_UINT8)
	{
		v3d_msg("Now the alignment program only supports 8bit data. Check your data first.");
		return false;
	}

	//prepare the data
	//#define PHC_CUR_TYPE unsigned char
#define PHC_CUR_TYPE MYFLOAT_JBA

	vector<Coord3D_JBA> matchTargetPos;
    vector<Coord3D_JBA> matchSubjectPos;
    Vol3DSimple<PHC_CUR_TYPE> * img_target = 0;
    Vol3DSimple<PHC_CUR_TYPE> * img_subject = 0;
	PointMatchScore matchScore;

	V3DLONG i,j,k;
	Coord3D_JBA tmp_pt;
	LocationSimple tmp_location(-1,-1,-1);
	for(i=0;i<listLandmarks.count();i++)
	{
		tmp_location = listLandmarks.at(i);
		//		if (tmp_location.howUseful()==pxLocaUseful)
		//		{
		tmp_pt.x = tmp_location.x-1; //note -1 should be needed
		tmp_pt.y = tmp_location.y-1;
		tmp_pt.z = tmp_location.z-1;

		matchTargetPos.push_back(tmp_pt);
		//		}
	}

	int targetRefCh=0, subjectRefCh=0;
	try
	{
		img_target = new Vol3DSimple<PHC_CUR_TYPE> (getXDim(), getYDim(), getZDim());
		img_subject = new Vol3DSimple<PHC_CUR_TYPE> (pSubjectImg->getXDim(), pSubjectImg->getYDim(), pSubjectImg->getZDim());
	}
	catch (...)
	{
		if (!img_target || !img_target->valid() || !img_subject || !img_subject->valid())
		{
			v3d_msg("Fail to allocate memory in proj_alignment_matching_point().\n");
			if (img_target) {delete img_target; img_target=0;}
			if (img_subject) {delete img_subject; img_subject=0;}
			return false;
		}
	}

	unsigned char *** src3d =0;

	src3d = data4d_uint8[targetRefCh];
	PHC_CUR_TYPE *** p3d_target = img_target->getData3dHandle();
	for (k=0;k<getZDim();k++)
	{
		for (j=0;j<getYDim();j++)
		{
			for (i=0;i<getXDim();i++)
			{
				p3d_target[k][j][i] = MYFLOAT_JBA(src3d[k][j][i])/255.0;
			}
		}
	}

	subjectRefCh = targetRefCh;
	src3d = pSubjectImg->data4d_uint8[subjectRefCh];
	PHC_CUR_TYPE *** p3d_subject = img_subject->getData3dHandle();
	for (k=0;k<pSubjectImg->getZDim();k++)
	{
		for (j=0;j<pSubjectImg->getYDim();j++)
		{
			for (i=0;i<pSubjectImg->getXDim();i++)
			{
				p3d_subject[k][j][i] = MYFLOAT_JBA(src3d[k][j][i])/255.0;
			}
		}
	}

	//do the matching

	vector<Coord3D_JBA> priorTargetPos = matchTargetPos;
	BasicWarpParameter bwp;
	bwp.method_match_landmarks = (PointMatchMethodType)getXWidget()->getMainControlWindow()->global_setting.GPara_landmarkMatchingMethod; //081012
	//bool res = detectBestMatchingCpt_virtual(matchTargetPos, matchSubjectPos, img_target, img_subject, matchScore, priorTargetPos, (PointMatchMethodType)getMainWidget()->getMainControlWindow()->global_setting.GPara_landmarkMatchingMethod);
	bool res = detectBestMatchingCpt_virtual(matchTargetPos, matchSubjectPos, img_target, img_subject, matchScore, priorTargetPos, bwp);
	if (!res) return false;

	//update the subject image landmarks

	if (img_target) {delete img_target; img_target=0;}
	if (img_subject) {delete img_subject; img_subject=0;}

	pSubjectImg->listLandmarks.clear(); //erase(pSubjectImg->listLandmarks.begin(), pSubjectImg->listLandmarks.end());
	pSubjectImg->listLocationRelationship.clear(); //erase(pSubjectImg->listLocationRelationship.begin(), pSubjectImg->listLocationRelationship.end());

	for(i=0;i<V3DLONG(matchSubjectPos.size());i++)
	{
		tmp_pt = matchSubjectPos.at(i);

		tmp_location.x = tmp_pt.x+1; //note +1 should be needed
		tmp_location.y = tmp_pt.y+1;
		tmp_location.z = tmp_pt.z+1;
		tmp_location.inputProperty = pxLocaUseful; //always set this as useful for convenience

		pSubjectImg->listLandmarks.append(tmp_location);
	}

	//also update the target list
	listLandmarks.clear();
	listLocationRelationship.clear();

	for(i=0;i<V3DLONG(matchTargetPos.size());i++)
	{
		tmp_pt = matchTargetPos.at(i);

		tmp_location.x = tmp_pt.x+1; //note +1 should be needed
		tmp_location.y = tmp_pt.y+1;
		tmp_location.z = tmp_pt.z+1;
		tmp_location.inputProperty = pxLocaUseful; //always set this as useful for convenience

		listLandmarks.append(tmp_location);
	}

	return true;
}

bool My4DImage::proj_alignment_matching_1single_pt(int markerIndex_target)
{
	if (datatype!=V3D_UINT8)
	{
		v3d_msg("Now the alignment program only supports 8bit data. Check your data first.");
		return false;
	}
	if (!p_mainWidget) return false;
	My4DImage * pSubjectImg = p_mainWidget->selectSubjectImage();
	int markerIndex_subject=-1;
	if (!pSubjectImg)
		return false;
	else
	{
		if (pSubjectImg->listLandmarks.size()>0)
		{
			bool ok1;
			markerIndex_subject = QInputDialog::getInteger(0, "landmark index for the subject image",
														   "which landmark in the subject image you'd like refine the matching? "
														   "(enter negative or 0 value to start a search using the 3D coordinate of the target landmark, - which needs target and subject images have the same size and also globally aligned):",
															0, -1, pSubjectImg->listLandmarks.count(), 1, &ok1);
			if (ok1)
				markerIndex_subject -= 1;
		}
		else
		{
			bool b_defineLandmark;
			if(QMessageBox::Yes == QMessageBox::question (0, "", "You do not have any landmark... Do you want to define landmark first and search around it? "
														  "If you choose NO, then the search will use the 3D coordinate of the target landmark; in this case target and subject images should have the same size and also globally aligned.",
														  QMessageBox::Yes, QMessageBox::No))
			{
				return false; //then allow a user to define a marker first
			}
		}
	}

	return proj_alignment_matching_1single_pt_real(pSubjectImg, markerIndex_target, markerIndex_subject);
}

bool My4DImage::proj_alignment_matching_1single_pt_real(My4DImage * pSubjectImg, int markerIndex_target, int markerIndex_subject)
{
	if (!pSubjectImg) return false;
	if (datatype!=V3D_UINT8 || pSubjectImg->getDatatype()!=V3D_UINT8)
	{
		v3d_msg("Now the alignment program only supports 8bit data. Check your data first.\n");
		return false;
	}

	//prepare the data
	if (markerIndex_target<0 || markerIndex_target>=listLandmarks.count())
	{
		v3d_msg("The specified landmark index [%d] is out of the effective range.", markerIndex_target);
		return false;
	}

	vector<Coord3D_JBA> matchTargetPos;
    vector<Coord3D_JBA> matchSubjectPos;
    Vol3DSimple<MYFLOAT_JBA> * img_target = 0;
    Vol3DSimple<MYFLOAT_JBA> * img_subject = 0;
	PointMatchScore matchScore;

	V3DLONG i,j,k;
	Coord3D_JBA tmp_pt;
	LocationSimple tmp_location(-1,-1,-1);

	tmp_location = listLandmarks.at(markerIndex_target);
	tmp_pt.x = tmp_location.x-1; //note -1 should be needed
	tmp_pt.y = tmp_location.y-1;
	tmp_pt.z = tmp_location.z-1;
	matchTargetPos.push_back(tmp_pt);

	int targetRefCh=0, subjectRefCh=0;
	try
	{
		img_target = new Vol3DSimple<MYFLOAT_JBA> (getXDim(), getYDim(), getZDim());
		img_subject = new Vol3DSimple<MYFLOAT_JBA> (pSubjectImg->getXDim(), pSubjectImg->getYDim(), pSubjectImg->getZDim());
	}
	catch (...)
	{
		if (!img_target || !img_target->valid() || !img_subject || !img_subject->valid())
		{
			v3d_msg("Fail to allocate memory in proj_alignment_matching_point().\n");
			if (img_target) {delete img_target; img_target=0;}
			if (img_subject) {delete img_subject; img_subject=0;}
			return false;
		}
	}

	unsigned char *** src3d =0;

	src3d = data4d_uint8[targetRefCh];
	MYFLOAT_JBA *** p3d_target = img_target->getData3dHandle();
	for (k=0;k<getZDim();k++)
	{
		for (j=0;j<getYDim();j++)
		{
			for (i=0;i<getXDim();i++)
			{
				p3d_target[k][j][i] = MYFLOAT_JBA(src3d[k][j][i])/255.0;
			}
		}
	}

	subjectRefCh = targetRefCh;
	src3d = pSubjectImg->data4d_uint8[subjectRefCh];
	MYFLOAT_JBA *** p3d_subject = img_subject->getData3dHandle();
	for (k=0;k<pSubjectImg->getZDim();k++)
	{
		for (j=0;j<pSubjectImg->getYDim();j++)
		{
			for (i=0;i<pSubjectImg->getXDim();i++)
			{
				p3d_subject[k][j][i] = MYFLOAT_JBA(src3d[k][j][i])/255.0;
			}
		}
	}

	//do the matching

	vector<Coord3D_JBA> priorTargetPos = matchTargetPos;
	BasicWarpParameter bwp;

	if (markerIndex_subject>=0) //090303
	{
		bwp.b_search_around_preset_subject_pos = true;
		tmp_pt.x = pSubjectImg->listLandmarks.at(markerIndex_subject).x-1;
		tmp_pt.y = pSubjectImg->listLandmarks.at(markerIndex_subject).y-1;
		tmp_pt.z = pSubjectImg->listLandmarks.at(markerIndex_subject).z-1;
		matchSubjectPos.push_back(tmp_pt);
	}
	bwp.method_match_landmarks = (PointMatchMethodType)getXWidget()->getMainControlWindow()->global_setting.GPara_landmarkMatchingMethod; //081012
	bool res = detectBestMatchingCpt(matchTargetPos, matchSubjectPos, img_target, img_subject, matchScore, priorTargetPos, 0, 0, bwp);
	if (!res) return false;


	//update the subject image landmarks

	if (img_target) {delete img_target; img_target=0;}
	if (img_subject) {delete img_subject; img_subject=0;}

	//pSubjectImg->listLandmarks.clear();
	//pSubjectImg->listLocationRelationship.clear();

	 //in this case, matchSubjectPos.size() must be 1
	{
		tmp_pt = matchSubjectPos.at(0);

		tmp_location.x = tmp_pt.x+1; //note +1 should be needed
		tmp_location.y = tmp_pt.y+1;
		tmp_location.z = tmp_pt.z+1;
		tmp_location.inputProperty = pxLocaUseful; //always set this as useful for convenience

		if (markerIndex_subject>=0)
			pSubjectImg->listLandmarks.replace(markerIndex_subject, tmp_location);
		else
			pSubjectImg->listLandmarks.append(tmp_location);
	}

	//note that for the matching of 1 single pt, do not update the target list

	return true;
}


bool My4DImage::proj_alignment_warp_using_landmarks_real(My4DImage * pSubjectImg)
{
	if (!pSubjectImg) return false;
	if (datatype!=V3D_UINT8 || pSubjectImg->getDatatype()!=V3D_UINT8)
	{
		v3d_msg("Now the alignment program only supports 8bit data. Check your data first.\n");
		return false;
	}

	//if target has N1 landmarks and subject has N2, then use the first min(N1,N2) landmarks as the corresponding ones
	int n_target, n_subject, n_warping;
	n_target = listLandmarks.count();
	n_subject = pSubjectImg->listLandmarks.count();
	n_warping = (n_target<n_subject)?n_target:n_subject;
	if (n_warping<=0)
	{
		v3d_msg("at least one image does not have any landmark defined. Do nothing.\n");
		return false;
	}

	// first prepare the paired series of control points

	bool b_useCornerConstraint=false;
	if (n_warping<=2) b_useCornerConstraint=true;

	vector <Coord3D_JBA> matchTargetPos;
	vector <Coord3D_JBA> matchSubjectPos;

	Coord3D_JBA tmp_pt;
	LocationSimple tmp_location(-1,-1,-1);
	for(int i=0;i<n_warping;i++)
	{
		//for target
		tmp_location = listLandmarks.at(i);
		tmp_pt.x = tmp_location.x-1; //note -1 should be needed
		tmp_pt.y = tmp_location.y-1;
		tmp_pt.z = tmp_location.z-1;

		matchTargetPos.push_back(tmp_pt);

		//for subject
		tmp_location = pSubjectImg->listLandmarks.at(i);
		tmp_pt.x = tmp_location.x-1; //note -1 should be needed
		tmp_pt.y = tmp_location.y-1;
		tmp_pt.z = tmp_location.z-1;

		matchSubjectPos.push_back(tmp_pt);
	}
	if (b_useCornerConstraint)
	{
		tmp_pt.x = 0; tmp_pt.y = 0; tmp_pt.z = 0;	matchTargetPos.push_back(tmp_pt);
		tmp_pt.x = 0; tmp_pt.y = 0; tmp_pt.z = 0;	matchSubjectPos.push_back(tmp_pt);

		tmp_pt.x = 0; tmp_pt.y = 0; tmp_pt.z = getZDim()-1;	matchTargetPos.push_back(tmp_pt);
		tmp_pt.x = 0; tmp_pt.y = 0; tmp_pt.z = pSubjectImg->getZDim()-1;	matchSubjectPos.push_back(tmp_pt);

		tmp_pt.x = 0; tmp_pt.y = getYDim()-1; tmp_pt.z = 0;	matchTargetPos.push_back(tmp_pt);
		tmp_pt.x = 0; tmp_pt.y = pSubjectImg->getYDim()-1; tmp_pt.z = 0;	matchSubjectPos.push_back(tmp_pt);

		tmp_pt.x = 0; tmp_pt.y = getYDim()-1; tmp_pt.z = getZDim()-1; matchTargetPos.push_back(tmp_pt);
		tmp_pt.x = 0; tmp_pt.y = pSubjectImg->getYDim()-1; tmp_pt.z = pSubjectImg->getZDim()-1; matchSubjectPos.push_back(tmp_pt);

		tmp_pt.x = getXDim()-1; tmp_pt.y = 0; tmp_pt.z = 0;	matchTargetPos.push_back(tmp_pt);
		tmp_pt.x = pSubjectImg->getXDim()-1; tmp_pt.y = 0; tmp_pt.z = 0;	matchSubjectPos.push_back(tmp_pt);

		tmp_pt.x = getXDim()-1; tmp_pt.y = 0; tmp_pt.z = getZDim()-1;	matchTargetPos.push_back(tmp_pt);
		tmp_pt.x = pSubjectImg->getXDim()-1; tmp_pt.y = 0; tmp_pt.z = pSubjectImg->getZDim()-1;	matchSubjectPos.push_back(tmp_pt);

		tmp_pt.x = getXDim()-1; tmp_pt.y = getYDim()-1; tmp_pt.z = 0;	matchTargetPos.push_back(tmp_pt);
		tmp_pt.x = pSubjectImg->getXDim()-1; tmp_pt.y = pSubjectImg->getYDim()-1; tmp_pt.z = 0;	matchSubjectPos.push_back(tmp_pt);

		tmp_pt.x = getXDim()-1; tmp_pt.y = getYDim()-1; tmp_pt.z = getZDim()-1; matchTargetPos.push_back(tmp_pt);
		tmp_pt.x = pSubjectImg->getXDim()-1; tmp_pt.y = pSubjectImg->getYDim()-1; tmp_pt.z = pSubjectImg->getZDim()-1; matchSubjectPos.push_back(tmp_pt);
	}

	//now compute DF

	Vol3DSimple<DisplaceFieldF3D> * df_local = compute_df_using_matchingpts(matchTargetPos, matchSubjectPos,
																			pSubjectImg->getXDim(), pSubjectImg->getYDim(), pSubjectImg->getZDim(),
																			(DFComputeMethodType)(getXWidget()->getMainControlWindow()->global_setting.GPara_df_compute_method));
	if (!df_local || !df_local->valid())
	{
		v3d_msg("Fail to generate the displacement field based on landmarks defined. Do nothing.\n");
		return false;
	}

	//finally do warping
	V3DLONG img0_sz_subject[4], startpos_subject[3];
	img0_sz_subject[0] = pSubjectImg->getXDim();
	img0_sz_subject[1] = pSubjectImg->getYDim();
	img0_sz_subject[2] = pSubjectImg->getZDim();
	img0_sz_subject[3] = pSubjectImg->getCDim();
	startpos_subject[0] = startpos_subject[1] = startpos_subject[2] = 0;

	bool res = applyDFtoChannel_simple(pSubjectImg->getRawData(),
									   img0_sz_subject,
									   -1, // if channelNo0_ref<0 then apply to all channels
									   1,
									   startpos_subject,
									   df_local);

	//free space
	if (df_local) {delete df_local; df_local=0;}
	return res;
}

bool My4DImage::proj_alignment_warp_using_landmarks(bool b_overwrite_original)
{
	if (datatype!=V3D_UINT8)
	{
		v3d_msg("Now the alignment program only supports 8bit data. Check your data first.");
		return false;
	}

	if (!p_mainWidget) return false;
	My4DImage * pSubjectImg = p_mainWidget->selectSubjectImage();
	if (!pSubjectImg) return false;
	if (pSubjectImg->getDatatype()!=V3D_UINT8)
	{
		v3d_msg("Now the alignment program only supports 8bit data. Check your data first.");
		return false;
	}

	if(QMessageBox::No == QMessageBox::question (0, "", "Start warping now (may take a few minutes)?", QMessageBox::Yes, QMessageBox::No))
		return false;

	bool b_useThread=false;

	if (b_overwrite_original)
	{
		if (b_useThread)
			reg_thread.do_registration(this, pSubjectImg);
		else
			return proj_alignment_warp_using_landmarks_real(pSubjectImg);
		return true;
	}

	//otherwise create a new image which will be displayed in a new window
	V3DLONG totalbytes = pSubjectImg->getTotalBytes();
	unsigned char * outvol1d = 0;
	try
	{
		outvol1d = new unsigned char [totalbytes];
	}
	catch (...)
	{
		if (!outvol1d)
		{
			v3d_msg("Fail to allocate memory in proj_alignment_warp_using_landmarks() for the warped image.\n");
			return false;
		}
	}
	unsigned char * pRaw = pSubjectImg->getRawData();
	for (V3DLONG i=0;i<totalbytes;i++)
		outvol1d[i] = pRaw[i];

	XFormWidget * newwin = getXWidget()->getMainControlWindow()->createMdiChild();
	newwin->newProcessedImage("warped", outvol1d, pSubjectImg->getXDim(), pSubjectImg->getYDim(), pSubjectImg->getZDim(), pSubjectImg->getCDim(), pSubjectImg->getDatatype());
	newwin->getImageData()->listLandmarks = pSubjectImg->listLandmarks;

	if (b_useThread)
	{
		reg_thread.do_registration(this, newwin->getImageData());
		newwin->show();
		newwin->getImageData()->updateViews();
	}
	else
	{
		if (proj_alignment_warp_using_landmarks_real(newwin->getImageData()))
		{
			newwin->show();
			newwin->getImageData()->updateViews();
			return true;
		}
		else
		{
			delete newwin; //in this case the imagedata will get deleted as well
			return false;
		}
	}

	return true;
}

bool My4DImage::proj_alignment_find_landmark_and_warp(bool b_overwrite_original)
{
	if (datatype!=V3D_UINT8)
	{
		v3d_msg("Now the alignment program only supports 8bit data. Check your data first.\n");
		return false;
	}
	if (!p_mainWidget) return false;
	My4DImage * pSubjectImg = p_mainWidget->selectSubjectImage();
	if (!pSubjectImg) return false;
	if (pSubjectImg->getDatatype()!=V3D_UINT8)
	{
		v3d_msg("Now the alignment program only supports 8bit data. Check your data first.\n");
		return false;
	}


	if(QMessageBox::No == QMessageBox::question (0, "", "Start warping now (may take a few minutes)?", QMessageBox::Yes, QMessageBox::No))
		return false;

	//first back-up the target landmark points
	QList <LocationSimple> target_listLandmarks = listLandmarks;

	//then do processing

	if (!proj_alignment_matching_point_real(pSubjectImg))
		return false;

	if (b_overwrite_original)
	{
		if (!proj_alignment_warp_using_landmarks_real(pSubjectImg))
			return false;

		//in this case as the subject image has been changed to warped image,
		//thus no need to keep the matching points of the subject image, instead
		//I simple copy the matching points of the target image to it for easy check of the warping accuracy

		pSubjectImg->listLandmarks = listLandmarks;

		//finally set the target landmarks back (in this case if we go through all landmarks, we will be able to see which landmarks are unmatched)
		listLandmarks = target_listLandmarks;

		return true;
	}
	else
	{
		V3DLONG totalbytes = pSubjectImg->getTotalBytes();
		unsigned char * outvol1d = 0;
		try {outvol1d = new unsigned char [totalbytes];}
		catch (...)
		{
			v3d_msg("Fail to allocate memory in proj_alignment_find_landmark_and_warp() for the warped image.\n");
			return false;
		}
		unsigned char * pRaw = pSubjectImg->getRawData();
		for (V3DLONG i=0;i<totalbytes;i++)
			outvol1d[i] = pRaw[i];

		XFormWidget * newwin = getXWidget()->getMainControlWindow()->createMdiChild();
		newwin->newProcessedImage("warped", outvol1d, pSubjectImg->getXDim(), pSubjectImg->getYDim(), pSubjectImg->getZDim(), pSubjectImg->getCDim(), pSubjectImg->getDatatype());
		newwin->getImageData()->listLandmarks = pSubjectImg->listLandmarks;

		if (proj_alignment_warp_using_landmarks_real(newwin->getImageData()))
		{
			//in this case as the subject image has been changed to warped image,
			//thus no need to keep the matching points of the subject image, instead
			//I simple copy the matching points of the target image to it for easy check of the warping accuracy

			newwin->getImageData()->listLandmarks = listLandmarks;

			//finally set the target landmarks back (in this case if we go through all landmarks, we will be able to see which landmarks are unmatched)
			listLandmarks = target_listLandmarks;

			newwin->show();
			newwin->getImageData()->updateViews();
			return true;
		}
		else
		{
			delete newwin; //in this case the imagedata will get deleted as well
			return false;
		}
	}
}

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





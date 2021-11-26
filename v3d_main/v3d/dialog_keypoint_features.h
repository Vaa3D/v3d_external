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
keypoint_features_dialog.h
 by Yang Yu
 2009-06-20.
 changed 2009-07-07. by Hanchuan Peng and Yang Yu
**
****************************************************************************/

#ifndef __KEYPOINT_FEATURES_DIALOG_H__
#define __KEYPOINT_FEATURES_DIALOG_H__

#include <QDialog>

#include "ui_dialog_keypoint_features.h"

#include "compute_win_pca.h"
#include "compute_win_diff.h"

#include "v3d_core.h"

class QTableWidgetItem;
class LocationSimple;
class My4DImage;

struct PCAValue {double lambda1, lambda2, lambda3;};
struct FeaturePointAttribute
{
	PCAValue imgpca, diffpca; //for original and difference images respectively
	unsigned char radius; // the radius
};

class ImgPixelFea : public LocationSimple
{
public:
	ImgPixelFea(int xx, int yy, int zz)
	{
		init();
		x = xx;
		y = yy;
		z = zz;
		fpa=new FeaturePointAttribute [4];
	}
	ImgPixelFea()
	{
		init();
		fpa=0;
		n=0;
	}

	~ImgPixelFea()
	{
		//if(fpa) {delete []fpa; fpa=0;}
	}

	bool setPCAscore(My4DImage *image4D, int nscales, int basicscalesize, V3DLONG channel_no, V3DLONG normalized_flag) //for example nscales=4, basicscalesize=5
	{
		if(nscales<=0 && basicscalesize<=0)
		{
			printf("Scale need to be set a positive number! \n");
			return false;
		}

		fpa = new FeaturePointAttribute [nscales];

		V3DLONG sz0 = image4D->getXDim();
    V3DLONG sz1 = image4D->getYDim();
    V3DLONG sz2 = image4D->getZDim();
    V3DLONG sz3 = image4D->getCDim();

		//initial a float type pointer for computing difference
		float **** pDiff = new float *** [sz3];
		if (!pDiff)
		{
			printf("Memory allocate error for pDiff! \n");
			return false;
		}

		for (V3DLONG c=0;c<sz3; c++)
		{
			pDiff[c] = new float ** [sz2];
			if (!pDiff[c])
			{
				for (V3DLONG i=0;i<c;i++) {delete [] (pDiff[c]);}
				delete []pDiff;
				pDiff=NULL;
				return false;
			}
			else
			{
				for (V3DLONG i=0;i<sz2; i++)
				{
					pDiff[c][i] = new float * [sz1];
					if (!pDiff[c][i])
					{
						for (V3DLONG j=0;j<i;j++) {delete [] (pDiff[c][i]);}
						delete [](pDiff[c]);
						pDiff[c]=NULL;
						return false;
					}
					else
					{
						for (V3DLONG j=0;j<sz1; j++)
						{
							pDiff[c][i][j] = new float [sz0];
							if (!pDiff[c][i][j])
							{
								for (V3DLONG k=0;k<j;k++) {delete [] (pDiff[c][i][j]);}
								delete [](pDiff[c][i]);
								pDiff[c][i]=NULL;
								return false;
							}
							else
							{
								for(V3DLONG k=0; k<sz0; k++)
								{
									pDiff[c][i][j][k]=0; //initialization
								}
							}
						}
					}
				}
			}
		}

		float **** data4d_float32 = 0;
		USHORTINT16 **** data4d_uint16 = 0;
		unsigned char **** data4d_uint8 = 0;

		int b_win_shape=1; //0 for cube and 1 for sphere for pca computation
		bool b_disp_CoM_etc=false; //if display center of mass and covariance info

		switch ( image4D->getDatatype() )
		{
			case V3D_UINT8:

				data4d_uint8 = (unsigned char ****)image4D->getData();

				for (int i=0;i<nscales;i++)
				{
					fpa[i].radius = basicscalesize*(i+1); //scale at least is 1

					compute_win3d_diff(data4d_uint8[channel_no], pDiff[channel_no], sz0, sz1, sz2,
									   x,y,z,
									   fpa[i].radius,fpa[i].radius,fpa[i].radius);

					compute_win3d_pca(data4d_uint8[channel_no], sz0, sz1, sz2,
									  x, y, z,
									  fpa[i].radius, fpa[i].radius, fpa[i].radius,
									  fpa[i].imgpca.lambda1, fpa[i].imgpca.lambda2, fpa[i].imgpca.lambda3, b_win_shape, b_disp_CoM_etc, normalized_flag);

					compute_win3d_pca(pDiff[channel_no], sz0, sz1, sz2,
									  x, y, z,
									  fpa[i].radius, fpa[i].radius, fpa[i].radius,
									  fpa[i].diffpca.lambda1, fpa[i].diffpca.lambda2, fpa[i].diffpca.lambda3, b_win_shape, b_disp_CoM_etc, normalized_flag);

				}

				break;

			case V3D_UINT16:

				data4d_uint16 = (USHORTINT16 ****)image4D->getData();

				for (int i=0;i<nscales;i++)
				{
					fpa[i].radius = basicscalesize*i;

					compute_win3d_diff(data4d_uint16[channel_no], pDiff[channel_no], sz0, sz1, sz2,
									   x,y,z,
									   fpa[i].radius,fpa[i].radius,fpa[i].radius);

					compute_win3d_pca(data4d_uint16[channel_no], sz0, sz1, sz2,
									  x, y, z,
									  fpa[i].radius, fpa[i].radius, fpa[i].radius,
									  fpa[i].imgpca.lambda1, fpa[i].imgpca.lambda2, fpa[i].imgpca.lambda3, b_win_shape, b_disp_CoM_etc, normalized_flag);

					compute_win3d_pca(pDiff[channel_no], sz0, sz1, sz2,
									  x, y, z,
									  fpa[i].radius, fpa[i].radius, fpa[i].radius,
									  fpa[i].diffpca.lambda1, fpa[i].diffpca.lambda2, fpa[i].diffpca.lambda3, b_win_shape, b_disp_CoM_etc, normalized_flag);
				}

				break;

			case V3D_FLOAT32:

				data4d_float32 = (float ****)image4D->getData();

				for (int i=0;i<nscales;i++)
				{
					fpa[i].radius = basicscalesize*i;

					compute_win3d_diff(data4d_float32[channel_no], pDiff[channel_no], sz0, sz1, sz2,
									   x,y,z,
									   fpa[i].radius,fpa[i].radius,fpa[i].radius);

					compute_win3d_pca(data4d_float32[channel_no], sz0, sz1, sz2,
									  x, y, z,
									  fpa[i].radius, fpa[i].radius, fpa[i].radius,
									  fpa[i].imgpca.lambda1, fpa[i].imgpca.lambda2, fpa[i].imgpca.lambda3, b_win_shape, b_disp_CoM_etc, normalized_flag);

					compute_win3d_pca(pDiff[channel_no], sz0, sz1, sz2,
									  x, y, z,
									  fpa[i].radius, fpa[i].radius, fpa[i].radius,
									  fpa[i].diffpca.lambda1, fpa[i].diffpca.lambda2, fpa[i].diffpca.lambda3, b_win_shape, b_disp_CoM_etc, normalized_flag);
				}

				break;
		}

		//de-alloc
		if (pDiff)
		{
			for (V3DLONG i=0;i<sz3; i++)
			{
				if(pDiff[i])
				{
					for (V3DLONG j=0;j<sz2; j++)
					{
						if (pDiff[i][j])
						{
							for(V3DLONG k=0; k<sz1; k++)
							{
								if(pDiff[i][j][k])
								{
									delete [] (pDiff[i][j][k]);
									pDiff[i][j][k]=NULL;
								}
							}

							delete [] (pDiff[i][j]);
							pDiff[i][j] = NULL;
						}
					}
				}

				delete [] pDiff[i];
				pDiff[i] = NULL;
			}

			delete [] pDiff;
			pDiff = NULL;
		}

		return true;
	}

public:
	FeaturePointAttribute * fpa;
	int n; //the length of fpa array
	int x,y,z; //the coordinate of a 3D point
	XFormWidget * newwin;

};

class KeypointFeaturesDialog : public QDialog, private Ui_KeypointFeaturesDialog
{
    Q_OBJECT

public:
    KeypointFeaturesDialog(const QList <ImgPixelFea> *p_anoTable, int curRowNum, My4DImage *p_imgdata);
	void fetchData(QList <ImgPixelFea>  *anoTable, int curRow);

private:
	QList <ImgPixelFea> *anoTable;
	int curRow;
	My4DImage *imgdata;
	ImgPixelFea pt;
	bool flag_compute[8]; //detect whether the scale is checked

	void create();
	void updateContent(const QList <ImgPixelFea> *p_anoTable, int curRowNum, My4DImage *p_imgdata);

public slots:
	void compute_pointfea();
	bool compute_similarmap();

};

#endif

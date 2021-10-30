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




//compute_win_pca.h
//by Hanchuan Peng
//090519
//090623: add thewindow size normalization factor

#ifndef __COMPUTE_WIN_PCA_H__
#define __COMPUTE_WIN_PCA_H__

#define WANT_STREAM       // include iostream and iomanipulators
#include "../jba/newmat11/newmatap.h"
#include "../jba/newmat11/newmatio.h"
#ifdef use_namespace
using namespace RBD_LIBRARIES;
#endif

#include "../basic_c_fun/basic_landmark.h"

#include <stdio.h>

template <class T> bool compute_cube_win3d_pca(T ***img3d, V3DLONG sx, V3DLONG sy, V3DLONG sz,
					   V3DLONG x0, V3DLONG y0, V3DLONG z0,
					   V3DLONG rx, V3DLONG ry, V3DLONG rz,
					   double &pc1, double &pc2, double &pc3, bool b_disp_CoM_etc=true, //b_disp_CoM_etc is the display option for center of mass
					   bool b_normalize_score=false)
{
	if (!img3d || sx<=0 || sy<=0 || sz<=0 ||
		x0<0 || x0>=sx || y0<0 || y0>=sy || z0<0 || z0>=sz ||
		rx<0 || ry<0 || rz<0)
		return false;

	//get max radius
	V3DLONG maxrr = (rx>ry)?rx:ry; maxrr = (maxrr>rz)?maxrr:rz;

	//get the boundary

	V3DLONG xb=x0-rx; if(xb<0) xb=0; else if (xb>=sx) xb=sx-1;
	V3DLONG xe=x0+rx; if(xe<0) xe=0; else if (xe>=sx) xe=sx-1;
	V3DLONG yb=y0-ry; if(yb<0) yb=0; else if (yb>=sy) yb=sy-1;
	V3DLONG ye=y0+ry; if(ye<0) ye=0; else if (ye>=sy) ye=sy-1;
	V3DLONG zb=z0-rz; if(zb<0) zb=0; else if (zb>=sz) zb=sz-1;
	V3DLONG ze=z0+rz; if(ze<0) ze=0; else if (ze>=sz) ze=sz-1;

	V3DLONG i,j,k;
	double w;

	//first get the center of mass
    double xm=0,ym=0,zm=0, s=0, mv=0;
	for (k=zb;k<=ze;k++)
	{
		for (j=yb;j<=ye;j++)
		{
			for (i=xb;i<=xe;i++)
			{
				w = double(img3d[k][j][i]);
				xm += w*i;
				ym += w*j;
				zm += w*k;
				s += w;
			}
		}
	}

	if (s>0)
	{
		xm /= s; ym /=s; zm /=s;
        mv = s/(double(ze-zb+1)*(ye-yb+1)*(xe-xb+1));
		if (b_disp_CoM_etc)
			printf("center of mass is (xm, ym, zm) = %5.3f, %5.3f, %5.3f\n",xm,ym,zm);
	}
	else
	{
		printf("Sum of window pixels equals or is smaller than 0. The window is not valid or some other problems in the data. Do nothing.\n");
		return false;
	}

	//get the covariance

	double cc11=0, cc12=0, cc13=0, cc22=0, cc23=0, cc33=0;
	double dfx, dfy, dfz;
	for (k=zb;k<=ze;k++)
	{
		dfz = double(k)-zm;
		if (b_normalize_score) dfz /= maxrr;
		for (j=yb;j<=ye;j++)
		{
			dfy = double(j)-ym;
			if (b_normalize_score) dfy /= maxrr;
			for (i=xb;i<=xe;i++)
			{
				dfx = double(i)-xm;
				if (b_normalize_score) dfx /= maxrr;

//                w = img3d[k][j][i]; //140128
                w = img3d[k][j][i] - mv;  if (w<0) w=0; //140128 try the new formula

				cc11 += w*dfx*dfx;
				cc12 += w*dfx*dfy;
				cc13 += w*dfx*dfz;
				cc22 += w*dfy*dfy;
				cc23 += w*dfy*dfz;
				cc33 += w*dfz*dfz;
			}
		}
	}

	cc11 /= s; 	cc12 /= s; 	cc13 /= s; 	cc22 /= s; 	cc23 /= s; 	cc33 /= s;
	if (b_disp_CoM_etc)
		printf("convariance value (c11,c12,c13,c22,c23,c33) = %5.3f, %5.3f, %5.3f, %5.3f, %5.3f, %5.3f\n",cc11, cc12, cc13, cc22, cc23, cc33);

	//now get the eigen vectors and eigen values

	try
	{
		//then find the eigen vector
		SymmetricMatrix Cov_Matrix(3);
		Cov_Matrix.Row(1) << cc11;
		Cov_Matrix.Row(2) << cc12 << cc22;
		Cov_Matrix.Row(3) << cc13 << cc23 << cc33;

		DiagonalMatrix DD;
		Matrix VV;
		EigenValues(Cov_Matrix,DD,VV);

		//output the result
		pc1 = DD(3);
		pc2 = DD(2);
		pc3 = DD(1);
	}
	catch (...)
	{
		pc1 = VAL_INVALID;
		pc2 = VAL_INVALID;
		pc3 = VAL_INVALID;
	}

	return true;
}


template <class T> bool compute_sphere_win3d_pca(T ***img3d, V3DLONG sx, V3DLONG sy, V3DLONG sz,
										  V3DLONG x0, V3DLONG y0, V3DLONG z0,
										  V3DLONG rx, V3DLONG ry, V3DLONG rz,
										  double &pc1, double &pc2, double &pc3,
										 bool b_disp_CoM_etc=true, //b_disp_CoM_etc is the display option for center of mass )
										 bool b_normalize_score=false)
{
	if (!img3d || sx<=0 || sy<=0 || sz<=0 ||
		x0<0 || x0>=sx || y0<0 || y0>=sy || z0<0 || z0>=sz ||
		rx<0 || ry<0 || rz<0)
		return false;

	//get max radius
	V3DLONG maxrr = (rx>ry)?rx:ry; maxrr = (maxrr>rz)?maxrr:rz;

	//get the boundary

	V3DLONG xb=x0-rx; if(xb<0) xb=0; else if (xb>=sx) xb=sx-1;
	V3DLONG xe=x0+rx; if(xe<0) xe=0; else if (xe>=sx) xe=sx-1;
	V3DLONG yb=y0-ry; if(yb<0) yb=0; else if (yb>=sy) yb=sy-1;
	V3DLONG ye=y0+ry; if(ye<0) ye=0; else if (ye>=sy) ye=sy-1;
	V3DLONG zb=z0-rz; if(zb<0) zb=0; else if (zb>=sz) zb=sz-1;
	V3DLONG ze=z0+rz; if(ze<0) ze=0; else if (ze>=sz) ze=sz-1;

	V3DLONG i,j,k;
	double w;

	//first get the center of mass
	double x2, y2, z2;
	double rx2 = double(rx+1)*(rx+1), ry2 = (double)(ry+1)*(ry+1), rz2 = (double)(rz+1)*(rz+1); //+1 because later need to do use it for radius cmp
	double tmpd;
    double xm=0,ym=0,zm=0, s=0, mv=0, n=0;
	for (k=zb;k<=ze;k++)
	{
		z2 = k-z0; z2*=z2;
		for (j=yb;j<=ye;j++)
		{
			y2 = j-y0; y2*=y2;
			tmpd = y2/ry2 + z2/rz2;
			if (tmpd>1.0)
				continue;

			for (i=xb;i<=xe;i++)
			{
				x2 = i-x0; x2*=x2;
				if (x2/rx2 + tmpd > 1.0)
					continue;

				w = double(img3d[k][j][i]);
				xm += w*i;
				ym += w*j;
				zm += w*k;
				s += w;
                n = n+1;
			}
		}
	}
	if (s>0)
	{
		xm /= s; ym /=s; zm /=s;
        mv = s/n;
		if (b_disp_CoM_etc)
			printf("center of mass is (xm, ym, zm) = %5.3f, %5.3f, %5.3f\n",xm,ym,zm);
	}
	else
	{
		printf("Sum of window pixels equals or is smaller than 0. The window is not valid or some other problems in the data. Do nothing.\n");
		return false;
	}

	//get the covariance. Note that the center of mass must be in the ellpsoid

	double cc11=0, cc12=0, cc13=0, cc22=0, cc23=0, cc33=0;
	double dfx, dfy, dfz;
	for (k=zb;k<=ze;k++)
	{
		z2 = k-z0; z2*=z2;

		dfz = double(k)-zm;
		if (b_normalize_score) dfz /= maxrr;

		for (j=yb;j<=ye;j++)
		{
			y2 = j-y0; y2*=y2;
			tmpd = y2/ry2 + z2/rz2;
			if (tmpd>1.0)
				continue;

			dfy = double(j)-ym;
			if (b_normalize_score) dfy /= maxrr;

			for (i=xb;i<=xe;i++)
			{
				x2 = i-x0; x2*=x2;
				if (x2/rx2 + tmpd > 1.0)
					continue;

				dfx = double(i)-xm;
				if (b_normalize_score) dfx /= maxrr;

                //                w = img3d[k][j][i]; //140128
                                w = img3d[k][j][i] - mv;  if (w<0) w=0; //140128 try the new formula

                cc11 += w*dfx*dfx;
				cc12 += w*dfx*dfy;
				cc13 += w*dfx*dfz;
				cc22 += w*dfy*dfy;
				cc23 += w*dfy*dfz;
				cc33 += w*dfz*dfz;
			}
		}
	}

	cc11 /= s; 	cc12 /= s; 	cc13 /= s; 	cc22 /= s; 	cc23 /= s; 	cc33 /= s;
	if (b_disp_CoM_etc)
		printf("convariance value (c11,c12,c13,c22,c23,c33) = %5.3f, %5.3f, %5.3f, %5.3f, %5.3f, %5.3f\n",cc11, cc12, cc13, cc22, cc23, cc33);

	//now get the eigen vectors and eigen values

	try
	{
		//then find the eigen vector
		SymmetricMatrix Cov_Matrix(3);
		Cov_Matrix.Row(1) << cc11;
		Cov_Matrix.Row(2) << cc12 << cc22;
		Cov_Matrix.Row(3) << cc13 << cc23 << cc33;

		DiagonalMatrix DD;
		Matrix VV;
		EigenValues(Cov_Matrix,DD,VV);

		//output the result
		pc1 = DD(3);
		pc2 = DD(2);
        pc3 = DD(1);
	}
	catch (...)
	{
		pc1 = VAL_INVALID;
		pc2 = VAL_INVALID;
		pc3 = VAL_INVALID;
	}

	return true;
}

template <class T> bool compute_win3d_pca(T ***img3d, V3DLONG sx, V3DLONG sy, V3DLONG sz,
										  V3DLONG x0, V3DLONG y0, V3DLONG z0,
										  V3DLONG rx, V3DLONG ry, V3DLONG rz,
										  double &pc1, double &pc2, double &pc3, int wintype=0,
										  bool b_disp_CoM_etc=true, 	//b_disp_CoM_etc is the display option for center of mass )
										  bool b_normalize_score=false) //if the score if normalized with respect to the window size
{
	if (wintype==0)
		return compute_cube_win3d_pca(img3d, sx, sy, sz, x0, y0, z0, rx, ry, rz, pc1, pc2, pc3, b_disp_CoM_etc, b_normalize_score);
	else //wintype==1
		return compute_sphere_win3d_pca(img3d, sx, sy, sz, x0, y0, z0, rx, ry, rz, pc1, pc2, pc3, b_disp_CoM_etc, b_normalize_score);
}

#endif


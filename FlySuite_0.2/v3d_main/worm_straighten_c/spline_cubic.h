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




//by Hanchuan Peng
//080403
//

#ifndef __SPLINE_CUBIC_H__
#define __SPLINE_CUBIC_H__

#include "../basic_c_fun/v3d_basicdatatype.h"

class parameterCubicSpline
{
private:
	double *a, *b, *c, *d;
	V3DLONG len;

	void freeSpace()
	{
		if (a) {delete []a; a=0;}
		if (b) {delete []b; b=0;}
		if (c) {delete []c; c=0;}
		if (d) {delete []d; d=0;}
		len=0;
	}
	bool createSpace(V3DLONG len0)
	{
		if (len0<=0)
			return false;
			
		if (len0==len) 
		{
			if (valid()) 
				return true; //do nothing if everything is valid
			else
			{
				freeSpace();
				a = new double [len0];
				b = new double [len0];
				c = new double [len0];
				d = new double [len0];
				len = len0;
				if (!valid())
				{
					freeSpace();
					return false;
				}
			}
		}
		else
		{
			freeSpace();
			a = new double [len0];
			b = new double [len0];
			c = new double [len0];
			d = new double [len0];
			len = len0;
			if (!valid())
			{
				freeSpace();
				return false;
			}
		}
		
		return true;
	}

public:
	parameterCubicSpline() {a=b=c=d=0; len=0;}
	parameterCubicSpline(V3DLONG len0) {a=b=c=d=0; len=0; createSpace(len0);} 
	~parameterCubicSpline() {freeSpace();}
	bool valid() {return (!a || !b || !c || !d || len<=0) ? false : true;}
	
	const double * get_a() {return a;}
	const double * get_b() {return b;}
	const double * get_c() {return c;}
	const double * get_d() {return d;}
	const V3DLONG get_len() {return len;}
};

class CubicSplineCurve
{
public:
	parameterCubicSpline ** cpara;
	int dim;
	bool b_closedCurve;

public:
	CubicSplineCurve(bool b_curveClosed)
	{
		cpara = 0; dim=0;
		b_closedCurve = b_curveClosed;
	}
	~CubicSplineCurve()
	{
		if (cpara) 
		{
			for (int i=0;i<dim;i++)
			{
				if (cpara[i]) {delete [](cpara[i]);}
			}
			delete []cpara; cpara=0;
		}
	} 
};

parameterCubicSpline ** est_cubic_spline_2d(double *x, double *y, V3DLONG n, bool b_curveClosed);
parameterCubicSpline ** est_cubic_spline_3d(double *x, double *y, double *z, V3DLONG n, bool b_curveClosed);
parameterCubicSpline * est_cubic_spline_1d(double *y, V3DLONG n, bool b_curveClosed); //estimate the spline model

bool interpolate_cubic_spline(parameterCubicSpline ** cparaArray, int arrayLen, 
	double * &bposx, double * &bposy, double * &bposz, double * &alpha, V3DLONG & cutPlaneNum); //do interpolation based on the model

bool interpolate_cubic_spline(parameterCubicSpline ** cparaArray, int arrayLen, V3DLONG NPtsPerSeg,
	double * &bposx, double * &bposy, double * &bposz, double * &alpha, V3DLONG & cutPlaneNum); //do interpolation based on the model
	
double length_cubic_spline_location(parameterCubicSpline ** cparaArray, int arrayLen, V3DLONG indPiece, double TT, double best_tstep);

//estimate the length of the cubic-spline, either the entire len or just a piece
double length_cubic_spline(parameterCubicSpline ** cparaArray, int arrayLen, V3DLONG indPiece, double * best_tstep); 
//double length_cubic_spline(parameterCubicSpline ** cparaArray, int arrayLen, V3DLONG indPiece, double * & cumsumlen); 
double length_cubic_spline(parameterCubicSpline ** cparaArray, int arrayLen, V3DLONG indPiece, double best_tstep, double *cumsumlen); 

double find_approximation_monofunc(
	double (*funval)(parameterCubicSpline **, int, V3DLONG, double, double), 
	parameterCubicSpline ** cparaArray, int arrayLen, V3DLONG indPiece, double best_t_step,
	double target_fval, double threshold, double t0, double t1);


#endif


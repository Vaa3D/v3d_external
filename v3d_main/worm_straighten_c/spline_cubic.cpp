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
//080404
//Note: any function calling the spline functions below should be responsible to free to spaces of parameterCubicSpline** 
//      and also delete the pointers in parameterCubicSpline[0], parameterCubicSpline[1], parameterCubicSpline[2], etc. 
//

#include <stdio.h>
#include <math.h>

#include "spline_cubic.h"

#define WANT_STREAM       // include iostream and iomanipulators
#include "../jba/newmat11/newmatap.h"
#include "../jba/newmat11/newmatio.h"
#ifdef use_namespace
using namespace RBD_LIBRARIES;
#endif

#define PI 3.141592635

parameterCubicSpline ** est_cubic_spline_2d(double *x, double *y, V3DLONG n, bool b_curveClosed)
{
	if (!x || !y || n<3) return 0;
	parameterCubicSpline ** cpara = new parameterCubicSpline * [2];
	if (!cpara)
	{
		printf("Fail to allocate memory in est_cubic_spline_3d. Do nothing.\n");
		return 0;
	}
	cpara[0] = est_cubic_spline_1d(x, n, b_curveClosed);
	cpara[1] = est_cubic_spline_1d(y, n, b_curveClosed);
	return cpara;
} 

parameterCubicSpline ** est_cubic_spline_3d(double *x, double *y, double *z, V3DLONG n, bool b_curveClosed)
{
	if (!x || !y || !z || n<3) return 0;
	parameterCubicSpline ** cpara = new parameterCubicSpline * [3];
	if (!cpara)
	{
		printf("Fail to allocate memory in est_cubic_spline_3d. Do nothing.\n");
		return 0;
	}
	cpara[0] = est_cubic_spline_1d(x, n, b_curveClosed);
	cpara[1] = est_cubic_spline_1d(y, n, b_curveClosed);
	cpara[2] = est_cubic_spline_1d(z, n, b_curveClosed);
	return cpara;
} 

parameterCubicSpline * est_cubic_spline_1d(double *y, V3DLONG n, bool b_curveClosed) //b_curveClosed means the curve's last pt equals the first one
{
	if (!y || n<3)
	{
		printf("Too few points to estimate the cubic spline. Do nothing.\n");
		return 0;
	}	

	if (b_curveClosed==true && y[0]!=y[n-1])
	{
		printf("You specify a closed curve but the begin/end points have different coordinates. Do nothing.\n");
		return 0;
	}	
	
	parameterCubicSpline *cpara = new parameterCubicSpline(n-1); //n control points come with other n-1 sets of parameters
	if (!cpara || !cpara->valid())
	{
		printf("Fail to allocate memory in est_cubic_spline(). Do nothing.\n");
		if (cpara) {delete cpara; cpara=0;}
		return 0;
	}
		
	double * a = (double *)cpara->get_a();
	double * b = (double *)cpara->get_b();
	double * c = (double *)cpara->get_c();
	double * d = (double *)cpara->get_d();
	
	//now formulate the matrix
	Matrix AA(n,n);
	ColumnVector YY(n);
	
	V3DLONG i;

	YY(1) = 3.0*(y[1]-y[0]); 
	for (i=1;i<=n-2;i++) {YY(i+1) = 3.0*(y[i+1]-y[i-1]); } //note YY matrix index starts from 1
	YY(n) = 3.0*(y[n-1]-y[n-2]);

	AA = 0; //initialize everything as 0
	AA(1,1) = 2; AA(1,2) = 1;
	for (i=1;i<=n-2;i++) {AA(i+1,i)=1; AA(i+1,i+1)=4; AA(i+1,i+2)=1;}
	AA(n,n-1) = 1; AA(n,n) = 2;
	if (b_curveClosed) {AA(1,n) = AA(n,1) = 1; AA(1,1) = AA(n,n) = 4;} //for closed curve

	ColumnVector DD = AA.i() * YY; //note: I know that with newmat this can be made faster by using CroutMatrix etc, 
	                               //especially I know the multiple-dimensional spline curve will run the same AA.i() multiple times.
								   //But I still do this for clarity.
	
	//now fill-back the parameter values
	for (i=0;i<=n-2;i++)
	{
		a[i] = y[i];
		b[i] = DD(i+1);
		c[i] = 3.0*(y[i+1]-y[i]) - 2.0*DD(i+1) - DD(i+2);
		d[i] = 2.0*(y[i]-y[i+1]) + DD(i+1) + DD(i+2);
	}
	
	return cpara;
}

double length_cubic_spline(parameterCubicSpline ** cparaArray, int arrayLen, V3DLONG indPiece, double * best_tstep) 
//if indPiece is between 0 and n-1, then return the respective length of that particular cubic curve;
//otherwise return the length of the entire cubic_spline
{
	if (!cparaArray || arrayLen<0 || !best_tstep) 
	{
		printf("Invalid pointer or length passed to length_cubic_spline(). Do nothing.\n");
		return -1; //-1 indicates invalid value
	}
	V3DLONG i,j;
	for (i=0;i<arrayLen;i++)
	{
		if (!cparaArray[i] || !cparaArray[i]->valid() || cparaArray[i]->get_len()!=cparaArray[0]->get_len())
		{
			printf("Invalid pointer or related error in one of pointer array elements in length_cubic_spline(). Do nothing.\n");
			return -1;
		}
	}
	
	bool b_computeEntireLen = false;
	if (indPiece<0 || indPiece>=cparaArray[0]->get_len())
		b_computeEntireLen = true;

	double ** a = new double * [arrayLen];
	double ** b = new double * [arrayLen]; 
	double ** c = new double * [arrayLen];
	double ** d = new double * [arrayLen];
	if (!a || !b || !c || !d)
	{
		printf("Fail to allocate memory in length_cubic_spline(). Do nothing.\n");
		if (a) {delete []a; a=0;}
		if (b) {delete []b; b=0;}
		if (c) {delete []c; c=0;}
		if (d) {delete []d; d=0;}
		return -1;
	}
	
	for (i=0;i<arrayLen;i++)
	{
		a[i] = (double *)cparaArray[i]->get_a();
		b[i] = (double *)cparaArray[i]->get_b();
		c[i] = (double *)cparaArray[i]->get_c();
		d[i] = (double *)cparaArray[i]->get_d();
	}
	
	V3DLONG j_start=0, j_end=cparaArray[0]->get_len()-1;
	if (!b_computeEntireLen)
	{
		j_start = j_end = indPiece;
	}

	double clen;
	for (j=j_start, clen=0.0;j<=j_end;j++)
	{
		//double stmp1=0.0, stmp0=0.0; //multi-dimensional aggregation
		double tmpsum;
		
		double xdiff, ydiff, zdiff;
		xdiff=b[0][j]+c[0][j]+d[0][j]; tmpsum = (xdiff*xdiff);
		if (arrayLen>=2) {ydiff=b[1][j]+c[1][j]+d[1][j]; tmpsum += ydiff*ydiff;}
		if (arrayLen>=3) {zdiff=b[2][j]+c[2][j]+d[2][j]; tmpsum += zdiff*zdiff;}
		double len_straightSeg = sqrt(tmpsum);
		V3DLONG nseg = 100; 
		double tstep=1.0/nseg;
		
		double len_curvedSeg, last_len_curvedSeg;
		last_len_curvedSeg = len_straightSeg;

		printf("\nspline seg j=%ld\n", j);
		while (1)
		{
			double cur_bx, cur_by, cur_bz, cur_cx, cur_cy, cur_cz, cur_dx, cur_dy, cur_dz;
			cur_bx = b[0][j]; cur_cx = c[0][j]; cur_dx = d[0][j];
			if (arrayLen>=2) {cur_by = b[1][j]; cur_cy = c[1][j]; cur_dy = d[1][j];}
			if (arrayLen>=3) {cur_bz = b[2][j]; cur_cz = c[2][j]; cur_dz = d[2][j];}
			
			V3DLONG k; double lastx, lasty, lastz, curx, cury, curz; 
			double t;
			for (k=0, t=0.0, len_curvedSeg=0.0, lastx = 0, lasty = 0, lastz=0;k<nseg;k++)
			{
				t += tstep;
				curx = (cur_bx + (cur_cx + cur_dx*t)*t)*t; double tmpdx = (curx-lastx); tmpsum=tmpdx*tmpdx;
				if (arrayLen>=2) { cury = (cur_by + (cur_cy + cur_dy*t)*t)*t; double tmpdy = (cury-lasty); tmpsum+=tmpdy*tmpdy;}
				if (arrayLen>=3) { curz = (cur_bz + (cur_cz + cur_dz*t)*t)*t; double tmpdz = (curz-lastz); tmpsum+=tmpdz*tmpdz;}
				len_curvedSeg += sqrt(tmpsum);
				lastx = curx; lasty = cury; lastz = curz;
			}
			
			if (fabs(len_curvedSeg-last_len_curvedSeg)<0.01)
			{
				printf("last_len=%10.4f\tcurve_len=%10.4f\t tstep=%10.4f\n", last_len_curvedSeg, len_curvedSeg, tstep);
				break;
			}
			else
			{
				printf("last_len=%10.4f\tcurve_len=%10.4f\t tstep=%10.4f\n", last_len_curvedSeg, len_curvedSeg, tstep);
				last_len_curvedSeg = len_curvedSeg;
				tstep /= 2.0;
				nseg = V3DLONG(ceil(1.0/tstep)); //note that as I start from nseg=100, thus tstep=0.01, and thus even I continue divide by 2, nseg should always be integer
			}	
		}

		clen += len_curvedSeg;
		best_tstep[j] = tstep;
	}

	//free space and return
	if (a) {delete []a; a=0;}
	if (b) {delete []b; b=0;}
	if (c) {delete []c; c=0;}
	if (d) {delete []d; d=0;}

	return clen;
}

double length_cubic_spline(parameterCubicSpline ** cparaArray, int arrayLen, V3DLONG indPiece, double best_tstep, double *cumsumlen) 
//indPiece must be between 0 and n-1; cumsumlen must have preallocated memory (length = ceil(1.0/best_tstep));
{
	if (!cparaArray || arrayLen<0 || !cumsumlen) 
	{
		printf("Invalid pointer or length passed to length_cubic_spline(). Do nothing.\n");
		return -1; //-1 indicates invalid value
	}
	V3DLONG i;
	for (i=0;i<arrayLen;i++)
	{
		if (!cparaArray[i] || !cparaArray[i]->valid() || cparaArray[i]->get_len()!=cparaArray[0]->get_len())
		{
			printf("Invalid pointer or related error in one of pointer array elements in length_cubic_spline(). Do nothing.\n");
			return -1;
		}
	}
	
	bool b_computeEntireLen = false;
	if (indPiece<0 || indPiece>=cparaArray[0]->get_len())
		b_computeEntireLen = true;

	double ** a = new double * [arrayLen];
	double ** b = new double * [arrayLen]; 
	double ** c = new double * [arrayLen];
	double ** d = new double * [arrayLen];
	if (!a || !b || !c || !d)
	{
		printf("Fail to allocate memory in length_cubic_spline(). Do nothing.\n");
		if (a) {delete []a; a=0;}
		if (b) {delete []b; b=0;}
		if (c) {delete []c; c=0;}
		if (d) {delete []d; d=0;}
		return -1;
	}
	
	for (i=0;i<arrayLen;i++)
	{
		a[i] = (double *)cparaArray[i]->get_a();
		b[i] = (double *)cparaArray[i]->get_b();
		c[i] = (double *)cparaArray[i]->get_c();
		d[i] = (double *)cparaArray[i]->get_d();
	}
	
	if (indPiece<0 || indPiece>=cparaArray[0]->get_len())
	{
		printf("The index of the cublic spline piece out of the range in length_cubic_spline(). Do nothing. \n");
		return -1;
	}

	//double stmp1=0.0, stmp0=0.0; //multi-dimensional aggregation
	double tmpsum;
	
	V3DLONG nseg = ceil(1.0/best_tstep); //This should be length of cumsumlen
	
	double len_curvedSeg;
	double cur_bx, cur_by, cur_bz, cur_cx, cur_cy, cur_cz, cur_dx, cur_dy, cur_dz;
	cur_bx = b[0][indPiece]; cur_cx = c[0][indPiece]; cur_dx = d[0][indPiece];
	if (arrayLen>=2) {cur_by = b[1][indPiece]; cur_cy = c[1][indPiece]; cur_dy = d[1][indPiece];}
	if (arrayLen>=3) {cur_bz = b[2][indPiece]; cur_cz = c[2][indPiece]; cur_dz = d[2][indPiece];}
	
	V3DLONG k; double lastx, lasty, lastz, curx, cury, curz; 
	double t;
	for (k=0, t=0.0, len_curvedSeg=0.0, lastx = 0, lasty = 0, lastz=0;k<nseg;k++)
	{
		t += best_tstep;
		curx = (cur_bx + (cur_cx + cur_dx*t)*t)*t; double tmpdx = (curx-lastx); tmpsum=tmpdx*tmpdx;
		if (arrayLen>=2) { cury = (cur_by + (cur_cy + cur_dy*t)*t)*t; double tmpdy = (cury-lasty); tmpsum+=tmpdy*tmpdy;}
		if (arrayLen>=3) { curz = (cur_bz + (cur_cz + cur_dz*t)*t)*t; double tmpdz = (curz-lastz); tmpsum+=tmpdz*tmpdz;}
		cumsumlen[k] = (len_curvedSeg += sqrt(tmpsum));
		lastx = curx; lasty = cury; lastz = curz;
	}

	//free space and return
	if (a) {delete []a; a=0;}
	if (b) {delete []b; b=0;}
	if (c) {delete []c; c=0;}
	if (d) {delete []d; d=0;}

	return len_curvedSeg;
}

double length_cubic_spline_location(parameterCubicSpline ** cparaArray, int arrayLen, V3DLONG indPiece, double TT, double best_tstep) 
//Note: indPiece must be between 0 and n-1
{
	if (!cparaArray || arrayLen<0) 
	{
		printf("Invalid pointer or length passed to length_cubic_spline(). Do nothing.\n");
		return -1; //-1 indicates invalid value
	}
	V3DLONG i,j;
	for (i=0;i<arrayLen;i++)
	{
		if (!cparaArray[i] || !cparaArray[i]->valid() || cparaArray[i]->get_len()!=cparaArray[0]->get_len())
		{
			printf("Invalid pointer or related error in one of pointer array elements in length_cubic_spline(). Do nothing.\n");
			return -1;
		}
	}
	
	if (indPiece<0 || indPiece>=cparaArray[0]->get_len())
		return -1;

	double ** a = new double * [arrayLen];
	double ** b = new double * [arrayLen]; 
	double ** c = new double * [arrayLen];
	double ** d = new double * [arrayLen];
	if (!a || !b || !c || !d)
	{
		printf("Fail to allocate memory in length_cubic_spline(). Do nothing.\n");
		if (a) {delete []a; a=0;}
		if (b) {delete []b; b=0;}
		if (c) {delete []c; c=0;}
		if (d) {delete []d; d=0;}
		return -1;
	}
	
	for (i=0;i<arrayLen;i++)
	{
		a[i] = (double *)cparaArray[i]->get_a();
		b[i] = (double *)cparaArray[i]->get_b();
		c[i] = (double *)cparaArray[i]->get_c();
		d[i] = (double *)cparaArray[i]->get_d();
	}
	
	V3DLONG j_start, j_end;
	j_start = j_end = indPiece;
		
	double clen;

	double cur_bx, cur_by, cur_bz, cur_cx, cur_cy, cur_cz, cur_dx, cur_dy, cur_dz;
	
	{
		j = indPiece;
		cur_bx = b[0][j]; cur_cx = c[0][j]; cur_dx = d[0][j];
		if (arrayLen>=2) {cur_by = b[1][j]; cur_cy = c[1][j]; cur_dy = d[1][j];}
		if (arrayLen>=3) {cur_bz = b[2][j]; cur_cz = c[2][j]; cur_dz = d[2][j];}
		
		double lastx, lasty, lastz, curx, cury, curz; 
		double t, tmpsum;
		lastx = 0; lasty = 0; 
		for (t=0.0, clen=0.0;t<TT;t+=best_tstep)
		{
			t += best_tstep;
			curx = (cur_bx + (cur_cx + cur_dx*t)*t)*t; double tmpdx = (curx-lastx); tmpsum=tmpdx*tmpdx;
			if (arrayLen>=2) { cury = (cur_by + (cur_cy + cur_dy*t)*t)*t; double tmpdy = (cury-lasty); tmpsum+=tmpdy*tmpdy;}
			if (arrayLen>=3) { curz = (cur_bz + (cur_cz + cur_dz*t)*t)*t; double tmpdz = (curz-lastz); tmpsum+=tmpdz*tmpdz;}
			clen += sqrt(tmpsum);
			lastx = curx; lasty = cury; lastz = curz;
		}
	}

	//free space and return
	if (a) {delete []a; a=0;}
	if (b) {delete []b; b=0;}
	if (c) {delete []c; c=0;}
	if (d) {delete []d; d=0;}

	return clen;
}

bool interpolate_cubic_spline(parameterCubicSpline ** cparaArray, int arrayLen, 
	double * &bposx, double * &bposy, double * &bposz, double * &alpha, V3DLONG & cutPlaneNum)
{
	//free space if needed
	if (bposx) {delete []bposx; bposx=0;}
	if (bposy) {delete []bposy; bposy=0;}
	if (bposz) {delete []bposz; bposz=0;}
	if (alpha) {delete []alpha; alpha=0;}
	cutPlaneNum = 0;
	
	//verify if input parameters make sense
	if (!cparaArray || arrayLen<2) //at least 2-dim curve
	{
		printf("Invalid pointer or length passed to length_cubic_spline(). Do nothing.\n");
		return false; 
	}
	if (arrayLen>=4) //at most 3-d curve
	{
		printf("This function supports at almost 3D cubic-spline; your arrayLen(dim) parameter is bigger than 3. Do nothing.\n");
		return false; 
	}

	V3DLONG i,j;
	for (i=0;i<arrayLen;i++)
	{
		if (!cparaArray[i] || !cparaArray[i]->valid() || cparaArray[i]->get_len()!=cparaArray[0]->get_len())
		{
			printf("Invalid pointer or related error in one of pointer array elements in length_cubic_spline(). Do nothing.\n");
			return false; 
		}
	}
	
	//prepare the computation
	
	//double (* curveLengthFun)(parameterCubicSpline **, int, V3DLONG, double, double) = length_cubic_spline_location;
	
	//const double optimization_th = 0.001;
	
	V3DLONG NPieces = cparaArray[0]->get_len();
	double *len = 0, *slen = 0, *t_step_val=0;
	len = new double [NPieces];
	slen = new double [NPieces+1];
	t_step_val = new double [NPieces];
	if (!len || !slen || !t_step_val)
	{
		printf("Fail to allocate memory in interpolate_cubic_spline(); do nothing. \n");
		if (len) {delete []len; len=0;}
		if (slen) {delete []slen; slen=0;}
		if (t_step_val) {delete []t_step_val; t_step_val=0;}
		return false; 
	}
	
	//get the piece-wise lengths and also total length of the cubic spline
	double totalLen;
	double best_t_step;
	for (j=0;j<NPieces;j++)
	{
		len[j] = length_cubic_spline(cparaArray, arrayLen, j, t_step_val); //note that in this case t_step_val[...] will get updated 
		if (j==0)
		  slen[j] = 0;
		else
		  slen[j] = len[j-1] + slen[j-1];
		  
		if (j==0)
			best_t_step=t_step_val[j];
		else
		{
			if (best_t_step>t_step_val[j])
				best_t_step = t_step_val[j];
		}	  
	}
	totalLen = slen[j] = len[j-1] + slen[j-1];
	
	//now recompute all the distances along the curve, and store the information for a quick table-looking. 080406
	best_t_step /= 2; //080407: make it even finer
	printf("A good step size for this spline is [%10.4f]\n", best_t_step);
	V3DLONG NTicksPerSeg = (V3DLONG)ceil(double(1.0)/best_t_step);
	V3DLONG totalTicks = (V3DLONG)NPieces*NTicksPerSeg+1; //in this way, make sure there are enough ticks. 
	                                              //Also note that (double(1.0)/best_t_step) should be an interger even without ceil(), 
												  //as I set nseg=100 and always double it
	double *cumsumlen = new double [totalTicks];
	if (!cumsumlen)
	{
		printf("Fail to allocate memory for cumsumlen in interpolate_cubic_spline(); do nothing. \n");
		if (len) {delete []len; len=0;}
		if (slen) {delete []slen; slen=0;}
		if (t_step_val) {delete []t_step_val; t_step_val=0;}
		return false; 
	}
	cumsumlen[0] = 0; //that corresponding to the very first one
	for (j=0;j<NPieces;j++)
	{
	    double *p_seg_cumsumlen = cumsumlen+1+j*NTicksPerSeg;
		len[j] = length_cubic_spline(cparaArray, arrayLen, j, best_t_step, p_seg_cumsumlen); //note this case best_t_step will not change
		if (j==0)
		  slen[j] = 0;
		else
		{
		  slen[j] = len[j-1] + slen[j-1];
		  double thisSLen = slen[j];
		  for (i=0;i<NTicksPerSeg;i++)
			  p_seg_cumsumlen[i] += thisSLen;
		}  
	}
	totalLen = slen[j] = len[j-1] + slen[j-1];
	
	//find out the respective t value for every point with the natural number distance (from the beginning of the cubic spline) on the spline curve
	
	cutPlaneNum = V3DLONG(floor(totalLen+1)); //so that set all cut-planes have 1-pixel spacing 
	if (cutPlaneNum<=0)
	{
		printf("This should never happen; check your data make sure it's correct; do nothing. \n");
		if (len) {delete []len; len=0;}
		if (slen) {delete []slen; slen=0;}
		if (t_step_val) {delete []t_step_val; t_step_val=0;}
		return false; 
	}
	bposx = new double [cutPlaneNum];
	bposy = new double [cutPlaneNum];
	if (arrayLen==3) bposz = new double [cutPlaneNum];
	alpha = new double [cutPlaneNum];

	//This is a fast method with complexity linear to totalTicks
	V3DLONG lasti = 0, ii;

	bposx[lasti] = cparaArray[0]->get_a()[j];
	bposy[lasti] = cparaArray[1]->get_a()[j];
	if (arrayLen==3) 
		bposz[lasti] = cparaArray[2]->get_a()[j];

	double delta_x = cparaArray[0]->get_b()[j], delta_y = cparaArray[1]->get_b()[j];	
	if (delta_x>0) alpha[lasti] = atan(delta_y/delta_x);
	else if (delta_x<0) alpha[lasti] = atan(delta_y/delta_x)-PI;
	else alpha[lasti] = (delta_y>=0) ? PI/2 : -PI/2;
	alpha[lasti] += PI/2.0;		
	if (alpha[lasti]>PI) alpha[lasti]=PI;	else if (alpha[lasti]<-PI) alpha[lasti]=-PI;

	lasti++;

	for (i=1;i<totalTicks;i++)
	{
		//printf("[%d=%4.3f] ", i, cumsumlen[i]);
		if (cumsumlen[i]>=lasti) //this means just pass the interger-distance tick, thus this is should be a good (not necessarily best, but should be good enough)
								// because the distance between each consecutive pair of t's should be very small ~ 0.01
		{
			//the indexes can be conveniently obtained
			ii = i-1;
			j = ii/NTicksPerSeg;
			double cur_t = (ii%NTicksPerSeg + 1)*best_t_step;
			//printf("->%d [j=%d len[j]=%10.3f]\n", lasti, j, slen[j]);

			//now find out the cutting plane locations
			double cur_t2 = cur_t*cur_t;
			double cur_t3 = cur_t*cur_t2;
			bposx[lasti] = cparaArray[0]->get_a()[j] + cparaArray[0]->get_b()[j]*cur_t + cparaArray[0]->get_c()[j]*cur_t2 + cparaArray[0]->get_d()[j]*cur_t3;
			bposy[lasti] = cparaArray[1]->get_a()[j] + cparaArray[1]->get_b()[j]*cur_t + cparaArray[1]->get_c()[j]*cur_t2 + cparaArray[1]->get_d()[j]*cur_t3;
			if (arrayLen==3) 
				bposz[lasti] = cparaArray[2]->get_a()[j] + cparaArray[2]->get_b()[j]*cur_t + cparaArray[2]->get_c()[j]*cur_t2 + cparaArray[2]->get_d()[j]*cur_t3;
			
			delta_x = (cparaArray[0]->get_b()[j] + 2.0*cparaArray[0]->get_c()[j]*cur_t + 3.0*cparaArray[0]->get_d()[j]*cur_t2);
			delta_y = (cparaArray[1]->get_b()[j] + 2.0*cparaArray[1]->get_c()[j]*cur_t + 3.0*cparaArray[1]->get_d()[j]*cur_t2);
			if (delta_x>0) alpha[lasti] = atan(delta_y/delta_x);
			else if (delta_x<0) alpha[lasti] = atan(delta_y/delta_x)-PI;
			else alpha[lasti] = (delta_y>=0) ? PI/2 : -PI/2;
			alpha[lasti] += PI/2.0;		
			if (alpha[lasti]>PI) alpha[lasti]=PI;	else if (alpha[lasti]<-PI) alpha[lasti]=-PI;
						
			//now increase lasti which is the distance
			lasti++;
			
			//just for verification
			if (lasti>=cutPlaneNum) //already finish up with the last cutting plane
				break; 
		}
	}
	
	//just for verification
	if (lasti!=cutPlaneNum)
	{
		printf("Warning: Strange error happen [%ld!=%ld]. Check the program. \n",lasti,cutPlaneNum);
		cutPlaneNum=lasti;	//add by Lei Qu @20101026
	}
		
	//free temporary spaces
	if (len) {delete []len; len=0;}
	if (slen) {delete []slen; slen=0;}
	if (t_step_val) {delete []t_step_val; t_step_val=0;}
	if (cumsumlen) {delete []cumsumlen; cumsumlen=0;}
	
	return true; 
}

bool interpolate_cubic_spline(parameterCubicSpline ** cparaArray, int arrayLen, V3DLONG NPtsPerSeg,
	double * &bposx, double * &bposy, double * &bposz, double * &alpha, V3DLONG & cutPlaneNum)
{
	//free space if needed
	if (bposx) {delete []bposx; bposx=0;}
	if (bposy) {delete []bposy; bposy=0;}
	if (bposz) {delete []bposz; bposz=0;}
	if (alpha) {delete []alpha; alpha=0;}
	cutPlaneNum = 0;
	
	//verify if input parameters make sense
	if (!cparaArray || arrayLen<2) //at least 2-dim curve
	{
		printf("Invalid pointer or length passed to length_cubic_spline(). Do nothing.\n");
		return false; 
	}
	if (arrayLen>=4) //at most 3-d curve
	{
		printf("This function supports at almost 3D cubic-spline; your arrayLen(dim) parameter is bigger than 3. Do nothing.\n");
		return false; 
	}

	V3DLONG i,j;
	for (i=0;i<arrayLen;i++)
	{
		if (!cparaArray[i] || !cparaArray[i]->valid() || cparaArray[i]->get_len()!=cparaArray[0]->get_len())
		{
			printf("Invalid pointer or related error in one of pointer array elements in length_cubic_spline(). Do nothing.\n");
			return false; 
		}
	}
	
	//prepare the computation
	
	//double (* curveLengthFun)(parameterCubicSpline **, int, V3DLONG, double, double) = length_cubic_spline_location;
	
	//const double optimization_th = 0.001;
	
	V3DLONG NPieces = cparaArray[0]->get_len();
	double *len = 0, *slen = 0, *best_t_step=0;
	len = new double [NPieces];
	slen = new double [NPieces+1];
	best_t_step = new double [NPieces];
	if (!len || !slen || !best_t_step)
	{
		printf("Fail to allocate memory in interpolate_cubic_spline(); do nothing. \n");
		if (len) {delete []len; len=0;}
		if (slen) {delete []slen; slen=0;}
		if (best_t_step) {delete []best_t_step; best_t_step=0;}
		return false; 
	}
	
	//get the piece-wise lengths and also total length of the cubic spline
	double totalLen;
	for (j=0;j<NPieces;j++)
	{
		len[j] = length_cubic_spline(cparaArray, arrayLen, j, best_t_step);
		if (j==0)
		  slen[j] = 0;
		else
		  slen[j] = len[j-1] + slen[j-1];
	}
	totalLen = slen[j] = len[j-1] + slen[j-1];
	
	//find out the respective t value for every point with the natural number distance (from the beginning of the cubic spline) on the spline curve
	
	cutPlaneNum = V3DLONG(NPieces)*NPtsPerSeg; 
	if (cutPlaneNum<=0)
	{
		printf("This should never happen; check your data make sure it's correct; do nothing. \n");
		if (len) {delete []len; len=0;}
		if (slen) {delete []slen; slen=0;}
		if (best_t_step) {delete []best_t_step; best_t_step=0;}
		return false; 
	}
	bposx = new double [cutPlaneNum];
	bposy = new double [cutPlaneNum];
	if (arrayLen==3) bposz = new double [cutPlaneNum];
	alpha = new double [cutPlaneNum];
	
	double delta_x, delta_y;
	V3DLONG ii; //V3DLONG lastUsePiece=0;
	for (i=0; i<NPieces; i++)
	{
		for (j=0;j<NPtsPerSeg;j++)
		{
			ii = i*NPtsPerSeg+j;
		
			//then find the self-varying variable for a target distance on the curve
			
			double cur_t = double(j)/NPtsPerSeg;
			
			//now find out the cutting plane locations
			double cur_t2 = cur_t*cur_t;
			double cur_t3 = cur_t*cur_t2;
			bposx[ii] = cparaArray[0]->get_a()[i] + cparaArray[0]->get_b()[i]*cur_t + cparaArray[0]->get_c()[i]*cur_t2 + cparaArray[0]->get_d()[i]*cur_t3;
			bposy[ii] = cparaArray[1]->get_a()[i] + cparaArray[1]->get_b()[i]*cur_t + cparaArray[1]->get_c()[i]*cur_t2 + cparaArray[1]->get_d()[i]*cur_t3;
			if (arrayLen==3) 
				bposz[ii] = cparaArray[2]->get_a()[i] + cparaArray[2]->get_b()[i]*cur_t + cparaArray[2]->get_c()[i]*cur_t2 + cparaArray[2]->get_d()[i]*cur_t3;

			delta_x = (cparaArray[0]->get_b()[i] + 2.0*cparaArray[0]->get_c()[i]*cur_t + 3.0*cparaArray[0]->get_d()[i]*cur_t2);
			delta_y = (cparaArray[1]->get_b()[i] + 2.0*cparaArray[1]->get_c()[i]*cur_t + 3.0*cparaArray[1]->get_d()[i]*cur_t2);
			if (delta_x>0) alpha[ii] = atan(delta_y/delta_x);
			else if (delta_x<0) alpha[ii] = atan(delta_y/delta_x)-PI;
			else alpha[ii] = (delta_y>=0) ? PI/2 : -PI/2;
			alpha[ii] += PI/2.0;		
			if (alpha[ii]>PI) alpha[ii]=PI;	else if (alpha[ii]<-PI) alpha[ii]=-PI;
		}
	}
		
	//free temporary spaces
	if (len) {delete []len; len=0;}
	if (slen) {delete []slen; slen=0;}
	if (best_t_step) {delete []best_t_step; best_t_step=0;}
	
	return true; 
}


double find_approximation_monofunc(
	double (*funval)(parameterCubicSpline **, int, V3DLONG, double, double), 
	parameterCubicSpline ** cparaArray, int arrayLen, V3DLONG indPiece, double best_t_step,
	double target_fval, double threshold, double t0, double t1)
{
	double t;
	
	if (threshold<0)
	{
		printf("Invalid threshold for optimization approximation in find_approximation_monofunc().\n");
		return t0-1;
	}
	
	bool b_reversedSearchDirection=false;
	double tmin=t0, tmax=t1;
	double fmin = (*funval)(cparaArray, arrayLen, indPiece, t0, best_t_step);
	double fmax = (*funval)(cparaArray, arrayLen, indPiece, t1, best_t_step);
	if (fmin>fmax)
		b_reversedSearchDirection=true;
	if ((fmin<target_fval && fmax<target_fval) || (fmin>target_fval && fmax>target_fval))
	{
		printf("Invalid searching range in find_approximation_monofunc().\n");
		return t0-1;
	}
	if (fabs(fmin-target_fval)<threshold)
		return t0;
	if (fabs(fmax-target_fval)<threshold)
		return t1;
	
	while (1)
	{
		double tmid = (tmin+tmax)/2.0;
		double fmid = (*funval)(cparaArray, arrayLen, indPiece, tmid, best_t_step);
		if (fabs(fmid-target_fval)<threshold)
		{
			t = tmid;
			break;
		}
		
		if (b_reversedSearchDirection==true)
		{
			if (fmid>target_fval)
			{
				tmax = tmid;
				fmax = fmid;
			}
			else if (fmid<target_fval)
			{
				tmin = tmid;
				fmin = fmid;
			}
			else
			{
				t = tmid;
				break;;
			}
		}
		else
		{
			if (fmid<target_fval)
			{
				tmax = tmid;
				fmax = fmid;
			}
			else if (fmid>target_fval)
			{
				tmin = tmid;
				fmin = fmid;
			}
			else
			{
				t = tmid;
				break;
			}
		}	
	}
	
	return t;
}


//Note 080405: interestingly there is a Java program online to estimate the cubic spline as well:
//http://www.cse.unsw.edu.au/~lambert/splines/NatCubic.java
//
//import java.awt.*;
//
//public class NatCubic extends ControlCurve{
//  
///* calculates the natural cubic spline that interpolates
//y[0], y[1], ... y[n]
//The first segment is returned as
//C[0].a + C[0].b*u + C[0].c*u^2 + C[0].d*u^3 0<=u <1
//the other segments are in C[1], C[2], ...  C[n-1] */
//
//  Cubic[] calcNaturalCubic(int n, int[] x) {
//    float[] gamma = new float[n+1];
//    float[] delta = new float[n+1];
//    float[] D = new float[n+1];
//    int i;
//    /* We solve the equation
//       [2 1       ] [D[0]]   [3(x[1] - x[0])  ]
//       |1 4 1     | |D[1]|   |3(x[2] - x[0])  |
//       |  1 4 1   | | .  | = |      .         |
//       |    ..... | | .  |   |      .         |
//       |     1 4 1| | .  |   |3(x[n] - x[n-2])|
//       [       1 2] [D[n]]   [3(x[n] - x[n-1])]
//       
//       by using row operations to convert the matrix to upper triangular
//       and then back sustitution.  The D[i] are the derivatives at the knots.
//       */
//    
//    gamma[0] = 1.0f/2.0f;
//    for ( i = 1; i < n; i++) {
//      gamma[i] = 1/(4-gamma[i-1]);
//    }
//    gamma[n] = 1/(2-gamma[n-1]);
//    
//    delta[0] = 3*(x[1]-x[0])*gamma[0];
//    for ( i = 1; i < n; i++) {
//      delta[i] = (3*(x[i+1]-x[i-1])-delta[i-1])*gamma[i];
//    }
//    delta[n] = (3*(x[n]-x[n-1])-delta[n-1])*gamma[n];
//    
//    D[n] = delta[n];
//    for ( i = n-1; i >= 0; i--) {
//      D[i] = delta[i] - gamma[i]*D[i+1];
//    }
//
//    /* now compute the coefficients of the cubics */
//    Cubic[] C = new Cubic[n];
//    for ( i = 0; i < n; i++) {
//      C[i] = new Cubic((float)x[i], D[i], 3*(x[i+1] - x[i]) - 2*D[i] - D[i+1],
//		       2*(x[i] - x[i+1]) + D[i] + D[i+1]);
//    }
//    return C;
//  }
//
//
//  final int STEPS = 12;
//
//  /* draw a cubic spline */
//  public void paint(Graphics g){
//    super.paint(g);
//    if (pts.npoints >= 2) {
//      Cubic[] X = calcNaturalCubic(pts.npoints-1, pts.xpoints);
//      Cubic[] Y = calcNaturalCubic(pts.npoints-1, pts.ypoints);
//    
//      /* very crude technique - just break each segment up into steps lines */
//      Polygon p = new Polygon();
//      p.addPoint((int) Math.round(X[0].eval(0)),
//		 (int) Math.round(Y[0].eval(0)));
//      for (int i = 0; i < X.length; i++) {
//	for (int j = 1; j <= STEPS; j++) {
//	  float u = j / (float) STEPS;
//	  p.addPoint(Math.round(X[i].eval(u)),
//	             Math.round(Y[i].eval(u)));
//	}
//      }
//    g.drawPolyline(p.xpoints, p.ypoints, p.npoints);
//    }
//  }
//}


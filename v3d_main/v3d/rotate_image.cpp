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




// Rotate the entire 4D volume image an angle around an axis (x,y, or z) at the specified center
//
// by Hanchuan Peng
// 2008-03-20
// Revised from my mex function rotate_inPlaneZ, X, Y in the volproc dir.
// 080808: remove redundant and unused parameters
//


#include <stdio.h>
#include <math.h>

//the folowing conditional compilation is added by PHC, 2010-05-20
#if defined (_MSC_VER)
#include "../basic_c_fun/vcdiff.h"
#else
#endif

#include "rotate_image.h"

#include "../basic_c_fun/basic_memory.cpp"


#define PI 3.141592635

template <class T> T max(T a, T b)
{
    return (a>=b)?a:b;
}

template <class T> T min(T a, T b)
{
    return (a<=b)?a:b;
}

bool rotate_inPlaneZ(UBYTE *invol1d, V3DLONG *insz, const Options_Rotate & opt, //xc, yc, zc are the rotation centers
                     UBYTE * &outvol1d, V3DLONG * &outsz)
{
	if (!invol1d || !insz) return false;
	if (outvol1d || outsz) return false;
	
	struct PixelPos{double x, y;};
	
	//check if parameters are correct
	
	V3DLONG i,j,k,c;
	
	//get image
	
	V3DLONG nx = insz[0];
	V3DLONG ny = insz[1];
	V3DLONG nz = insz[2];
	V3DLONG nc = insz[3];
	
	//get angles
	
	double alpha; //, beta, gamma;
	alpha = opt.degree;
	double xc = opt.center_x, yc = opt.center_y; //, zc = opt.center_z;
	UBYTE fillcolor = opt.fillcolor;
	
	//==================== matrix of forward transform ===================================
	
	//rotation around z axis: angle alpha
	// [x1,y1,z1,0]' = [ cos(alpha) sin(alpha) 0 0;
	//                  -sin(alpha) cos(alpha) 0 0;
	//                           0         0   1 0;
	//                           0         0   0 1] * [x, y, z, 0]'.
	//
	//
	//rotation around x axis: angle beta
	// [x1,y1,z1,0]' = [         1         0        0    0;
	//                           0   cos(beta) sin(beta) 0;
	//                           0  -sin(beta) cos(beta) 0;
	//                           0         0        0    1] * [x, y, z, 0]';
	//
	//
	//rotation around y axis: angle gamma
	// [x1,y1,z1,0]' = [ cos(gamma) 0 -sin(gamma) 0;
	//                           0  1        0   0;
	//                   sin(gamma) 0  cos(gamma) 0;
	//                           0  0        0   1] * [x, y, z, 0]'.
	//
	//
	// For the arbitrary axis rotation angles: alpha, beta, and gamma, this is formulated as matrix product
	//
	// T = Ty * Tx * Tz = [
	//
	
	// [x1,y1,z1,0]' = [cos(alpha) sin(alpha) 0 0;
	// [x1,y1,z1,0]' = [cos(alpha) sin(alpha) 0 0;
	// [x1,y1,z1,0]' = [cos(alpha) sin(alpha) 0 0;
	// [x1,y1,z1,0]' = [cos(alpha) sin(alpha) 0 0;
	// [x1,y1,z1,0]' = [cos(alpha) sin(alpha) 0 0;
	// [x1,y1,z1,0]' = [cos(alpha) sin(alpha) 0 0;
	// [x1,y1,z1,0]' = [cos(alpha) sin(alpha) 0 0;
	
	//==================== set up the input and output volume matrix =====================
	
	PixelPos e00, e01, e10, e11; //for the four corners
	e00.x = 0 - xc;
	e00.y = 0 - yc;
	
	e01.x = 0 - xc;
	e01.y = (ny-1) - yc;
	
	e10.x = (nx-1) - xc;
	e10.y = 0 - yc;
	
	e11.x = (nx-1) - xc;
	e11.y = (ny-1) - yc;
	
	double c00, c01, c10, c11;
	c00 = cos(alpha);
	c01 = sin(alpha);
	c10 = -sin(alpha);
	c11 = cos(alpha);
	
	PixelPos e00t, e01t, e10t, e11t; //the coordinates of the transformed corners
	e00t.x = c00*e00.x + c01*e00.y;
	e00t.y = c10*e00.x + c11*e00.y;
	
	e01t.x = c00*e01.x + c01*e01.y;
	e01t.y = c10*e01.x + c11*e01.y;
	
	e10t.x = c00*e10.x + c01*e10.y;
	e10t.y = c10*e10.x + c11*e10.y;
	
	e11t.x = c00*e11.x + c01*e11.y;
	e11t.y = c10*e11.x + c11*e11.y;
	
	double px_min = min(min(min(e00t.x, e01t.x), e10t.x), e11t.x);
	double px_max = max(max(max(e00t.x, e01t.x), e10t.x), e11t.x);
	double py_min = min(min(min(e00t.y, e01t.y), e10t.y), e11t.y);
	double py_max = max(max(max(e00t.y, e01t.y), e10t.y), e11t.y);
	
	V3DLONG xlen_out = lround(px_max - px_min + 1); if (opt.b_keepSameSize) {xlen_out = nx; px_min = 0.0-(xlen_out-1.0)/2;}
	V3DLONG ylen_out = lround(py_max - py_min + 1); if (opt.b_keepSameSize) {ylen_out = ny; py_min = 0.0-(ylen_out-1.0)/2;}
	V3DLONG zlen_out = nz;
	V3DLONG clen_out = nc;
	
	outvol1d = new UBYTE [xlen_out*ylen_out*zlen_out*clen_out];
	if (!outvol1d)
	{
		printf("Fail to allocate memory for the output volume.\n");
		return false;
	}
	UBYTE **** outvol4d = 0;
	new4dpointer(outvol4d, xlen_out, ylen_out, zlen_out, clen_out, outvol1d);
	
	UBYTE **** invol4d = 0;
	new4dpointer(invol4d, nx, ny, nz, nc, invol1d);
	
	//printf("xmin=%5.2f, ymin=%5.2f, xmax=%5.2f, ymax=%5.2f\n", px_min, py_min, px_max, py_max);
	
	//============ generate nearest interpolation ===================
	
	double my_eps=1e-8;
	double c00b, c01b, c10b, c11b;
	c00b = cos(-alpha); //c00b=(c00b<my_eps)?0:c00b; c00b=(c00b>1-my_eps)?1:c00b;  
	c01b = sin(-alpha); //c01b=(c01b<my_eps)?0:c01b; c01b=(c01b>1-my_eps)?1:c01b;
	c10b = -sin(-alpha);//c10b=(c10b<my_eps)?0:c10b; c10b=(c10b>1-my_eps)?1:c10b;
	c11b = cos(-alpha); //c11b=(c11b<my_eps)?0:c11b; c11b=(c11b>1-my_eps)?1:c11b;
	
	for (j=0;j<ylen_out; j++)
	{
		for (k=0; k<xlen_out; k++)
		{
			double curpx = c00b*(k+px_min) + c01b*(j+py_min) + xc;
			double curpy = c10b*(k+px_min) + c11b*(j+py_min) + yc;
			
			if (curpx<0) 
			{
				curpx += my_eps; 
			}
			else
			{
				if (curpx>nx-1)
					curpx -= my_eps;
			}
			
			if (curpy<0) 
			{
				curpy += my_eps; 
			}
			else
			{
				if (curpy>ny-1)
					curpy -= my_eps;
			}
			
			if (curpx<0 || curpx>nx-1 || curpy<0 || curpy>ny-1)
			{
				//printf("curpx=%5.2f, curpy=%5.2f, c00b=%5.2f, c01b=%5.2f, c10b=%5.2f, c11b=%5.2f\n", curpx, curpy, c00b, c01b, c10b, c11b);
				for (i=0;i<zlen_out; i++)
				{
					for (c=0;c<nc;c++)
						outvol4d[c][i][j][k] = fillcolor;
				}
			}
			else
			{
				V3DLONG cpx0 = (V3DLONG)(floor(curpx)), cpx1 = (V3DLONG)(ceil(curpx));
				V3DLONG cpy0 = (V3DLONG)(floor(curpy)), cpy1 = (V3DLONG)(ceil(curpy));
				
				if (cpx1==cpx0)
				{
					if (cpy1==cpy0)
					{
						for (i=0;i<zlen_out; i++)
						{
							for (c=0;c<nc;c++)
								outvol4d[c][i][j][k] = invol4d[c][i][cpy0][cpx0];
						}
					}
					else
					{
						double w0x0y = (cpy1-curpy);
						double w0x1y = (curpy-cpy0);
						
						for (i=0;i<zlen_out; i++)
						{
							for (c=0;c<nc;c++)
								outvol4d[c][i][j][k] = (UBYTE)(w0x0y * double(invol4d[c][i][cpy0][cpx0]) + w0x1y * double(invol4d[c][i][cpy1][cpx0]));
						}
					}
				}
				else
				{
					if (cpy1==cpy0)
					{
						double w0x0y = (cpx1-curpx);
						double w1x0y = (curpx-cpx0);
						
						for (i=0;i<zlen_out; i++)
						{
							for (c=0;c<nc;c++)
								outvol4d[c][i][j][k] = (UBYTE)(w0x0y * double(invol4d[c][i][cpy0][cpx0]) + 
															   w1x0y * double(invol4d[c][i][cpy0][cpx1]));
							
						}
					}
					else //(cpx1!=cpx0 && cpy1!=cpy0) 
					{
						double w0x0y = (cpx1-curpx)*(cpy1-curpy);
						double w0x1y = (cpx1-curpx)*(curpy-cpy0);
						double w1x0y = (curpx-cpx0)*(cpy1-curpy);
						double w1x1y = (curpx-cpx0)*(curpy-cpy0);
						
						for (i=0;i<zlen_out; i++)
						{
							for (c=0;c<nc;c++)
								outvol4d[c][i][j][k] = (UBYTE)(w0x0y * double(invol4d[c][i][cpy0][cpx0]) + w0x1y * double(invol4d[c][i][cpy1][cpx0]) +
															   w1x0y * double(invol4d[c][i][cpy0][cpx1]) + w1x1y * double(invol4d[c][i][cpy1][cpx1]));
							
						}
					}
				}   
			}
		}
	}
	
	// ====free memory=============
	if (!outsz) {outsz = new V3DLONG [4];}
	outsz[0] = xlen_out; outsz[1] = ylen_out; outsz[2] = zlen_out; outsz[3] = clen_out; 
	delete4dpointer(outvol4d, xlen_out, ylen_out, zlen_out, clen_out);
	delete4dpointer(invol4d, nx, ny, nz, nc);
	
	return true;
}

bool rotate_inPlaneX(UBYTE *invol1d, V3DLONG *insz, const Options_Rotate & opt, //xc, yc, zc are the rotation centers
                     UBYTE * &outvol1d, V3DLONG * &outsz)
{
	if (!invol1d || !insz) return false;
	if (outvol1d || outsz) return false;
	
	struct PixelPos { double  y, z;};//x};
		
	//check if parameters are correct
	
	V3DLONG i,j,k,c;
	
	//get image
	
	V3DLONG nx = insz[0];
	V3DLONG ny = insz[1];
	V3DLONG nz = insz[2];
	V3DLONG nc = insz[3];
	
	//get angles
	
	double alpha; //, beta, gamma;
	alpha = opt.degree;
	double xc = opt.center_x, yc = opt.center_y, zc = opt.center_z;
	UBYTE fillcolor = opt.fillcolor;
	
	//==================== matrix of forward transform ===================================
	
	//rotation around z axis: angle alpha
	// [x1,y1,z1,0]' = [ cos(alpha) sin(alpha) 0 0;
	//                  -sin(alpha) cos(alpha) 0 0;
	//                           0         0   1 0;
	//                           0         0   0 1] * [x, y, z, 0]'.
	//
	//
	//rotation around x axis: angle beta
	// [x1,y1,z1,0]' = [         1         0        0    0;
	//                           0   cos(beta) sin(beta) 0;
	//                           0  -sin(beta) cos(beta) 0;
	//                           0         0        0    1] * [x, y, z, 0]';
	//
	//
	//rotation around y axis: angle gamma
	// [x1,y1,z1,0]' = [ cos(gamma) 0 -sin(gamma) 0;
	//                           0  1        0   0;
	//                   sin(gamma) 0  cos(gamma) 0;
	//                           0  0        0   1] * [x, y, z, 0]'.
	//
	//
	// For the arbitrary axis rotation angles: alpha, beta, and gamma, this is formulated as matrix product
	//
	// T = Ty * Tx * Tz = [
	//
	
	// [x1,y1,z1,0]' = [cos(alpha) sin(alpha) 0 0;
	// [x1,y1,z1,0]' = [cos(alpha) sin(alpha) 0 0;
	// [x1,y1,z1,0]' = [cos(alpha) sin(alpha) 0 0;
	// [x1,y1,z1,0]' = [cos(alpha) sin(alpha) 0 0;
	// [x1,y1,z1,0]' = [cos(alpha) sin(alpha) 0 0;
	// [x1,y1,z1,0]' = [cos(alpha) sin(alpha) 0 0;
	// [x1,y1,z1,0]' = [cos(alpha) sin(alpha) 0 0;
	
	//==================== set up the input and output volume matrix =====================
	
	PixelPos e00, e01, e10, e11; //for the four corners
	e00.y = 0 - yc;
	e00.z = 0 - zc;
	
	e01.y = 0 - yc;
	e01.z = (nz-1) - zc;
	
	e10.y = (ny-1) - yc;
	e10.z = 0 - zc;
	
	e11.y = (ny-1) - yc;
	e11.z = (nz-1) - zc;
	
	double c00, c01, c10, c11;
	c00 = cos(alpha);
	c01 = sin(alpha);
	c10 = -sin(alpha);
	c11 = cos(alpha);
	
	PixelPos e00t, e01t, e10t, e11t; //the coordinates of the transformed corners
	e00t.y = c00*e00.y + c01*e00.z;
	e00t.z = c10*e00.y + c11*e00.z;
	
	e01t.y = c00*e01.y + c01*e01.z;
	e01t.z = c10*e01.y + c11*e01.z;
	
	e10t.y = c00*e10.y + c01*e10.z;
	e10t.z = c10*e10.y + c11*e10.z;
	
	e11t.y = c00*e11.y + c01*e11.z;
	e11t.z = c10*e11.y + c11*e11.z;
	
	double py_min = min(min(min(e00t.y, e01t.y), e10t.y), e11t.y);
	double py_max = max(max(max(e00t.y, e01t.y), e10t.y), e11t.y);
	double pz_min = min(min(min(e00t.z, e01t.z), e10t.z), e11t.z);
	double pz_max = max(max(max(e00t.z, e01t.z), e10t.z), e11t.z);
	
	V3DLONG xlen_out = nx;
	V3DLONG ylen_out = lround(py_max - py_min + 1); if (opt.b_keepSameSize) {ylen_out = ny; py_min = 0.0-(ylen_out-1.0)/2;}
	V3DLONG zlen_out = lround(pz_max - pz_min + 1); if (opt.b_keepSameSize) {zlen_out = nz; pz_min = 0.0-(zlen_out-1.0)/2;}
	V3DLONG clen_out = nc;
	
	outvol1d = new UBYTE [xlen_out*ylen_out*zlen_out*clen_out];
	if (!outvol1d)
	{
		printf("Fail to allocate memory for the output volume.\n");
		return false;
	}
	UBYTE **** outvol4d = 0;
	new4dpointer(outvol4d, xlen_out, ylen_out, zlen_out, clen_out, outvol1d);
	
	UBYTE **** invol4d=0;
	new4dpointer(invol4d, nx, ny, nz, nc, invol1d);
	
	//printf("ymin=%5.2f, zmin=%5.2f, ymax=%5.2f, zmax=%5.2f\n", py_min, pz_min, py_max, pz_max);
		
	//============ generate nearest interpolation ===================
		
	double my_eps=1e-8;
	double c00b, c01b, c10b, c11b;
	c00b = cos(-alpha); 
	c01b = sin(-alpha); 
	c10b = -sin(-alpha);
	c11b = cos(-alpha); 
	
	for (j=0;j<ylen_out; j++)
	{
		for (i=0; i<zlen_out; i++)
		{
			double curpy = c00b*(j+py_min) + c01b*(i+pz_min) + yc;
			double curpz = c10b*(j+py_min) + c11b*(i+pz_min) + zc;
			
			if (curpy<0) 
			{
				curpy += my_eps; 
			}
			else
			{
				if (curpy>ny-1)
					curpy -= my_eps;
			}
			
			if (curpz<0) 
			{
				curpz += my_eps; 
			}
			else
			{
				if (curpz>nz-1)
					curpz -= my_eps;
			}
			
			if (curpy<0 || curpy>ny-1 || curpz<0 || curpz>nz-1)
			{
				//printf("curpy=%5.2f, curpz=%5.2f, c00b=%5.2f, c01b=%5.2f, c10b=%5.2f, c11b=%5.2f\n", curpy, curpz, c00b, c01b, c10b, c11b);
				for (k=0;k<xlen_out; k++)
				{
					for (c=0;c<nc;c++)
						outvol4d[c][i][j][k] = fillcolor;
				}
			}
			else
			{
				//                 printf("curpy=%5.2f, curpz=%5.2f\n");
				
				V3DLONG cpy0 = (V3DLONG)(floor(curpy)), cpy1 = (V3DLONG)(ceil(curpy));
				V3DLONG cpz0 = (V3DLONG)(floor(curpz)), cpz1 = (V3DLONG)(ceil(curpz));
				
				if (cpy1==cpy0)
				{
					if (cpz1==cpz0)
					{
						for (k=0;k<xlen_out; k++)
						{
							for (c=0;c<nc;c++)
								outvol4d[c][i][j][k] = invol4d[c][cpz0][cpy0][k];
						}
					}
					else
					{
						double w0y0z = (cpz1-curpz);
						double w0y1z = (curpz-cpz0);
						
						for (k=0;k<xlen_out; k++)
						{
							for (c=0;c<nc;c++)
								outvol4d[c][i][j][k] = (UBYTE)(w0y0z * double(invol4d[c][cpz0][cpy0][k]) + w0y1z * double(invol4d[c][cpz1][cpy0][k]));
						}
					}
				}
				else
				{
					if (cpz1==cpz0)
					{
						double w0y0z = (cpy1-curpy);
						double w1y0z = (curpy-cpy0);
						
						for (k=0;k<xlen_out; k++)
						{
							for (c=0;c<nc;c++)
								outvol4d[c][i][j][k] = (UBYTE)(w0y0z * double(invol4d[c][cpz0][cpy0][k]) + 
														   w1y0z * double(invol4d[c][cpz0][cpy1][k]));
							
						}
					}
					else 
					{
						//printf("curpy=%5.2f, curpz=%5.2f i=%d j=%d cpz0=%d cpy0=%d\n", curpy, curpz, i, j, cpz0, cpy0);
						
						double w0y0z = (cpy1-curpy)*(cpz1-curpz);
						double w0y1z = (cpy1-curpy)*(curpz-cpz0);
						double w1y0z = (curpy-cpy0)*(cpz1-curpz);
						double w1y1z = (curpy-cpy0)*(curpz-cpz0);
						
						for (k=0;k<xlen_out; k++)
						{
							for (c=0;c<nc;c++)
								outvol4d[c][i][j][k] = (UBYTE)(w0y0z * double(invol4d[c][cpz0][cpy0][k]) + w0y1z * double(invol4d[c][cpz1][cpy0][k]) +
														   w1y0z * double(invol4d[c][cpz0][cpy1][k]) + w1y1z * double(invol4d[c][cpz1][cpy1][k]));
							
						}
					}
				}   
			}
		}
	}
	
	// ====free memory=============
	if (!outsz) {outsz = new V3DLONG [4];}
	outsz[0] = xlen_out; outsz[1] = ylen_out; outsz[2] = zlen_out; outsz[3] = clen_out; 
	delete4dpointer(outvol4d, xlen_out, ylen_out, zlen_out, clen_out);
	delete4dpointer(invol4d, nx, ny, nz, nc);
	
	return true;
}


bool rotate_inPlaneY(UBYTE *invol1d, V3DLONG *insz, const Options_Rotate & opt, //xc, yc, zc are the rotation centers
                     UBYTE * &outvol1d, V3DLONG * &outsz)
{
	if (!invol1d || !insz) return false;
	if (outvol1d || outsz) return false;
	
	struct PixelPos{ double  x, z;}; //y};
		
	//check if parameters are correct
	
	V3DLONG i,j,k,c;
	
	//get image
	
	V3DLONG nx = insz[0];
	V3DLONG ny = insz[1];
	V3DLONG nz = insz[2];
	V3DLONG nc = insz[3];
	
	//get options
	
	double alpha; //, beta, gamma;
	alpha = opt.degree;
	double xc = opt.center_x, yc = opt.center_y, zc = opt.center_z;
	UBYTE fillcolor = opt.fillcolor;
	
	//==================== matrix of forward transform ===================================
	
	//rotation around z axis: angle alpha
	// [x1,y1,z1,0]' = [ cos(alpha) sin(alpha) 0 0;
	//                  -sin(alpha) cos(alpha) 0 0;
	//                           0         0   1 0;
	//                           0         0   0 1] * [x, y, z, 0]'.
	//
	//
	//rotation around x axis: angle beta
	// [x1,y1,z1,0]' = [         1         0        0    0;
	//                           0   cos(beta) sin(beta) 0;
	//                           0  -sin(beta) cos(beta) 0;
	//                           0         0        0    1] * [x, y, z, 0]';
	//
	//
	//rotation around y axis: angle gamma
	// [x1,y1,z1,0]' = [ cos(gamma) 0 -sin(gamma) 0;
	//                           0  1        0   0;
	//                   sin(gamma) 0  cos(gamma) 0;
	//                           0  0        0   1] * [x, y, z, 0]'.
	//
	//
	// For the arbitrary axis rotation angles: alpha, beta, and gamma, this is formulated as matrix product
	//
	// T = Ty * Tx * Tz = [
	//
	
	// [x1,y1,z1,0]' = [cos(alpha) sin(alpha) 0 0;
	// [x1,y1,z1,0]' = [cos(alpha) sin(alpha) 0 0;
	// [x1,y1,z1,0]' = [cos(alpha) sin(alpha) 0 0;
	// [x1,y1,z1,0]' = [cos(alpha) sin(alpha) 0 0;
	// [x1,y1,z1,0]' = [cos(alpha) sin(alpha) 0 0;
	// [x1,y1,z1,0]' = [cos(alpha) sin(alpha) 0 0;
	// [x1,y1,z1,0]' = [cos(alpha) sin(alpha) 0 0;
	
	//==================== set up the input and output volume matrix =====================
	
	PixelPos e00, e01, e10, e11; //for the four corners
	e00.x = 0 - xc;
	e00.z = 0 - zc;
	
	e01.x = 0 - xc;
	e01.z = (nz-1) - zc;
	
	e10.x = (nx-1) - xc;
	e10.z = 0 - zc;
	
	e11.x = (nx-1) - xc;
	e11.z = (nz-1) - zc;
	
	double c00, c01, c10, c11;
	c00 = cos(alpha);
	c01 = -sin(alpha);
	c10 = sin(alpha);
	c11 = cos(alpha);
	
	PixelPos e00t, e01t, e10t, e11t; //the coordinates of the transformed corners
	e00t.x = c00*e00.x + c01*e00.z;
	e00t.z = c10*e00.x + c11*e00.z;
	
	e01t.x = c00*e01.x + c01*e01.z;
	e01t.z = c10*e01.x + c11*e01.z;
	
	e10t.x = c00*e10.x + c01*e10.z;
	e10t.z = c10*e10.x + c11*e10.z;
	
	e11t.x = c00*e11.x + c01*e11.z;
	e11t.z = c10*e11.x + c11*e11.z;
	
	double px_min = min(min(min(e00t.x, e01t.x), e10t.x), e11t.x);
	double px_max = max(max(max(e00t.x, e01t.x), e10t.x), e11t.x);
	double pz_min = min(min(min(e00t.z, e01t.z), e10t.z), e11t.z);
	double pz_max = max(max(max(e00t.z, e01t.z), e10t.z), e11t.z);
	
	V3DLONG xlen_out = lround(px_max - px_min + 1); if (opt.b_keepSameSize) {xlen_out = nx; px_min = 0.0-(xlen_out-1.0)/2;}
	V3DLONG ylen_out = ny;
	V3DLONG zlen_out = lround(pz_max - pz_min + 1); if (opt.b_keepSameSize) {zlen_out = nz; pz_min = 0.0-(zlen_out-1.0)/2;}
	V3DLONG clen_out = nc;
	
	outvol1d = new UBYTE [xlen_out*ylen_out*zlen_out*clen_out];
	if (!outvol1d)
	{
		printf("Fail to allocate memory for the output volume.\n");
		return false;
	}
	UBYTE **** outvol4d = 0;
	new4dpointer(outvol4d, xlen_out, ylen_out, zlen_out, clen_out, outvol1d);
	
	UBYTE **** invol4d=0;
	new4dpointer(invol4d, nx, ny, nz, nc, invol1d);
	
	//============ generate nearest interpolation ===================
	
	double my_eps=1e-8;
	double c00b, c01b, c10b, c11b;
	c00b = cos(-alpha); 
	c01b = -sin(-alpha); 
	c10b = sin(-alpha);
	c11b = cos(-alpha); 
	
	for (k=0;k<xlen_out; k++)
	{
		for (i=0; i<zlen_out; i++)
		{
			double curpx = c00b*(k+px_min) + c01b*(i+pz_min) + xc;
			double curpz = c10b*(k+px_min) + c11b*(i+pz_min) + zc;
			
			if (curpx<0) 
			{
				curpx += my_eps; 
			}
			else
			{
				if (curpx>nx-1)
					curpx -= my_eps;
			}
			
			if (curpz<0) 
			{
				curpz += my_eps; 
			}
			else
			{
				if (curpz>nz-1)
					curpz -= my_eps;
			}
			
			if (curpx<0 || curpx>nx-1 || curpz<0 || curpz>nz-1)
			{
				//printf("curpy=%5.2f, curpz=%5.2f, c00b=%5.2f, c01b=%5.2f, c10b=%5.2f, c11b=%5.2f\n", curpy, curpz, c00b, c01b, c10b, c11b);
				for (j=0;j<ylen_out; j++)
				{
					for (c=0;c<nc;c++)
						outvol4d[c][i][j][k] = fillcolor;
				}
			}
			else
			{
				//                 printf("curpy=%5.2f, curpz=%5.2f\n");
				
				V3DLONG cpx0 = (V3DLONG)(floor(curpx)), cpx1 = (V3DLONG)(ceil(curpx));
				V3DLONG cpz0 = (V3DLONG)(floor(curpz)), cpz1 = (V3DLONG)(ceil(curpz));
				
				if (cpx1==cpx0)
				{
					if (cpz1==cpz0)
					{
						for (j=0;j<ylen_out; j++)
						{
							for (c=0;c<nc;c++)
								outvol4d[c][i][j][k] = invol4d[c][cpz0][j][cpx0];
						}
					}
					else
					{
						double w0x0z = (cpz1-curpz);
						double w0x1z = (curpz-cpz0);
						
						for (j=0;j<ylen_out; j++)
						{
							for (c=0;c<nc;c++)
								outvol4d[c][i][j][k] = (UBYTE)(w0x0z * double(invol4d[c][cpz0][j][cpx0]) + w0x1z * double(invol4d[c][cpz1][j][cpx0]));
						}
					}
				}
				else
				{
					if (cpz1==cpz0)
					{
						double w0x0z = (cpx1-curpx);
						double w1x0z = (curpx-cpx0);
						
						for (j=0;j<ylen_out; j++)
						{
							for (c=0;c<nc;c++)
								outvol4d[c][i][j][k] = (UBYTE)(w0x0z * double(invol4d[c][cpz0][j][cpx0]) + 
														   w1x0z * double(invol4d[c][cpz0][j][cpx1]));
							
						}
					}
					else 
					{
						//printf("curpy=%5.2f, curpz=%5.2f i=%d j=%d cpz0=%d cpy0=%d\n", curpy, curpz, i, j, cpz0, cpy0);
						
						double w0x0z = (cpx1-curpx)*(cpz1-curpz);
						double w0x1z = (cpx1-curpx)*(curpz-cpz0);
						double w1x0z = (curpx-cpx0)*(cpz1-curpz);
						double w1x1z = (curpx-cpx0)*(curpz-cpz0);
						
						for (j=0;j<ylen_out; j++)
						{
							for (c=0;c<nc;c++)
								outvol4d[c][i][j][k] = (UBYTE)(w0x0z * double(invol4d[c][cpz0][j][cpx0]) + w0x1z * double(invol4d[c][cpz1][j][cpx0]) +
														   w1x0z * double(invol4d[c][cpz0][j][cpx1]) + w1x1z * double(invol4d[c][cpz1][j][cpx1]));
							
						}
					}
				}   
			}
		}
	}
	
	// ====free memory=============
	if (!outsz) {outsz = new V3DLONG [4];}
	outsz[0] = xlen_out; outsz[1] = ylen_out; outsz[2] = zlen_out; outsz[3] = clen_out; 
	delete4dpointer(outvol4d, xlen_out, ylen_out, zlen_out, clen_out);
	delete4dpointer(invol4d, nx, ny, nz, nc);
	
	return true;
}


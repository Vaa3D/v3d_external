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




//remove non_affine_points.h
//This is changed from the ealier version of q_killwrongmatch.h
// kill the wrong matches(outliers) by comparing the distance between the point and affine inverse projected correponding point
//	where the affine matrix is computed by using RANSAC
// by Lei Qu
// 2009-07-08
//last update, 090710 by Hanchuan Peng
// LAST UPDATE 2010-05-21. PHC. REVISE THE __REMOVE_NONAFFINE_POINTS_H__ 
//

#ifndef __REMOVE_NONAFFINE_POINTS_H__
#define __REMOVE_NONAFFINE_POINTS_H__

#include <stdio.h>
#include <vector>
using namespace std;

#define WANT_STREAM
#include "../newmat11/newmatap.h"
#include "../newmat11/newmatio.h"

struct Coord3D_PCR
{
	double x,y,z;
	Coord3D_PCR(double x0,double y0,double z0) {x=x0;y=y0;z=z0;}
	Coord3D_PCR() {x=y=z=0;}
};


//kill the wrong matches(outliers) by comparing the distance between the point and affine inverse projected correponding point
//	where the affine matrix is computed by using RANSAC
//
//	input:
//		1. arr_1,arr_2: 			two matched pairs needed to be checked, matched pairs have the same index
//		2. n_sampling: 				number of perform RANSAC sampling (default 1000)
//		3. n_pairs_per_sampling:	number of sample pairs used to compute affine matrix in each RANSAC sampling iteration (default 5)
//		4. f_kill_factor:			if the distance between a point and the affine inverse projected correponding point is bigger than
//									kill_factor*average_dis, then this pair is killed (default 2.0)
//	output:
//		1. x4x4_affinematrix_2to1: 	the fitting affine matrix   arr_1 ~= T * arr_2
//		2. arr1d_1to2index: 		matching index, arr1d_A2Bindex(i)=j <==> A(i)-->B(j)
//		3. arr_2_invp:				inverse projection of arr_2: arr_2_invp=T * arr_2
//		4. arr_1/2_afterkill:		the output matched pairs after kill wrong matches, matched pairs have the same index
//
bool q_killwrongmatch(const vector<Coord3D_PCR> &arr_1,const vector<Coord3D_PCR> &arr_2,
					const int n_sampling, const int n_pairs_per_sampling, const float f_kill_factor,
					Matrix &x4x4_affinematrix_2to1,vector<int> &arr1d_1to2index,vector<Coord3D_PCR> &arr_2_invp,
					vector<Coord3D_PCR> &arr_1_afterkill,vector<Coord3D_PCR> &arr_2_afterkill);



//************************************************************************************************************************************
//centrilize and scale the point set
bool q_normalize_points(const vector<Coord3D_PCR> arr_input,vector<Coord3D_PCR> &vec_output,Matrix &x4x4_normalize);



//find the best affine matrix based on the initial matching result using RANSAC method
bool q_RANSAC_affinmatrix_from_initialmatch(const vector<Coord3D_PCR> &arr_A,const vector<Coord3D_PCR> &arr_B,
											const vector<int> &arr1d_A2Bindex,
											const int n_sampling,const int n_pairs_per_sampling,
											Matrix &x4x4_affinematrix);
//compute the affine matraix
bool q_compute_affinmatrix3D(const vector<Coord3D_PCR> &arr_A,const vector<Coord3D_PCR> &arr_B,Matrix &x4x4_affinematrix);
//compute the inverse projection error
bool q_compute_invp_err(const vector<Coord3D_PCR> &arr_A,const vector<Coord3D_PCR> &arr_B,const Matrix &x4x4_affinematrix,
						vector< vector<double> > &arr_dismatrix,double &d_invp_err);



//output the matched pair to swc format
bool q_export_matches2swc(const vector<Coord3D_PCR> &arr_1,const vector<Coord3D_PCR> &arr_2,const vector<int> &arr1d_1to2index,const char *filename);
//output the matched pair to apo format (only points corrdinate, no matching information)
bool q_export_matches2apo(const vector<Coord3D_PCR> &arr_1,const vector<Coord3D_PCR> &arr_2,const vector<Coord3D_PCR> &arr_2_invp,const char *filename,const int output_option);
//output the matched pair to marker format (only points corrdinate, no matching information)
bool q_export_matches2marker(const vector<Coord3D_PCR> &arr_1,const vector<Coord3D_PCR> &arr_2,const vector<Coord3D_PCR> &arr_2_invp,const char *filename,const int output_option);

#endif


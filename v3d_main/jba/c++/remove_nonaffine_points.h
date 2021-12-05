//remove non_affine_points.h
//by Lei Qu and Hanchuan Peng
// 2009-2011

#ifndef __REMOVE_NONAFFINE_POINTS_H__
#define __REMOVE_NONAFFINE_POINTS_H__

#include <stdio.h>
#include <vector>
using namespace std;

#define WANT_STREAM
#include "newmatap.h"
#include "newmatio.h"

struct Coord3D_PCR
{
	double x,y,z;
	Coord3D_PCR(double x0,double y0,double z0) {x=x0;y=y0;z=z0;}
	Coord3D_PCR() {x=y=z=0;}
};


bool q_killwrongmatch(const vector<Coord3D_PCR> &arr_1,const vector<Coord3D_PCR> &arr_2,
					const int n_sampling, const int n_pairs_per_sampling, const float f_kill_factor,
					Matrix &x4x4_affinematrix_2to1,vector<int> &arr1d_1to2index,vector<Coord3D_PCR> &arr_2_invp,
					vector<Coord3D_PCR> &arr_1_afterkill,vector<Coord3D_PCR> &arr_2_afterkill);

bool q_normalize_points(const vector<Coord3D_PCR> arr_input,vector<Coord3D_PCR> &vec_output,Matrix &x4x4_normalize);

bool q_RANSAC_affinmatrix_from_initialmatch(const vector<Coord3D_PCR> &arr_A,const vector<Coord3D_PCR> &arr_B,
											const vector<int> &arr1d_A2Bindex,
											const int n_sampling,const int n_pairs_per_sampling,
											Matrix &x4x4_affinematrix);
bool q_compute_affinmatrix3D(const vector<Coord3D_PCR> &arr_A,const vector<Coord3D_PCR> &arr_B,Matrix &x4x4_affinematrix);
bool q_compute_invp_err(const vector<Coord3D_PCR> &arr_A,const vector<Coord3D_PCR> &arr_B,const Matrix &x4x4_affinematrix,
						vector< vector<double> > &arr_dismatrix,double &d_invp_err);

bool q_export_matches2swc(const vector<Coord3D_PCR> &arr_1,const vector<Coord3D_PCR> &arr_2,const vector<int> &arr1d_1to2index,const char *filename);
bool q_export_matches2apo(const vector<Coord3D_PCR> &arr_1,const vector<Coord3D_PCR> &arr_2,const vector<Coord3D_PCR> &arr_2_invp,const char *filename,const int output_option);
bool q_export_matches2marker(const vector<Coord3D_PCR> &arr_1,const vector<Coord3D_PCR> &arr_2,const vector<Coord3D_PCR> &arr_2_invp,const char *filename,const int output_option);

#endif


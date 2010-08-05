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




//jba_match_landmarks.h
// by Hanchuan Peng//
// separated from jba_mainfunc.h
//2008-11-27

#ifndef __JBA_MATCH_LANDMARKS_H__
#define __JBA_MATCH_LANDMARKS_H__

#include "jba_mainfunc.h"

template <class T> double cmpFeaturesonTwoImages(Vol3DSimple<T>* img_target, V3DLONG txpos, V3DLONG typos, V3DLONG tzpos,
												 Vol3DSimple<T>* img_subject, V3DLONG sxpos, V3DLONG sypos, V3DLONG szpos,
												 V3DLONG R_compare, const KernelSet * KS); //R_compare is the radius of comparison is to be performed
template <class T> double cmpFeaturesonTwoImages(Vol3DSimple<T>* img_target, V3DLONG txpos, V3DLONG typos, V3DLONG tzpos,
												 Vol3DSimple<T>* img_subject, V3DLONG sxpos, V3DLONG sypos, V3DLONG szpos,
												 V3DLONG R_compare, const PointMatchMethodType mMethod, Vector1DSimple<double> * matchingScore); //R_compare is the radius of comparison is to be performed

bool detectBestMatchingCpt_virtual(vector<Coord3D_JBA> & matchTargetPos,
								   vector<Coord3D_JBA> & matchSubjectPos,
								   Vol3DSimple<MYFLOAT_JBA> * img_target,
								   Vol3DSimple<MYFLOAT_JBA> * img_subject,
								   PointMatchScore & matchScore,
								   const vector<Coord3D_JBA> & priorTargetPos,
								   //PointMatchMethodType mMethod = MATCH_MULTIPLE_MI_INT_CORR
								   const BasicWarpParameter & bwp
								   );

bool detectBestMatchingCpt_virtual(vector<Coord3D_JBA> & matchTargetPos,
								   vector<Coord3D_JBA> & matchSubjectPos,
								   Vol3DSimple<unsigned char> * img_target,
								   Vol3DSimple<unsigned char> * img_subject,
								   PointMatchScore & matchScore,
								   const vector<Coord3D_JBA> & priorTargetPos,
								   //PointMatchMethodType mMethod = MATCH_MULTIPLE_MI_INT_CORR
								   const BasicWarpParameter & bwp
								   );

template <class T> bool detectBestMatchingCpt(vector<Coord3D_JBA> & matchTargetPos,
											  vector<Coord3D_JBA> & matchSubjectPos,
											  Vol3DSimple<T> * img_target,
											  Vol3DSimple<T> * img_subject,
											  PointMatchScore & matchScore,
											  const vector<Coord3D_JBA> & priorTargetPos0,
											  Vol3DSimple<unsigned short int> *img_target_matchrange,				  
											  Vol3DSimple<unsigned short int> *img_subject_matchrange,				  
											  //PointMatchMethodType mMethod = MATCH_MULTIPLE_MI_INT_CORR
											  const BasicWarpParameter & bwp
											  );

bool analyze_model_matching_smoothness(vector<Coord3D_JBA> & matchTargetPos, vector<Coord3D_JBA> & matchSubjectPos, PointMatchScore & matchScore);

double compute_angle_triangle_edge(double e_target, double e1, double e2);
double compute_dist_two_pts(const Coord3D_JBA & pt1, const Coord3D_JBA &  pt2);
double compute_dist_pt_plane(const Coord3D_JBA & pt1, const Coord3D_JBA &  pt2, const Coord3D_JBA &  pt3, const Coord3D_JBA &  pt_n);

template <class T> bool regularMoment(double & valMoments, Vol3DSimple<T> *img, V3DLONG x0, V3DLONG y0, V3DLONG z0, int r, int pp, int qq, int rr);
template <class T> bool computeGMI(Vector1DSimple<double> & momentVec, Vol3DSimple<T> *data3d, V3DLONG x0, V3DLONG y0, V3DLONG z0, int r);

template <class T> bool regularMoment(double & valMoments, Vol3DSimple<T> *img, V3DLONG x0, V3DLONG y0, V3DLONG z0, int r, int pp, int qq, int rr, const KernelSet * ks);
template <class T> bool computeGMI(Vector1DSimple<double> & momentVec, Vol3DSimple<T> *data3d, V3DLONG x0, V3DLONG y0, V3DLONG z0, int r, const KernelSet * ks);

//compute the average at different radii as a feature vector
template <class T> bool computeCircularMeanVector(Vector1DSimple<double> & meanVec, Vector1DSimple<double> & cntVec, Vol3DSimple<T> *img, V3DLONG x0, V3DLONG y0, V3DLONG z0, int r);

//template <class T> double calCorrelation(T * img1, T * img2, V3DLONG imglen);

template <class T> bool computeCircularScatteringScore(Vector1DSimple<double> & meanVec, Vector1DSimple<double> & cntVec, Vector1DSimple<double> & scatterVec, Vol3DSimple<T> *img, V3DLONG x0, V3DLONG y0, V3DLONG z0, int r);

#endif


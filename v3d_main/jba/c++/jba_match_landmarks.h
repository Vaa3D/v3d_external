//jba_match_landmarks.h
// by Hanchuan Peng
// 2006-2011

#ifndef __JBA_MATCH_LANDMARKS_H__
#define __JBA_MATCH_LANDMARKS_H__

#include "jba_mainfunc.h"

template <class T> double cmpFeaturesonTwoImages(Vol3DSimple<T>* img_target, V3DLONG txpos, V3DLONG typos, V3DLONG tzpos,
												 Vol3DSimple<T>* img_subject, V3DLONG sxpos, V3DLONG sypos, V3DLONG szpos,
												 V3DLONG R_compare, const KernelSet * KS); 
template <class T> double cmpFeaturesonTwoImages(Vol3DSimple<T>* img_target, V3DLONG txpos, V3DLONG typos, V3DLONG tzpos,
												 Vol3DSimple<T>* img_subject, V3DLONG sxpos, V3DLONG sypos, V3DLONG szpos,
												 V3DLONG R_compare, const PointMatchMethodType mMethod, Vector1DSimple<double> * matchingScore); 

bool detectBestMatchingCpt_virtual(vector<Coord3D_JBA> & matchTargetPos,
								   vector<Coord3D_JBA> & matchSubjectPos,
								   Vol3DSimple<MYFLOAT_JBA> * img_target,
								   Vol3DSimple<MYFLOAT_JBA> * img_subject,
								   PointMatchScore & matchScore,
								   const vector<Coord3D_JBA> & priorTargetPos,
								   const BasicWarpParameter & bwp
								   );

bool detectBestMatchingCpt_virtual(vector<Coord3D_JBA> & matchTargetPos,
								   vector<Coord3D_JBA> & matchSubjectPos,
								   Vol3DSimple<unsigned char> * img_target,
								   Vol3DSimple<unsigned char> * img_subject,
								   PointMatchScore & matchScore,
								   const vector<Coord3D_JBA> & priorTargetPos,
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
											  const BasicWarpParameter & bwp
											  );

bool analyze_model_matching_smoothness(vector<Coord3D_JBA> & matchTargetPos, vector<Coord3D_JBA> & matchSubjectPos, PointMatchScore & matchScore);

double compute_angle_triangle_edge(double e_target, double e1, double e2);
double compute_dist_two_pts(const Coord3D_JBA & pt1, const Coord3D_JBA &  pt2);
double compute_dist_pt_plane(const Coord3D_JBA & pt1, const Coord3D_JBA &  pt2, const Coord3D_JBA &  pt3, const Coord3D_JBA &  pt_n);

#endif


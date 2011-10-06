//jba_affine_xform.h
// by Hanchuan Peng
// 2006-2011

#ifndef __JBA_AFFINE_XFORM_H__
#define __JBA_AFFINE_XFORM_H__

#include "jba_mainfunc.h"



bool compute_affine_xform_parameters_from_landmarks(
													const vector<Coord3D_JBA> & matchTargetPos,
													const vector<Coord3D_JBA> & matchSubjectPos,
													WarpParameterAffine3D * p,
													bool b_auto_recenter);
													
#endif


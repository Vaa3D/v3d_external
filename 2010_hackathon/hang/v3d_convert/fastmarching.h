/******************************************************
 * perform_fastmarching
 *
 * parameters: seed  (the initial know points)
 *             phi   (the level set matrix)
 *             width, height, depth ( the size )
 *
 * reference: 
 * Fast extraction of minimal paths in 3D images and application to virtual endoscopy.  
 *   T.Deschamps and L.D. Cohen. 
 *   September 2000. To appear in  Medical Image Analysis.
 ******************************************************/
#ifndef _FASTMARCHING_H_H
#define _FASTMARCHING_H_H

#include "v3d_basicdatatype.h"
#include <vector>
using namespace std;

bool perform_fastmarching(double *&phi, unsigned char* inimg1d, V3DLONG * &sz, double thresh);

#endif

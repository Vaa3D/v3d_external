#ifndef __WAVELETTRANSFORM_H__
#define __WAVELETTRANSFORM_H__

#include <iostream.h>
#include <stdlib.h>
#include <math.h>

#include "waveletConfigException.h"


double** b3WaveletScales(double* dataIn, int szx, int szy, int szZ, int numScales)  throw (WaveletConfigException);
void b3WaveletCoefficientsInplace(double** coefficients, double* originalImage, double* lowPass, int numScales, int numVoxels);
void b3WaveletReconstruction(double** inputCoefficients, double* lowPassResidual, double* output, int numScales, int numVoxels);

#endif

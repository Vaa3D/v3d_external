#ifndef __WAVELETTRANSFORM_H__
#define __WAVELETTRANSFORM_H__

#include <iostream>
#include <stdlib.h>
#include <math.h>

#include "waveletConfigException.h"

void b3WaveletCoefficientsInplace(double** coefficients, double* originalImage, double* lowPass, int numScales, int numVoxels);
void b3WaveletReconstruction(double** inputCoefficients, double* lowPassResidual, double* output, int numScales, int numVoxels);

double**  b3WaveletScales2D(double* dataIn, int width, int height, int numScales) throw (WaveletConfigException);
void filterAndSwap2D(double* arrayIn, double* arrayOut, int width, int height, int stepS);

double**  b3WaveletScales(double* dataIn, int width, int height, int depth, int numScales) throw (WaveletConfigException);
void filterAndSwap(double* arrayIn, double* arrayOut, int width, int height, int depth, int stepS);

#endif

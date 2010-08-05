#include "waveletTransform.h"

void checkImageDimensions(int width, int height, int depth, int numScales) throw (WaveletConfigException)
{
	 //check that image dimensions complies with the number of chosen scales
 	int minSize = 5+(pow(2, numScales-1))*4;
 	if (width < minSize || height < minSize || depth < minSize)
  	{
  		char* buffer = new char[150];
  		sprintf(buffer, "Number of scales too large for the size of the image. These settings require: width>%d, height >%d and depth >%d", minSize-1, minSize-1, minSize-1);
  		throw WaveletConfigException(buffer);
  	}
}
void checkImageDimensions2D(int width, int height, int numScales) throw (WaveletConfigException)
{
	int minSize = 5+(pow(2, numScales-1)-1)*4;
 	if (width < minSize || height < minSize)
  	{
  		char* buffer = new char[150];
  		sprintf(buffer, "Number of scales too large for the size of the image. These settings require: width>%d, height >%d", minSize-1, minSize-1);
  		throw WaveletConfigException(buffer);
  	}
}

double** b3WaveletCoefficients(double** scaleCoefficients, double* originalImage, int numScales, int numVoxels)
{
	//numScales wavelet images to store, + one image for the low pass residual
	double** waveletCoefficients = new double*[numScales+1];

	//compute wavelet coefficients as the difference between scale coefficients of subsequent scales
	double* iterPrev = originalImage;
	double* iterCurrent =  scaleCoefficients[0];
 	double* currentImgEnd = iterCurrent + numVoxels;
 	
 	int j = 0;
// 	for (int j = 0; j <numScales; j++)
	while (j<numScales)
 	{
 		waveletCoefficients[j] = new double[numVoxels];
		double* iterResult = waveletCoefficients[j];
		while(iterCurrent<currentImgEnd)
 		{
 			(*iterResult) = (*iterPrev)-(*iterCurrent);
 			iterCurrent++;
 			iterPrev++;
 			iterResult++;
 		}
 		j++;
 		double* iterPrev = scaleCoefficients[j-1];
		double* iterCurrent =  scaleCoefficients[j];
 		double* currentImgEnd = iterCurrent + numVoxels;
 	}	
	//residual low pass image is the last wavelet Scale
	waveletCoefficients[numScales] = new double[numVoxels];
	memcpy(waveletCoefficients[numScales], scaleCoefficients[numScales-1], numVoxels*sizeof(double));
	return waveletCoefficients;
}

void b3WaveletCoefficientsInplace(double** coefficients, double* originalImage, double* lowPass, int numScales, int numVoxels)
{
	//residual low pass image is the last wavelet Scale
	memcpy(lowPass, coefficients[numScales-1], numVoxels*sizeof(double));
	
	//then subtract wavelet scales to get wavelet coefficients 
	double* prevImg;
 	double* prevImgEnd;
 	double* iterPrev;
 	double* currentImg;
 	double* currentImgEnd;
 	double* iterCurrent;
 	
 	for (int j = numScales-1; j >0; j--)
 	{
 		currentImg = coefficients[j];
 		currentImgEnd = currentImg+numVoxels;
 		iterCurrent = currentImg;
 		
 		prevImg = coefficients[j-1];
 		prevImgEnd = prevImg+numVoxels;
 		iterPrev = prevImg;
 		
 		while(iterCurrent<currentImgEnd)
 		{
 			(*iterCurrent) = (*iterPrev)-(*iterCurrent);
 			iterCurrent++;
 			iterPrev++;
 		}
 	}
 	//finest scale
 	currentImg = coefficients[0];
 	currentImgEnd = currentImg+numVoxels;
 	iterCurrent = currentImg;
 		
 	prevImg = originalImage;
 	prevImgEnd = prevImg+numVoxels;
 	iterPrev = prevImg;
 	
 	while(iterCurrent<currentImgEnd)
 	{
 		(*iterCurrent) = (*iterPrev)-(*iterCurrent);
 		iterCurrent++;
 		iterPrev++;
 	}
}

void b3WaveletReconstruction(double** inputCoefficients, double* lowPassResidual, double* output, int numScales, int numVoxels)
{
	double** iter = new double*[numScales];
	for (int i =0; i < numScales; i++)
		iter[i] = inputCoefficients[i];
	double* iterLowPass = lowPassResidual;
	double* iterOut = output;
	double* endIter = output+numVoxels;
	double tmp;
	while(iterOut<endIter)
	{
		tmp = (*iterLowPass);
		iterLowPass++;
		for  (int i =0; i < numScales; i++)
		{
			tmp += (*(iter[i]));
			iter[i]++;
		}
		*iterOut = tmp;
		iterOut++;
	}
}


void filterAndSwap2D(double* arrayIn, double* arrayOut, int width, int height, int stepS)
{
	//B3 spline wavelet configuration
	double w2 =  ((double)1)/16;
	double w1 = ((double)1)/4;
	double w0 = ((double)3)/8;
	
	double* w0idx;
 	double* w1idx1;
 	double* w2idx1;
 	double* w1idx2;
 	double* w2idx2;	
 	double* arrayOutiter;
 	
 	int cntX;
 	w0idx = arrayIn;
 	
 	for (int y=0; y<height; y++)
 	{
 		//manage the left border with mirror symmetry
 		arrayOutiter = arrayOut + y;
 		//w0idx = arrayIn + y*width;
 		w1idx1 = w0idx + stepS-1;
 		w2idx1 = w1idx1 + stepS;
 		w1idx2 = w0idx + stepS;
 		w2idx2 = w1idx2 + stepS;
 						
 		cntX = 0;
 		while(cntX < stepS)
 		{
 			*arrayOutiter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);						
 			w1idx1--;
 			w2idx1--;
 			w1idx2++;
 			w2idx2++;
 			w0idx++;
 			arrayOutiter+=height;
 			cntX++;
 		}
 		w1idx1++;
 		while(cntX < 2*stepS)
 		{
 			*arrayOutiter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);							
 			w1idx1++;
 			w2idx1--;
 			w1idx2++;
 			w2idx2++;
 			w0idx++;
 			arrayOutiter+=height;
 			cntX++;
 		}
 		w2idx1++;
 		//filter the center area of the image (no border issue)
 		while(cntX < width - 2*stepS)
 		{	
 			*arrayOutiter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);	
 			w1idx1++;
 			w2idx1++;
 			w1idx2++;
 			w2idx2++;
 			w0idx++;
 			arrayOutiter+=height;
 			cntX++;
 		}
 		w2idx2--;
 		//manage the right border with mirror symmetry
 		while (cntX < width - stepS)
 		{
 			*arrayOutiter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);						
 			w1idx1++;
 			w2idx1++;
 			w1idx2++;
 			w2idx2--;
 			w0idx++;
 			arrayOutiter+=height;
 			cntX++;
 		}
 		w1idx2--;
 		while (cntX < width)
 		{
 			*arrayOutiter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);						
 			w1idx1++;
 			w2idx1++;
 			w1idx2--;
 			w2idx2--;
 			w0idx++;
 			arrayOutiter+=height;
 			cntX++;
 		}
 	}
}

double**  b3WaveletScales2D(double* dataIn, int width, int height, int numScales) throw (WaveletConfigException)
{
	if (numScales < 1)
 	{
			throw WaveletConfigException("Invalid number of wavelet scales. Number of scales should be an integer >=1");
 	}
 	//check that image dimensions complies with the number of chosen scales
 	try{checkImageDimensions2D(width, height, numScales);}
 	catch(WaveletConfigException e)
 	{
 		throw(e);
 	}
		
	int s;
	int stepS;
	int wh = width*height;
	double** resArray = new double*[numScales];
 	double* prevArray = dataIn;
 	double* currentArray = new double [wh];
 
 	for (s = 1; s <= numScales; s++)//for each scale
 	{
 		 stepS = pow(2, s-1);
 		//convolve along the x direction
 		filterAndSwap2D(prevArray, currentArray, width, height, stepS);
 		//swap current and previous array pointers
 		if (s==1)
 		{
 			prevArray = currentArray;
 			currentArray = new double[wh];//re-allocate current array to preserve original data
 		}
 		else
 		{
 			double* tmp = currentArray;
 			currentArray = prevArray;
 			prevArray = tmp;
 		}		
 		//convolve along the y direction
		filterAndSwap2D(prevArray, currentArray, height, width, stepS);//swap size of dimensions
 		//swap current and previous array pointers
 		double* tmp = currentArray;
 		currentArray = prevArray;
 		prevArray = tmp;
 				
 		resArray[s-1] = new double[wh];
 		memcpy(resArray[s-1], prevArray, wh*sizeof(double));
 	}
 	delete[] currentArray;
 	delete[] prevArray;
	return resArray;
}

double**  b3WaveletScales(double* dataIn, int width, int height, int depth, int numScales) throw (WaveletConfigException)
{
	if (numScales < 1)
 	{
			throw WaveletConfigException("Invalid number of wavelet scales. Number of scales should be an integer >=1");
 	}
 	//check that image dimensions complies with the number of chosen scales
 	try{checkImageDimensions(width, height, depth, numScales);}
 	catch(WaveletConfigException e)
 	{
 		throw(e);
 	}
		
	int s;
	int stepS;
	int wh = width*height;
	int whd = width*height*depth;
	
	double** resArray = new double*[numScales];
 	double* prevArray = dataIn;
 	double* currentArray = new double [whd];
 
 	for (s = 1; s <= numScales; s++)//for each scale
 	{
 		 stepS = pow(2, s-1);
 		//convolve along the x direction
 		filterAndSwap(prevArray, currentArray, width, height, depth, stepS);
 		//swap current and previous array pointers
 		if (s==1)
 		{
 			prevArray = currentArray;
 			currentArray = new double[whd];//re-allocate current array to preserve original data
 		}
 		else
 		{
 			double* tmp = currentArray;
 			currentArray = prevArray;
 			prevArray = tmp;
 		}		
 		//convolve along the y direction
		filterAndSwap(prevArray, currentArray, height, depth, width, stepS);//swap size of dimensions
 		//swap current and previous array pointers
 		double* tmp = currentArray;
 		currentArray = prevArray;
 		prevArray = tmp;
 		
 		//convolve along the z direction
		filterAndSwap(prevArray, currentArray, depth, width, height, stepS);//swap size of dimensions
 		//swap current and previous array pointers
 		tmp = currentArray;
 		currentArray = prevArray;
 		prevArray = tmp;
 				
 		resArray[s-1] = new double[whd];
 		memcpy(resArray[s-1], prevArray, whd*sizeof(double));
 	}
 	delete[] currentArray;
 	delete[] prevArray;
	return resArray;
}

void filterAndSwap(double* arrayIn, double* arrayOut, int width, int height, int depth, int stepS)
{
	//B3 spline wavelet configuration
	double w2 =  ((double)1)/16;
	double w1 = ((double)1)/4;
	double w0 = ((double)3)/8;
	
	int wh = width*height;
	int hd = height*depth;
	
	double* w0idx;
 	double* w1idx1;
 	double* w2idx1;
 	double* w1idx2;
 	double* w2idx2;	
 	double* arrayOutIter;
 	
 	int cntX;
 	w0idx = arrayIn;
 	
	for (int z=0; z<depth; z++)
 	{
 		for (int y=0; y<height; y++)
 		{
 			//manage the left border with mirror symmetry
			arrayOutIter = arrayOut + y + z*height;
			//w0idx = arrayIn + (y*width + z*wh);
			w1idx1 = w0idx + stepS-1;
			w2idx1 = w1idx1 + stepS;
			w1idx2 = w0idx + stepS;
			w2idx2 = w1idx2 + stepS;
			
		//	cntX = 0;
			
			double* end0 = w0idx + stepS;
			while (w0idx<end0)
	//		while(cntX < stepS)
			{
				*arrayOutIter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);						
				w1idx1--;
				w2idx1--;
				w1idx2++;
				w2idx2++;
				w0idx++;
				arrayOutIter+=hd;
				//cntX++;
			}
			w1idx1++;
			end0 = w0idx + stepS;
			while (w0idx<end0)
			//while(cntX < 2*stepS)
			{
				*arrayOutIter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);							
				w1idx1++;
				w2idx1--;
				w1idx2++;
				w2idx2++;
				w0idx++;
				arrayOutIter+=hd;
				//cntX++;
			}
			w2idx1++;
			//filter the center area of the image (no border issue)
			end0 = w0idx + width - 4*stepS;
			while (w0idx<end0)
			//while(cntX < width - 2*stepS)
			{	
				*arrayOutIter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);	
				w1idx1++;
				w2idx1++;
				w1idx2++;
				w2idx2++;
				w0idx++;
				arrayOutIter+=hd;
				//cntX++;
			}
			w2idx2--;
			//manage the right border with mirror symmetry
			end0 = w0idx + stepS;
			while (w0idx<end0)
			//while (cntX < width - stepS)
			{
				*arrayOutIter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);						
				w1idx1++;
				w2idx1++;
				w1idx2++;
				w2idx2--;
				w0idx++;
				arrayOutIter+=hd;
				//cntX++;
			}
			w1idx2--;
			end0 = w0idx + stepS;
			while (w0idx<end0)
//			while (cntX < width)
			{
				*arrayOutIter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);						
				w1idx1++;
				w2idx1++;
				w1idx2--;
				w2idx2--;
				w0idx++;
				arrayOutIter+=hd;
				//cntX++;
			}
		}
	}
}
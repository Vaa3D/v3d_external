#include "waveletTransform.h"


double** b3WaveletScalesOptimized(double* dataIn, int width, int height, int depth, int numScales) throw (WaveletConfigException)
{
	if (numScales < 1)
 	{
			throw WaveletConfigException("Invalid number of wavelet scales. Number of scales should be an integer >=1");
 	}
 	//check that image dimensions complies with the number of chosen scales
 	int minSize = 5+(numScales-1)*4;
 	if (width < minSize || height < minSize || depth < minSize)
  	{
  		char* buffer = new char[150];
  		sprintf(buffer, "Number of scales too large for the size of the image. These settings require: width>%d, height >%d and depth >%d", minSize-1, minSize-1, minSize-1);
  		throw WaveletConfigException(buffer);
  	}
 	
 	//TODO: check dimensions vs number of scales
			//B3 spline wavelet configuration
			double w2 =  ((double)1)/16;;
			double w1 = ((double)1)/4;;
			double w0 = ((double)3)/8;;
		
			int s;
			int stepS;
			int wh = width*height;
			int whd = wh*depth;
			double** resArray = new double*[numScales];
 			double* prevArray = dataIn;
 			double* currentArray = new double [whd];
 
 			int cntX, cntY, cntZ;
 			for (s = 1; s <= numScales; s++)//for each scale
 			{
 			 	double* w0idx;
 				double* w1idx1;
 				double* w2idx1;
 				double* w1idx2;
 				double* w2idx2;

 				//convolve along the x direction
 				stepS = pow(2, s-1);
 				
 				double* arrayXiter;			
 				for (int z=0; z<depth; z++)
 				{
 					for (int y=0; y<height; y++)
 					{
 					 	//manage the left border with mirror symmetry
 						arrayXiter = currentArray + (y*width + z*wh);//can be optimized further
 						w0idx = prevArray + (y*width + z*wh);
 						w1idx1 = w0idx + stepS-1;
 						w2idx1 = w1idx1 + stepS;
 						w1idx2 = w0idx + stepS;
 						w2idx2 = w1idx2 + stepS;
 						
 						cntX = 0;
 						while(cntX < stepS)
 						{
 							*arrayXiter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);						
 							w1idx1--;
 							w2idx1--;
 							w1idx2++;
 							w2idx2++;
 							w0idx++;
 							arrayXiter++;
 							cntX++;
 						}
 						w1idx1++;
 						while(cntX < 2*stepS)
 						{
 							*arrayXiter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);							
 							w1idx1++;
 							w2idx1--;
 							w1idx2++;
 							w2idx2++;
 							w0idx++;
 							arrayXiter++;
 							cntX++;
 						}
 						w2idx1++;
 						//filter the center area of the image (no border issue)
 						while(cntX < width - 2*stepS)
 						{	
 							*arrayXiter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);	
 							w1idx1++;
 							w2idx1++;
 							w1idx2++;
 							w2idx2++;
 							w0idx++;
 							arrayXiter++;
 							cntX++;
 						}
 						w2idx2--;
 						//manage the right border with mirror symmetry
 						while (cntX < width - stepS)
 						{
 							*arrayXiter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);						
 							w1idx1++;
 							w2idx1++;
 							w1idx2++;
 							w2idx2--;
 							w0idx++;
 							arrayXiter++;
 							cntX++;
 						}
 						w1idx2--;
 						while (cntX < width)
 						{
 							*arrayXiter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);						
 							w1idx1++;
 							w2idx1++;
 							w1idx2--;
 							w2idx2--;
 							w0idx++;
 							arrayXiter++;
 							cntX++;
 						}
 						//arrayXiter += width;
 						//w0idx += width; 	
 					}
 						//arrayXiter += wh;
 						//w0idx += wh; 	
 				}
 				//Y axis transforms
 				//swap current and previous array;
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
 				int sw = stepS*width;
 				double* arrayYiter;
 				for (int z=0; z<depth; z++)
 				{
 					for (int x=0; x<width; x++)
 					{
 						arrayYiter = currentArray + (x + z * wh); //can be optimized further
 						w0idx = prevArray + (x + z * wh);
 						w1idx1 = w0idx + sw - width;
 						w2idx1 = w1idx1 + sw;
 						w1idx2 = w0idx + sw;
 						w2idx2 = w1idx2 + sw;
 						cntY = 0;
 						while(cntY < stepS)
 						{
 							*arrayYiter = w2* ((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);						
 							w0idx+=width;
 							arrayYiter+=width;
 							w1idx1-=width;
 							w2idx1-=width;
 							w1idx2+=width;
 							w2idx2+=width;
 							cntY++;
 						}
 						w1idx1+=width;
 						while(cntY < 2*stepS)
 						{
 							*arrayYiter = w2* ((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);						
 							w0idx+=width;
 							arrayYiter+=width;
 							w1idx1+=width;
 							w2idx1-=width;
 							w1idx2+=width;
 							w2idx2+=width;
 							cntY++;
 						}
 						w2idx1+=width;
 						while(cntY < height - 2*stepS)
 						{
 							*arrayYiter = w2* ((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);						
 							w0idx+=width;
 							arrayYiter+=width;
 							w1idx1+=width;
 							w2idx1+=width;
 							w1idx2+=width;
 							w2idx2+=width;
 							cntY++;
 						}
 						w2idx2-=width;
 						while (cntY < height - stepS)
 						{
 							*arrayYiter = w2* ((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);						
 							w0idx+=width;
 							arrayYiter+=width;
 							w1idx1+=width;
 							w2idx1+=width;
 							w1idx2+=width;
 							w2idx2-=width;
 							cntY++;
 						}
 						w1idx2-=width;
 						while (cntY < height)
 						{
 							*arrayYiter = w2* ((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);						
 							w0idx+=width;
 							arrayYiter+=width;
 							w1idx1+=width;
 							w2idx1+=width;
 							w1idx2-=width;
 							w2idx2-=width;
 							cntY++;
 						}
 					}
 				}
 				//Z axis transforms
 				//swap current and previous array
 				double* tmp = currentArray;
 				currentArray = prevArray;
 				prevArray = tmp;
 				double* arrayZiter;
 				int swh = sw*height;
 				for (int x=0; x<width; x++)
 				{
 					for (int y=0; y<height; y++)
 					{
 						w0idx = prevArray + (x + y*width);
 						arrayZiter = currentArray + (x + y*width);
 						w1idx1 = w0idx + swh - wh;
 						w2idx1 = w1idx1 + swh;
 						w1idx2 = w0idx + swh;
 						w2idx2 = w1idx2 + swh;
 
 						cntZ = 0;
 						while(cntZ < stepS)
 						{
 							*arrayZiter = w2*((*w2idx1)+(*w2idx2))+ w1*((*w1idx1)+(*w1idx2))+ w0*(*w0idx);						
 							arrayZiter+=wh;
 							w0idx+=wh;
 							w1idx1-=wh;
 							w2idx1-=wh;
 							w1idx2+=wh;
 							w2idx2+=wh;
 							cntZ++;
 						}
 						w1idx1+=wh;
 						while(cntZ < 2*stepS)
 						{
 					    	*arrayZiter = w2*((*w2idx1)+(*w2idx2))+ w1*((*w1idx1)+(*w1idx2))+ w0*(*w0idx);
 							arrayZiter+=wh;
 							w0idx+=wh;
 							w1idx1+=wh;
 							w2idx1-=wh;
 							w1idx2+=wh;
 							w2idx2+=wh;
 							cntZ++;
 						}
 						w2idx1+=wh;
 						while(cntZ < depth - 2*stepS)
 						{	
 							*arrayZiter = w2*((*w2idx1)+(*w2idx2))+ w1*((*w1idx1)+(*w1idx2))+ w0*(*w0idx);
 							arrayZiter+=wh;
 							w0idx+=wh;
 							w1idx1+=wh;
 							w2idx1+=wh;
 							w1idx2+=wh;
 							w2idx2+=wh;
 							cntZ++;
 						}
 						w2idx2-=wh;
 						while (cntZ < depth - stepS)
 						{
 							*arrayZiter = w2*((*w2idx1)+(*w2idx2))+ w1*((*w1idx1)+(*w1idx2))+ w0*(*w0idx);						
							arrayZiter+=wh;
 							w0idx+=wh;
							w1idx1+=wh;
 							w2idx1+=wh;
 							w1idx2+=wh;
							w2idx2-=wh;
 							cntZ++;
 						}
 						w1idx2-=wh;
 						while (cntZ < depth)
 						{
 							*arrayZiter = w2*((*w2idx1)+(*w2idx2))+ w1*((*w1idx1)+(*w1idx2))+ w0*(*w0idx);
 							arrayZiter+=wh;
 							w0idx+=wh;
 							w1idx1+=wh;
 							w2idx1+=wh;
 							w1idx2-=wh;
 							w2idx2-=wh;
 							cntZ++;
 						}
 					}
 				}
 				//swap current and previous array
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

double** b3WaveletScales(double* dataIn, int width, int height, int depth, int numScales) throw (WaveletConfigException)
{
	if (numScales < 1)
 	{
			throw WaveletConfigException("Invalid number of wavelet scales. Number of scales should be an integer >=1");
 	}
 	//check that image dimensions complies with the number of chosen scales
 	int minSize = 5+(numScales-1)*4;
 	if (width < minSize || height < minSize || depth < minSize)
  	{
  		char* buffer = new char[150];
  		sprintf(buffer, "Number of scales too large for the size of the image. These settings require: width>%d, height >%d and depth >%d", minSize-1, minSize-1, minSize-1);
  		throw WaveletConfigException(buffer);
  	}
		
			//B3 spline wavelet configuration
			double w2 =  ((double)1)/16;;
			double w1 = ((double)1)/4;;
			double w0 = ((double)3)/8;;
		
			int s;
			int stepS;
			int wh = width*height;
			int whd = wh*depth;
			double** resArray = new double*[numScales];
			double* array1 = dataIn;
			
			double* arrayX = new double[whd];
 			double* arrayY = new double[whd];
 			double* arrayZ = new double[whd];
 
 			int cntX, cntY, cntZ;
 			for (s = 1; s <= numScales; s++)//for each scale
 			{
	 			stepS = pow(2, s-1);
 				//convolve along the x direction
 				int idx0 = 0;
 				int w2idx1;
 				int w1idx1;
 				int w1idx2;
 				int w2idx2;
 				for (int z=0; z<depth; z++)
 				{
 					for (int y=0; y<height; y++)
 					{
 						w1idx1 = idx0 + stepS-1;
 						w2idx1 = w1idx1+stepS;
 						w1idx2 = idx0+stepS;
 						w2idx2 = w1idx2+stepS;
 
 						cntX = 0;
 						while(cntX < stepS)
 						{
 							arrayX[idx0] = w2*(array1[w2idx1]+ array1[w2idx2])+ w1*(array1[w1idx1]+array1[w1idx2]) + w0*array1[idx0];						
 							w1idx1--;
 							w2idx1--;
 							w1idx2++;
 							w2idx2++;
 							idx0++;
 							cntX++;
 						}
 						w1idx1++;
 						while(cntX < 2*stepS)
 						{
 							arrayX[idx0] =  w2* (array1[w2idx1] + array1[w2idx2])+ w1*(array1[w1idx1] +array1[w1idx2])+ w0*array1[idx0];						
 							w1idx1++;
 							w2idx1--;
 							w1idx2++;
 							w2idx2++;
 							idx0++;
 							cntX++;
 						}
 						w2idx1++;
 						while(cntX < width - 2*stepS)
 						{	
 							arrayX[idx0] = w2*(array1[w2idx1] + array1[w2idx2])+ w1*(array1[w1idx1]+ array1[w1idx2])+ w0*array1[idx0];						
 							w1idx1++;
 							w2idx1++;
 							w1idx2++;
 							w2idx2++;
 							idx0++;
 							cntX++;
 						}
 						w2idx2--;
 						while (cntX < width - stepS)
 						{
 							arrayX[idx0] = w2* (array1[w2idx1]+array1[w2idx2])+ w1*(array1[w1idx1]+array1[w1idx2]) + w0*array1[idx0];						
 							w1idx1++;
 							w2idx1++;
 							w1idx2++;
 							w2idx2--;
 							idx0++;
 							cntX++;
 						}
 						w1idx2--;
 						while (cntX < width)
 						{
 							arrayX[idx0] = w2*( array1[w2idx1]+array1[w2idx2]) + w1*(array1[w1idx1]+array1[w1idx2]) + w0*array1[idx0];						
 							w1idx1++;
 							w2idx1++;
 							w1idx2--;
 							w2idx2--;
 							idx0++;
 							cntX++;
 						}
 					}
 				}
 				//Y axis transforms
 				idx0 = 0;
 				int sw = stepS*width;
 				for (int z=0; z<depth; z++)
 				{
 					for (int x=0; x<width; x++)
 					{
 						idx0 = x + z * wh;
 						w1idx1 = idx0 + sw-width;
 						w2idx1 = w1idx1 + sw;
 						w1idx2 = idx0+sw;
 						w2idx2 = w1idx2+sw;
 						cntY = 0;
 						while(cntY < stepS)
 						{
 							arrayY[idx0] = w2* (arrayX[w2idx1]+arrayX[w2idx2])+ w1*(arrayX[w1idx1]+ arrayX[w1idx2])+ w0*arrayX[idx0];						
 							w1idx1-=width;
 							w2idx1-=width;
 							w1idx2+=width;
 							w2idx2+=width;
 							idx0+=width;
 							cntY++;
 						}
 						w1idx1+=width;
 						while(cntY < 2*stepS)
 						{
 							arrayY[idx0] = w2* (arrayX[w2idx1]+ arrayX[w2idx2])+ w1*(arrayX[w1idx1]+arrayX[w1idx2])+ w0*arrayX[idx0];						
 							w1idx1+=width;
 							w2idx1-=width;
 							w1idx2+=width;
 							w2idx2+=width;
 							idx0+=width;
 							cntY++;
 						}
 						w2idx1+=width;
 						while(cntY < height - 2*stepS)
 						{	
 							arrayY[idx0] = w2* (arrayX[w2idx1]+arrayX[w2idx2] )+ w1*(arrayX[w1idx1]+arrayX[w1idx2])+ w0*arrayX[idx0];						
 							w1idx1+=width;
 							w2idx1+=width;
 							w1idx2+=width;
 							w2idx2+=width;
 							idx0+=width;
 							cntY++;
 						}
 						w2idx2-=width;
 						while (cntY < height - stepS)
 						{
 							arrayY[idx0] = w2*(arrayX[w2idx1]+arrayX[w2idx2])+ w1*(arrayX[w1idx1]+arrayX[w1idx2])+ w0*arrayX[idx0];						
 							w1idx1+=width;
 							w2idx1+=width;
 							w1idx2+=width;
 							w2idx2-=width;
 							idx0+=width;
 							cntY++;
 						}
 						w1idx2-=width;
 						while (cntY < height)
 						{
 							arrayY[idx0] = w2*( arrayX[w2idx1]+arrayX[w2idx2])+ w1*(arrayX[w1idx1]+ arrayX[w1idx2])+ w0*arrayX[idx0];						
 							w1idx1+=width;
 							w2idx1+=width;
 							w1idx2-=width;
 							w2idx2-=width;
 							idx0+=width;
 							cntY++;
 						}
 					}
 				}
 					int swh = sw*height;
 					for (int x=0; x<width; x++)
 					{
 						for (int y=0; y<height; y++)
 						{
 							idx0 = x + y*width;
 							w1idx1 = idx0 + swh-wh;
 							w2idx1 = w1idx1 + swh;
 							w1idx2 = idx0+swh;
 							w2idx2 = w1idx2+swh;
 
 							cntZ = 0;
 							while(cntZ < stepS)
 							{
 								arrayZ[idx0] = w2*(arrayY[w2idx1]+arrayY[w2idx2])+ w1*(arrayY[w1idx1]+arrayY[w1idx2])+ w0*arrayY[idx0];						
 								w1idx1-=wh;
 								w2idx1-=wh;
 								w1idx2+=wh;
 								w2idx2+=wh;
 								idx0+=wh;
 								cntZ++;
 							}
 							w1idx1+=wh;
 							while(cntZ < 2*stepS)
 							{
 								arrayZ[idx0] =  w2*(arrayY[w2idx1]+arrayY[w2idx2])+ w1*(arrayY[w1idx1]+arrayY[w1idx2])+ w0*arrayY[idx0];						
 								w1idx1+=wh;
 								w2idx1-=wh;
 								w1idx2+=wh;
 								w2idx2+=wh;
 								idx0+=wh;
 								cntZ++;
 							}
 							w2idx1+=wh;
 							while(cntZ < depth - 2*stepS)
 							{	
 								arrayZ[idx0] = w2*(arrayY[w2idx1]+arrayY[w2idx2])+ w1*(arrayY[w1idx1] + arrayY[w1idx2])+ w0*arrayY[idx0];						
 								w1idx1+=wh;
 								w2idx1+=wh;
 								w1idx2+=wh;
 								w2idx2+=wh;
 								idx0+=wh;
 								cntZ++;
 							}
 							w2idx2-=wh;
 							while (cntZ < depth - stepS)
 							{
 								arrayZ[idx0] = w2*(arrayY[w2idx1]+arrayY[w2idx2])+ w1*(arrayY[w1idx1]+arrayY[w1idx2])+ w0*arrayY[idx0];						
 								w1idx1+=wh;
 								w2idx1+=wh;
 								w1idx2+=wh;
 								w2idx2-=wh;
 								idx0+=wh;
 								cntZ++;
 							}
 							w1idx2-=wh;
 							while (cntZ < depth)
 							{
 								arrayZ[idx0] = w2*(arrayY[w2idx1]+ arrayY[w2idx2] )+ w1*(arrayY[w1idx1]+arrayY[w1idx2])+ w0*arrayY[idx0];						
 								w1idx1+=wh;
 								w2idx1+=wh;
 								w1idx2-=wh;
 								w2idx2-=wh;
 								idx0+=wh;
 								cntZ++;
 							}
 						}
 					}
 					resArray[s-1] = new double[whd];
 					memcpy(resArray[s-1], arrayZ, whd*sizeof(double));
 					array1 = arrayZ;
 			}
 			delete(arrayX);
 			delete(arrayY);
 			delete(arrayZ);
			return resArray;
}

double**  b3WaveletScales2DOptimized(double* dataIn, int width, int height, int numScales) throw (WaveletConfigException)
{
	if (numScales < 1)
 	{
			throw WaveletConfigException("Invalid number of wavelet scales. Number of scales should be an integer >=1");
 	}
 	//check that image dimensions complies with the number of chosen scales
 	int minSize = 5+(numScales-1)*4;
 	if (width < minSize || height < minSize)
  	{
  		char* buffer = new char[150];
  		sprintf(buffer, "Number of scales too large for the size of the image. These settings require: width>%d, height >%d", minSize-1, minSize-1);
  		throw WaveletConfigException(buffer);
  	}
 	
 	//TODO: check dimensions vs number of scales
	//B3 spline wavelet configuration
	double w2 =  ((double)1)/16;;
	double w1 = ((double)1)/4;;
	double w0 = ((double)3)/8;;
		
	int s;
	int stepS;
	int wh = width*height;
	double** resArray = new double*[numScales];
 	double* prevArray = dataIn;
 	double* currentArray = new double [wh];
 
 	int cntX, cntY, cntZ;
 	for (s = 1; s <= numScales; s++)//for each scale
 	{
 		double* w0idx;
 		double* w1idx1;
 		double* w2idx1;
 		double* w1idx2;
 		double* w2idx2;

 		//convolve along the x direction
 		stepS = pow(2, s-1);
 				
 		double* arrayXiter;
 		for (int y=0; y<height; y++)
 		{
 			//manage the left border with mirror symmetry
 			arrayXiter = currentArray + y*width;//can be optimized further
 			w0idx = prevArray + y*width;
 			w1idx1 = w0idx + stepS-1;
 			w2idx1 = w1idx1 + stepS;
 			w1idx2 = w0idx + stepS;
 			w2idx2 = w1idx2 + stepS;
 						
 			cntX = 0;
 			while(cntX < stepS)
 			{
 				*arrayXiter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);						
 				w1idx1--;
 				w2idx1--;
 				w1idx2++;
 				w2idx2++;
 				w0idx++;
 				arrayXiter++;
 				cntX++;
 			}
 			w1idx1++;
 			while(cntX < 2*stepS)
 			{
 							*arrayXiter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);							
 							w1idx1++;
 							w2idx1--;
 							w1idx2++;
 							w2idx2++;
 							w0idx++;
 							arrayXiter++;
 							cntX++;
 						}
 						w2idx1++;
 						//filter the center area of the image (no border issue)
 						while(cntX < width - 2*stepS)
 						{	
 							*arrayXiter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);	
 							w1idx1++;
 							w2idx1++;
 							w1idx2++;
 							w2idx2++;
 							w0idx++;
 							arrayXiter++;
 							cntX++;
 						}
 						w2idx2--;
 						//manage the right border with mirror symmetry
 						while (cntX < width - stepS)
 						{
 							*arrayXiter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);						
 							w1idx1++;
 							w2idx1++;
 							w1idx2++;
 							w2idx2--;
 							w0idx++;
 							arrayXiter++;
 							cntX++;
 						}
 						w1idx2--;
 						while (cntX < width)
 						{
 							*arrayXiter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);						
 							w1idx1++;
 							w2idx1++;
 							w1idx2--;
 							w2idx2--;
 							w0idx++;
 							arrayXiter++;
 							cntX++;
 						}
 						//arrayXiter += width;
 						//w0idx += width; 	
 					}
 				//Y axis transforms
 				//swap current and previous array;
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
 				int sw = stepS*width;
 				double* arrayYiter;
 				for (int x=0; x<width; x++)
 					{
 						arrayYiter = currentArray + x; //can be optimized further
 						w0idx = prevArray + x;
 						w1idx1 = w0idx + sw - width;
 						w2idx1 = w1idx1 + sw;
 						w1idx2 = w0idx + sw;
 						w2idx2 = w1idx2 + sw;
 						cntY = 0;
 						while(cntY < stepS)
 						{
 							*arrayYiter = w2* ((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);						
 							w0idx+=width;
 							arrayYiter+=width;
 							w1idx1-=width;
 							w2idx1-=width;
 							w1idx2+=width;
 							w2idx2+=width;
 							cntY++;
 						}
 						w1idx1+=width;
 						while(cntY < 2*stepS)
 						{
 							*arrayYiter = w2* ((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);						
 							w0idx+=width;
 							arrayYiter+=width;
 							w1idx1+=width;
 							w2idx1-=width;
 							w1idx2+=width;
 							w2idx2+=width;
 							cntY++;
 						}
 						w2idx1+=width;
 						while(cntY < height - 2*stepS)
 						{
 							*arrayYiter = w2* ((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);						
 							w0idx+=width;
 							arrayYiter+=width;
 							w1idx1+=width;
 							w2idx1+=width;
 							w1idx2+=width;
 							w2idx2+=width;
 							cntY++;
 						}
 					w2idx2-=width;
 					while (cntY < height - stepS)
 					{
 						*arrayYiter = w2* ((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);						
 						w0idx+=width;
 						arrayYiter+=width;
 						w1idx1+=width;
 						w2idx1+=width;
 						w1idx2+=width;
 						w2idx2-=width;
 						cntY++;
 					}
 					w1idx2-=width;
 					while (cntY < height)
 					{
 						*arrayYiter = w2* ((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);						
 						w0idx+=width;
 						arrayYiter+=width;
 						w1idx1+=width;
 						w2idx1+=width;
 						w1idx2-=width;
 						w2idx2-=width;
 						cntY++;
 					}
 				}
 				//swap current and previous array
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


double** b3WaveletScales2D(double* dataIn, int width, int height, int numScales) throw (WaveletConfigException)
{
	if (numScales < 1)
 	{
			throw WaveletConfigException("Invalid number of wavelet scales. Number of scales should be an integer >=1");
 	}
 	//check that image dimensions complies with the number of chosen scales
 	int minSize = 5+(numScales-1)*4;
 	if (width < minSize || height < minSize)
  	{
  		char* buffer = new char[150];
  		sprintf(buffer, "Number of scales too large for the size of the image. These settings require: width>%d, height >%d", minSize-1, minSize-1);
  		throw WaveletConfigException(buffer);
  	}
		
			//B3 spline wavelet configuration
			double w2 =  ((double)1)/16;;
			double w1 = ((double)1)/4;;
			double w0 = ((double)3)/8;;
		
			int s;
			int stepS;
			int wh = width*height;
			double** resArray = new double*[numScales];
			double* array1 = dataIn;
			
			double* arrayX = new double[wh];
 			double* arrayY = new double[wh];
 			
 			 int cntX, cntY;
 			for (s = 1; s <= numScales; s++)//for each scale
 			{
	 			stepS = pow(2, s-1);
 				//convolve along the x direction			
 				int idx0 = 0;
 				int w2idx1;
 				int w1idx1;
 				int w1idx2;
 				int w2idx2;
 					for (int y=0; y<height; y++)
 					{
 						w1idx1 = idx0 + stepS-1;
 						w2idx1 = w1idx1+stepS;
 						w1idx2 = idx0+stepS;
 						w2idx2 = w1idx2+stepS;
 
 						cntX = 0;
 						while(cntX < stepS)
 						{
 							arrayX[idx0] = w2*(array1[w2idx1]+ array1[w2idx2])+ w1*(array1[w1idx1]+array1[w1idx2]) + w0*array1[idx0];						
 							w1idx1--;
 							w2idx1--;
 							w1idx2++;
 							w2idx2++;
 							idx0++;
 							cntX++;
 						}
 						w1idx1++;
 						while(cntX < 2*stepS)
 						{
 							arrayX[idx0] =  w2* (array1[w2idx1] + array1[w2idx2])+ w1*(array1[w1idx1] +array1[w1idx2])+ w0*array1[idx0];						
 							w1idx1++;
 							w2idx1--;
 							w1idx2++;
 							w2idx2++;
 							idx0++;
 							cntX++;
 						}
 						w2idx1++;
 						while(cntX < width - 2*stepS)
 						{	
 							arrayX[idx0] = w2*(array1[w2idx1] + array1[w2idx2])+ w1*(array1[w1idx1]+ array1[w1idx2])+ w0*array1[idx0];						
 							w1idx1++;
 							w2idx1++;
 							w1idx2++;
 							w2idx2++;
 							idx0++;
 							cntX++;
 						}
 						w2idx2--;
 						while (cntX < width - stepS)
 						{
 							arrayX[idx0] = w2* (array1[w2idx1]+array1[w2idx2])+ w1*(array1[w1idx1]+array1[w1idx2]) + w0*array1[idx0];						
 							w1idx1++;
 							w2idx1++;
 							w1idx2++;
 							w2idx2--;
 							idx0++;
 							cntX++;
 						}
 						w1idx2--;
 						while (cntX < width)
 						{
 							arrayX[idx0] = w2*( array1[w2idx1]+array1[w2idx2]) + w1*(array1[w1idx1]+array1[w1idx2]) + w0*array1[idx0];						
 							w1idx1++;
 							w2idx1++;
 							w1idx2--;
 							w2idx2--;
 							idx0++;
 							cntX++;
 						}
 					}
 				//Y axis transforms
 				idx0 = 0;
 				int sw = stepS*width;
 					for (int x=0; x<width; x++)
 					{
 						idx0 = x;
 						w1idx1 = idx0 + sw-width;
 						w2idx1 = w1idx1 + sw;
 						w1idx2 = idx0+sw;
 						w2idx2 = w1idx2+sw;
 						cntY = 0;
 						while(cntY < stepS)
 						{
 							arrayY[idx0] = w2* (arrayX[w2idx1]+arrayX[w2idx2])+ w1*(arrayX[w1idx1]+ arrayX[w1idx2])+ w0*arrayX[idx0];						
 							w1idx1-=width;
 							w2idx1-=width;
 							w1idx2+=width;
 							w2idx2+=width;
 							idx0+=width;
 							cntY++;
 						}
 						w1idx1+=width;
 						while(cntY < 2*stepS)
 						{
 							arrayY[idx0] = w2* (arrayX[w2idx1]+ arrayX[w2idx2])+ w1*(arrayX[w1idx1]+arrayX[w1idx2])+ w0*arrayX[idx0];						
 							w1idx1+=width;
 							w2idx1-=width;
 							w1idx2+=width;
 							w2idx2+=width;
 							idx0+=width;
 							cntY++;
 						}
 						w2idx1+=width;
 						while(cntY < height - 2*stepS)
 						{	
 							arrayY[idx0] = w2* (arrayX[w2idx1]+arrayX[w2idx2] )+ w1*(arrayX[w1idx1]+arrayX[w1idx2])+ w0*arrayX[idx0];						
 							w1idx1+=width;
 							w2idx1+=width;
 							w1idx2+=width;
 							w2idx2+=width;
 							idx0+=width;
 							cntY++;
 						}
 						w2idx2-=width;
 						while (cntY < height - stepS)
 						{
 							arrayY[idx0] = w2*(arrayX[w2idx1]+arrayX[w2idx2])+ w1*(arrayX[w1idx1]+arrayX[w1idx2])+ w0*arrayX[idx0];						
 							w1idx1+=width;
 							w2idx1+=width;
 							w1idx2+=width;
 							w2idx2-=width;
 							idx0+=width;
 							cntY++;
 						}
 						w1idx2-=width;
 						while (cntY < height)
 						{
 							arrayY[idx0] = w2*( arrayX[w2idx1]+arrayX[w2idx2])+ w1*(arrayX[w1idx1]+ arrayX[w1idx2])+ w0*arrayX[idx0];						
 							w1idx1+=width;
 							w2idx1+=width;
 							w1idx2-=width;
 							w2idx2-=width;
 							idx0+=width;
 							cntY++;
 						}
 					}
 				resArray[s-1] = new double[wh];
 				memcpy(resArray[s-1], arrayY, wh*sizeof(double));
 				array1 = arrayY;
 			}
 			delete(arrayX);
 			delete(arrayY);
 			return resArray;
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




//////////////////////////// NEW OPTIMIZED IMPLEMENTATION WITH DIMENSIONS SWAPPING
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

double**  b3WaveletScales2DWithDimensionSwapping(double* dataIn, int width, int height, int numScales) throw (WaveletConfigException)
{
	if (numScales < 1)
 	{
			throw WaveletConfigException("Invalid number of wavelet scales. Number of scales should be an integer >=1");
 	}
 	//check that image dimensions complies with the number of chosen scales
 	int minSize = 5+(numScales-1)*4;
 	if (width < minSize || height < minSize)
  	{
  		char* buffer = new char[150];
  		sprintf(buffer, "Number of scales too large for the size of the image. These settings require: width>%d, height >%d", minSize-1, minSize-1);
  		throw WaveletConfigException(buffer);
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

double**  b3WaveletScalesWithDimensionSwapping(double* dataIn, int width, int height, int depth, int numScales) throw (WaveletConfigException)
{
	if (numScales < 1)
 	{
			throw WaveletConfigException("Invalid number of wavelet scales. Number of scales should be an integer >=1");
 	}
 	//check that image dimensions complies with the number of chosen scales
 	int minSize = 5+(numScales-1)*4;
 	if (width < minSize || height < minSize || depth < minSize)
  	{
  		char* buffer = new char[150];
  		sprintf(buffer, "Number of scales too large for the size of the image. These settings require: width>%d, height >%d and depth >%d", minSize-1, minSize-1, minSize-1);
  		throw WaveletConfigException(buffer);
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
			
			cntX = 0;
			while(cntX < stepS)
			{
				*arrayOutIter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);						
				w1idx1--;
				w2idx1--;
				w1idx2++;
				w2idx2++;
				w0idx++;
				arrayOutIter+=hd;
				cntX++;
			}
			w1idx1++;
			while(cntX < 2*stepS)
			{
				*arrayOutIter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);							
				w1idx1++;
				w2idx1--;
				w1idx2++;
				w2idx2++;
				w0idx++;
				arrayOutIter+=hd;
				cntX++;
			}
			w2idx1++;
			//filter the center area of the image (no border issue)
			while(cntX < width - 2*stepS)
			{	
				*arrayOutIter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);	
				w1idx1++;
				w2idx1++;
				w1idx2++;
				w2idx2++;
				w0idx++;
				arrayOutIter+=hd;
				cntX++;
			}
			w2idx2--;
			//manage the right border with mirror symmetry
			while (cntX < width - stepS)
			{
				*arrayOutIter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);						
				w1idx1++;
				w2idx1++;
				w1idx2++;
				w2idx2--;
				w0idx++;
				arrayOutIter+=hd;
				cntX++;
			}
			w1idx2--;
			while (cntX < width)
			{
				*arrayOutIter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);						
				w1idx1++;
				w2idx1++;
				w1idx2--;
				w2idx2--;
				w0idx++;
				arrayOutIter+=hd;
				cntX++;
			}
		}
	}
}
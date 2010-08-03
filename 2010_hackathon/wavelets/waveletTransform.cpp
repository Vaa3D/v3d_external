#include "waveletTransform.h"



double** b3WaveletScalesOptimized(double* dataIn, int width, int height, int depth, int numScales) throw (WaveletConfigException)
{

	// if (numScales < 1)
//  	{
// 			throw WaveletConfigException("Invalid number of wavelet scales. Number of scales should be an integer >=1");
//  	}
//  	//TODO: check dimensions vs number of scales
// 		
// 			//B3 spline wavelet configuration
// 			int pad[] = {2, 4, 8, 16, 32, 64, 128, 256, 512};
// 			int step[] = {1, 2, 4, 8, 16, 32, 64, 128, 256};
// 			double w2 =  ((double)1)/16;;
// 			double w1 = ((double)1)/4;;
// 			double w0 = ((double)3)/8;;
// 		
// 			int s;
// 			int padMax = pad[numScales-1];
// 			int stepS;
// 			int wh = width*height;
// 			int whd = wh*depth;
// 			double** resArray = new double*[numScales];
// 			double* array1 = dataIn;
// 			
// 			double* arrayX = new double[whd];
//  			double* arrayY = new double[whd];
//  			double* arrayZ = new double[whd];
//  			
//  			int maxSz = 0;
//  			for (s = 1; s <= numScales; s++)
//  			{
//  				if(depth >= 2*pad[s-1]) maxSz = s;
//  			}
//  
//  			int cntX, cntY, cntZ;
//  			for (s = 1; s <= numScales; s++)//for each scale
//  			{
//  				//convolve along the x direction
//  				stepS = step[s-1];
//  				
//  				//manage left border with mirror symmetry
//  // 				int idx0 = 0;
// //  				double* w2idx1;
// //  				double* w1idx1;
// //  				double* w1idx2;
// //  				double* w2idx2;
// //				int idx0 = 0;
//  				double* w1idx1 = array1 + stepS-1;
//  				double* w2idx1 = w1idx1 + stepS;
//  				double* w1idx2 = array1 + stepS;
//  				double* w2idx2 = w1idx2 + stepS;
//  				double* arrayXiter = arrayX;
//  				for (int z=0; z<depth; z++)
//  				{
//  					for (int y=0; y<height; y++)
//  					{
//  						//w1idx1 = idx0 + stepS-1;
//  						//w2idx1 = w1idx1+stepS;
//  						//w1idx2 = idx0+stepS;
//  						//w2idx2 = w1idx2+stepS;
//  						cntX = 0;
//  						while(cntX < stepS)
//  						{
//  							*arrayXiter = w2*(array1[w2idx1]+ array1[w2idx2])+ w1*(array1[w1idx1]+array1[w1idx2]) + w0*array1[idx0];						
//  							w1idx1--;
//  							w2idx1--;
//  							w1idx2++;
//  							w2idx2++;
//  							idx0++;
//  							cntX++;
//  							// arrayX[idx0] = w2*(array1[w2idx1]+ array1[w2idx2])+ w1*(array1[w1idx1]+array1[w1idx2]) + w0*array1[idx0];						
// //  							w1idx1--;
// //  							w2idx1--;
// //  							w1idx2++;
// //  							w2idx2++;
// //  							idx0++;
// //  							cntX++;
//  						}
//  						w1idx1++;
//  						while(cntX < 2*stepS)
//  						{
//  							arrayX[idx0] =  w2* (array1[w2idx1] + array1[w2idx2])+ w1*(array1[w1idx1] +array1[w1idx2])+ w0*array1[idx0];						
//  							w1idx1++;
//  							w2idx1--;
//  							w1idx2++;
//  							w2idx2++;
//  							idx0++;
//  							cntX++;
//  						}
//  						w2idx1++;
//  						while(cntX < width - 2*stepS)
//  						{	
//  							arrayX[idx0] = w2*(array1[w2idx1] + array1[w2idx2])+ w1*(array1[w1idx1]+ array1[w1idx2])+ w0*array1[idx0];						
//  							w1idx1++;
//  							w2idx1++;
//  							w1idx2++;
//  							w2idx2++;
//  							idx0++;
//  							cntX++;
//  						}
//  						w2idx2--;
//  						while (cntX < width - stepS)
//  						{
//  							arrayX[idx0] = w2* (array1[w2idx1]+array1[w2idx2])+ w1*(array1[w1idx1]+array1[w1idx2]) + w0*array1[idx0];						
//  							w1idx1++;
//  							w2idx1++;
//  							w1idx2++;
//  							w2idx2--;
//  							idx0++;
//  							cntX++;
//  						}
//  						w1idx2--;
//  						while (cntX < width)
//  						{
//  							arrayX[idx0] = w2*( array1[w2idx1]+array1[w2idx2]) + w1*(array1[w1idx1]+array1[w1idx2]) + w0*array1[idx0];						
//  							w1idx1++;
//  							w2idx1++;
//  							w1idx2--;
//  							w2idx2--;
//  							idx0++;
//  							cntX++;
//  						}
//  					}
//  				}
//  				//Y axis transforms
//  				idx0 = 0;
//  				int sw = stepS*width;
//  				for (int z=0; z<depth; z++)
//  				{
//  					for (int x=0; x<width; x++)
//  					{
//  						idx0 = x + z * wh;
//  						w1idx1 = idx0 + sw-width;
//  						w2idx1 = w1idx1 + sw;
//  						w1idx2 = idx0+sw;
//  						w2idx2 = w1idx2+sw;
//  						cntY = 0;
//  						while(cntY < stepS)
//  						{
//  							arrayY[idx0] = w2* (arrayX[w2idx1]+arrayX[w2idx2])+ w1*(arrayX[w1idx1]+ arrayX[w1idx2])+ w0*arrayX[idx0];						
//  							w1idx1-=width;
//  							w2idx1-=width;
//  							w1idx2+=width;
//  							w2idx2+=width;
//  							idx0+=width;
//  							cntY++;
//  						}
//  						w1idx1+=width;
//  						while(cntY < 2*stepS)
//  						{
//  							arrayY[idx0] = w2* (arrayX[w2idx1]+ arrayX[w2idx2])+ w1*(arrayX[w1idx1]+arrayX[w1idx2])+ w0*arrayX[idx0];						
//  							w1idx1+=width;
//  							w2idx1-=width;
//  							w1idx2+=width;
//  							w2idx2+=width;
//  							idx0+=width;
//  							cntY++;
//  						}
//  						w2idx1+=width;
//  						while(cntY < height - 2*stepS)
//  						{	
//  							arrayY[idx0] = w2* (arrayX[w2idx1]+arrayX[w2idx2] )+ w1*(arrayX[w1idx1]+arrayX[w1idx2])+ w0*arrayX[idx0];						
//  							w1idx1+=width;
//  							w2idx1+=width;
//  							w1idx2+=width;
//  							w2idx2+=width;
//  							idx0+=width;
//  							cntY++;
//  						}
//  						w2idx2-=width;
//  						while (cntY < height - stepS)
//  						{
//  							arrayY[idx0] = w2*(arrayX[w2idx1]+arrayX[w2idx2])+ w1*(arrayX[w1idx1]+arrayX[w1idx2])+ w0*arrayX[idx0];						
//  							w1idx1+=width;
//  							w2idx1+=width;
//  							w1idx2+=width;
//  							w2idx2-=width;
//  							idx0+=width;
//  							cntY++;
//  						}
//  						w1idx2-=width;
//  						while (cntY < height)
//  						{
//  							arrayY[idx0] = w2*( arrayX[w2idx1]+arrayX[w2idx2])+ w1*(arrayX[w1idx1]+ arrayX[w1idx2])+ w0*arrayX[idx0];						
//  							w1idx1+=width;
//  							w2idx1+=width;
//  							w1idx2-=width;
//  							w2idx2-=width;
//  							idx0+=width;
//  							cntY++;
//  						}
//  					}
//  				}
//  					if(depth <= 2*pad[s-1])
//  					{
//  						stepS = step[maxSz-1];
//  						sw = stepS*width;
//  					}
//  					int swh = sw*height;
//  					for (int x=0; x<width; x++)
//  					{
//  						for (int y=0; y<height; y++)
//  						{
//  							idx0 = x + y*width;
//  							w1idx1 = idx0 + swh-wh;
//  							w2idx1 = w1idx1 + swh;
//  							w1idx2 = idx0+swh;
//  							w2idx2 = w1idx2+swh;
//  
//  							cntZ = 0;
//  							while(cntZ < stepS)
//  							{
//  								arrayZ[idx0] = w2*(arrayY[w2idx1]+arrayY[w2idx2])+ w1*(arrayY[w1idx1]+arrayY[w1idx2])+ w0*arrayY[idx0];						
//  								w1idx1-=wh;
//  								w2idx1-=wh;
//  								w1idx2+=wh;
//  								w2idx2+=wh;
//  								idx0+=wh;
//  								cntZ++;
//  							}
//  							w1idx1+=wh;
//  							while(cntZ < 2*stepS)
//  							{
//  								arrayZ[idx0] =  w2*(arrayY[w2idx1]+arrayY[w2idx2])+ w1*(arrayY[w1idx1]+arrayY[w1idx2])+ w0*arrayY[idx0];						
//  								w1idx1+=wh;
//  								w2idx1-=wh;
//  								w1idx2+=wh;
//  								w2idx2+=wh;
//  								idx0+=wh;
//  								cntZ++;
//  							}
//  							w2idx1+=wh;
//  							while(cntZ < depth - 2*stepS)
//  							{	
//  								arrayZ[idx0] = w2*(arrayY[w2idx1]+arrayY[w2idx2])+ w1*(arrayY[w1idx1] + arrayY[w1idx2])+ w0*arrayY[idx0];						
//  								w1idx1+=wh;
//  								w2idx1+=wh;
//  								w1idx2+=wh;
//  								w2idx2+=wh;
//  								idx0+=wh;
//  								cntZ++;
//  							}
//  							w2idx2-=wh;
//  							while (cntZ < depth - stepS)
//  							{
//  								arrayZ[idx0] = w2*(arrayY[w2idx1]+arrayY[w2idx2])+ w1*(arrayY[w1idx1]+arrayY[w1idx2])+ w0*arrayY[idx0];						
//  								w1idx1+=wh;
//  								w2idx1+=wh;
//  								w1idx2+=wh;
//  								w2idx2-=wh;
//  								idx0+=wh;
//  								cntZ++;
//  							}
//  							w1idx2-=wh;
//  							while (cntZ < depth)
//  							{
//  								arrayZ[idx0] = w2*(arrayY[w2idx1]+ arrayY[w2idx2] )+ w1*(arrayY[w1idx1]+arrayY[w1idx2])+ w0*arrayY[idx0];						
//  								w1idx1+=wh;
//  								w2idx1+=wh;
//  								w1idx2-=wh;
//  								w2idx2-=wh;
//  								idx0+=wh;
//  								cntZ++;
//  							}
//  						}
//  					}
//  					resArray[s-1] = new double[whd];
//  					memcpy(resArray[s-1], arrayZ, whd*sizeof(double));
//  					array1 = arrayZ;
//  			}
//  			delete(arrayX);
//  			delete(arrayY);
//  			delete(arrayZ);
// 			return resArray;
return NULL;
}

double** b3WaveletScales(double* dataIn, int width, int height, int depth, int numScales) throw (WaveletConfigException)
{
	if (numScales < 1)
 	{
			throw WaveletConfigException("Invalid number of wavelet scales. Number of scales should be an integer >=1");
 	}
 	//TODO: check dimensions vs number of scales
		
			//B3 spline wavelet configuration
			int pad[] = {2, 4, 8, 16, 32, 64, 128, 256, 512};
			int step[] = {1, 2, 4, 8, 16, 32, 64, 128, 256};
			double w2 =  ((double)1)/16;;
			double w1 = ((double)1)/4;;
			double w0 = ((double)3)/8;;
		
			int s;
			int padMax = pad[numScales-1];
			int stepS;
			int wh = width*height;
			int whd = wh*depth;
			double** resArray = new double*[numScales];
			double* array1 = dataIn;
			
			double* arrayX = new double[whd];
 			double* arrayY = new double[whd];
 			double* arrayZ = new double[whd];
 
 			int maxSz = 0;
 			for (s = 1; s <= numScales; s++)
 			{
 				if(depth >= 2*pad[s-1]) maxSz = s;
 			}
 
 			int cntX, cntY, cntZ;
 			for (s = 1; s <= numScales; s++)//for each scale
 			{
 				//convolve along the x direction
 				stepS = step[s-1];
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
 					if(depth <= 2*pad[s-1])
 					{
 						stepS = step[maxSz-1];
 						sw = stepS*width;
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

double** b3WaveletScales2D(double* dataIn, int width, int height, int numScales) throw (WaveletConfigException)
{
	if (numScales < 1)
 	{
			throw WaveletConfigException("Invalid number of wavelet scales. Number of scales should be an integer >=1");
 	}
 	//TODO: check dimensions vs number of scales
		
			//B3 spline wavelet configuration
			int pad[] = {2, 4, 8, 16, 32, 64, 128, 256, 512};
			int step[] = {1, 2, 4, 8, 16, 32, 64, 128, 256};
			double w2 =  ((double)1)/16;;
			double w1 = ((double)1)/4;;
			double w0 = ((double)3)/8;;
		
			int s;
			int padMax = pad[numScales-1];
			int stepS;
			int wh = width*height;
			double** resArray = new double*[numScales];
			double* array1 = dataIn;
			
			double* arrayX = new double[wh];
 			double* arrayY = new double[wh];
 			
 			 int cntX, cntY;
 			for (s = 1; s <= numScales; s++)//for each scale
 			{
 				//convolve along the x direction
 				stepS = step[s-1];
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

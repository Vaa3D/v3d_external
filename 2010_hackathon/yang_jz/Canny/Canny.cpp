/*
 *  CANNY.cpp
 *
 *  Created by Yang, Jinzhu, on 1/15/10.
 *
 */

#include "Canny.h"
#include "v3d_message.h"
#include <deque>
#include <algorithm>
#include <functional>
#include<math.h>

#define BACKGROUND -1 
#define DISOFENDS 5 


//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(CANNY, CannyPlugin);

//plugin funcs
const QString title = "Canny";
QStringList CannyPlugin::menulist() const
{
    return QStringList()
	<< tr("Canny")
	<< tr("Help");
}

void CannyPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
   if (menu_name == tr("Canny"))
	{
		Canny(callback, parent,1);
	}
	else if (menu_name == tr("Help"))
	{
		v3d_msg("Canny edge detection");
	}
}
//

void CannyPlugin::IterateSeg(unsigned char *apsInput, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer, unsigned char *apsOutput)
{
	V3DLONG pMax, pMin;
	V3DLONG i,j,k;
	double T1 ;
	double T2;
	V3DLONG S0 , n0;
	V3DLONG S1, n1 ;
	double allow; 
	double d ;	
	V3DLONG mCount = iImageHeight * iImageWidth;
	pMax = pMin = 0;
	for(V3DLONG k=0; k<iImageLayer; k++)
	{		
				
		for(j = 0; j<iImageHeight; j++)
		{
			for(i=0; i<iImageWidth; i++)
			{
				pMax = (apsInput[k*mCount+j *iImageWidth + i] > pMax)? apsInput[k*mCount+j *iImageWidth + i]:pMax;
				pMin = (apsInput[k*mCount+j *iImageWidth + i] < pMin)? apsInput[k*mCount+j *iImageWidth + i]:pMin;
			}
		}
	}
	for(V3DLONG m=0; m<iImageLayer; m++)
	{		
		pMax = pMin =apsInput[m*mCount];		
		for(i=0; i<mCount; i++)
		{
			for(k=0; k<mCount; k++)
			{
				pMax = (apsInput[m*mCount+k] > pMax)? apsInput[m*mCount+k]:pMax;
				pMin = (apsInput[m*mCount+k] < pMin)? apsInput[m*mCount+k]:pMin;
				
			}
			T1 = (pMax + pMin) / 2.0;
			T2 = 0;
			S0 = 0;
			n0 = 0;
			S1 = 0;
			n1 = 0;
			allow = 1.0; 
			if ( T1 > T2) 
			{
				d = T1 - T2;
			}else
			{
				d = T2 - T1;
			}
			
			//	d = abs(T - TT);			
			while(d > allow) 
			{	
				for(j=0; j<mCount; j++)
				{
					if(apsInput[m*mCount+j] > T1) 
					{
						S0 += apsInput[m*mCount+j];
						n0++;
					}
					else
					{
						S1 += apsInput[m*mCount+j];
						n1++;
					}
				}				
				if(n0 ==0 || n1 == 0)
					return  ;
				else
				{   
					T2 = (S0 / n0 + S1 / n1) / 2;
				}
				
				//	d = abs (T - TT);
				if ( T1 > T2) 
				{
					d = T1 -T2;
				}else
				{
					d = T2-T1;
				}
				T1 = T2;
			}
			for(i=0; i<mCount; i++)
			{
				if(apsInput[m*mCount+i] > T1)
				{
					apsOutput[m*mCount+i] = 255;				
				}
				else
				{
					apsOutput[m*mCount+i] = 0;				
				}
			}				
		}			
	}
//	for(V3DLONG m=0; m<iImageLayer; m++)
//	{		
//		pMax = pMin =apsInput[m*mCount];		
//		for(i=0; i<mCount; i++)
//		{
//			for(k=0; k<mCount; k++)
//			{
//				pMax = (apsInput[m*mCount+k] > pMax)? apsInput[m*mCount+k]:pMax;
//				pMin = (apsInput[m*mCount+k] < pMin)? apsInput[m*mCount+k]:pMin;
//				
//			}
//			T1 = (pMax + pMin) / 2.0;
//			T2 = 0;
//			S0 = 0;
//			n0 = 0;
//			S1 = 0;
//			n1 = 0;
//			allow = 1.0; 
//			if ( T1 > T2) 
//			{
//				d = T1 - T2;
//			}else
//			{
//				d = T2 - T1;
//			}
//
//		//	d = abs(T - TT);			
//			while(d > allow) 
//			{	
//				for(j=0; j<mCount; j++)
//				{
//					if(apsInput[m*mCount+j] > T1) 
//					{
//						S0 += apsInput[m*mCount+j];
//						n0++;
//					}
//					else
//					{
//						S1 += apsInput[m*mCount+j];
//						n1++;
//					}
//				}				
//				if(n0 ==0 || n1 == 0)
//					return  ;
//				else
//				{   
//					T2 = (S0 / n0 + S1 / n1) / 2;
//				}
//				
//			//	d = abs (T - TT);
//				if ( T1 > T2) 
//				{
//					d = T1 -T2;
//				}else
//				{
//					d = T2-T1;
//				}
//				T1 = T2;
//			}
//			for(i=0; i<mCount; i++)
//			{
//				if(apsInput[m*mCount+i] > T1)
//				{
//					apsOutput[m*mCount+i] = 255;				
//				}
//				else
//				{
//					apsOutput[m*mCount+i] = 0;				
//				}
//			}				
//		}			
//	}
}
void CannyPlugin::BinaryProcess(unsigned char*apsInput, unsigned char * aspOutput, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer, V3DLONG h, V3DLONG d)
{
	V3DLONG i, j,k,n,count;
	double t, temp;
	V3DLONG mCount = iImageHeight * iImageWidth;
	for (i=0; i<iImageLayer; i++)
	{
		for (j=0; j<iImageHeight; j++)
		{
			for (k=0; k<iImageWidth; k++)
			{
				V3DLONG curpos = i * mCount + j*iImageWidth + k;
				V3DLONG curpos1 = i* mCount + j*iImageWidth;
				V3DLONG curpos2 = j* iImageWidth + k;
				temp = 0;					
				count = 0;
				for(n =1 ; n <= d  ;n++)
				{
					if (k>h*n) {temp += apsInput[curpos1 + k-(h*n)]; count++;}  
					if (k+(h*n)< iImageWidth) { temp += apsInput[curpos1 + k+(h*n)]; count++;}
                    if (j>h*n) {temp += apsInput[i* mCount + (j-(h*n))*iImageWidth + k]; count++;}//	
					if (j+(h*n)<iImageHeight) {temp += apsInput[i* mCount + (j+(h*n))*iImageWidth + k]; count++;}//
					if (i>(h*n)) {temp += apsInput[(i-(h*n))* mCount + curpos2]; count++;}//	
					if (i+(h*n)< iImageLayer) {temp += apsInput[(i+(h*n))* mCount + j* iImageWidth + k ]; count++;}
				}
				t =  apsInput[curpos]-temp/(count);
				aspOutput[curpos]= (t > 0)? t : 0;
			}
		}
	}						
}

void CannyPlugin::Doublelinear_inserting (unsigned char*apsInput,V3DLONG &polValue,float x0, float y0, V3DLONG z0, V3DLONG currX, V3DLONG currY)
{
	//V3DLONG mCount = m_OiImgWidth * m_OiImgHeight;
  //  printf("m_OiImgWidth=%d m_OiImgHeight=%d x0=%lf y0=%lf z0=%d\n",m_OiImgWidth,m_OiImgHeight,x0,y0,z0);
	//printf("x00=%d currx=%d curry=%d\n",apsInput[currY * m_OiImgWidth + currX],currX,currY);	
	float deltaX, deltaY;
	deltaX = x0 - currX;
	deltaY = y0 - currY;
//	printf("deltax=%lf deltay=%lf\n",deltaX,deltaY);
	float x0y0 = (1-deltaX) * (1-deltaY);
	float x1y0 = deltaX * (1-deltaY);
	float x1y1 = deltaX * deltaY;
	float x0y1 = (1-deltaX) * deltaY;
	V3DLONG x = currX;
	V3DLONG y = currY;
	//printf("x=%d y=%d\n",x,y);
    polValue = (x0y0 * apsInput[y * m_OiImgWidth + x] +  
				x1y0 * apsInput[y * m_OiImgWidth + x +1]+
				x1y1 * apsInput[(y +1)* m_OiImgWidth + x +1] +
				x0y1 * apsInput[(y +1) * m_OiImgWidth + x ]
				);
	//printf("x0y0=%lf x1y0=%lf x1y1=%lf x0y1=%lf pixel=%d polvalue=%d\n",x0y0,x1y0,x1y1,x0y1,apsInput[currY * m_OiImgWidth + currX],polValue);	
}
void CannyPlugin::Canny(V3DPluginCallback &callback, QWidget *parent, int method_code)
{
	
	v3dhandle curwin = callback.currentImageWindow();
	
	int start_t = clock(); // record time point
	V3DLONG i,j,k;
	
	Image4DSimple* subject = callback.getImage(curwin);

	QString m_InputFileName = callback.getImageName(curwin);
	
	if (!subject)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return;
	}	
	Image4DProxy<Image4DSimple> pSub(subject);	
	V3DLONG sx = subject->getXDim();
	V3DLONG sy = subject->getYDim();
	V3DLONG sz = subject->getZDim();
	V3DLONG pagesz_sub = sx*sy*sz;
	unsigned char * pData = pSub.begin();
	
	unsigned char *apsInput = new unsigned char[sx*sy*sz];
	memset(apsInput, 0, sx*sy*sz * sizeof(unsigned char));
	
	int iSize = 3;
	
	unsigned char *apsOutput = new unsigned char[sx*sy*sz];
	memset(apsOutput, 0, sx*sy*sz * sizeof(unsigned char));
	
	unsigned char *apsInput1 = new unsigned char[sx*sy*sz];
	memset(apsInput1, 0, sx*sy*sz * sizeof(unsigned char));
	
	float *apfGaussTemplate3D = new float[iSize*iSize*iSize];
	
	memset(apfGaussTemplate3D, 0, iSize*iSize*iSize* sizeof(float));
	CreateGaussFilterTemplet3D(apfGaussTemplate3D,iSize,0.8);

	GaussFilter3D(pData, apsInput,sx,sy,sz,apfGaussTemplate3D,iSize);
	
	V3DLONG *pGradX = new V3DLONG[sx*sy*sz];
	memset(pGradX, 0, sx*sy*sz * sizeof(V3DLONG));
	
	V3DLONG *pGradY = new V3DLONG[sx*sy*sz];
	memset(pGradY, 0, sx*sy*sz * sizeof(V3DLONG));
	
	unsigned char *Gradient = new unsigned char[sx*sy*sz];
	memset(Gradient, 0, sx*sy*sz*sizeof(unsigned char));
	
	unsigned char *NonMax = new unsigned char[sx*sy*sz];
	memset(NonMax, 0, sx*sy*sz*sizeof(unsigned char));
	
	Getgrad(sx,sy,sz,apsInput,pGradX,pGradY,Gradient,NonMax);		

	DetectCannyEdges(sx,sy,sz,apsInput1,pGradX,pGradY,Gradient,NonMax);    	

		
	Image4DSimple p4DImage;
	p4DImage.setData(apsInput1, sx, sy, sz, 1, V3D_UINT8);
	v3dhandle newwin;
	if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
		newwin = callback.currentImageWindow();
	else
		newwin = callback.newImageWindow();
	callback.setImage(newwin, &p4DImage);
	callback.setImageName(newwin, QString("gaussian3D image"));
	callback.updateImageWindow(newwin);	
	
	if (pGradX) 
	{
		delete []pGradX;
		pGradX =NULL;
	}
	if (pGradY) 
	{
		delete []pGradY;
		pGradY =NULL;
	}
	
	if (Gradient) 
	{
		delete []Gradient;
		Gradient =NULL;
	}
	if (NonMax) 
	{
		delete []NonMax;
		NonMax =NULL;
	}
	if (apsInput) 
	{
		//delete []apsInput;
		//apsInput =NULL;
	}
	
}

void CannyPlugin::Getgrad(V3DLONG m_iWid, V3DLONG m_iHei, V3DLONG m_iCount, unsigned char*apsInput,V3DLONG *pGradX, V3DLONG *pGradY, unsigned char *Gradient,unsigned char*NonMax)
{
	V3DLONG j,i,k;
	double dSqt1;
	double dSqt2;
	for(k = 0; k<m_iCount; k++)
	{
		for(j=1;j<m_iHei-1;j++)
		{
			for(i=1;i<m_iWid-1;i++)
			{
				V3DLONG cur = k*m_iWid*m_iHei+j*m_iWid+i;
			//	V3DLONG px = apsInput[cur+1]-apsInput[cur-1];
			//	V3DLONG py = apsInput[k*m_iWid*m_iHei+(j+1)*m_iWid +i] - apsInput[k*m_iWid*m_iHei+(j-1)*m_iWid +i];
				//V3DLONG p45 = apsInput[k*m_iWid*m_iHei+(j+1)*m_iWid +i-1] - apsInput[k*m_iWid*m_iHei+(j-1)*m_iWid +i+1];
				//V3DLONG p135 = apsInput[k*m_iWid*m_iHei+(j+1)*m_iWid +i+1] - apsInput[k*m_iWid*m_iHei+(j-1)*m_iWid +i-1];
				//pGradX[cur] = px+(p45+p135)/2;
				//pGradY[cur] = py+(p45-p135)/2;
				
				pGradX[cur] = apsInput[cur+1]-apsInput[cur-1];
				pGradY[cur] = apsInput[k*m_iWid*m_iHei+(j+1)*m_iWid +i] - apsInput[k*m_iWid*m_iHei+(j-1)*m_iWid +i];
				dSqt1 = pGradX[cur]*pGradX[cur];
				dSqt2 = pGradY[cur]*pGradY[cur];
				Gradient[cur] = (sqrt(dSqt1+dSqt2)+0.5);
				//NonMax[cur]=Gradient[cur];
			}
		}		
		
	}
	
}
void CannyPlugin::DetectCannyEdges(V3DLONG m_iWid, V3DLONG m_iHei, V3DLONG m_iCount, unsigned char*apsInput,V3DLONG *pGradX, V3DLONG *pGradY, unsigned char *Gradient,unsigned char *NonMax)
{
	
	float Tangent;
	
	float PI=3.1416;
	
	V3DLONG k , j ,i,cur;
	
	V3DLONG nThrHigh,nThrLow;
	
	V3DLONG gx,gy;
	
	V3DLONG g1,g2,g3,g4;
	
	double weight;
	
	double Tmp,Tmp1,Tmp2;
	
	nThrLow = 15;
	nThrHigh = 25;
	bool type = false;
	if(type) 
	{
		for(k = 0; k<m_iCount; k++)
		{
			for(j=1;j<m_iHei-1;j++)
			{
				for(i=1;i<m_iWid-1;i++)
				{
					cur = k*m_iWid*m_iHei+j*m_iWid+i;
					if (pGradX[cur] == 0)
						Tangent = 90;
					else
						Tangent = (float)(atan(pGradY[cur]/pGradX[cur]) * 180 / PI); //rad to degree
				    
					//Horizontal Edge
					if (((-22.5 < Tangent) && (Tangent <= 22.5)) || ((157.5 < Tangent) && (Tangent <= -157.5)))
					{
						if ((Gradient[cur] < Gradient[k*m_iWid*m_iHei+(j+1)*m_iWid+i]) || (Gradient[cur] < Gradient[k*m_iWid*m_iHei+(j-1)*m_iWid+i]))
							NonMax[cur] = 0;
						else 
						{
							NonMax[cur] = 128;
							
						}
						
					}
					//Vertical Edge
					if (((-112.5 < Tangent) && (Tangent <= -67.5)) || ((67.5 < Tangent) && (Tangent <= 112.5)))
					{
						if ((Gradient[cur] < Gradient[k*m_iWid*m_iHei+j*m_iWid+i+1]) || (Gradient[cur] < Gradient[k*m_iWid*m_iHei+j*m_iWid+i]-1))
							NonMax[cur] = 0;
						else 
						{
							NonMax[cur] = 128;
							
						}
					}
					//-45 Degree Edge
					if (((-67.5 < Tangent) && (Tangent <= -22.5)) || ((112.5 < Tangent) && (Tangent <= 157.5)))
					{
						if ((Gradient[cur] < Gradient[k*m_iWid*m_iHei+(j-1)*m_iWid+i-1]) || (Gradient[cur]< Gradient[k*m_iWid*m_iHei+(j+1)*m_iWid+i-1]))
							NonMax[cur] = 0;
						else 
						{
							NonMax[cur] = 128;
							
						}
					}
					
					//+45 Degree Edge
					if (((-157.5 < Tangent) && (Tangent <= -112.5)) || ((67.5 < Tangent) && (Tangent <= 22.5)))
					{
						if ((Gradient[cur]< Gradient[k*m_iWid*m_iHei+(j+1)*m_iWid+i+1]) || (Gradient[cur] < Gradient[k*m_iWid*m_iHei+(j-1)*m_iWid+i-1]))
							NonMax[cur] = 0;
						else 
						{
							NonMax[cur] = 128;
							
						}
					}
					
				}
			}		
			
		}
	}else 	
	{
		for(k = 0; k<m_iCount; k++)
		{
			for(j=1;j<m_iHei-1;j++)
			{
				for(i=1;i<m_iWid-1;i++)
				{
					cur = k*m_iWid*m_iHei+j*m_iWid+i;
					if(Gradient[cur] == 0)
					{
						NonMax[cur] = 0;
					}
					else
					{
						Tmp = Gradient[cur];
						gx = pGradX[cur];
						gy = pGradY[cur];
						if(abs(gy) > abs(gx))
						{
							weight = fabs(gx)/fabs(gy);
							
							g2 = Gradient[cur-m_iWid];
							g4 = Gradient[cur+m_iWid];

							if(gx*gy>0)
							{
								g1 = Gradient[cur-m_iWid-1];
								g3 = Gradient[cur+m_iWid+1];
							}

							else
							{
								g1 = Gradient[cur-m_iWid+1];
								g3 = Gradient[cur+m_iWid-1];
							}
						}
						else
						{
							weight = fabs(gy)/fabs(gx);
							
							g2 = Gradient[cur+1];
							g4 = Gradient[cur-1];
							
							if(gx * gy > 0)
							{
								g1 = Gradient[cur+m_iWid+1];
								g3 = Gradient[cur-m_iWid-1];
							}
							
							else
							{
								g1 = Gradient[cur-m_iWid+1];
								g3 = Gradient[cur+m_iWid-1];
							}
						}
						{
							Tmp1 = weight*g1 + (1-weight)*g2;
							Tmp2 = weight*g3 + (1-weight)*g4;
						
							if(Tmp>=Tmp1 && Tmp>=Tmp2)
							{
								NonMax[cur] = 128;
							}
							else
							{
								NonMax[cur] = 0;
							}
						}
					}
					
					
				}
			}		
			
		}
	}

	for(k = 0; k<m_iCount; k++)
	{
		for(j=1;j<m_iHei-1;j++)
		{
			for(i=1;i<m_iWid-1;i++)
			{
				cur = k*m_iWid*m_iHei+j*m_iWid+i;
				
				if((NonMax[cur]==128) && (Gradient[cur] >= nThrHigh))
				{
					apsInput[cur] = 255;
					TraceEdge(k,j,i,nThrLow,apsInput,Gradient,m_iWid,m_iHei,m_iCount);
				}
			}
		}
	}
	for(k = 0; k<m_iCount; k++)
	{
		for(j=1;j<m_iHei-1;j++)
		{
			for(i=1;i<m_iWid-1;i++)
			{
				cur = k*m_iWid*m_iHei+j*m_iWid+i;
				
				if(apsInput[cur]!=255)
				{
					apsInput[cur] = 0;
				}
			}
		}
	}
	
}

void CannyPlugin::TraceEdge(V3DLONG k, V3DLONG y, V3DLONG x, V3DLONG nThrLow,unsigned char*apsInput,unsigned char*Gradient,V3DLONG m_iWid, V3DLONG m_iHei, V3DLONG m_iCount)
{
	
    static int nDx[] = {1,1,0,-1,-1,-1,0,1};
	static int nDy[] = {0,1,1,1,0,-1,-1,-1};
	
	V3DLONG yy,xx;
	
	for(k=0;k<8;k++)
	{
		yy = y+nDy[k];
		xx = x+nDx[k];
		if(apsInput[k*m_iWid*m_iHei+yy*m_iWid+xx]==128 && Gradient[k*m_iWid*m_iHei+yy*m_iWid+xx]>=nThrLow )
		{
			apsInput[k*m_iWid*m_iHei+yy*m_iWid+xx] = 255;
			
			TraceEdge(k,yy,xx,nThrLow,apsInput,Gradient,m_iWid,m_iHei,m_iCount);			
		}
	}
}
void CannyPlugin::Thinning(unsigned char *apsInput,unsigned char *apsOutput,V3DLONG m_iWid, V3DLONG m_iHei, V3DLONG m_iCount)
{
	bool bCondition1;
	bool bCondition2;
	bool bCondition3;
	bool bCondition4;
	int neighbour[5][5];
    int nCount;
	int m;
	V3DLONG curPos;
	V3DLONG i,j ;
	V3DLONG* pTmpMark  = new V3DLONG[m_iWid * m_iHei];
	V3DLONG* pTmpMark1 = new V3DLONG[m_iWid * m_iHei];
	bool bModify;
	bModify = true;
	for(i =0; i< m_iHei; i++)
	{
		for(j = 0; j < m_iWid;j++)
		{
			pTmpMark1[i*m_iWid+j] = apsInput[i*m_iWid+j];
			printf("tmp=%d\n",pTmpMark1[i*m_iWid+j]);
		}
	}
	//memcpy(pTmpMark1, apsInput, m_iWid * m_iHei);
	while (bModify) 
	{
		bModify = false;
		
		memset(pTmpMark, 0, m_iWid * m_iHei * sizeof(V3DLONG));
		
		for(int i = 2 ; i< m_iHei - 2 ; i++ )
		{
			for(int j = 2; j<m_iWid - 2; j++)
			{
				bCondition1 = false;
				bCondition2 = false;
				
				bCondition3 = false;
				bCondition4 = false;
				curPos = i*m_iWid+j;
				if( pTmpMark1[curPos] == 255)
				{
					for (int a = 0; a < 5;a++ )
					{
						for (int n = 0; n < 5;n++)
						{
							neighbour[a][n] = pTmpMark1[curPos + (2 - a)*m_iWid +( n - 2)]/ 255;
						}
					}
//					for(int ii=-1;ii<2;ii++)
//						for(int jj=-1;jj<2;jj++)
//						{
//							V3DLONG y = j+ii;
//							V3DLONG x = i+jj;
//							neighbour[ii+2][jj+2] = (pTmpMark1[m_iWid * y + x]/255);
//						}
					
					//逐个判断条件。
					//判断2<=NZ(P1)<=6
					nCount =  neighbour[1][1] + neighbour[1][2] + neighbour[1][3] 
					+ neighbour[2][1] + neighbour[2][3] + 
					+ neighbour[3][1] + neighbour[3][2] + neighbour[3][3];
					if ( nCount >= 2 && nCount <=6)
						bCondition1 = true;
					//判断Z0(P1)=1
					nCount = 0;
					if (neighbour[1][2] == 0 && neighbour[1][1] == 1)
						nCount++;
					if (neighbour[1][1] == 0 && neighbour[2][1] == 1)
						nCount++;
					if (neighbour[2][1] == 0 && neighbour[3][1] == 1)
						nCount++;
					if (neighbour[3][1] == 0 && neighbour[3][2] == 1)
						nCount++;
					if (neighbour[3][2] == 0 && neighbour[3][3] == 1)
						nCount++;
					if (neighbour[3][3] == 0 && neighbour[2][3] == 1)
						nCount++;
					if (neighbour[2][3] == 0 && neighbour[1][3] == 1)
						nCount++;
					if (neighbour[1][3] == 0 && neighbour[1][2] == 1)
						nCount++;
					if (nCount == 1)
						bCondition2 = true;
					//判断P2*P4*P8=0 or Z0(p2)!=1
					if (neighbour[1][2]*neighbour[2][1]*neighbour[2][3] == 0)
						bCondition3 = true;
					else
					{
						nCount = 0;
						if (neighbour[0][2] == 0 && neighbour[0][1] == 1)
							nCount++;
						if (neighbour[0][1] == 0 && neighbour[1][1] == 1)
							nCount++;
						if (neighbour[1][1] == 0 && neighbour[2][1] == 1)
							nCount++;
						if (neighbour[2][1] == 0 && neighbour[2][2] == 1)
							nCount++;
						if (neighbour[2][2] == 0 && neighbour[2][3] == 1)
							nCount++;
						if (neighbour[2][3] == 0 && neighbour[1][3] == 1)
							nCount++;
						if (neighbour[1][3] == 0 && neighbour[0][3] == 1)
							nCount++;
						if (neighbour[0][3] == 0 && neighbour[0][2] == 1)
							nCount++;
						if (nCount != 1)
							bCondition3 = true;
					}
					//判断P2*P4*P6=0 or Z0(p4)!=1
					if (neighbour[1][2]*neighbour[2][1]*neighbour[3][2] == 0)
						bCondition4 = true;
					else
					{
						nCount = 0;
						if (neighbour[1][1] == 0 && neighbour[1][0] == 1)
							nCount++;
						if (neighbour[1][0] == 0 && neighbour[2][0] == 1)
							nCount++;
						if (neighbour[2][0] == 0 && neighbour[3][0] == 1)
							nCount++;
						if (neighbour[3][0] == 0 && neighbour[3][1] == 1)
							nCount++;
						if (neighbour[3][1] == 0 && neighbour[3][2] == 1)
							nCount++;
						if (neighbour[3][2] == 0 && neighbour[2][2] == 1)
							nCount++;
						if (neighbour[2][2] == 0 && neighbour[1][2] == 1)
							nCount++;
						if (neighbour[1][2] == 0 && neighbour[1][1] == 1)
							nCount++;
						if (nCount != 1)
							bCondition4 = true;
					}
					if(bCondition1 && bCondition2 && bCondition3 && bCondition4)
					{
						pTmpMark[curPos] = 0;
						bModify = true;
					}
					else
					{
						pTmpMark[curPos] = 255;
					}
				} 
			}
		}
		for(i =0; i< m_iHei; i++)
		{
			for(j = 0; j < m_iWid;j++)
			{
				pTmpMark1[i*m_iWid+j] = pTmpMark[i*m_iWid+j];
			}
		}		
		//memcpy(pTmpMark1, pTmpMark, m_iWid*m_iHei);
	} 

	for(int i = 1 ; i< m_iHei  ; i++ )
	{
		for(int j = 1; j<m_iWid ; j++)
		{
			curPos = i*m_iWid+j;
			apsOutput[curPos] |= pTmpMark1[curPos];
		}
	}
    delete []pTmpMark;
	delete []pTmpMark1;
}

void CannyPlugin::CreateGaussFilterTemplet2D(float *apfTemplet, int iSize, float fLamb)
{
	if((iSize % 2) == 0)
	{
		return;
	}	
	int x;
	int y;
	int z;
	int i;
	int j;
	int k;
	
	int iHalfSize = iSize / 2;
	float fTemp;
	int iSizeS;
	
	int matrix_length;
	float std_dev;
	float sum;
	
	float radius = iHalfSize ;
	
	radius = (float)fabs(0.5*radius) + 0.25f;
	
	std_dev = radius;
	
	radius = std_dev * 2;
	
	matrix_length = int (2 * ceil(radius-0.5) + 1);
	
	if (matrix_length <= 0) matrix_length = 1;
	
	bool type = false;
	
	for (j = 0; j < iSize; j++)
	{
		for (i = 0; i < iSize; i++)
		{
			if(type) //not sampling
			{
				x = 3*(i - iHalfSize)/iHalfSize;
				y = 3*(j - iHalfSize)/iHalfSize;
				fTemp = (x * x + y * y ) / 2;
				apfTemplet[j*iSize+i] = (float)exp(-fTemp);
			//	printf("gaussian=%lf aa=%d\n",apfTemplet[j*iSize+i],aa);
				
			}else ///from -2*std_dev to 2*std_dev, sampling 50 points per pixel
			{
				x = i - iHalfSize;
				y = j - iHalfSize;				
				
				float base_x = abs(x) - (float)floor((float)(matrix_length/2)) - 0.5f;
				float base_y = abs(y) - (float)floor((float)(matrix_length/2)) - 0.5f;
				
				sum = 0;
				for (k = 1; k <= 50; k++)
				{
					if ( base_x+0.02*k <= radius ) 
					sum += (float)exp (-((base_x+0.02*k)*(base_x+0.02*k)+ (base_y+0.02*k)*(base_y+0.02*k))/ 
										   (2*std_dev*std_dev));
					
				}
				apfTemplet[j*iSize+i] = sum/50;
				//printf("gaussian=%lf\n",apfTemplet[j*iSize+i]);
			}
		}
	}	
	if (type==false) 
	{
		sum = 0;
		for (j=0; j<=50; j++)
		{
			sum += (float)exp (-(0.5+0.02*j)*(0.5+0.02*j)+(0.5+0.02*j)*(0.5+0.02*j)/
							   (2*std_dev*std_dev));
			
		}
		for(k = 0; k < iSize; k++)
			apfTemplet[iHalfSize*iSize+k] = sum/51;
		//   printf("gaussianhalf=%lf\n",apfTemplet[iHalfSize*iSize+k]);
	}
	
	return;
}

void CannyPlugin::CreateGaussFilterTemplet3D(float *apfTemplet, int iSize, float fLamb)
{
	if((iSize % 2) == 0)
	{
		return;
	}	
	int x;
	int y;
	int z;
	int i;
	int j;
	int k;
	
	int iHalfSize = iSize / 2;
	float fTemp;
	int iSizeS;
	
	int matrix_length;
	float std_dev;
	float sum;

	float radius = iHalfSize ;
	
	radius = (float)fabs(0.5*radius) + 0.25f;
	
	std_dev = radius;
	
	radius = std_dev * 2;
	
	matrix_length = int (2 * ceil(radius-0.5) + 1);
	
	if (matrix_length <= 0) matrix_length = 1;
	
	for(k = 0; k < iSize; k++)
	{
		for (j = 0; j < iSize; j++)
		{
			for (i = 0; i < iSize; i++)
			{
				{
				//	x = 3*(i - iHalfSize)/iHalfSize;
//					y = 3*(j - iHalfSize)/iHalfSize;
//					z = 3*(k - iHalfSize)/iHalfSize;
//					fTemp = (z * z + x * x + y * y ) / 2;
//					apfTemplet[k*iSize*iSize+j*iSize+i] = (float)exp(-fTemp);
					
					x = i - iHalfSize;
					y = j - iHalfSize;				
					z = k - iHalfSize;
					float base_x = abs(x) - (float)floor((float)(matrix_length/2)) - 0.5f;
					float base_y = abs(y) - (float)floor((float)(matrix_length/2)) - 0.5f;
					float base_z = abs(z) - (float)floor((float)(matrix_length/2)) - 0.5f;
					sum = 0;
					for (int kk = 1; kk <= 50; kk++)
					{
						if ( base_x+0.02*kk <= radius ) 
							sum += (float)exp (-((base_x+0.02*kk)*(base_x+0.02*kk)+ (base_y+0.02*kk)*(base_y+0.02*kk)+(base_z+0.02*kk)*(base_z+0.02*kk))/ 
											   (2*std_dev*std_dev));
						
					}
					apfTemplet[j*iSize+i] = sum/50;
					
				}
			}
		}	
	}
	return;
}

void CannyPlugin::GaussFilter2D(unsigned char *apsImgInput, unsigned char *apfImgOutput,V3DLONG m_iWid, V3DLONG m_iHei, V3DLONG m_iCount, float *apfTemplate,int iTmpLen)
{
	
	int i;
	int j;
	int k;
	int m;
	int n;
	int l;
	
	float fSum;
	float fTotal = 0.0;
	int iSize = iTmpLen / 2;
	
	for (n = -iSize; n < iSize + 1; n++)
	{
		for (m = -iSize; m < iSize + 1; m++)
		{
			fTotal += apfTemplate[(n + iSize)*iTmpLen+m + iSize];
		}
	}
	
	for (j = iSize; j < m_iHei - iSize; j++)
	{
		for (i = iSize; i < m_iWid - iSize ; i++)
		{
			fSum = 0;
			{
				for (n = -iSize; n < iSize + 1; n++)
				{
					for (m = -iSize; m < iSize + 1; m++)
					{
						fSum += apsImgInput[(j + n)*m_iWid + i + m] * 
						apfTemplate[(n + iSize)*iTmpLen+m + iSize];
					}
				}
			}
			apfImgOutput[j*m_iWid+i] = (char)(0.5f + fSum / fTotal);
		}
	}
		
}

void CannyPlugin::GaussFilter3D(unsigned char *apsImgInput, unsigned char *apfImgOutput,V3DLONG m_iWid, V3DLONG m_iHei, V3DLONG m_iCount, float *apfTemplate,int iTmpLen)
{
	
	int i;
	int j;
	int k;
	int m;
	int n;
	int l;
	float fSum;
	float fTotal = 0.0;
	int iSize = iTmpLen / 2;
	
	for (l = -iSize; l < iSize + 1; l++)
	{
		for (n = -iSize; n < iSize + 1; n++)
		{
			for (m = -iSize; m < iSize + 1; m++)
			{
				
				fTotal += apfTemplate[(l + iSize)*iTmpLen*iTmpLen+(n + iSize)*iTmpLen+m + iSize];
			}
		}
	}
	
	for (k =  iSize; k < m_iCount - iSize; k++)
	{
		for (j = iSize; j < m_iHei - iSize; j++)
		{
			for (i = iSize; i < m_iWid - iSize ; i++)
			{
				fSum = 0;
				{
					for (l = -iSize; l < iSize + 1; l++)
					{
						for (n = -iSize; n < iSize + 1; n++)
						{
							for (m = -iSize; m < iSize + 1; m++)
							{
								fSum += apsImgInput[(k + l)*m_iHei*m_iWid+(j + n)*m_iWid+i + m] * 
								apfTemplate[(l + iSize)*iTmpLen*iTmpLen+(n + iSize)*iTmpLen+m + iSize];
							}
						}
					}
				}
				apfImgOutput[k*m_iHei*m_iWid+j*m_iWid+i] = fSum / fTotal;
			}
		}
	}
	
}


void CannyDialog::update()
{
	//get current data
	Dn = Dnumber->text().toLong()-1;
	Dh = Ddistance->text().toLong()-1;
		//printf("channel %ld val %d x %ld y %ld z %ld ind %ld \n", c, data1d[ind], nx, ny, nz, ind);
}


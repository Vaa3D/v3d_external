#include "NeuronSelector.h"
#include <QtAlgorithms>
#include <iostream>

#include "../3drenderer/renderer_tex2.h"
#include "../3drenderer/v3dr_glwidget.h"

using namespace std;

// mouse left click to select neuron
XYZ Renderer_tex2::selectPosition(int x, int y)
{
	// _appendMarkerPos
	MarkerPos pos;
	pos.x = x;
	pos.y = y;
	for (int i=0; i<4; i++)
		pos.view[i] = viewport[i];
	for (int i=0; i<16; i++)
	{
		pos.P[i]  = projectionMatrix[i];
		pos.MV[i] = markerViewMatrix[i];
	}
	
	// getCenterOfMarkerPos
	XYZ P1, P2;
	
	//_MarkerPos_to_NearFarPoint
	Matrix P(4,4);		P << pos.P;   P = P.t();    // OpenGL is row-inner / C is column-inner
	Matrix M(4,4);		M << pos.MV;  M = M.t();
	Matrix PM = P * M;
	
	double xd = (pos.x             - pos.view[0])*2.0/pos.view[2] -1;
	double yd = (pos.view[3]-pos.y - pos.view[1])*2.0/pos.view[3] -1; // OpenGL is bottom to top
	//double z = 0,1;                              // the clip space depth from 0 to 1
	
	ColumnVector pZ0(4); 	pZ0 << xd << yd << 0 << 1;
	ColumnVector pZ1(4); 	pZ1 << xd << yd << 1 << 1;
	if (bOrthoView)
	{
		pZ0(3) = -1;  //100913
	}
	ColumnVector Z0 = PM.i() * pZ0;       //cout << "Z0 \n" << Z0 << endl;
	ColumnVector Z1 = PM.i() * pZ1;       //cout << "Z1 \n" << Z1 << endl;
	Z0 = Z0 / Z0(4);
	Z1 = Z1 / Z1(4);
	
	P1 = XYZ(Z0(1), Z0(2), Z0(3));
	P2 = XYZ(Z1(1), Z1(2), Z1(3));
	
	// getCenterOfLineProfile
	XYZ loc = (P1+P2)*.5;
	
	//
	int chno;
	
	V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
	My4DImage* curImg = 0;
	
	if (w)
	{
		curImg = v3dr_getImage4d(_idep);
		//when chno<0, then need to recheck the current chno
		if (chno<0 || chno>dim4) chno = w->getNumKeyHolding()-1;
		if (chno<0 || chno>dim4) chno = curChannel; //100802 RZC: default channel set by user
	}
	
	double clipplane[4] = { 0.0,  0.0, -1.0,  0 };
	// [0, 1] ==> [+1, -1]*(s)
	clipplane[3] = viewClip;
	ViewPlaneToModel(markerViewMatrix, clipplane);
	
	if (curImg && data4dp && chno>=0 && chno<dim4)
	{
		double f = 0.8; // must be LESS 1 to converge, close to 1 is better
		
		XYZ D = P2-P1; normalize(D);
		
		unsigned char* vp = 0;
		switch (curImg->getDatatype())
		{
			case V3D_UINT8:
				vp = data4dp + (chno + volTimePoint*dim4)*(dim3*dim2*dim1);
				break;
			case V3D_UINT16:
				vp = data4dp + (chno + volTimePoint*dim4)*(dim3*dim2*dim1)*sizeof(short int);
				break;
			case V3D_FLOAT32:
				vp = data4dp + (chno + volTimePoint*dim4)*(dim3*dim2*dim1)*sizeof(float);
				break;
			default:
				v3d_msg("Unsupported data type found. You should never see this.", 0);
				return loc;
		}
		
		for (int i=0; i<200; i++) // iteration, (2-f)^200 is big enough
		{
			double length = norm(P2-P1);
			if (length < 0.5) // pixel
				break; //////////////////////////////////
			
			int nstep = int(length + 0.5);
			double step = length/nstep;
			
			XYZ sumloc(0,0,0);
			float sum = 0;
			for (int i=0; i<=nstep; i++)
			{
				XYZ P = P1 + D*step*(i);
				float value;
				switch (curImg->getDatatype())
				{
					case V3D_UINT8:
						value = sampling3dAllTypesatBounding( vp, dim1, dim2, dim3,  P.x, P.y, P.z, dataViewProcBox.box, clipplane);
						break;
					case V3D_UINT16:
						value = sampling3dAllTypesatBounding( (short int *)vp, dim1, dim2, dim3,  P.x, P.y, P.z, dataViewProcBox.box, clipplane);
						break;
					case V3D_FLOAT32:
						value = sampling3dAllTypesatBounding( (float *)vp, dim1, dim2, dim3,  P.x, P.y, P.z, dataViewProcBox.box, clipplane);
						break;
					default:
						v3d_msg("Unsupported data type found. You should never see this.", 0);
						return loc;
				}
				
				sumloc = sumloc + P*(value);
				sum = sum + value;
			}
			
			if (sum)
				loc = sumloc / sum;
			else
				break; //////////////////////////////////
			
			P1 = loc - D*(length*f/2);
			P2 = loc + D*(length*f/2);
		}
	}
	
	qDebug()<<"0-based pos ... "<<loc.x<<loc.y<<loc.z;
	
	return loc;
}

// NeuronSelector init func
void NeuronSelector::init()
{
	//
	index = -1;
}

// swith status of selected neuron
void NeuronSelector::switchSelectedNeuron(int index)
{	
        if(annotationSession->getNeuronSelectList().at(index) == true)
        {
                annotationSession->getNeuronSelectList().replace(index, false);
        }
        else
        {
                annotationSession->getNeuronSelectList().replace(index, true);
        }
}

// get the index of selected neuron
int NeuronSelector::getIndexSetectedNeuron()
{
	// find in mask stack
	sx = annotationSession->getNeuronMaskAsMy4DImage()->getXDim();
	sy = annotationSession->getNeuronMaskAsMy4DImage()->getYDim();
	sz = annotationSession->getNeuronMaskAsMy4DImage()->getZDim();
		
	// sum of pixels of each neuron mask in the cube 
	int *sum = NULL;
	
	int numNeuron = annotationSession->getMaskStatusList().size();
	
	qDebug()<<"how many neurons ... "<<numNeuron;
	
	try
	{
		sum = new int [numNeuron];		
		
		// init 0 
		//memset(sum, 0, numNeuron); 
		for(V3DLONG i=0; i<numNeuron; i++)
		{
			sum[i] = 0;
		}
	}
	catch (...) 
	{
		printf("Out of memory\n");
		return -1;
	}
	
	//
	V3DLONG xb = xlc-NB; if(xb<0) xb = 0;
	V3DLONG xe = xlc+NB; if(xe>sx) xe = sx-1;
	V3DLONG yb = ylc-NB; if(yb<0) yb = 0;
	V3DLONG ye = ylc+NB; if(ye>sy) ye = sy-1;
	V3DLONG zb = zlc-NB; if(zb<0) zb = 0;
	V3DLONG ze = zlc+NB; if(ze>sz) ze = sz-1;
	
	for(V3DLONG k=zb; k<=ze; k++)
	{
		V3DLONG offset_k = k*sx*sy;
		for(V3DLONG j=yb; j<=ye; j++)
		{
			V3DLONG offset_j = offset_k + j*sx;
			for(V3DLONG i=xb; i<=xe; i++)
			{
				V3DLONG idx = offset_j + i;
				
				int cur_idx = annotationSession->getNeuronMaskAsMy4DImage()->getRawData()[idx]; // value of mask stack
				
				if(cur_idx>0 && annotationSession->getMaskStatusList().at(cur_idx)) // active masks
				{
					sum[cur_idx]++;
				}
			}
		}
	}
	
	//
	index = 0;
	for(V3DLONG i=1; i<numNeuron; i++)
	{
		//qDebug()<<"sum ["<<i<<"] ..."<<sum[i];
		
		if(sum[i]>0 && sum[i]>sum[index])
		{
			index = i;
		}
	}
	
	// de-alloc
	if(sum) {delete []sum; sum = NULL;}
	
	//
	qDebug()<<"index ..."<<index;
	
	if(index>0) 
	{
		switchSelectedNeuron(index);
	}
	else
	{
		index = -1; // 0 is background
	}
	
	return index;
}

//
void NeuronSelector::getCurNeuronBoundary()
{
	index = getIndexSetectedNeuron();
	
	if(index<=0) return;
	
	//
	curNeuronBDxb = sx-1;
	curNeuronBDxe = 0;
	
	curNeuronBDyb = sy-1;
	curNeuronBDye = 0;
	
	curNeuronBDzb = sz-1;
	curNeuronBDze = 0;
	
	//
	for(V3DLONG k=0; k<sz; k++)
	{
		V3DLONG offset_k = k*sx*sy;
		for(V3DLONG j=0; j<sy; j++)
		{
			V3DLONG offset_j = offset_k + j*sx;
			for(V3DLONG i=0; i<sx; i++)
			{
				V3DLONG idx = offset_j + i;
				
				if(annotationSession->getNeuronMaskAsMy4DImage()->getRawData()[idx] == index)
				{
					if(i<curNeuronBDxb) curNeuronBDxb = i;
					if(i>curNeuronBDxe) curNeuronBDxe = i;
					
					if(j<curNeuronBDyb) curNeuronBDyb = j;
					if(j>curNeuronBDye) curNeuronBDye = j;
					
                                        if(k<curNeuronBDzb) curNeuronBDzb = k;
                                        if(k>curNeuronBDze) curNeuronBDze = k;
				}
				
			}
		}
	}
	
	// x
	if( curNeuronBDxb-NB > 0 )
	{
		curNeuronBDxb -= NB;
	}
	else
	{
		curNeuronBDxb = 0;
	}
	
	if(curNeuronBDxe+NB < sx)
	{
		curNeuronBDxe += NB;
	}
	else
	{
		curNeuronBDxe = sx - 1;
	}
	
	// y
	if( curNeuronBDyb-NB > 0 )
	{
		curNeuronBDyb -= NB;
	}
	else
	{
		curNeuronBDyb = 0;
	}
	
	if(curNeuronBDye+NB < sy)
	{
		curNeuronBDye += NB;
	}
	else
	{
		curNeuronBDye = sy - 1;
	}
	
	// z
	if( curNeuronBDzb-NB > 0 )
	{
		curNeuronBDzb -= NB;
	}
	else
	{
		curNeuronBDzb = 0;
	}
	
	if(curNeuronBDze+NB < sz)
	{
		curNeuronBDze += NB;
	}
	else
	{
		curNeuronBDze = sz - 1;
	}
}

//
bool NeuronSelector::inNeuronMask(V3DLONG x, V3DLONG y, V3DLONG z)
{
	if(x<=curNeuronBDxe && x>=curNeuronBDxb && y<=curNeuronBDye && y>=curNeuronBDyb && z<=curNeuronBDze && z>=curNeuronBDzb)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//
void NeuronSelector::setAnnotationSession(AnnotationSession* annotationSession) {
	this->annotationSession=annotationSession;
}

// 
void NeuronSelector::updateSelectedPosition(double x, double y, double z) {
        xlc = x + 0.5;
        ylc = y + 0.5;
        zlc = z + 0.5;
	
	qDebug()<<"test signal/slot passing parameters ..."<<xlc<<ylc<<zlc;
	
	//
	highlightSelectedNeuron();
}

// highlight selected neuron
void NeuronSelector::highlightSelectedNeuron()
{
    getCurNeuronBoundary();
	
	if(index<=0) return;
	if(curNeuronBDxb>curNeuronBDxe || curNeuronBDyb>curNeuronBDye || curNeuronBDzb>curNeuronBDze) return;

	//
	// index neuron selected status is true
	
	// list of markers
	QList<LocationSimple> listLandmarks;
	
    // mesh grids
	for(V3DLONG k=0; k<sz; k+=STEP)
	{
		//V3DLONG offset_k = k*sx*sy;
		for(V3DLONG j=0; j<sy; j+=STEP)
		{
			//V3DLONG offset_j = offset_k + j*sx;
			for(V3DLONG i=0; i<sx; i+=STEP)
			{
				//V3DLONG idx = offset_j + i; // lattice point
				
				if(inNeuronMask(i,j,k))
				{
					// find avg position in cube around lattice point
					float sumx, sumy, sumz;
					V3DLONG count;
					sumx = 0.0;
					sumy = 0.0;
					sumz = 0.0;
					count = 0;
					
					//
					V3DLONG xb = i-NB; if(xb<0) xb = 0;
					V3DLONG xe = i+NB; if(xe>sx) xe = sx-1;
					V3DLONG yb = j-NB; if(yb<0) yb = 0;
					V3DLONG ye = j+NB; if(ye>sy) ye = sy-1;
					V3DLONG zb = k-NB; if(zb<0) zb = 0;
					V3DLONG ze = k+NB; if(ze>sz) ze = sz-1;
					
					for(V3DLONG kk=zb; kk<=ze; kk++)
					{
						V3DLONG offset_kk = kk*sx*sy;
						for(V3DLONG jj=yb; jj<=ye; jj++)
						{
							V3DLONG offset_jj = offset_kk + jj*sx;
							for(V3DLONG ii=xb; ii<=xe; ii++)
							{
								V3DLONG idx = offset_jj + ii;
								
								if(annotationSession->getNeuronMaskAsMy4DImage()->getRawData()[idx] == index)
								{
									count++;
									sumx += ii;
									sumy += jj;
									sumz += kk;
								}
							}
						}
					}

					// append a marker
					if(count>0)
					{
						LocationSimple p((V3DLONG)(sumx/(float)count+1.5), (V3DLONG)(sumy/(float)count+1.5), (V3DLONG)(sumz/(float)count+1.5)); // 1-based
						RGBA8 c;
						c.r = 0; c.g = 255; c.b = 255; c.a = 128;// cyan
						p.color = c; // instead of random_rgba8(255);
						p.radius = 1; // instead of 5
						
						listLandmarks.append(p);
					}
				}
			}
		}
	}
	
	// highlight result
	annotationSession->getOriginalImageStackAsMy4DImage()->listLandmarks = listLandmarks;
	
	qDebug()<<"highlight selected neuron ..."<<listLandmarks.size();
	
	// synchronize markers shown in 3d viewer
	emit neuronHighlighted(false);

}


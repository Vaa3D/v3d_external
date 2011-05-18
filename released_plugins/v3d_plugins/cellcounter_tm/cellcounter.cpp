/* cellcounter.cpp
 * 2010-01-14: the program is created by Yang Yu
 */

#include <QtGui>

#include <cmath>
#include <stdlib.h>
#include <ctime>

#include <vector>

#include <sstream>
#include <iostream>

#include "cellcounter.h"

#include "../../../v3d_main/basic_c_fun/basic_surf_objs.h"
#include "../../../v3d_main/basic_c_fun/stackutil.h"
#include "../../../v3d_main/basic_c_fun/volimg_proc.h"
#include "../../../v3d_main/basic_c_fun/img_definition.h"
#include "../../../v3d_main/basic_c_fun/basic_landmark.h"

#define INF 1E9
#define PI 3.14159265

//Defining Puncta structure
struct CellStr
{
	double x, y, z;
	double rx, ry, rz;
	double volsize;
	double intensity;
	double sdev, pixmax, mass;
	long num;
};

//Gaussian kernel and correlation compuating copy from ../cellseg/template_matching_seg.cpp
Vol3DSimple<double> * genGaussianKernal3D(long szx, long szy, long szz, double sigmax, double sigmay, double sigmaz)
{
	if (szx<=0 || szy<=0 || szz<=0) {printf("Invalid sz parameter in genGaussianKernal3D().\n"); return 0;}
	
	Vol3DSimple<double> * g = 0;
	try 
	{
		g = new Vol3DSimple<double> (szx, szy, szz);
	}
	catch (...) 
	{
		printf("Fail to create a kernel object.\n");
		return 0;
	}
	
	double *** d3 = g->getData3dHandle();
	double sx2 = 2.0*sigmax*sigmax, sy2=2.0*sigmay*sigmay, sz2=2.0*sigmaz*sigmaz;
	long cx=(szx-1)>>1, cy=(szy-1)>>1, cz=(szz-1)>>1;
	long i,j,k;
	for (k=0;k<=cz;k++)
		for (j=0;j<=cy;j++)
			for (i=0;i<=cx;i++)
			{
				d3[szz-1-k][szy-1-j][szx-1-i] = d3[szz-1-k][szy-1-j][i] = 
				d3[szz-1-k][j][szx-1-i] = d3[szz-1-k][j][i] = 
				d3[k][szy-1-j][szx-1-i] = d3[k][szy-1-j][i] = 
				d3[k][j][szx-1-i] = d3[k][j][i] = 
				exp(-double(i-cx)*(i-cx)/sx2-double(j-cy)*(j-cy)/sy2-double(k-cz)*(k-cz)/sz2);
			}
	return g;
}
template <class T1, class T2> double compute_corrcoef_two_vectors(T1 *v1, T2 *v2, long len)
{
	if (!v1 || !v2 || len<=1) return 0;
	
	//first compute mean
	double m1=0,m2=0;
	long i;
	for (i=0;i<len;i++)
	{
		m1+=v1[i];
		m2+=v2[i];
	}
	m1/=len;
	m2/=len;
	
	//now compute corrcoef
	double tmp_s=0, tmp_s1=0, tmp_s2=0, tmp1, tmp2;
	for (i=0;i<len;i++)
	{
		tmp1 = v1[i]-m1;
		tmp2 = v2[i]-m2;
		tmp_s += tmp1*tmp2;
		tmp_s1 += tmp1*tmp1;
		tmp_s2 += tmp2*tmp2;
	}
	
	//the final score
	double s;
	s = (tmp_s / sqrt(tmp_s1) / sqrt(tmp_s2) + 1 )/2;
	return s;
}

// down sampling
void down_sampling(unsigned char *pOut, unsigned char *pIn, long ssx, long ssy, long ssz, long N, long M, long P, double scale_x, double scale_y, double scale_z)
{
	
	//linear interpolation
	for (long k=0;k<ssz;k++)
	{
		long kstart=long(floor(k/scale_z)), kend=kstart+long(1/scale_z);
		if (kend>P-1) kend = P-1;
		
		if(P==1) kend=1;
		
		for (long j=0;j<ssy;j++)
		{
			long jstart=long(floor(j/scale_y)), jend=jstart+long(1/scale_y);
			if (jend>M-1) jend = M-1;
			
			if(M==1) jend=1;
			
			for (long i=0;i<ssx;i++)
			{
				long istart=long(floor(i/scale_x)), iend=istart+long(1/scale_x);
				if (iend>N-1) iend = N-1;
				
				if(N==1) iend=1;
				
				long idx_out = k*ssx*ssy + j*ssx + i;
				
				long sum=0;
				
				for(long kk=kstart; kk<kend; kk++)
				{
					for(long jj=jstart; jj<jend; jj++)
					{
						for(long ii=istart; ii<iend; ii++)
						{
							sum += pIn[kk*M*N + jj*N + ii];
						}
						
					}
				}
				
				if(iend==istart)
					pOut[idx_out] = pIn[k*M*N + j*N + istart];
				else if(jend==jstart)
					pOut[idx_out] = pIn[k*M*N + jstart*N + i];
				else if(kend==kstart)
					pOut[idx_out] = pIn[kstart*M*N + j*N + i];
				else
					pOut[idx_out] =  sum / ( (kend-kstart)*(jend-jstart)*(iend-istart) );
				
			}
		}
	}
	
}

// gaussian filtering
void gaussianfiltering(unsigned char *data1d, long sx, long sy, long sz, int Wx, int Wy, int Wz)
{
	//filtering 
	long N=sx, M=sy, P=sz, pagesz=N*M*P;
	double max_val=0, min_val=INF;

	//declare temporary pointer
	float *pImage = new float [pagesz];
	if (!pImage)
	{
		printf("Fail to allocate memory.\n");
		return;
	}
	else
	{
		for(long i=0; i<pagesz; i++)
			pImage[i] = data1d[i];  //first channel data (red in V3D, green in ImageJ)
	}
	
	//Filtering
	//
	//   Filtering along x
	if(N<2)
	{
		//do nothing
	}
	else
	{
		//create Gaussian kernel
		float  *WeightsX = 0;
		WeightsX = new float [Wx];
		if (!WeightsX)
			return;
		
		unsigned int Half = Wx >> 1;
		WeightsX[Half] = 1.;
		
		for (unsigned int Weight = 1; Weight < Half + 1; ++Weight)
		{
			const float  x = 3.* float (Weight) / float (Half);
			WeightsX[Half - Weight] = WeightsX[Half + Weight] = exp(-x * x / 2.);	// Corresponding symmetric WeightsX
		}
		
		double k = 0.;
		for (unsigned int Weight = 0; Weight < Wx; ++Weight)
			k += WeightsX[Weight];
		
		for (unsigned int Weight = 0; Weight < Wx; ++Weight)
			WeightsX[Weight] /= k;
		
		
		//   Allocate 1-D extension array
		float  *extension_bufferX = 0;
		extension_bufferX = new float [N + (Wx<<1)];
		
		unsigned int offset = Wx>>1;
		
		//	along x
		const float  *extStop = extension_bufferX + N + offset;
		
		for(long iz = 0; iz < P; iz++)
		{
			for(long iy = 0; iy < M; iy++)
			{
				float  *extIter = extension_bufferX + Wx;
				for(long ix = 0; ix < N; ix++)
				{
					*(extIter++) = pImage[iz*M*N + iy*N + ix];
				}
				
				//   Extend image
				const float  *const stop_line = extension_bufferX - 1;
				float  *extLeft = extension_bufferX + Wx - 1;
				const float  *arrLeft = extLeft + 2;
				float  *extRight = extLeft + N + 1;
				const float  *arrRight = extRight - 2;
				
				while (extLeft > stop_line)
				{
					*(extLeft--) = *(arrLeft++);
					*(extRight++) = *(arrRight--);
				}
				
				//	Filtering
				extIter = extension_bufferX + offset;
				
				float  *resIter = &(pImage[iz*M*N + iy*N]);
				
				while (extIter < extStop)
				{
					double sum = 0.;
					const float  *weightIter = WeightsX;
					const float  *const End = WeightsX + Wx;
					const float * arrIter = extIter;
					while (weightIter < End)
						sum += *(weightIter++) * float (*(arrIter++));
					extIter++;
					*(resIter++) = sum;
					
					//for rescale
					if(max_val<*arrIter) max_val = *arrIter;
					if(min_val>*arrIter) min_val = *arrIter;
				}
			}
		}
		
		//de-alloc
		if (WeightsX) {delete []WeightsX; WeightsX=0;}
		if (extension_bufferX) {delete []extension_bufferX; extension_bufferX=0;}
		
	}
	
	//   Filtering along y
	if(M<2)
	{
		//do nothing
	}
	else
	{
		//create Gaussian kernel
		float  *WeightsY = 0;
		WeightsY = new float [Wy];
		if (!WeightsY)
			return;
		
		unsigned int Half = Wy >> 1;
		WeightsY[Half] = 1.;
		
		for (unsigned int Weight = 1; Weight < Half + 1; ++Weight)
		{
			const float  y = 3.* float (Weight) / float (Half);
			WeightsY[Half - Weight] = WeightsY[Half + Weight] = exp(-y * y / 2.);	// Corresponding symmetric WeightsY
		}
		
		double k = 0.;
		for (unsigned int Weight = 0; Weight < Wy; ++Weight)
			k += WeightsY[Weight];
		
		for (unsigned int Weight = 0; Weight < Wy; ++Weight)
			WeightsY[Weight] /= k;
		
		//	along y
		float  *extension_bufferY = 0;
		extension_bufferY = new float [M + (Wy<<1)];
		
		unsigned int offset = Wy>>1;
		const float *extStop = extension_bufferY + M + offset;
		
		for(long iz = 0; iz < P; iz++)
		{
			for(long ix = 0; ix < N; ix++)
			{
				float  *extIter = extension_bufferY + Wy;
				for(long iy = 0; iy < M; iy++)
				{
					*(extIter++) = pImage[iz*M*N + iy*N + ix];
				}
				
				//   Extend image
				const float  *const stop_line = extension_bufferY - 1;
				float  *extLeft = extension_bufferY + Wy - 1;
				const float  *arrLeft = extLeft + 2;
				float  *extRight = extLeft + M + 1;
				const float  *arrRight = extRight - 2;
				
				while (extLeft > stop_line)
				{
					*(extLeft--) = *(arrLeft++);
					*(extRight++) = *(arrRight--);
				}
				
				//	Filtering
				extIter = extension_bufferY + offset;
				
				float  *resIter = &(pImage[iz*M*N + ix]);
				
				while (extIter < extStop)
				{
					double sum = 0.;
					const float  *weightIter = WeightsY;
					const float  *const End = WeightsY + Wy;
					const float * arrIter = extIter;
					while (weightIter < End)
						sum += *(weightIter++) * float (*(arrIter++));
					extIter++;
					*resIter = sum;
					resIter += N;
					
					//for rescale
					if(max_val<*arrIter) max_val = *arrIter;
					if(min_val>*arrIter) min_val = *arrIter;
				}
			}
		}
		
		//de-alloc
		if (WeightsY) {delete []WeightsY; WeightsY=0;}
		if (extension_bufferY) {delete []extension_bufferY; extension_bufferY=0;}
	}
	
	//  Filtering  along z
	if(P<2)
	{
		//do nothing
	}
	else
	{
		//create Gaussian kernel
		float  *WeightsZ = 0;
		WeightsZ = new float [Wz];
		if (!WeightsZ)
			return;
		
		unsigned int Half = Wz >> 1;
		WeightsZ[Half] = 1.;
		
		for (unsigned int Weight = 1; Weight < Half + 1; ++Weight)
		{
			const float  z = 3.* float (Weight) / float (Half);
			WeightsZ[Half - Weight] = WeightsZ[Half + Weight] = exp(-z * z / 2.);	// Corresponding symmetric WeightsZ
		}
		
		double k = 0.;
		for (unsigned int Weight = 0; Weight < Wz; ++Weight)
			k += WeightsZ[Weight];
		
		for (unsigned int Weight = 0; Weight < Wz; ++Weight)
			WeightsZ[Weight] /= k;
		
		
		//	along z
		float  *extension_bufferZ = 0;
		extension_bufferZ = new float [P + (Wz<<1)];
		
		unsigned int offset = Wz>>1;
		const float *extStop = extension_bufferZ + P + offset;
		
		for(long iy = 0; iy < M; iy++)
		{
			for(long ix = 0; ix < N; ix++)
			{
				
				float  *extIter = extension_bufferZ + Wz;
				for(long iz = 0; iz < P; iz++)
				{
					*(extIter++) = pImage[iz*M*N + iy*N + ix];
				}
				
				//   Extend image
				const float  *const stop_line = extension_bufferZ - 1;
				float  *extLeft = extension_bufferZ + Wz - 1;
				const float  *arrLeft = extLeft + 2;
				float  *extRight = extLeft + P + 1;
				const float  *arrRight = extRight - 2;
				
				while (extLeft > stop_line)
				{
					*(extLeft--) = *(arrLeft++);
					*(extRight++) = *(arrRight--);
				}
				
				//	Filtering
				extIter = extension_bufferZ + offset;
				
				float  *resIter = &(pImage[iy*N + ix]);
				
				while (extIter < extStop)
				{
					double sum = 0.;
					const float  *weightIter = WeightsZ;
					const float  *const End = WeightsZ + Wz;
					const float * arrIter = extIter;
					while (weightIter < End)
						sum += *(weightIter++) * float (*(arrIter++));
					extIter++;
					*resIter = sum;
					resIter += M*N;
					
					//for rescale
					if(max_val<*arrIter) max_val = *arrIter;
					if(min_val>*arrIter) min_val = *arrIter;
				}
				
			}
		}
		
		//de-alloc
		if (WeightsZ) {delete []WeightsZ; WeightsZ=0;}
		if (extension_bufferZ) {delete []extension_bufferZ; extension_bufferZ=0;}
		
	}
	
	//rescaling for display
	float dist = max_val - min_val;
	for(long k=0; k<P; k++)
	{
		long offsetk = k*M*N;
		for(long j=0; j<M; j++)
		{
			long offsetj = j*N;
			for(long i=0; i<N; i++)
			{
				long indLoop = offsetk + offsetj + i;
				
				data1d[indLoop] = 255*(pImage[indLoop]-min_val)/(dist);
			}
		}
	}
	
	//de-alloc
	if (pImage) {delete []pImage; pImage=0;}
	
}

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(cellcounter, CellCounterPlugin);

void cellcounting(V3DPluginCallback &callback, QWidget *parent);

//plugin funcs
const QString title = "Cell Counter";
QStringList CellCounterPlugin::menulist() const
{
    return QStringList() << tr("Cell Counter");
}

void CellCounterPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
    if (menu_name == tr("Cell Counter"))
    {
    	cellcounting(callback, parent);
    }
}

void cellcounting(V3DPluginCallback &callback, QWidget *parent)
{
    v3dhandleList win_list = callback.getImageWindowList();
	
	if(win_list.size()<1) 
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return;
	}
	
	CellCounterDialog dialog(callback, parent);
	if (dialog.exec()!=QDialog::Accepted)
		return;
	
	int i1 = dialog.combo_subject->currentIndex();
	
	Image4DSimple* subject = callback.getImage(win_list[i1]);
	ROIList pRoiList=callback.getROI(win_list[i1]);
	
	QString m_InputFileName = callback.getImageName(win_list[i1]);
	
	qDebug()<<"datatype ..."<<subject->getDatatype();
	
	if (!subject)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return;
	}
	if (subject->getDatatype()!=V3D_UINT8)
	{
		QMessageBox::information(0, title, QObject::tr("This demo program only supports 8-bit data. Your current image data type is not supported."));
		return;
	}
	
    unsigned char* pSbject = subject->getRawData();
	
	long sz0 = subject->getXDim();
    long sz1 = subject->getYDim();
    long sz2 = subject->getZDim();
	long sz3 = subject->getCDim();
	
	long pagesz_sub = sz0*sz1*sz2;
	
	
	//finding the bounding box of ROI
	bool vxy=true,vyz=true,vzx=true; // 3 2d-views
	
	QRect b_xy = pRoiList.at(0).boundingRect();
	QRect b_yz = pRoiList.at(1).boundingRect();
	QRect b_zx = pRoiList.at(2).boundingRect();
	
	if(b_xy.left()==-1 || b_xy.top()==-1 || b_xy.right()==-1 || b_xy.bottom()==-1)
		vxy=false;
	if(b_yz.left()==-1 || b_yz.top()==-1 || b_yz.right()==-1 || b_yz.bottom()==-1)
		vyz=false;
	if(b_zx.left()==-1 || b_zx.top()==-1 || b_zx.right()==-1 || b_zx.bottom()==-1)
		vzx=false;
		
	long bpos_x, bpos_y, bpos_z, bpos_c, epos_x, epos_y, epos_z, epos_c;
	
	// 8 cases
	if(vxy && vyz && vzx) // all 3 2d-views
	{
		bpos_x = qBound(long(0), long(qMax(b_xy.left(), b_zx.left())), sz0-1);
		bpos_y = qBound(long(0), long(qMax(b_xy.top(),  b_yz.top())), sz1-1);
		bpos_z = qBound(long(0), long(qMax(b_yz.left(), b_zx.top())), sz2-1);
		
		epos_x = qBound(long(0), long(qMin(b_xy.right(), b_zx.right())), sz0-1);
		epos_y = qBound(long(0), long(qMin(b_xy.bottom(), b_yz.bottom())), sz1-1);
		epos_z = qBound(long(0), long(qMin(b_yz.right(), b_zx.bottom())), sz2-1);
	}
	else if(!vxy && vyz && vzx) // 2 of 3
	{
		bpos_x = qBound(long(0), long(qMax(0, b_zx.left())), sz0-1);
		bpos_y = qBound(long(0), long(qMax(0,  b_yz.top())), sz1-1);
		bpos_z = qBound(long(0), long(qMax(b_yz.left(), b_zx.top())), sz2-1);
		
		epos_x = qBound(long(0), long(fmin(sz0-1, b_zx.right())), sz0-1);
		epos_y = qBound(long(0), long(fmin(sz1-1, b_yz.bottom())), sz1-1);
		epos_z = qBound(long(0), long(qMin(b_yz.right(), b_zx.bottom())), sz2-1);
	}
	else if(vxy && !vyz && vzx)
	{
		bpos_x = qBound(long(0), long(qMax(b_xy.left(), b_zx.left())), sz0-1);
		bpos_y = qBound(long(0), long(qMax(b_xy.top(),  0)), sz1-1);
		bpos_z = qBound(long(0), long(qMax(0, b_zx.top())), sz2-1);
		
		epos_x = qBound(long(0), long(qMin(b_xy.right(), b_zx.right())), sz0-1);
		epos_y = qBound(long(0), long(fmin(b_xy.bottom(), sz1-1)), sz1-1);
		epos_z = qBound(long(0), long(fmin(sz2-1, b_zx.bottom())), sz2-1);
	}
	else if(vxy && vyz && !vzx)
	{
		bpos_x = qBound(long(0), long(qMax(b_xy.left(), 0)), sz0-1);
		bpos_y = qBound(long(0), long(qMax(b_xy.top(),  b_yz.top())), sz1-1);
		bpos_z = qBound(long(0), long(qMax(b_yz.left(), 0)), sz2-1);
		
		epos_x = qBound(long(0), long(fmin(b_xy.right(), sz0-1)), sz0-1);
		epos_y = qBound(long(0), long(qMin(b_xy.bottom(), b_yz.bottom())), sz1-1);
		epos_z = qBound(long(0), long(fmin(b_yz.right(), sz2-1)), sz2-1);
	}
	else if(vxy && !vyz && !vzx) // only 1 of 3
	{
		bpos_x = qBound(long(0), long(qMax(b_xy.left(), 0)), sz0-1);
		bpos_y = qBound(long(0), long(qMax(b_xy.top(),  0)), sz1-1);
		bpos_z = 0;
		
		epos_x = qBound(long(0), long(fmin(b_xy.right(), sz0-1)), sz0-1);
		epos_y = qBound(long(0), long(fmin(b_xy.bottom(), sz1-1)), sz1-1);
		epos_z = sz2-1;
	}
	else if(!vxy && vyz && !vzx)
	{
		bpos_x = 0;
		bpos_y = qBound(long(0), long(qMax(0,  b_yz.top())), sz1-1);
		bpos_z = qBound(long(0), long(qMax(b_yz.left(), 0)), sz2-1);
		
		epos_x = sz0-1;
		epos_y = qBound(long(0), long(fmin(sz1-1, b_yz.bottom())), sz1-1);
		epos_z = qBound(long(0), long(fmin(b_yz.right(), sz2-1)), sz2-1);
	}
	else if(!vxy && !vyz && vzx)
	{
		bpos_x = qBound(long(0), long(qMax(0, b_zx.left())), sz0-1);
		bpos_y = 0;
		bpos_z = qBound(long(0), long(qMax(0, b_zx.top())), sz2-1);
		
		epos_x = qBound(long(0), long(fmin(sz0-1, b_zx.right())), sz0-1);
		epos_y = sz1-1;
		epos_z = qBound(long(0), long(fmin(sz2-1, b_zx.bottom())), sz2-1);
	}
	else // 0
	{
		bpos_x = 0;
		bpos_y = 0;
		bpos_z = 0;
		
		epos_x = sz0-1;
		epos_y = sz1-1;
		epos_z = sz2-1;
	}

	//qDebug("x %d y %d z %d x %d y %d z %d ",bpos_x,bpos_y,bpos_z,epos_x,epos_y,epos_z);

	//ROI extraction
	long sx = (epos_x-bpos_x)+1;
    long sy = (epos_y-bpos_y)+1;
    long sz = (epos_z-bpos_z)+1;
	long sc = 3; // 0,1,2
	
	//choose the channel stack
	long pagesz = sx*sy*sz;
	
	double meanv=0;
	
	// mmparameters
	//-------------------------------------------------------------------------------------------------------------------------------
	double gthresh = 4, Lthresh =30, lthresh = 5; //global and local thresholding        ///////////////////////////////////////
	double delta_th = 10;
	double rx = 2, ry = 2, rz = 1;                        ////////////////////////////////////////////////////
	double sigmax=1.5, sigmay=1.5, sigmaz=0.5;               ///////////////////////////////////////////////////////
	double t_corrcoef=0.66;//=0.3; //0.66           ////////////////////////////////////////////////
	double shift_thresh = 10;
	
	long ch = 0; ////////////////////////////////////////
	
	int start_t = clock();
	
	dialog.update();
	
	rx = dialog.r_x;
	ry = dialog.r_y;
	rz = dialog.r_z;
	
	//qDebug("rx %lf ry %lf rz %lf ", rx, ry, rz);
	
	ch = dialog.ch_rgb; 
	
	sigmax = dialog.sigma_x;
	sigmay = dialog.sigma_y;
	sigmaz = dialog.sigma_z;
	
	t_corrcoef = dialog.correlation_coeff;
	
	gthresh = dialog.global_th;
	
	Lthresh = dialog.high_th;
	lthresh = dialog.low_th;
	
	//qDebug("Lthresh %lf lthresh %lf", Lthresh, lthresh);
	
	delta_th = dialog.delta_th;
	
	long offset_sub = ch*pagesz_sub;
	
	//qDebug("%ld %ld", ch, offset_sub);
	//------------------------------------------------------------------------------------------------------------------------------------
	// scans time
	int i_progress=0;
	int num_progress = int((Lthresh-lthresh)/delta_th) + 8; // 5 preprocess
	
	//
	QProgressDialog progress("Counting cell ...", "Abort Cell Counter", 0, num_progress, parent);
	progress.setWindowModality(Qt::WindowModal);
	
	// a scan
	progress.setValue(++i_progress); 
	
	unsigned char *data1d = new unsigned char [sc*pagesz];
	if (!data1d) 
	{
		printf("Fail to allocate memory.\n");
		return;
	}
	else
	{
		for(long k=bpos_z; k<=epos_z; k++)
		{
			long offset_z = k*sz0*sz1;
			long offset_crop_z = (k-bpos_z)*sx*sy;
			for(long j=bpos_y; j<=epos_y; j++)
			{
				long offset_y = j*sz0 + offset_z;
				long offset_crop_y = (j-bpos_y)*sx + offset_crop_z;
				for(long i=bpos_x; i<=epos_x; i++)
				{
					data1d[(i-bpos_x) + offset_crop_y] = pSbject[offset_sub + i+offset_y];
					
					meanv += data1d[(i-bpos_x) + offset_crop_y];
					
					if(sz3>1)
						data1d[(i-bpos_x) + offset_crop_y + pagesz] = pSbject[i+offset_y+pagesz_sub];
					else
						data1d[(i-bpos_x) + offset_crop_y + pagesz] = 0;
					
					data1d[(i-bpos_x) + offset_crop_y + 2*pagesz] = 0; //pSbject[i+offset_y+2*pagesz_sub];
				}
			}
		}
	}
	meanv /= pagesz;
	
	qDebug("mean value %lf", meanv);
	
	// a scan
	progress.setValue(++i_progress); 
	
	// down sampling
	double scale_x = dialog.scale_x;
	double scale_y = dialog.scale_y;
	double scale_z = dialog.scale_z;
	
	long ds_sx = sx*scale_x;
	long ds_sy = sy*scale_y;
	long ds_sz = sz*scale_z;
	
	long ds_pagesz = ds_sx*ds_sy*ds_sz;
	
	unsigned char *ds_subject1d = new unsigned char [ds_pagesz];
	if (!ds_subject1d) 
	{
		printf("Fail to allocate memory.\n");
		return;
	}
	
	down_sampling(ds_subject1d, data1d, ds_sx, ds_sy, ds_sz, sx, sy, sz, scale_x, scale_y, scale_z);
	
	// de-alloc
	if (data1d) {delete []data1d; data1d=0;}
	
	// --
	unsigned char *subject1d = ds_subject1d;
	sx = ds_sx;
	sy = ds_sy;
	sz = ds_sz;
	sc = 1;
	
	// a scan
	progress.setValue(++i_progress); 
	
	// gaussian filtering
	int w_x = dialog.w_x;
	int w_y = dialog.w_y;
	int w_z = dialog.w_z;
	
	gaussianfiltering(subject1d, sx, sy, sz, w_x, w_y, w_z);
	
	// display
	Image4DSimple p4DGF;
	p4DGF.setData((unsigned char*)subject1d, sx, sy, sz, sc, V3D_UINT8);
	
	v3dhandle newGF = callback.newImageWindow();
	callback.setImage(newGF, &p4DGF);
	callback.setImageName(newGF, "DownSampled_GaussFiltered");
	callback.updateImageWindow(newGF);
	
	// a scan
	progress.setValue(++i_progress); 
	
	// pre processing for template matching
	
	//local maxima
	unsigned char *flag_lm = new unsigned char [pagesz];
	if (!flag_lm) 
	{
		printf("Fail to allocate memory.\n");
		return;
	}
	else
	{
		//max filter
		double maxfl = 0, minfl = INF;
		unsigned int Wx=3, Wy=3, Wz=3;
		
		for(long iz = 0; iz < sz; iz++)
		{
			long offsetk = iz*sx*sy;
			for(long iy = 0; iy < sy; iy++)
			{
				long offsetj = iy*sx;
				for(long ix = 0; ix < sx; ix++)
				{
					maxfl = 0; //minfl = INF;
					
					long xb = ix-Wx; if(xb<0) xb = 0;
					long xe = ix+Wx; if(xe>=sx-1) xe = sx-1;
					long yb = iy-Wy; if(yb<0) yb = 0;
					long ye = iy+Wy; if(ye>=sy-1) ye = sy-1;
					long zb = iz-Wz; if(zb<0) zb = 0;
					long ze = iz+Wz; if(ze>=sz-1) ze = sz-1;
					
					for(long k=zb; k<=ze; k++)
					{
						long offsetkl = k*sx*sy;
						for(long j=yb; j<=ye; j++)
						{
							long offsetjl = j*sx;
							for(long i=xb; i<=xe; i++)
							{
								long dataval = subject1d[ offsetkl + offsetjl + i];
								
								if(maxfl<dataval) maxfl = dataval;
								//if(minfl>dataval) minfl = dataval;
							}
						}
					}
					
					//set value
					flag_lm[offsetk + offsetj + ix] = maxfl;
				}
			}
		}
	}

	// a scan
	progress.setValue(++i_progress); 
	
	for(long iz = 0; iz < sz; iz++)
	{
		long offsetk = iz*sx*sy;
		for(long iy = 0; iy < sy; iy++)
		{
			long offsetj = iy*sx;
			for(long ix = 0; ix < sx; ix++)
			{
				long idx = offsetk + offsetj + ix;
				
				if( (flag_lm[idx] == subject1d[idx]) && subject1d[idx]>meanv )
					flag_lm[idx] = 255;
				else
					flag_lm[idx] = 0;
			}
		}
	}

	//template matching
	//-------------------------------------------------------------------------------------------------------------------------------
	
	Vol3DSimple <unsigned char> *inimg3d = new Vol3DSimple <unsigned char> (sx, sy, sz); 
	unsigned char ***img3d = inimg3d->getData3dHandle();
	
	Vol3DSimple <unsigned short int> *outimg3d = new Vol3DSimple <unsigned short int> (sx, sy, sz); 
	unsigned short int ***labelimgdata3d = outimg3d->getData3dHandle();
	
	Vol3DSimple<unsigned char> * flagimg = new Vol3DSimple<unsigned char> (inimg3d);
	if (!flagimg) return;
	
	// a scan
	progress.setValue(++i_progress); 
	
	//initialization
	long i,j,k;
	unsigned char *** flag_p3d = flagimg->getData3dHandle();
	long cellcnt=0;
	
	for(long iz = 0; iz < sz; iz++)
	{
		long offset_z = iz*sx*sy;
		for(long iy =0; iy < sy; iy++)
		{
			long offset_y = offset_z + iy*sx;
			for(long ix = 0; ix < sx; ix++)
			{
				img3d[iz][iy][ix] = subject1d[offset_y+ix];
				flag_p3d[iz][iy][ix] = 0;
				labelimgdata3d[iz][iy][ix] = 0;
			}
		}
	}
	
	//Set Parameters		
	std::vector <CellStr> detectedList;
	CellStr pos;
	
	double t_pixval=gthresh, t_rgnval=lthresh;
	long wx=2*rx+1, wy=2*ry+1, wz=2*rz+1;
	
	//qDebug("thresholding %f \n", t_pixval);
	
	
	//computing
	Vol3DSimple<double> * g = genGaussianKernal3D(wx, wy, wz, sigmax, sigmay, sigmaz);
	if (!g) return;
	Vol3DSimple<unsigned char> * d = new Vol3DSimple<unsigned char> (wx, wy, wz);
	if (!d) return;
	long kernel_len = g->getTotalElementNumber();
	
	double * g_1d = g->getData1dHandle();
	unsigned char * d_1d = d->getData1dHandle();
	unsigned char *** d_3d = d->getData3dHandle();
	
	std::vector <LocationSimple> detectedPos;
	
	for(t_rgnval = Lthresh; t_rgnval >=lthresh; 	t_rgnval-=delta_th)
	{

		//display the progress
		//printf("t_rgnval %f \n", t_rgnval);
		progress.setValue(++i_progress);
			
		if (progress.wasCanceled())
			break;
		
		
		for (k=0;k<sz;k++)
		{
			if (k<rz || k>(sz-1-rz)) //if at the border, then skip
				continue; 
			
			for (j=0;j<sy;j++)
			{
				if (j<ry || j>(sy-1-ry)) //if at the border, then skip
					continue; 
				
				for (i=0;i<sx;i++)
				{
					if (i<rx || i>(sx-1-rx)) //if at the border, then skip
						continue; 
					
					flag_p3d[k][j][i]=0;
				}
			}
		}
		
		
		for (k=0;k<sz;k++)
		{
			if (k<rz || k>(sz-1-rz)) //if at the border, then skip
				continue; 
			
			for (j=0;j<sy;j++)
			{
				if (j<ry || j>(sy-1-ry)) //if at the border, then skip
					continue; 
				
				for (i=0;i<sx;i++)
				{
					if (i<rx || i>(sx-1-rx)) //if at the border, then skip
						continue; 
					
					if (flag_p3d[k][j][i]) //if the location has been masked, then skip
						continue;
					
					if (img3d[k][j][i]<=t_pixval)//|| (img_subject + pagesz)[k*sx*sy+j*sx+i]<2*mean_green) //|| pImMask[k*sx*sy+j*sx+i]==0) //do not compute dark pixels and non-ROI region
					{	
						flag_p3d[k][j][i]=1;
						continue;
					}
					
					if(labelimgdata3d[k][j][i])
						continue;
					
					if(!flag_lm[k*sx*sy + j*sx +i])
						continue;
					
					bool b_skip=false;
					//copy data
					long i1,j1,k1, i2,j2,k2;
					for (k1=k-rz,k2=0;k1<=k+rz;k1++,k2++)
						for (j1=j-ry,j2=0;j1<=j+ry;j1++,j2++)
							for (i1=i-rx,i2=0;i1<=i+rx;i1++,i2++)
							{
								d_3d[k2][j2][i2] = img3d[k1][j1][i1];
								if (labelimgdata3d[k1][j1][i1])
									b_skip=true;
							}	
					
					if (b_skip==true)
						continue;
					
					//test regional mean
					unsigned char d_mean, d_std;
					mean_and_std(d_1d, kernel_len, d_mean, d_std);
					if (d_mean<1.5*t_rgnval)
					{	
						flag_p3d[k][j][i]=1;
						continue;
					}
					
					//compute similarity measure using cross correlation
					double score = compute_corrcoef_two_vectors(d_1d, g_1d, kernel_len);
					flag_p3d[k][j][i] = 1; //do not search later
					double rx_ms, ry_ms, rz_ms; 
					double ncx=i,ncy=j,ncz=k; //new center position
					double ocx,ocy,ocz; //old center position
					if (score>=t_corrcoef)
					{
						//first re-estimate the center
						double scx=0,scy=0,scz=0,si=0;
						while (1) //mean shift to estimate the true center
						{
							ocx=ncx; ocy=ncy; ocz=ncz;
							
							rx_ms = 1.2*rx + 0.5, ry_ms = 1.2*ry + 0.5, rz_ms = 1.2*rz + 0.5; //enlarge the radius
							
							for (k1=ocz-rz_ms;k1<=ocz+rz_ms;k1++)
							{	
								if (k1<0 || k1>=sz)
									continue;
								for (j1=ocy-ry_ms;j1<=ocy+ry_ms;j1++)
								{
									if (j1<0 || j1>=sy)
										continue;
									for (i1=ocx-rx_ms;i1<=ocx+rx_ms;i1++)
									{
										if (i1<0 || i1>=sx)
											continue;
										double cv = img3d[k1][j1][i1];
										
										if(cv<shift_thresh) cv=0; /////////////////////////////////////////
										
										scz += k1*cv;
										scy += j1*cv;
										scx += i1*cv;
										si += cv;
									}
								}
							}
							if (si>0)
							{ncx = scx/si; ncy = scy/si; ncz = scz/si;}
							else
							{ncx = ocx; ncy = ocy; ncz = ocz;}
							
							if (ncx<rx || ncx>=sx-1-rx || ncy<ry || ncy>=sy-1-ry || ncz<rz || ncz>=sz-1-rz) //move out of boundary
							{
								ncx = ocx; ncy = ocy; ncz = ocz; //use the last valid center
								break;
							}
							
							if (sqrt((ncx-ocx)*(ncx-ocx)+(ncy-ocy)*(ncy-ocy)+(ncz-ocz)*(ncz-ocz))<=1)
							{
								break;
							}
						}
						
						double lncx=ncx,lncy=ncy,lncz=ncz;
						
						scz=0; scy=0;scx=0;si=0;
						double stdx, stdy, stdz;

						for (k1=lncz-rz_ms;k1<=lncz+rz_ms;k1++)
						{	
							for (j1=lncy-ry_ms;j1<=lncy+ry_ms;j1++)
							{
								for (i1=lncx-rx_ms;i1<=lncx+rx_ms;i1++)
								{
									if(i1<0 || i1>sx-1 || j1 < 0 || j1 > sy-1 || k1 < 0 || k1 > sz-1)
										continue;
									else
									{
										double cv = img3d[k1][j1][i1];
										
										scz += cv*(k1-lncz)*(k1-lncz);
										scy += cv*(j1-lncy)*(j1-lncy);
										scx += cv*(i1-lncx)*(i1-lncx);
										si += cv;
									}
								}
							}
						}

						if (si>0)
						{
							stdx = sqrt(scx/si); stdy = sqrt(scy/si); stdz = sqrt(scz/si); 
							//printf("cell=%d, %5.3f (%5.3f) %5.3f (%5.3f) %5.3f (%5.3f)\n", cellcnt+1, lncx, stdx, lncy, stdy, lncz, stdz);
						}
						else
						{
							printf("Error happens in estimating the standard deviation in template_matching_seg(). Force setting std to be 1.\n");
							stdx = 1; stdy = 1; stdz = 1; 
						}
						
						//mask
						LocationSimple pp(lncx, lncy, lncz);
						detectedPos.push_back(pp);
						cellcnt++;
						long celllabel = cellcnt;

						//merging
						bool merg_flag=false;						
						
						double sa2b2c2 = 8*stdx*stdx*stdy*stdy*stdz*stdz;
						double sa2b2 = 4*stdx*stdx*stdy*stdy;
						double sb2c2 = 4*stdy*stdy*stdz*stdz;
						double sa2c2 = 4*stdx*stdx*stdz*stdz;
						
						//output
						pos.pixmax = 0;
						pos.intensity = 0;
						pos.sdev = 0;
						pos.volsize = 0;
						pos.mass = 0;
						
						for (k1=round(lncz-2*stdz);k1<=round(lncz+2*stdz);k1++)
						{
							for (j1=round(lncy-2*stdy);j1<=round(lncy+2*stdy);j1++)
							{
								for (i1=round(lncx-2*stdx);i1<=round(lncx+2*stdx);i1++)
								{	
									if (k1>=0 && k1<sz && j1>=0 && j1<sy && i1>=0 && i1<sx)
									{	
										if ((k1-lncz)*(k1-lncz)*sa2b2+(j1-lncy)*(j1-lncy)*sa2c2+(i1-lncx)*(i1-lncx)*sb2c2 <= sa2b2c2)
										{
											flag_p3d[k1][j1][i1] = 1;
											labelimgdata3d[k1][j1][i1] = celllabel;  //labelling
											
											double tmp_val = img3d[k1][j1][i1];
											
											if(pos.pixmax<tmp_val) pos.pixmax = tmp_val;
											pos.volsize ++;
											pos.mass += tmp_val;
											
										}
									}
								}
							}
						}
						//printf("%ld done \n", cellcnt);
						
						pos.intensity = pos.mass / pos.volsize;
						for (k1=round(lncz-2*stdz);k1<=round(lncz+2*stdz);k1++)
						{
							for (j1=round(lncy-2*stdy);j1<=round(lncy+2*stdy);j1++)
							{
								for (i1=round(lncx-2*stdx);i1<=round(lncx+2*stdx);i1++)
								{
									if (k1>=0 && k1<sz && j1>=0 && j1<sy && i1>=0 && i1<sx)
									{	
										if ((k1-lncz)*(k1-lncz)*sa2b2+(j1-lncy)*(j1-lncy)*sa2c2+(i1-lncx)*(i1-lncx)*sb2c2 <= sa2b2c2)
										{
											
											double tmp_val = img3d[k1][j1][i1];
											
											pos.sdev += (tmp_val-pos.intensity)*(tmp_val-pos.intensity);
										}
									}
								}
							}
						}
						
						if(!merg_flag)
						{
							pos.sdev = sqrt(pos.sdev);
							pos.x = lncx; pos.y = lncy; pos.z = lncz;
							pos.rx = stdx; pos.ry = stdy; pos.rz = stdz;
							pos.num = celllabel;

							detectedList.push_back(pos);
						}
					}
				}
			}
		}
	}
	
	//free space
	if (inimg3d) {delete inimg3d; inimg3d=0;}
	if (flagimg) {delete flagimg; flagimg=0;}
	if (d) {delete d; d=0;}
	if (g) {delete g; g=0;}
	
	int end_segmentation = clock();
	
	// a scan
	progress.setValue(++i_progress); 
	
	QString m_OutputFileName = m_InputFileName + QString(".x_%1_%2_y_%3_%4_z_%5_%6").arg(bpos_x).arg(epos_x).arg(bpos_y).arg(epos_y).arg(bpos_z).arg(epos_z) + ".apo";
	
	long LABELBIN = detectedList.size();
	
	//info
	QString sta_info;
	sta_info=QString("The number of cells in the ROI between (%2,%4,%6) and (%3,%5,%7) is %1.<br>").arg(LABELBIN).arg(bpos_x).arg(epos_x).arg(bpos_y).arg(epos_y).arg(bpos_z).arg(epos_z);
	
	QString tmp=QString("The corresponding .apo file is saved as %1<br>").arg(m_OutputFileName);
	sta_info.append(tmp);
	
	//save the output to a marker file
	m_OutputFileName = m_InputFileName + QString(".x_%1_%2_y_%3_%4_z_%5_%6").arg(bpos_x).arg(epos_x).arg(bpos_y).arg(epos_y).arg(bpos_z).arg(epos_z) + ".marker";
	
	FILE *pFileMarker=0;
	
	pFileMarker = fopen(m_OutputFileName.toStdString().c_str(),"wt");
	
	LandmarkList cellList;
	
	for(long ii = 0; ii < LABELBIN; ii++)
	{
		ImageMarker S;
		
		S.z = detectedList.at(ii).z + 0.5 + 1; //for display 0-base -> 1-base
		S.x = detectedList.at(ii).x + 0.5 + 1;
		S.y = detectedList.at(ii).y + 0.5 + 1;
		

		double vols = detectedList.at(ii).volsize;

		double r = pow( vols*3/4/PI, 1.0/3.0);
		
		S.radius = int(r+0.5);
		S.shape = 1;
		S.name = "";
		S.comment = "";

		S.color = random_rgba8(255);
		S.color.r = 128;
		S.color.g = 128;
		S.color.b = 128;
		
		S.type = (S.x==-1 || S.y==-1 || S.z==-1) ? 0 : 2;
		
		S.on = true;
		S.selected = false;
		
		fprintf(pFileMarker, "%5.3f, %5.3f, %5.3f, %d, %ld, %s, %s, %d,%d,%d\n",
				S.x,
				S.y,
				S.z,
				int(S.radius), S.shape,
				qPrintable(S.name), qPrintable(S.comment),
				S.color.r,S.color.g,S.color.b
				);
		
		LocationSimple landmark(S.x, S.y, S.z);
		cellList.push_back(landmark);
		
	}
	fclose(pFileMarker);
	
	callback.setImageName(newGF, "cell_counted");
	callback.setLandmark(newGF, cellList); //
	
	// save a marker file for original image
	m_OutputFileName = m_InputFileName + ".marker";
	
	FILE *pFileMarkerUS=0;
	
	pFileMarkerUS = fopen(m_OutputFileName.toStdString().c_str(),"wt");
	
	for(long ii = 0; ii < LABELBIN; ii++)
	{
		ImageMarker S;
		
		S.z = detectedList.at(ii).z / scale_z + bpos_z + 0.5 + 1; //for display 0-base -> 1-base
		S.x = detectedList.at(ii).x / scale_x + bpos_x + 0.5 + 1;
		S.y = detectedList.at(ii).y / scale_y + bpos_y + 0.5 + 1;
		
		
		double vols = detectedList.at(ii).volsize;
		
		double r = pow( vols*3/4/PI, 1.0/3.0);
		
		S.radius = int(r+0.5);
		S.shape = 1;
		S.name = "";
		S.comment = "";
		
		S.color = random_rgba8(255);
		S.color.r = 128;
		S.color.g = 128;
		S.color.b = 128;
		
		S.type = (S.x==-1 || S.y==-1 || S.z==-1) ? 0 : 2;
		
		S.on = true;
		S.selected = false;
		
		fprintf(pFileMarkerUS, "%5.3f, %5.3f, %5.3f, %d, %ld, %s, %s, %d,%d,%d\n",
				S.x,
				S.y,
				S.z,
				int(S.radius), S.shape,
				qPrintable(S.name), qPrintable(S.comment),
				S.color.r,S.color.g,S.color.b
				);
		
	}
	fclose(pFileMarkerUS); //
	
	tmp=QString("The corresponding .marker file is saved as %1<br>").arg(m_OutputFileName);
	sta_info.append(tmp);
	
	tmp=QString("<br>time eclapse %1 seconds for cell counting.<br>").arg((end_segmentation-start_t)/1000000);
	sta_info.append(tmp);
	
	//showing statistics info
	QTextEdit *pText=new QTextEdit(sta_info);
	pText->resize(800, 200); 
	pText->setReadOnly(true);
	pText->setFontPointSize(12);
	pText->show();

	
	//de-alloc
	if(outimg3d) {delete outimg3d; outimg3d=0;}
	if(flag_lm) {delete flag_lm; flag_lm=0;}
	
}

/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).  
 * All rights reserved.
 */


/************
                                            ********* LICENSE NOTICE ************

This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it. 

You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.

1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.

2. You agree to appropriately cite this work in your related studies and publications.

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




// fit_gmm.h
// By Hanchuan Peng
// 2007-April-23: create this file
// 2007-April-26: add a null debug function
//090108: move from the palm_c project here and revise for the v3d project

#ifndef __FIT_GMM__
#define __FIT_GMM__

//the folowing conditional compilation is added by PHC, 2010-05-20
#if defined (_MSC_VER)
#include "../basic_c_fun/vcdiff.h"
#else
#endif


#ifndef BYTE
#define BYTE signed char
#endif

#ifndef UBYTE
#define UBYTE unsigned char
#endif

#ifndef BYTE16
#define BYTE16 signed short int
#endif

#ifndef UBYTE16
#define UBYTE16 unsigned short int
#endif

#ifndef MYFLOAT
#define MYFLOAT float
#endif


//#include "palm_c.h"
#include "../basic_c_fun/img_definition.h"

class EMClustering  
{
public:
	
	EMClustering(double **data2d, V3DLONG numcases, V3DLONG ndims);
	EMClustering();
	virtual ~EMClustering();
	
	void SetData(double **data2d, V3DLONG numcases, V3DLONG ndims);
	void DoEMClustering(int clusternum);
	void GetDiscretization(UBYTE * data1d, V3DLONG numcases);
	
	void GetOptimalMixtures(double * & optprior, double ** & optmean, double ** & optvar, V3DLONG & ncluster, V3DLONG & ndim);  //070409
	
private:
		
	void Initialization();
	void UnsetData();
	
	void GetDataRange();
	void NormalizeData();
	
	void InitializeClusters(V3DLONG nClusters);
	double DoClustering(V3DLONG nClusters);
	void SaveDiscretization(V3DLONG nClusters);
	double CalculateProbDensity(V3DLONG clusterIndex, 
								V3DLONG dataIndex, 
								double **data, 
								double **mean, 
								double **variance);
	void CopyToOptimal(V3DLONG nClusters);
	
	void PrintClusters(V3DLONG nClusters,double* prior,double** mean,double** variance);
	
	V3DLONG			gNVars;
	V3DLONG			gNCases;
	V3DLONG			gNColsToProcess;
	
	double**		gData;
	
    V3DLONG			gNVarCases;
    double*			gLogFact;
	
	double*			gMin;
	double*			gMax;
	
	double**		gMean;
	double**		gVariance;
	double*			gPrior;
	double**		gPosterior;
	
	V3DLONG			gOptNClusters;
	double**		gOptMean;
	double**		gOptVariance;
	double*			gOptPrior;
	double**		gOptPosterior;
	
	V3DLONG			gMaxNTrials;
    V3DLONG			gMaxNClusters;
	V3DLONG			gNRepeats;
    
	
	double*			gMedian;
	
	bool bDataExist;
	bool bFinishClustering;
	bool bNormalizeData;
	
};


void CHECK_ALLOC(void *ptr);
double PseudoRand();

template <class T> struct Element2
{
  T e1, e2;
};
template <class T> struct Element3
{
  T e1, e2, e3;
};


class GMM2D_Est
{
public:
  V3DLONG nNonzeroPixel;
  double totalMass;
  UBYTE * clusteringRes;
  V3DLONG nGauss;
  double * prior;
  Element2 <double> * mean;
  Element2 <double> * std;
  GMM2D_Est() {prior=0; mean=0; std=0; clusteringRes=0; nGauss=0; nNonzeroPixel=0; totalMass=0;}
  ~GMM2D_Est()
  {
    if (prior) {delete []prior; prior=0;}
	if (mean) {delete []mean; mean=0;}
	if (std) {delete []std; std=0;}
	if (clusteringRes) {delete []clusteringRes; clusteringRes=0;}
  }
};

class GMM3D_Est
{
public:
  V3DLONG nNonzeroPixel;
  double totalMass;
  UBYTE * clusteringRes;
  V3DLONG nGauss;
  double * prior;
  Element3 <double> * mean;
  Element3 <double> * std;
  GMM3D_Est() {prior=0; mean=0; std=0; clusteringRes=0; nGauss=0; nNonzeroPixel=0; totalMass=0;}
  ~GMM3D_Est()
  {
    if (prior) {delete []prior; prior=0;}
	if (mean) {delete []mean; mean=0;}
	if (std) {delete []std; std=0;}
	if (clusteringRes) {delete []clusteringRes; clusteringRes=0;}
  }
};


//template <class T> class Image2DSimple;
GMM2D_Est * fit_gmm(Image2DSimple <MYFLOAT> * fitImg, const double photonConversionFactor, const double winRadius, int clusternum);
GMM3D_Est * fit_gmm(Vol3DSimple <MYFLOAT> * fitImg, const double photonConversionFactor, const double winRadius, int clusternum);

//a null function just finding the center of the peak, without doing any other things. This is for debug purpose. by PHC, 070426
GMM2D_Est * fit_gmm_null(Image2DSimple <MYFLOAT> * fitImg, const double photonConversionFactor, const double winRadius, int clusternum);
GMM3D_Est * fit_gmm_null(Vol3DSimple <MYFLOAT> * fitImg, const double photonConversionFactor, const double winRadius, int clusternum);

#endif

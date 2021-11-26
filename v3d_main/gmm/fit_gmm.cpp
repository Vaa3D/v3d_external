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




// fit_gmm.cpp
// by Hanchuan Peng
// revised from emcnd_count.cpp on 070423
//
//n-d point clustering based on adaptive EM
//
// By Hanchuan Peng
// 2007-April-23: create this file and add an interface to call from other C program
// 2009-01-08: further revise for the v3d project
//////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

const int    kMaxNClusters	    = 4;
const int    kMaxNTrials	    = 100; //200; 070322
const int    kNMedian		    = 3; //7; 070426 //5, 070408
const double kMth2pi	        = 6.283185307179586476925286766558;
const double kMthLog2pi	        = 1.83787706640935;
const double kMthSqrt2pi        = 2.506628274631000502415765284811;
const double kMthLogSqrt2pi	    = 0.91893853320467;
const double kMthSqrt2		    = 1.4142135623730950488016887242097;
const double kMthStdev		    = 1.0;
const double kMthTolerance	    = 0.0001;
const double kMthMinDblLog	    = -708.396418532264;	/* log(DBL_MIN)					*/
const double kMthImpossibleProb	= -1.0;
const double MISSINGVALUE       = -9999;
int    b_isotropicGaussian = 1; //061102: use isotropic Gaussian for Eric Betzig's PALM data

const int bDispInfo = 0;//070416; change 1 to 0

#include "fit_gmm.h"
//#include "palm_c.h"

#ifndef BYTE
typedef signed char BYTE;
#endif

#ifndef BYTE16
#define BYTE16 signed short int
#endif

#ifndef UBYTE16
#define UBYTE16 unsigned short int
#endif

void CHECK_ALLOC(void *ptr)
{if (!ptr){fprintf(stderr, "Fail to alloc memory\n");exit(1);}}

double PseudoRand()
{return (double) (rand() / (double) (RAND_MAX + 1.0));}

EMClustering::EMClustering(double **data2d, V3DLONG numcases, V3DLONG ndims)
{
	Initialization();
	SetData(data2d, numcases,ndims);
}


EMClustering::EMClustering()
{
	Initialization();
}

EMClustering::~EMClustering()
{
	UnsetData();
}

void EMClustering::Initialization()
{
	gData = NULL;

	gLogFact = NULL;

	gMin = NULL;
	gMax = NULL;
	gMean = NULL;
	gOptMean = NULL;

	gVariance = NULL;
	gOptVariance = NULL;

	gPrior = NULL;
	gOptPrior = NULL;

	gPosterior = NULL;
	gOptPosterior = NULL;

	gMedian = NULL;

	//set the parameters to defaults
	gNCases			= -1;
	gNVars			= -1;
	gNColsToProcess = -1;
	gMaxNTrials		= kMaxNTrials;
	gMaxNClusters	= 0;
	gNRepeats		= kNMedian;

	gNVarCases = gNVars;
	gOptNClusters   = 1; //by default set as 1

	bDataExist = false;
	bFinishClustering = false;
	bNormalizeData = false; //061102. "true"
}

void EMClustering::UnsetData()
{
	V3DLONG c,n;

	if (gData)
	{
		for (c = 0; c < gNCases; ++c)
		{
			if (gData[c]) delete gData[c];
		}
		delete [] gData; gData=NULL;
	}

	if (gLogFact) {delete [] gLogFact; gLogFact=NULL;}

	if (gMin) {delete [] gMin;gMin=NULL;}
	if (gMax) {delete [] gMax;gMax=NULL;}

	if (gMean)
	{
		for (n = 0; n < gMaxNClusters; ++n)
		{
			if (gMean[n]) delete [] gMean[n];
		}
		delete [] gMean;gMean=NULL;
	}
	if (gOptMean)
	{
		for (n = 0; n < gMaxNClusters; ++n)
		{
			if (gOptMean[n]) delete [] gOptMean[n];
		}
		delete [] gOptMean;gOptMean=NULL;
	}

	if (gVariance)
	{
		for (n = 0; n < gMaxNClusters; ++n)
		{
			if (gVariance[n]) delete [] gVariance[n];
		}
		delete [] gVariance;gVariance=NULL;
	}
	if (gOptVariance)
	{
		for (n = 0; n < gMaxNClusters; ++n)
		{
			if (gOptVariance[n]) delete [] gOptVariance[n];
		}
		delete [] gOptVariance;gOptVariance=NULL;
	}

	if (gPrior) {delete [] gPrior;gPrior=NULL;}
	if (gOptPrior) {delete [] gOptPrior;gOptPrior=NULL;}

	if (gPosterior)
	{
		for (n = 0; n < gMaxNClusters; ++n)
		{
			if (gPosterior[n]) delete [] gPosterior[n];
		}
		delete [] gPosterior;gPosterior=NULL;
	}

	if (gOptPosterior)
	{
		for (n = 0; n < gMaxNClusters; ++n)
		{
			if (gOptPosterior[n]) delete [] gOptPosterior[n];
		}
		delete [] gOptPosterior;gOptPosterior=NULL;
	}


	if (gMedian) {delete [] gMedian; gMedian = NULL;}

	//set the parameters to defaults
	gNCases			= -1;
	gNVars			= -1;
	gNColsToProcess = -1;
	gMaxNTrials		= kMaxNTrials;
	gMaxNClusters	= 0;
	gNRepeats		= kNMedian;

	//gNVarCases = gNVars; //061102: I don't understand why I need this, and this should be wrong
	gOptNClusters   = 1; //by default set as 1

	bDataExist = false;
	bFinishClustering = false;
	bNormalizeData = false; //061102
}

void EMClustering::SetData(double **data2d, V3DLONG numcases, V3DLONG ndims)
{
	// Initialize global variables
	UnsetData();

	V3DLONG n,c,v;

	//initialize the gData and gDataMissing

	gNCases = numcases; //case number of data
	gNVars = ndims; //dimension of data

	if (gNColsToProcess > gNVars || gNColsToProcess < 1)
	{gNColsToProcess = gNVars-1;} //-1 : 061102, because the last column is the count

	gData = new double * [gNCases]; CHECK_ALLOC((void *)gData);

	for (c = 0; c < gNCases; ++c)
	{
		gData[c] = new double [gNVars]; CHECK_ALLOC((void *)gData[c]);
		for (v = 0; v < gNVars; ++v)
			gData[c][v] = data2d[c][v];
	}

	if (gNCases > kMaxNClusters) {gMaxNClusters = kMaxNClusters;}
	else {gMaxNClusters = gNCases;}

	if (bNormalizeData==true)
	{
		NormalizeData();
	}


	//allocate space for gNVarCases, gMin, and gMax

	//gNVarCases = gNCases; //061102: I don't understand why I need this, and this should be wrong
	gNVarCases = 0; for (n=0;n<gNCases; n++) gNVarCases += gData[n][gNVars-1]; //061102

	gMin = new double [gNVars]; CHECK_ALLOC((void *)gMin);
	gMax = new double [gNVars];	CHECK_ALLOC((void *)gMax);

	GetDataRange();

	//	RTN_ON_ERR(PrintData());

	gLogFact = new double [gNCases];CHECK_ALLOC((void *)gLogFact);
	gLogFact[0] = 0;
	gLogFact[1] = 0;
	gLogFact[2] = log(2);
	for (n = 3; n < gNCases; ++n)
	{gLogFact[n] = gLogFact[n - 1] + log(double(n));}

	// Allocate the mean vector for each cluster.
	gMean = new double * [gMaxNClusters];	CHECK_ALLOC((void *)gMean);
	gOptMean = new double * [gMaxNClusters]; CHECK_ALLOC((void *)gOptMean);
	for (n = 0; n < gMaxNClusters; ++n)
	{
		gMean[n] = new double [gNVars]; CHECK_ALLOC((void *)gMean[n]);
		gOptMean[n] = new double [gNVars]; CHECK_ALLOC((void *)gOptMean[n]);
	}

	// Allocate a diagonal covariance matrix  for each cluster.
	gVariance = new double * [gMaxNClusters];	CHECK_ALLOC((void *)gVariance);
	gOptVariance = new double * [gMaxNClusters];CHECK_ALLOC((void *)gOptVariance);
	for (n = 0; n < gMaxNClusters; ++n)
	{
		gVariance[n] = new double [gNVars]; CHECK_ALLOC((void *)gVariance[n]);
		gOptVariance[n] = new double [gNVars]; CHECK_ALLOC((void *)gOptVariance[n]);
	}

	// Allocate a vector of prior probabilities over clusters.
	gPrior = new double [gMaxNClusters];CHECK_ALLOC((void *)gPrior);
	gOptPrior = new double [gMaxNClusters];	CHECK_ALLOC((void *)gOptPrior);

	// Allocate a vector of posterior probabilities of cluster membership.
	gPosterior = new double * [gMaxNClusters];CHECK_ALLOC((void *)gPosterior);
	for (n = 0; n < gMaxNClusters; ++n)
	{
		gPosterior[n] = new double [gNCases]; CHECK_ALLOC((void *)gPosterior[n]);
	}

	// The optimal posterior distribution is stored for each variable.
	gOptPosterior = new double * [gMaxNClusters]; CHECK_ALLOC((void *)gOptPosterior);
	for (n = 0; n < gMaxNClusters; ++n)
	{
		gOptPosterior[n] = new double [gNCases];CHECK_ALLOC((void *)gOptPosterior[n]);
	}

	// Allocate a list of the optimal number of clusters (states) for each	variable

	gOptNClusters = 1;

	// Allocate a vector from which a median cluster score can be computed.

	gMedian = new double [gNRepeats]; CHECK_ALLOC((void *)gMedian);

	bDataExist = true;
}


void EMClustering::InitializeClusters(V3DLONG nClusters)
{
	bool bRandSeed = true; //'true' for rand initialize prior, otherwise uniform prior
	//bool bRandSeed = false; //'true' for rand initialize prior, otherwise uniform prior //070408, just for comparison of bRandSeed=true/false

	if (bRandSeed==true)
		srand((unsigned)time(NULL));

	V3DLONG cluster;

	if (bRandSeed==true)
	{
		double tmpsum = 0.0;
		for (cluster = 0; cluster < nClusters; ++cluster)
		{
			gPrior[cluster] = PseudoRand();
			tmpsum += gPrior[cluster];
		}
		for (cluster = 0; cluster < nClusters; ++cluster)
			gPrior[cluster] /= tmpsum;
	}
	else
	{
		for (cluster = 0; cluster < nClusters; ++cluster)
			gPrior[cluster] = 1.0 / nClusters;
	}


	for (V3DLONG i=0;i<gNColsToProcess;i++)
	{

		// Allocate a vector of posterior probabilities of cluster membership.

		// Assign seed means for each cluster.
		double range = gMax[i] - gMin[i];

		for (cluster = 0; cluster < nClusters; ++cluster)
		{
			// Generate a value in the range (min, max).
			if (bRandSeed==true)
			{
				gMean[cluster][i] = PseudoRand() * range + gMin[i];
				//	gVariance[cluster] = CLMathRand() * range;
				gVariance[cluster][i] = range;
			}
			else //??????? right ????
			{
				gMean[cluster][i] = gMin[i] + (range * cluster) / nClusters;
				gVariance[cluster][i] = range / 3;
			}
		}
	}

	return;
}

double EMClustering::DoClustering(V3DLONG nClusters)
{
	V3DLONG				nTrials;
	V3DLONG				c;
	V3DLONG				cluster;
	double				pData_Cluster;
	double				difference;
	double				distance;
	double				previous;
	bool 				done;
	double				logPData_Cluster;
	double				logVarianceSum;
	double				logPriorSum;
	double				logPosteriorSum;
	double				logH;
	V3DLONG				nParameters;		// # parameters in the model

	//V3DLONG				nVars =1 ;// Clustering on 1 variable at a time.

	double score;

	InitializeClusters(nClusters); //no necessary to do realloccation of space in this function.

	nTrials = 0;

	// Compute the statistics for each cluster using the EM algorithm.
	do {
		for (c = 0; c < gNCases; ++c)
		{
			double tmpsum = 0.0;
			for (cluster = 0; cluster < nClusters; cluster++)
			{
				pData_Cluster = CalculateProbDensity(cluster, c, gData, gMean, gVariance);// * gData[c][gNColsToProcess]; //070325: * gData[c][gNColsToProcess];

				// Compute the joint probability of datum c being in each cluster.
				gPosterior[cluster][c] = gPrior[cluster] * pData_Cluster;
				tmpsum += gPosterior[cluster][c];
			}
            //printf("c[%d]=%7.5f", c, tmpsum);

			for (cluster = 0; cluster < nClusters; cluster++)
			{
				// Compute the conditional probability of datum c being in each cluster.
				gPosterior[cluster][c] /= tmpsum;
			}
		}

		// Update the parameters of the clusters' distributions, given the posterior probabilities.
		// Update prior probabilities of cluster membership using eq.(17) on page 1135.
		done = true;
		for (cluster = 0; cluster < nClusters; cluster++)
		{
			previous = gPrior[cluster];
			gPrior[cluster] = 0.0;

			for (c = 0; c < gNCases; c++)
			{
				gPrior[cluster] += gPosterior[cluster][c] * gData[c][gNColsToProcess]; //061102
			}
			gPrior[cluster] /= gNVarCases;
			if (done == true && fabs(previous - gPrior[cluster]) > kMthTolerance)
				done = false;
		}

		// Update mean value for each cluster along each dimension using eq.(17) on page 1135.
		V3DLONG i;
		for (i=0;i<gNColsToProcess;i++)
		{
			for (cluster = 0; cluster < nClusters; cluster++)
			{
				previous = gMean[cluster][i];
				gMean[cluster][i] = 0.0;
				for (c = 0; c < gNCases; c++)
				{
					gMean[cluster][i] += (gPosterior[cluster][c] * gData[c][i] * gData[c][gNColsToProcess]);//061102: * gData[c][gNColsToProcess]
				}

				if (gPrior[cluster] > 0.0)
				{
					gMean[cluster][i] /= (gNVarCases * gPrior[cluster]); //this can be simplfied later
					if (done == true && fabs(previous - gMean[cluster][i]) > kMthTolerance)
						done = false;
				}
				else
				{
					gMean[cluster][i] = 0.0;
				}
			}
		}

		// Update variance matrix for each cluster using eq.(17) on page 1135.
		for (i=0;i<gNColsToProcess;i++)
		{
			for (cluster = 0; cluster < nClusters; cluster++)
			{
				previous = gVariance[cluster][i];
				gVariance[cluster][i] = 0.0;
				for (c = 0; c < gNCases; ++c)
				{
					difference = gMean[cluster][i] - gData[c][i];
					distance = difference * difference * gData[c][gNColsToProcess]; //061102
					gVariance[cluster][i] += (gPosterior[cluster][c] * distance);
				}

				if (gPrior[cluster] > 0.0)
				{
					gVariance[cluster][i] /= (gNVarCases * gPrior[cluster]);
					// Don't let variance drop, otherwise matrix is singular.
					if (gVariance[cluster][i] < kMthTolerance)
						gVariance[cluster][i] = kMthTolerance;
					if (done == true && fabs(previous - gVariance[cluster][i]) > kMthTolerance)
						done = false;
				}
				else
				{
					gVariance[cluster][i] = 1.0;
				}
			}
		}

		//061103 to force the Gaussian is round
		if (b_isotropicGaussian==1)
		{
			for (cluster=0;cluster<nClusters; cluster++)
			{
				double tmps=0;
				for (i=0;i<gNColsToProcess;i++)
				{
					tmps += gVariance[cluster][i];
				}
				tmps /= gNColsToProcess;
				for (i=0;i<gNColsToProcess;i++)
				{
					gVariance[cluster][i] = tmps;
				}
			}
		}


		nTrials++;

        //comment the following printing stuff on 070416
//		if (done == true || nTrials >= gMaxNTrials - 1)
//		{
//			printf("Final values (%ld iterations):\n", nTrials);
//			PrintClusters(nClusters, gPrior, gMean, gVariance);
//		}

	} while (done == false && nTrials < gMaxNTrials);


	logPData_Cluster = 0.0; //use eqs.(3) and (2) on page 1133
	for (c = 0; c < gNCases; c++)
	{
		// Marginal probability of the data.
		double tmpsum = 0.0;
		for (cluster = 0; cluster < nClusters; cluster++)
		{
			pData_Cluster = CalculateProbDensity(cluster, c, gData, gMean, gVariance); // * gData[c][gNColsToProcess]; //070321: * gData[c][gNColsToProcess]
			// Multiply pData_Cluster by the prior, per p. 1135.
			tmpsum += (pData_Cluster * gPrior[cluster]);
		}
		logPData_Cluster += log(tmpsum) * gData[c][gNColsToProcess];//070325: * gData[c][gNColsToProcess] //070408: move * gData[c][gNColsToProcess] outside of log()
	}

	logVarianceSum = 0.0; //use eq.(29) on page 1136
	logPriorSum = 0.0;    //use eq.(29) on page 1136
	for (cluster = 0; cluster < nClusters; cluster++)
	{
		logPriorSum += log(kMthSqrt2 * gNVarCases * gPrior[cluster]); //070325 change gNCases as gNVarCases
		for (V3DLONG i=0;i<gNColsToProcess;i++)
			logVarianceSum += log(gVariance[cluster][i]);
	}

	logPosteriorSum = 0.0; //use eq.(29) on page 1136
	for (cluster = 0; cluster < nClusters - 1; cluster++)
	{
		double posteriorSum = 0.0;
		for (c = 0; c < gNCases; ++c)
		{
			double posteriorDiff = (gPosterior[cluster][c] / gPrior[cluster]) - (gPosterior[nClusters - 1][c] / gPrior[nClusters - 1]); //??, 070325
			posteriorSum += (posteriorDiff * posteriorDiff) * gData[c][gNColsToProcess];//070325: * gData[c][gNColsToProcess]
		}
		logPosteriorSum += log(posteriorSum);
	}

	//use eq.(29) on page 1136
	//logH = logPosteriorSum + 2 * nVars * logPriorSum - 2 * logVarianceSum;
	logH = logPosteriorSum + 2 * gNColsToProcess * logPriorSum - 2 * logVarianceSum;

	// There is a prior for each cluster, and a mean and variance for each
	// dimension of each Gaussian.
	// Subtract 1, since the last cluster prior is redundant.
	//use eq.(15) on page 1134

	nParameters = nClusters * (1 + gNColsToProcess * 2) - 1; //use the explaination of eq.(8) on page 1134

	//use eq.(15) on page 1134;
	//Kdln(alpha*beita*2*sigma^2) use info on page 1134, i.e. alpha=beita=sigma=1;
	score = logPData_Cluster
        - nClusters * gNColsToProcess * log(2)
        + gLogFact[nClusters - 1]   //070321
        + (nParameters / 2) * log(kMth2pi)
        - (logH / 2)
       ;
	score = - score;

	//#define METRIC_MDL
#ifdef METRIC_MDL
	//score = (nParameters / 2) * log(gNCases) - logPData_Cluster;
	score = (nParameters / 2) * log(gNVarCases) - logPData_Cluster; //070325: change gNCases to gNVarCases
#endif

	return score;
}


double EMClustering::CalculateProbDensity(V3DLONG clusterIndex,
										  V3DLONG dataIndex,
										  double **data,
										  double **mean,
										  double **variance)
{
	//eq.(19) on page 1135

	double probability= 1.0;
	for (V3DLONG i=0;i<gNColsToProcess;i++)
	{
		double stdev = sqrt(variance[clusterIndex][i]);
		if (stdev < kMthTolerance) {stdev = kMthTolerance;}
		double dis = (data[dataIndex][i] - mean[clusterIndex][i]) / stdev;
		dis *= dis;

		if (dis > -kMthMinDblLog) {dis = -kMthMinDblLog;}
		probability *= exp(-dis/2.0) / (kMthSqrt2pi * stdev); //070410
//		probability *= exp(-dis) / (kMthSqrt2pi * stdev); //070410
	}
//    printf("%7.2f | ", probability);
	return probability;
}


void EMClustering::PrintClusters(V3DLONG nClusters, double *prior, double **mean, double **variance)
{
	for (V3DLONG cluster = 0; cluster < nClusters; ++cluster)
	{
		V3DLONG i;
		printf("prior = %5.3lf: \n", prior[cluster]);
		printf("mean = ");
		for (i=0;i<gNColsToProcess;i++) printf("%8.4lf ", mean[cluster][i]);
		printf("\n");
		printf("std = ");
		for (i=0;i<gNColsToProcess;i++) printf("%8.4lf ", sqrt(variance[cluster][i]));
		printf("\n");
	}
	printf("\n");
}


void EMClustering::DoEMClustering(int specificlusternum)
{
	if (bDataExist==false)
		return;

	bFinishClustering = false;

	V3DLONG				nClusters;
	double				score, bestScore;
	V3DLONG				bestNClusters;
	double				minScore;
	V3DLONG				minIndex;
	double				score1, score2, score3;
	bool				done;

	V3DLONG				r, n;

	//#define WIN32
//#ifdef WIN32
	const double MYDBL_MAX = 1000000000; //??????
//#endif

	//initialize at the construction function; not here

	int nClusterRangeLow = 1;
	int nClusterRangeHigh = gMaxNClusters;
	if (specificlusternum>=1 && specificlusternum<=gMaxNClusters)
	{
		nClusterRangeLow = nClusterRangeHigh = specificlusternum;
	}

	bestScore = MYDBL_MAX;
	score1 = MYDBL_MAX;
	score2 = MYDBL_MAX;
	score3 = MYDBL_MAX;

	done = false;

	bestNClusters = 0;
	for (nClusters = nClusterRangeLow; done == false && nClusters <= nClusterRangeHigh; nClusters++)
	{
		// Increase the maximum # of EM trials as the number of clusters increases.
		//gMaxNTrials = kMaxNTrials * nClusters;
		gMaxNTrials = kMaxNTrials; //reduce computation?
        if (bDispInfo) printf("ncluster=[%d]; ", nClusters);
		for (r = 0; r < gNRepeats; r++)
		{
			//DoClustering(dimension, nClusters, score);
			score = DoClustering(nClusters);
			gMedian[r] = score;
            if (bDispInfo) printf("%d=[%7.3f]; ", r, gMedian[r]);
		}
        if (bDispInfo) printf("\n");

		for (n = 0; n < gNRepeats / 2; n++)
		{
			minScore = gMedian[0];
			minIndex = 0;
			for (r = 1; r < gNRepeats - n; r++)
			{
				if (gMedian[r] < minScore)
				{
					minScore = gMedian[r];
					minIndex = r;
				}
			}
			if (minIndex != gNRepeats - n - 1)
			{
				gMedian[minIndex] = gMedian[gNRepeats - n - 1];
				gMedian[gNRepeats - n - 1] = minScore;
			}
		}

		score = minScore;
		if (bDispInfo) {
			printf("Score[%ld][%ld]=%10.6lf\n", nClusters, minIndex, score);
		}
		if (score < bestScore || (nClusters == 1 && r == 1))
		{
			bestScore = score;
			bestNClusters = nClusters;

			CopyToOptimal(nClusters);
		}

		score1 = score2;
		score2 = score3;
		score3 = score;

		if (score3 > score2 && score2 > score1) {done = true;}

	}

	if (bDispInfo) {
		printf("Optimal number of clusters for this %ld-dimensional variable is %ld, score is %10.6lf\n",
			   gNColsToProcess, bestNClusters, bestScore);
		PrintClusters(bestNClusters, gOptPrior, gOptMean, gOptVariance);
	}

	SaveDiscretization(bestNClusters);

	bFinishClustering = true;
	return;
}


void EMClustering::CopyToOptimal(V3DLONG nClusters)
{
	//	DoubleArray3		arrays;

	for (V3DLONG c = 0; c < nClusters; ++c)
	{
		for (V3DLONG i=0;i<gNColsToProcess;i++)
		{
			gOptMean[c][i] = gMean[c][i];
			gOptVariance[c][i] = gVariance[c][i];
		}
		gOptPrior[c] = gPrior[c];
	}

	// Sort all 3 arrays by value of the mean. //why sorting is necessary????

	/*	arrays.arr1 = gOptMean;
	arrays.arr2 = gOptVariance;
	arrays.arr3 = gOptPrior;
	*/
	//CLSort(nClusters, &arrays, CompareArray1, SwapArrays); //????????

	return;
}

void EMClustering::GetDataRange()
{
	V3DLONG c, v;

	//CHECK_ALLOC((void *)gNVarCases); //061102 : this should be error
	CHECK_ALLOC((void *)gMin);
	CHECK_ALLOC((void *)gMax);

	//gNVarCases = 0; //061102: I don't understand why I need this, and this should be wrong
	for (v = 0; v < gNVars; ++v)
	{
		gMin[v] = 1.0;
		gMax[v] = 0.0;
	}

	for (c = 0; c < gNCases; ++c)
	{
		for (v = 0; v < gNVars; ++v)
		{
			//++gNVarCases; //061102: I don't understand why I need this, and this should be wrong
			// If the minimum is greater than the maximum, the arrays
			// haven't been initialized.
			if (gMin[v] > gMax[v])
			{
				gMin[v] = gData[c][v];
				gMax[v] = gData[c][v];
			}
			else
			{
				// Update the minimum and maximum values.
				if (gMin[v] > gData[c][v]) 	{gMin[v] = gData[c][v];}
				else if (gMax[v] < gData[c][v]) {gMax[v] = gData[c][v];}
			}
		}
	}

	return;
}


void EMClustering::NormalizeData() //need correction of case numbers, 070408. But can do this later, as this function does not get called
{
	V3DLONG c, v;

	CHECK_ALLOC((void *)gData);
	for (c = 0; c < gNCases; ++c) {CHECK_ALLOC((void *)gData[c]);}

	for (v = 0; v < gNColsToProcess; ++v)
	{
		// Compute count, gNormMean and standard deviation.
		double gNormMean = 0.0;
		double gNormStdev = 0.0;

		for (c = 0; c < gNCases; ++c)
		{
			gNormMean += gData[c][v];
			gNormStdev += (gData[c][v] * gData[c][v]) / (gNCases - 1);
		}

		gNormStdev = sqrt(gNormStdev - (gNormMean / gNCases * gNormMean / (gNCases - 1)));
		gNormStdev /= kMthStdev;
		gNormMean /= gNCases;

		if (bDispInfo) {
			printf("Data normalized using (%6.3lf, %6.3lf)\n", gNormMean, gNormStdev);
		}

		// Normalize the data to have gNormMean = 0 and variance = kMthStdev.


		for (c = 0; c < gNCases; ++c)
		{
			gData[c][v] -= gNormMean;
			gData[c][v] /= gNormStdev;
		}
	}

	return;
}



void EMClustering::SaveDiscretization(V3DLONG nClusters)
{
	V3DLONG				n, c;
	double				pData_Cluster;
	double				pData;

	if (nClusters < 1 || nClusters > gMaxNClusters)
	{
		printf("Error input parameter to SaveDiscretization()\n");
		return;
	}

	gOptNClusters = nClusters;
	for (n = 0; n < gNCases; ++n)
	{
		//CHECK_ALLOC((void *)gOptPosterior[n]);

		// If there's only 1 cluster, we know the posterior....
		if (nClusters == 1)
		{
			gOptPosterior[0][n] = 1.0;
		}
		else
		{
			pData = 0.0;
			for (c = 0; c < nClusters; ++c)
			{
				pData_Cluster = CalculateProbDensity(c, n, gData, gOptMean, gOptVariance);
				gOptPosterior[c][n] = gOptPrior[c] * pData_Cluster;
				pData += gOptPosterior[c][n];
			}

            /*//061120: not necessay at the output stage

			if (fabs(pData) < kMthTolerance)
			{
				printf("attempted division by 0 at line %d; normalizing to uniform: fabs(pData)=%8.6f < kMthTolerance=%8.6f\n", __LINE__, fabs(pData), kMthTolerance);
				for (c = 0; c < nClusters; ++c)
				{
					gOptPosterior[c][n] = 1.0 / nClusters;
				}
				pData = 1.0;
			}
			else
			{
			*/
				for (c = 0; c < nClusters; ++c)
				{
					gOptPosterior[c][n] /= pData;
				}
			/*}*/
		}
	}

	return;
}


void EMClustering::GetDiscretization(UBYTE * data1d, V3DLONG numcases)
{
	if (bDataExist==false || bFinishClustering == false)
		return;

	V3DLONG c, n;

	if (!data1d || numcases!=gNCases)
	{
		fprintf(stderr,"input parameters invalid\n");
		return;
	}

	for (n = 0; n < gNCases; n++)
	{
		//CHECK_ALLOC((void *)gOptPosterior[n]);

		V3DLONG nClusters = gOptNClusters;

		// To discretize this data point, there must be at least 2
		// clusters, and this datum must exist.

		if (nClusters < 2)
		{
			data1d[n] = (BYTE)(1); //070416: change BYTE(0) to BYTE(1), so it is more consistent in the output results (i.e. always start from 1)
		}
		else
		{
			V3DLONG maxCluster = 0;
			double maxProb = gOptPosterior[maxCluster][n]; //070416: preset as the first one, this is more correct and faster
			for (c = 1; c < nClusters; c++)
			{
#ifdef WRITE_DISTRIBUTIONS
				printf("\t%8.6lf", gOptPosterior[c][n]);
#endif
				if (gOptPosterior[c][n] > maxProb)
				{
					maxProb = gOptPosterior[c][n];
					maxCluster = c;
				}
			}

			data1d[n] = (BYTE)(maxCluster+1);

			//#ifndef WRITE_DISTRIBUTIONS
			//			printf("%ld\t", data1d[n]);
			//#endif
		}
	}

	return;
}


void EMClustering::GetOptimalMixtures(double * & optprior, double ** & optmean, double ** & optvar, V3DLONG & ncluster, V3DLONG & ndim)  //070409
{
    ncluster = gOptNClusters;
	ndim = gNColsToProcess;
    optprior = gOptPrior;
	optmean = gOptMean;
	optvar = gOptVariance;
}
//=========================================


/*

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	if(nrhs < 1)
	{
		printf("Usage: [clustering_res, prior, mean, std] = emcnd_count(data2d,clusternum,b_isotropic). If clusternum<=0, then use Current Max Cluster # is %d. b_isotropic default=1 (if 0 then unisotropic).\n\n",kMaxNClusters);
		mexErrMsgTxt("At least one input is required.\n ");
	}

	if(nlhs > 4)
	{
		printf("Usage: [clustering_res, prior, mean, std] = emcnd_count(data2d,clusternum,b_isotropic). If clusternum<=0, then use Current Max Cluster # is %d. b_isotropic default=1 (if 0 then unisotropic).\n\n",kMaxNClusters);
		mexErrMsgTxt("Too many output arguments (at most 4: clustering result, prior, mean, std).");
    }

	if (!mxIsInt8(prhs[0]) && !mxIsUint8(prhs[0]) && !mxIsDouble(prhs[0]))
		mexErrMsgTxt("The input argument must be types of INT8 or UINT8 or DOUBLE.");

	int clusternum;
	clusternum = (nrhs <2) ? -1 : (int)(*((double *)mxGetData(prhs[1])));

	b_isotropicGaussian = (nrhs <3) ? 1 : (int)(*((double *)mxGetData(prhs[2])));

	mxClassID category = mxGetClassID(prhs[0]);

	V3DLONG totalvoxelnum = mxGetNumberOfElements(prhs[0]);
	int ndims = mxGetNumberOfDimensions(prhs[0]);
	if (ndims!=2) mexErrMsgTxt("The input should be 2D array.");
	const int *dims = mxGetDimensions(prhs[0]);

	V3DLONG i,j;

	void * indata = (void *)mxGetData(prhs[0]);
	double ** indata2d = new double * [dims[0]];

	EMClustering myClustering;
	switch (category)
	{
		case mxINT8_CLASS:
			for (i=0;i<dims[0];i++)
			{
				indata2d[i] = new double [dims[1]];
				for (j=0;j<dims[1];j++)
					indata2d[i][j] = double(*((BYTE *)indata + i + j * dims[0]));
			}
			myClustering.SetData(indata2d,V3DLONG(dims[0]),V3DLONG(dims[1]));
			break;

		case mxUINT8_CLASS:
			for (i=0;i<dims[0];i++)
			{
				indata2d[i] = new double [dims[1]];
				for (j=0;j<dims[1];j++)
					indata2d[i][j] = double(*((UBYTE *)indata + i + j * dims[0]));
			}
			myClustering.SetData(indata2d,V3DLONG(dims[0]),V3DLONG(dims[1]));
			break;

		case mxUINT16_CLASS:
			for (i=0;i<dims[0];i++)
			{
				indata2d[i] = new double [dims[1]];
				for (j=0;j<dims[1];j++)
					indata2d[i][j] = double(*((UBYTE16*)indata + i + j * dims[0]));
			}
			printf("done copy\n");
			return;
			myClustering.SetData(indata2d,V3DLONG(dims[0]),V3DLONG(dims[1]));
			break;

		case mxDOUBLE_CLASS:
			for (i=0;i<dims[0];i++)
			{
				indata2d[i] = new double [dims[1]];
				for (j=0;j<dims[1];j++)
					indata2d[i][j] = *((double *)indata + i + j * dims[0]);
			}
			myClustering.SetData(indata2d,V3DLONG(dims[0]),V3DLONG(dims[1]));
			break;

	}
	if (indata2d)
	{
		for (i=0;i<dims[0];i++)
		{
			if (indata2d[i]) delete [] indata2d[i];
		}
		delete [] indata2d; indata2d = NULL;
	}

	mxArray *resCLUSTERDATA = mxCreateNumericMatrix(dims[0],1,mxUINT8_CLASS,mxREAL);
	UBYTE * resClusteringData = (UBYTE *)mxGetData(resCLUSTERDATA);

	double * optprior = 0, ** optmean = 0, ** optvar = 0;
	V3DLONG myncluster, myndim;
	//	printf("done copy2\n");	return;

	myClustering.DoEMClustering(clusternum);
	myClustering.GetDiscretization(resClusteringData, dims[0]);
	plhs[0] = resCLUSTERDATA;

    if (nlhs>1) //then must return at least prior
	{
		myClustering.GetOptimalMixtures(optprior,  optmean,  optvar,  myncluster,  myndim);

	}
	else
	    return;

	if (nlhs>=2)
	{
		mxArray *resOptPriorMatrix = mxCreateNumericMatrix(myncluster, 1, mxDOUBLE_CLASS, mxREAL);
		double *tmp_ptr = (double *)mxGetPr(resOptPriorMatrix);
		for (i=0;i<myncluster;i++) tmp_ptr[i] = optprior[i];
		plhs[1] = resOptPriorMatrix;
	}

    if (nlhs>=3)
	{
		mxArray *resOptMeanMatrix = mxCreateNumericMatrix(myncluster, myndim, mxDOUBLE_CLASS, mxREAL);
		double *tmp_ptr = (double *)mxGetPr(resOptMeanMatrix);
		for (i=0;i<myncluster;i++)
		{
		   for (j=0;j<myndim;j++)
		     tmp_ptr[j*myncluster+i] = optmean[i][j]; //note that Matlab's first dim is column
		}
		plhs[2] = resOptMeanMatrix;
	}

    if (nlhs>=4)
	{
		mxArray *resOptStdMatrix = mxCreateNumericMatrix(myncluster, myndim, mxDOUBLE_CLASS, mxREAL);
		double *tmp_ptr = (double *)mxGetPr(resOptStdMatrix);
		for (i=0;i<myncluster;i++)
		{
		   for (j=0;j<myndim;j++)
		     tmp_ptr[j*myncluster+i] = sqrt(optvar[i][j]); //note that Matlab's first dim is column
		}
		plhs[3] = resOptStdMatrix;
	}

	return;
}

*/

GMM2D_Est * fit_gmm(Image2DSimple <MYFLOAT> * fitImg, const double photonConversionFactor, const double winRadius, int clusternum)
{
    if (!fitImg || !fitImg->valid()) return 0;
	MYFLOAT ** fitImg_p = fitImg->getData2dHandle();

	clusternum = (clusternum<=0) ? -1 : clusternum;
	//int b_isotropicGaussian = 1;

	V3DLONG totalvoxelnum = fitImg->getTotalElementNumber();
	int ndims = 2;

	V3DLONG i,j,k;

	Image2DSimple <double>  data2d (ndims+1, totalvoxelnum);
	double ** data2d_p = data2d.getData2dHandle();

	EMClustering myClustering;

	double fx0=(double)(fitImg->sz0()-1)/2;
	double fy0=(double)(fitImg->sz1()-1)/2;
	double winRadius2 = winRadius*winRadius;

	double totalphoton=0.0;

	k=0;
	for (j=0;j<fitImg->sz1();j++)
	{
	  for (i=0;i<fitImg->sz0();i++)
	  {
	    if (fitImg_p[j][i]>0 && ((j-fy0)*(j-fy0)+(i-fx0)*(i-fx0)<=winRadius2))
		{
		  data2d_p[k][0]=i;
		  data2d_p[k][1]=j;
		  data2d_p[k][2]=fitImg_p[j][i]*photonConversionFactor;
	  	  totalphoton += data2d_p[k][2];
		  k++;
		}
	  }
	}

	myClustering.SetData(data2d_p,k,ndims+1);

    GMM2D_Est * res = new GMM2D_Est;
	res->nNonzeroPixel=k;
	res->clusteringRes = new UBYTE [res->nNonzeroPixel];
	res->totalMass = totalphoton;

	double * optprior = 0, ** optmean = 0, ** optvar = 0;
	V3DLONG myndim;
	//	printf("done copy2\n");	return;

	myClustering.DoEMClustering(clusternum);
	myClustering.GetDiscretization(res->clusteringRes, res->nNonzeroPixel);

	myClustering.GetOptimalMixtures(optprior,  optmean,  optvar,  res->nGauss,  myndim);

	res->prior = new double [res->nGauss];
	res->mean = new Element2 <double> [res->nGauss];
	res->std = new Element2 <double> [res->nGauss];

	for (i=0;i<res->nGauss;i++)
	{
	  res->prior[i] = optprior[i];
	  res->mean[i].e1 = optmean[i][0];
	  res->mean[i].e2 = optmean[i][1];
	  res->std[i].e1 = sqrt(optvar[i][0]);
	  res->std[i].e2 = sqrt(optvar[i][1]);
	}

	return res;
}

GMM3D_Est * fit_gmm(Vol3DSimple <MYFLOAT> * fitImg, const double photonConversionFactor, const double winRadius, int clusternum)
{
    if (!fitImg || !fitImg->valid()) return 0;
	MYFLOAT *** fitImg_p = fitImg->getData3dHandle();

	clusternum = (clusternum<=0) ? -1 : clusternum;
	//int b_isotropicGaussian = 1;

	V3DLONG totalvoxelnum = fitImg->getTotalElementNumber();
	int ndims = 3;

	V3DLONG i,j,k,n;

	Image2DSimple <double>  data2d (ndims+1, totalvoxelnum);
	double ** data2d_p = data2d.getData2dHandle();

	EMClustering myClustering;

	double fx0=(double)(fitImg->sz0()-1)/2;
	double fy0=(double)(fitImg->sz1()-1)/2;
	double fz0=(double)(fitImg->sz2()-1)/2;
	double winRadius2 = winRadius*winRadius;

	double totalphoton=0.0;

	n=0;
	for (k=0;k<fitImg->sz2();k++)
	{
		for (j=0;j<fitImg->sz1();j++)
		{
		  for (i=0;i<fitImg->sz0();i++)
		  {
			if (fitImg_p[k][j][i]>0 && ((k-fz0)*(k-fz0)+(j-fy0)*(j-fy0)+(i-fx0)*(i-fx0)<=winRadius2))
			{
			  data2d_p[n][0]=i;
			  data2d_p[n][1]=j;
			  data2d_p[n][2]=i;
			  data2d_p[n][3]=fitImg_p[k][j][i]*photonConversionFactor;
			  totalphoton += data2d_p[n][3];
			  n++;
			}
		  }
		}
	}

	myClustering.SetData(data2d_p,n,ndims+1);

    GMM3D_Est * res = new GMM3D_Est;
	res->nNonzeroPixel=k;
	res->clusteringRes = new UBYTE [res->nNonzeroPixel];
	res->totalMass = totalphoton;

	double * optprior = 0, ** optmean = 0, ** optvar = 0;
	V3DLONG myndim;
	//	printf("done copy2\n");	return;

	myClustering.DoEMClustering(clusternum);
	myClustering.GetDiscretization(res->clusteringRes, res->nNonzeroPixel);

	myClustering.GetOptimalMixtures(optprior,  optmean,  optvar,  res->nGauss,  myndim);

	res->prior = new double [res->nGauss];
	res->mean = new Element3 <double> [res->nGauss];
	res->std = new Element3 <double> [res->nGauss];

	for (i=0;i<res->nGauss;i++)
	{
	  res->prior[i] = optprior[i];
	  res->mean[i].e1 = optmean[i][0];
	  res->mean[i].e2 = optmean[i][1];
	  res->mean[i].e3 = optmean[i][2];
	  res->std[i].e1 = sqrt(optvar[i][0]);
	  res->std[i].e2 = sqrt(optvar[i][1]);
	  res->std[i].e3 = sqrt(optvar[i][2]);
	}

	return res;
}


//a null function just finding the center of the peak, without doing any other things. This is for debug purpose. by PHC, 070426
GMM2D_Est * fit_gmm_null(Image2DSimple <MYFLOAT> * fitImg, const double photonConversionFactor, const double winRadius, int clusternum)
{
    if (!fitImg || !fitImg->valid()) return 0;
	MYFLOAT ** fitImg_p = fitImg->getData2dHandle();

	clusternum = (clusternum<=0) ? -1 : clusternum;

	V3DLONG totalvoxelnum = fitImg->getTotalElementNumber();
	int ndims = 2;

	V3DLONG i,j,k;

	Image2DSimple <double>  data2d (ndims+1, totalvoxelnum);
	double ** data2d_p = data2d.getData2dHandle();

	double fx0=(double)(fitImg->sz0()-1)/2;
	double fy0=(double)(fitImg->sz1()-1)/2;
	double winRadius2 = winRadius*winRadius;

	double totalphoton=0.0;
	double xsum=0.0, ysum=0.0;
	k=0;
	for (j=0;j<fitImg->sz1();j++)
	{
	  for (i=0;i<fitImg->sz0();i++)
	  {
	    if (fitImg_p[j][i]>0 && ((j-fy0)*(j-fy0)+(i-fx0)*(i-fx0)<=winRadius2))
		{
		  data2d_p[k][0]=i;
		  data2d_p[k][1]=j;
		  data2d_p[k][2]=fitImg_p[j][i]*photonConversionFactor;
	  	  totalphoton += data2d_p[k][2];
		  xsum += i*data2d_p[k][2];
		  ysum += j*data2d_p[k][2];
		  k++;
		}
	  }
	}
	if (totalphoton>0)
	{
		xsum /= totalphoton;
		ysum /= totalphoton;
	}
	else
	{
		xsum = fx0;
		ysum = fy0;
	}

    GMM2D_Est * res = new GMM2D_Est;
	res->nNonzeroPixel=k;
	res->clusteringRes = 0;
	res->totalMass = totalphoton;

    res->nGauss=1;
	res->prior = new double [res->nGauss];
	res->mean = new Element2 <double> [res->nGauss];
	res->std = new Element2 <double> [res->nGauss];

	for (i=0;i<res->nGauss;i++)
	{
	  res->prior[i] = 1;
	  res->mean[i].e1 = xsum;
	  res->mean[i].e2 = ysum;
	  res->std[i].e1 = 1;
	  res->std[i].e2 = 1;
	}

	return res;
}


//a null function just finding the center of the peak, without doing any other things. This is for debug purpose. by PHC
GMM3D_Est * fit_gmm_null(Vol3DSimple <MYFLOAT> * fitImg, const double photonConversionFactor, const double winRadius, int clusternum)
{
    if (!fitImg || !fitImg->valid()) return 0;
	MYFLOAT *** fitImg_p = fitImg->getData3dHandle();

	clusternum = (clusternum<=0) ? -1 : clusternum;

	V3DLONG totalvoxelnum = fitImg->getTotalElementNumber();
	int ndims = 3;

	V3DLONG i,j,k,n;

	Image2DSimple <double>  data2d (ndims+1, totalvoxelnum);
	double ** data2d_p = data2d.getData2dHandle();

	double fx0=(double)(fitImg->sz0()-1)/2;
	double fy0=(double)(fitImg->sz1()-1)/2;
	double fz0=(double)(fitImg->sz2()-1)/2;
	double winRadius2 = winRadius*winRadius;

	double totalphoton=0.0;
	double xsum=0.0, ysum=0.0, zsum=0.0;
	n=0;
	for (k=0;k<fitImg->sz2();k++)
	{
		for (j=0;j<fitImg->sz1();j++)
		{
		  for (i=0;i<fitImg->sz0();i++)
		  {
            if (fitImg_p[j][i]>(float*)0 && ((j-fy0)*(j-fy0)+(i-fx0)*(i-fx0)<=winRadius2))
			{
			  data2d_p[n][0]=i;
			  data2d_p[n][1]=j;
			  data2d_p[n][2]=k;
			  data2d_p[n][3]=fitImg_p[k][j][i]*photonConversionFactor;
			  totalphoton += data2d_p[n][3];
			  xsum += i*data2d_p[n][3];
			  ysum += j*data2d_p[n][3];
			  zsum += k*data2d_p[n][3];
			  n++;
			}
		  }
		}
	}
	if (totalphoton>0)
	{
		xsum /= totalphoton;
		ysum /= totalphoton;
		zsum /= totalphoton;
	}
	else
	{
		xsum = fx0;
		ysum = fy0;
		zsum = fz0;
	}

    GMM3D_Est * res = new GMM3D_Est;
	res->nNonzeroPixel=n;
	res->clusteringRes = 0;
	res->totalMass = totalphoton;

    res->nGauss=1;
	res->prior = new double [res->nGauss];
	res->mean = new Element3 <double> [res->nGauss];
	res->std = new Element3 <double> [res->nGauss];

	for (i=0;i<res->nGauss;i++)
	{
	  res->prior[i] = 1;
	  res->mean[i].e1 = xsum;
	  res->mean[i].e2 = ysum;
	  res->mean[i].e3 = zsum;
	  res->std[i].e1 = 1;
	  res->std[i].e2 = 1;
	  res->std[i].e3 = 1;
	}

	return res;
}

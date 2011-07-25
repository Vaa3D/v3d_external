#ifndef SIM_MEASURE_H
#define SIM_MEASURE_H

#define VOID -1000000000

#include <math.h>
#include <iostream>
using namespace std;

void pick_max_n(int* idx, double* sim, int n, int siz);
int binSearch(double* max, double v, int start, int end);

//use euclidean distance as measurement of similarity
bool euxDist(double * qf, QList<double*> & featureList, int sbjnum, int * sbj, int fnum)
{
	int siz = featureList.size();
	double * dist = new double[siz];
	for (int i=0;i<siz;i++)
	{
		double * curr = featureList.at(i);
		dist[i] = 0;
		for (int j=0;j<fnum;j++)
			dist[i] += (curr[j]-qf[j])*(curr[j]-qf[j]);
		dist[i] *= -1; //this makes dist a measurement of similarity rather than difference
	}
	if (sbjnum>siz)
	{
		cerr<<"Too many subjects required!"<<endl;
		return false;
	}
	pick_max_n(sbj,dist,sbjnum,siz);
	delete dist;
	dist = NULL;
	return true;
}


//use cos function as measurement of similarity
bool cosSim(double *qf, QList<double*> & featureList, int sbjnum, int * sbj, int fnum)
{
	int siz = featureList.size();
	double * sim = new double[siz];
	for (int i=0;i<siz;i++)
	{
		double * curr = featureList.at(i);
		double qlen=0,slen=0;
		sim[i] = 0;
		for (int j=0;j<fnum;j++)
		{
			qlen += qf[j]*qf[j];
			slen += curr[j]*curr[j];
		}
		qlen = sqrt(qlen);
		slen = sqrt(slen);
		for (int j=0;j<fnum;j++)
			sim[i] += (curr[j]/slen)*(qf[j]/qlen);
	}
	if (sbjnum>siz)
	{
		cerr<<"Too many subjects required!"<<endl;
		return false;
	}
	pick_max_n(sbj,sim,sbjnum,siz);
	delete sim;
	sim = NULL;
	return true;
}


//normalize features by linear scale to unit range
//similarity of the normalized features are computed as the cosine of the two vectors' angel
bool unitRange(double *qf, QList<double*> & featureList, int sbjnum, int * sbj, int fnum)
{
	int siz = featureList.size();
	double * sim = new double[siz];
	double * min = new double[fnum];
	double * max = new double[fnum];
	double flen;
	for (int i=0;i<fnum;i++)
	{
		min[i] = qf[i];
		max[i] = qf[i];
		for (int j=0;j<siz;j++)
		{
			double * curr = featureList.at(j);
			min[i] = (min[i]<curr[i])?min[i]:curr[i];
			max[i] = (max[i]>curr[i])?max[i]:curr[i];
		}
	}
	for (int i=0;i<siz;i++)
	{
		sim[i] = 0;
		flen = 0;
		double * curr = featureList.at(i);
		
		for (int j=0;j<fnum;j++)
		{
			if (max[j]<=min[j]) {cout<<"invalid range"<<endl; continue;}
			double norm = (curr[j]-min[j])/(max[j]-min[j]);
			flen += norm*norm;
			sim[i] += norm*((qf[j]-min[j])/(max[j]-min[j]));
		}
		sim[i] /= sqrt(flen);
	}
	if (sbjnum>siz)
	{
		cout<<"Too many subjects required!"<<endl;
		return false;
	}
	pick_max_n(sbj,sim,sbjnum,siz);
	delete sim; sim = NULL;
	delete min; min = NULL;
	delete max; max = NULL;
	return true;
}



//normalize features by linear scale to unit variance
//similarity of the normalized features are computed as the cosine of the two vectors' angel
bool unitVar(double *qf, QList<double*> & featureList, int sbjnum, int * sbj, int fnum)
{
	int siz = featureList.size();
	double * sim = new double[siz];
	double * avg = new double[fnum];
	double * std = new double[fnum];
	double flen;
	for (int i=0;i<fnum;i++)
	{
		avg[i] += qf[i];
		for (int j=0;j<siz;j++)
		{
			double * curr = featureList.at(j);
			avg[i] += curr[i];
		}
		avg[i] /= siz+1;
	}

	for (int i=0;i<fnum;i++)
	{
		std[i] += (qf[i]-avg[i])*(qf[i]-avg[i]);
		for (int j=0;j<siz;j++)
		{
			double * curr = featureList.at(j);
			std[i] += (curr[i]-avg[i])*(curr[i]-avg[i]);
		}
		std[i] /= siz+1;
		std[i] = sqrt(std[i]);
	}

	for (int i=0;i<siz;i++)
	{
		sim[i] = 0;
		flen = 0;
		double * curr = featureList.at(i);
		for (int j=0;j<fnum;j++)
		{
			double norm = (curr[j]-avg[j])/(6*std[j])+0.5;
			if (norm<0) norm = 0;
			else if (norm>1) norm = 1;
			flen += norm*norm;
			sim[i] += norm*((qf[j]-avg[j])/(6*std[j])+0.5);
		}
		sim[i] /= sqrt(flen);
	}
	if (sbjnum>siz)
	{
		cerr<<"Too many subjects required!"<<endl;
		return false;
	}
	pick_max_n(sbj,sim,sbjnum,siz);
	delete sim; sim = NULL;
	delete avg; avg = NULL;
	delete std; std = NULL;
	return true;
}

//normalize features by rank score
bool rankScore(double *qf, QList<double*> & featureList, int sbjnum, int * sbj, int fnum)
{
	int siz = featureList.size();
	double * score = new double[siz];
	int * idx = new int[siz];
	double * dist =  new double[siz];

	for (int i=0;i<siz;i++) score[i] = 0;
	for (int j=0;j<fnum;j++)
	{
		for (int i=0;i<siz;i++)
		{
			double * curr = featureList.at(i);
			dist[i] = fabs(qf[j]-curr[j]);
		}
		//sort each feature according to their distance rank. more similar ones (with shorter dist to query) have higher score.
		pick_max_n(idx,dist,siz,siz);
		for (int i=0;i<siz;i++)
			score[(idx[i])] += i;
	}
	if (sbjnum>siz)
	{
		cerr<<"Too many subjects required!"<<endl;
		return false;
	}

	pick_max_n(sbj,score,sbjnum,siz);
	delete score; score = NULL;
	delete idx; idx = NULL;
	delete dist; dist = NULL;
	return true;

}


//this function computes the max n elements' index number in the array idx[]
//the elements stored in idx[] are aligned with a decrease of similarity
void pick_max_n(int* idx, double* sim, int n, int siz)
{
	double * sim_max = new double[n];
	int offset;
	for (int i=0;i<n;i++)
	{
		idx[i] = VOID;
		sim_max[i] = VOID;
	}
	for (int i=0;i<siz;i++)
	{
		offset = binSearch(sim_max, sim[i], 0, n-1);
		if (offset!=VOID)
		{
			for (int j=n-1;j>offset;j--)
			{
				sim_max[j] = sim_max[j-1];
				idx[j] = idx[j-1];
			}
			sim_max[offset] = sim[i];
			idx[offset] = i;
		}
	}
	delete sim_max;
	sim_max = NULL;
}

int binSearch(double* max, double v, int start, int end)
{
	if (v<max[end]) return VOID;
	if (v>max[start]) return 0;
	if (end==start+1) return end;
	if (end<=start) return start;

	int mid = ((end+start)%2==0)?((end+start)/2):((end+start+1)/2);
	if (v<=max[mid])
		return binSearch(max, v, mid, end);
	else
		return binSearch(max, v, start, mid);
}

#endif

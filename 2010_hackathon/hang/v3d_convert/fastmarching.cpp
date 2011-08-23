/******************************************************
 * perform_fastmarching
 *
 * parameters: seed  (the initial know points)
 *             phi   (the level set matrix)
 *             sz[0], sz[1], sz[2] ( the size )
 *
 * reference: 
 * Fast extraction of minimal paths in 3D images and application to virtual endoscopy.  
 *   T.Deschamps and L.D. Cohen. 
 *   September 2000. To appear in  Medical Image Analysis.
 *
 * Modification:
 * Oct 9, 2010 : check if phi[index] > solver_result, then heapchangevalue
 ******************************************************/
#ifndef __FASTMARCHING_CPP__
#define __FASTMARCHING_CPP__

#include "v3d_basicdatatype.h"

#include <cmath>  // sqrt
#include <cassert>
#include <vector>
#include <map>
#include <iostream> // cerr
using namespace std;

double upwind_solver(vector<double> &parameters, double p = 1.0);
double upwind_solver(double u1, double u2, double u3, double p);
double upwind_solver(double u2, double u3, double p);
double swap(double & a, double & b);

void heapInsert(int index, double value,vector<double> &trialValue, vector<int> &trialIndex, map<int, int> &trialLabel);
int heapDeleteMin(vector<double> &trialValue, vector<int> &trialIndex, map<int, int> &trialLabel);
void heapChangeValue(int index, double value, vector<double> &trialValue, vector<int> &trialIndex, map<int, int> &trialLabel);
void upHeap(int child_label, vector<double> &trialValue, vector<int> &trialIndex, map<int, int> &trialLabel);
void downHeap(int parent_label, vector<double> &trialValue, vector<int> &trialIndex, map<int, int> &trialLabel);
bool swapHeap(int child_label, int parent_label, vector<double> &trialValue, vector<int> &trialIndex, map<int, int> &trialLabel);


enum{ALIVE = -1, TRIAL = 0, FAR = 1};

static const double Inf = 1.0e+308;
static const double epsion = 0.000001;

bool perform_fastmarching(double * & phi, unsigned char* inimg1d, V3DLONG * &sz, double thresh)
{
	if(inimg1d == 0 || sz[0] <= 0 || sz[1] <= 0 || sz[2] <= 0) return false;
	V3DLONG tol_sz = sz[0] * sz[1] * sz[2];
	if(phi == 0) phi = new double[tol_sz];

	vector<V3DLONG> seed;
	// construct seed
	for(V3DLONG k = 0; k < sz[2]; k++)
	{
		for(V3DLONG j = 0; j < sz[1]; j++)
		{
			for(V3DLONG i = 0; i < sz[0]; i++)
			{
				V3DLONG x = k * sz[1] * sz[0] + j * sz[0] + i;
				if(inimg1d[x] < thresh) continue;
				for(V3DLONG kk = k -1; kk <= k + 1; kk++)
				{
					if(kk < 0 || kk >= sz[2]) continue;
					for(V3DLONG jj = j-1; jj <= j+1; jj++)
					{
						if(jj < 0 || jj >= sz[1]) continue;
						for(V3DLONG ii = i-1; ii < i+1; ii++)
						{
							if(ii < 0 || ii >= sz[0]) continue;
							if((ii -i)*(ii-i) + (jj -j)*(jj-j) + (kk-k)*(kk-k) > 1)continue;
							V3DLONG y = kk * sz[1] * sz[0] + jj * sz[0] + ii;
							if(inimg1d[y] < thresh){
								seed.push_back(x);	
								goto Exit1;
							}
						}
					}
				}
Exit1:
				x=x;
			}
		}
	}

#define COND1 (ii >= 1)
#define COND2 (ii < sz[0] - 1)
#define COND3 (jj >= 1)
#define COND4 (jj < sz[1] - 1)
#define COND5 (kk >= 1)
#define COND6 (kk < sz[2] - 1)
	// initialization
	vector<char> state (tol_sz, FAR);
	for(int i = 0; i < tol_sz; i++) phi[i] = Inf;
	for(int i = 0; i < seed.size(); i++) {state[seed[i]] = ALIVE; phi[seed[i]] = 0.0;}
	vector<double> trialValue;     // the trial's value store as minHeap, used to get the minimum value
	vector<int> trialIndex;       // the corresponding index, used to locate the minimum value's correspond index
	map<int, int> trialLabel;  // index -> sub label , used to locate the trial's location in trialValue with certain value when we change some trial point's value
	int label = 0;
	for(vector<V3DLONG>::iterator it = seed.begin(); it != seed.end(); it++)
	{
		V3DLONG x= *it;
		V3DLONG i = x % sz[0];
		V3DLONG j = x % (sz[1] * sz[0]) / sz[0];
		V3DLONG k = x / (sz[1] * sz[0]);
		for(V3DLONG kk = k - 1; kk <= k+1; kk++)
		{
			if(kk < 0 || kk >= sz[2]) continue;
			for(V3DLONG jj = j-1; jj <= j+1; jj++)
			{
				if(jj < 0 || jj >= sz[1]) continue;
				for(V3DLONG ii = i-1; ii <=i+1; ii++)
				{
					if(ii < 0 || ii >= sz[0]) continue;
					if((ii-i)*(ii-i) + (jj-j)*(jj-j) + (kk-k)*(kk-k) > 1) continue;
					V3DLONG y = kk * sz[1] * sz[0] + jj * sz[0] + ii;
					if(state[y] == FAR)
					{
						state[y] = TRIAL;
						double u1 = Inf;
						double u2 = Inf;
						double u3 = Inf;
						if(COND1 && state[y - 1] == ALIVE) u1 = u1 < phi[y - 1]? u1: phi[y -1 ];
						if(COND2 && state[y + 1] == ALIVE) u1 = u1 < phi[y + 1] ? u1: phi[y + 1];
						if(COND3 && state[y - sz[0]] == ALIVE) u2 = u2 < phi[y - sz[0]] ? u2:phi[y-sz[0]];
						if(COND4 && state[y + sz[0]] == ALIVE) u2 = u2 < phi[y + sz[0]] ? u2:phi[y + sz[0]];
						if(COND5 && state[y - sz[0]*sz[1]] == ALIVE) u3 = u3 < phi[y - sz[0]*sz[1]] ? u3: phi[y -sz[0]*sz[1]];
						if(COND6 && state[y + sz[0]*sz[1]] == ALIVE) u3 = u3 < phi[y + sz[0]*sz[1]] ? u3: phi[y + sz[0]*sz[1]];
						vector<double> parameters;
						if( u1 < Inf - 1) parameters.push_back(u1);
						if( u2 < Inf - 1) parameters.push_back(u2);
						if( u3 < Inf - 1) parameters.push_back(u3);
						phi[y] = upwind_solver(parameters);
						heapInsert(y, phi[y],trialValue, trialIndex, trialLabel);
					}
				}
			}
		}
	}
	// loop
	cout<<"trialValue size : "<<trialValue.size()<<endl;
	while(!trialValue.empty())
	{
		V3DLONG x = heapDeleteMin(trialValue, trialIndex, trialLabel);
		state[x] = ALIVE;
		V3DLONG i = x % sz[0];
		V3DLONG j = x % (sz[1] * sz[0]) / sz[0];
		V3DLONG k = x / (sz[1] * sz[0]);
		for(V3DLONG kk = k - 1; kk <= k+1; kk++)
		{
			if(kk < 0 || kk >= sz[2]) continue;
			for(V3DLONG jj = j-1; jj <= j+1; jj++)
			{
				if(jj < 0 || jj >= sz[1]) continue;
				for(V3DLONG ii = i-1; ii <=i+1; ii++)
				{
					if(ii < 0 || ii >= sz[0]) continue;
					if((ii-i)*(ii-i) + (jj-j)*(jj-j) + (kk-k)*(kk-k) > 1) continue;
					V3DLONG y = kk * sz[1] * sz[0] + jj * sz[0] + ii;

					if(state[y] != ALIVE)
					{
						double u1 = Inf;
						double u2 = Inf;
						double u3 = Inf;
						if(COND1 && state[y - 1] == ALIVE) u1 = u1 < phi[y - 1]? u1: phi[y -1 ];
						if(COND2 && state[y + 1] == ALIVE) u1 = u1 < phi[y + 1] ? u1: phi[y + 1];
						if(COND3 && state[y - sz[0]] == ALIVE) u2 = u2 < phi[y - sz[0]] ? u2:phi[y-sz[0]];
						if(COND4 && state[y + sz[0]] == ALIVE) u2 = u2 < phi[y + sz[0]] ? u2:phi[y + sz[0]];
						if(COND5 && state[y - sz[0]*sz[1]] == ALIVE) u3 = u3 < phi[y - sz[0]*sz[1]] ? u3: phi[y -sz[0]*sz[1]];
						if(COND6 && state[y + sz[0]*sz[1]] == ALIVE) u3 = u3 < phi[y + sz[0]*sz[1]] ? u3: phi[y + sz[0]*sz[1]];
						vector<double> parameters;
						if( u1 < Inf - 1) parameters.push_back(u1);
						if( u2 < Inf - 1) parameters.push_back(u2);
						if( u3 < Inf - 1) parameters.push_back(u3);
						double solver_result = upwind_solver(parameters);
						if(state[y] == FAR)
						{
							phi[y] = solver_result;
							heapInsert(y, phi[y],trialValue, trialIndex, trialLabel);
							state[y] = TRIAL;
						}
						else if(state[y] == TRIAL)
						{
							if(phi[y] > solver_result)
							{
								phi[y] = solver_result;
								heapChangeValue(y,phi[y],trialValue, trialIndex, trialLabel);
							}
						}
					}
				}
			}
		}
	}
	
	return true;
}

// solve (u - U1)^2 + (u - U2)^2 + (u - U3)^2 = 1
// assume u1 >= u2 >= u3
// return u
double upwind_solver(vector<double> &parameters, double p)
{
	if(parameters.size() == 3) return upwind_solver(parameters[0], parameters[1], parameters[2], p);
	else if(parameters.size() == 2) return upwind_solver(parameters[0], parameters[1], p);
	else if(parameters.size() == 1) return parameters[0] + abs(p);
	else 
	{
		cerr<<"upwind_solver : parameters with zero element"<<endl;
		exit(0);
	}
}

double upwind_solver(double u1, double u2, double u3, double p)
{
	if(u1 < u2)  swap(u1,u2);
	if(u1 < u3) swap(u1,u3);
	if(u2 < u3) swap (u2,u3);

	double a = 3;
	double b = -2 * (u1 + u2 + u3);
	double c = u1*u1 + u2*u2 + u3*u3 - p*p;
	double delta = b*b - 4*a*c;
	if(delta >= 0.0) 
	{
		double x1 = -b /(2*a);
		double x2 = sqrt(delta)/(2*a);
		double u = x2 > 0 ? (x1 + x2):(x1-x2);
		if(u > u1) return u;
		else 
			return upwind_solver(u2,u3,p);
	}
	else 
		return upwind_solver(u2,u3,p);

}

double upwind_solver(double u2, double u3, double p)
{
	if(u2 < u3) swap(u2,u3);

	double a = 2;
	double b = -2 * (u2 + u3);
	double c = u2*u2 + u3*u3 - p*p;
	double delta = b*b - 4*a*c;
	if(delta >= 0.0)
	{
		double x1 = -b/(2*a);
		double x2 = sqrt(delta)/(2*a);
		double u = x2 > 0 ? (x1 + x2):(x1-x2);
		if(u > u2) return u;
		else return (u3 + abs(p));
	}
	else return (u3 + abs(p));
}

double swap(double & a, double & b)
{
	double temp = a;
	a = b;
	b = temp;
}

void heapInsert(int index, double value,vector<double> &trialValue, vector<int> &trialIndex, map<int, int> &trialLabel)
{
	if(trialValue.empty())
	{
		trialValue.push_back(value);
		trialIndex.push_back(index);
		trialLabel[index] = trialIndex.size() - 1;
	}
	else
	{
		int currentLabel = trialValue.size();
		trialValue.push_back(value);
		trialIndex.push_back(index);
		trialLabel[index] = currentLabel;
		upHeap(currentLabel,trialValue, trialIndex, trialLabel);
	}
}

// delete the root with minimum value, and return the min index;
int heapDeleteMin(vector<double> &trialValue, vector<int> &trialIndex, map<int, int> &trialLabel)
{
	assert(!trialValue.empty());
	int minIndex = trialIndex[0];
	if(trialValue.size() == 1) 
	{
		trialValue.clear();
		trialIndex.clear();
		trialLabel.clear();
	}
	else
	{
		int lastLabel = trialValue.size() - 1;
		int lastIndex = trialIndex[lastLabel];
		double lastValue = trialValue[lastLabel];
		trialValue[0] = lastValue;
		trialIndex[0] = lastIndex;
		trialLabel[lastIndex] = 0;

		trialValue.erase(trialValue.begin() + lastLabel);
		trialIndex.erase(trialIndex.begin() + lastLabel);
		trialLabel.erase(trialLabel.find(minIndex));
		downHeap(0, trialValue, trialIndex, trialLabel);
	}
	return minIndex;
}

void heapChangeValue(int index, double value, vector<double> &trialValue, vector<int> &trialIndex, map<int, int> &trialLabel)
{
	int label = trialLabel[index];
	double old_value = trialValue[label];
	trialValue[label] = value;
	if(value < old_value) upHeap(label, trialValue, trialIndex, trialLabel);
	else if(value > old_value) downHeap(label, trialValue, trialIndex, trialLabel);
}

void upHeap(int child_label, vector<double> &trialValue, vector<int> &trialIndex, map<int, int> &trialLabel)
{
	if(child_label == 0) return;
	int parent_label = (child_label+1)/2 - 1;
	if(swapHeap(child_label, parent_label, trialValue, trialIndex, trialLabel)) 
		upHeap(parent_label, trialValue, trialIndex, trialLabel);
}

void downHeap(int parent_label, vector<double> &trialValue, vector<int> &trialIndex, map<int, int> &trialLabel)
{
	int child_label1 = 2 * (parent_label + 1) -1;
	int child_label2 = 2 * (parent_label + 1) ;
	if(child_label1 > trialValue.size() -1) return ;
	// only one child
	if(child_label1 == trialValue.size() - 1) 
	{
		swapHeap(child_label1, parent_label, trialValue, trialIndex, trialLabel);
	}
	else
	{
		double child_value1 = trialValue[child_label1];
		double child_value2 = trialValue[child_label2];
		int child_label = child_value1 < child_value2 ? child_label1 : child_label2;
		if(swapHeap(child_label, parent_label, trialValue, trialIndex, trialLabel))
			downHeap(child_label, trialValue, trialIndex, trialLabel);
	}
}

// return true means the potential to continue swap
// return false means unable to swap;
bool swapHeap(int child_label, int parent_label, vector<double> &trialValue, vector<int> &trialIndex, map<int, int> &trialLabel)
{
	double child_value = trialValue[child_label];
	int child_index = trialIndex[child_label];

	int parent_index = trialIndex[parent_label];
	double parent_value = trialValue[parent_label];

	if(child_value < parent_value) 
	{
		trialValue[parent_label] = child_value;
		trialIndex[parent_label] = child_index;
		trialLabel[child_index] = parent_label;

		trialValue[child_label] = parent_value;
		trialIndex[child_label] = parent_index;
		trialLabel[parent_index] = child_label;
		return true;
	}
	return false;
}

#endif

#include "compute.h"
#include <math.h>
#include <iostream>
using namespace std;

#define VOID 1000000000
#define PI 3.14159265359
#define min(a,b) (a)<(b)?(a):(b)
#define max(a,b) (a)>(b)?(a):(b)

static double Width=0, Height=0, Depth=0, Length=0, Volume=0, Surface=0, Hausdorff=0;
static int N_node=0, N_stem=0, N_bifs=0, N_branch=0, N_tips=0;
static double Pd_ratio=0, Contraction=0, Fragmentation=0, Fractal_Dim=0;

void computeFeature(const NeuronTree & nt, double * features)
{
	Width=0, Height=0, Depth=0, Length=0, Volume=0, Surface=0, Hausdorff=0;
	N_node=0, N_stem=0, N_bifs=0, N_branch=0, N_tips=0;
	Pd_ratio=0, Contraction=0, Fragmentation=0, Fractal_Dim=0;
	QList<NeuronSWC> list = nt.listNeuron;
	QHash<int, int> LUT = QHash<int, int>();
	for (int i=0;i<list.size();i++)
		LUT.insert(list.at(i).n,i);
	N_node = list.size();
	computeLinear(list, LUT);
	computeTree(list,LUT);
	Hausdorff = computeHausdorff(list,LUT);
	
	//feature # 0: N_node
	features[0] = N_node;
	//feature # 1: N_stem
	features[1] = N_stem;
	//feature # 2: N_bifs
	features[2] = N_bifs;
	//feature # 3: N_branch
	features[3] = N_branch;
	//feature # 4: N_tips
	features[4] = N_tips;
	//feature # 5: Width
	features[5] = Width;
	//feature # 6: Height
	features[6] = Height;
	//feature # 7: Depth
	features[7] = Depth;
	//feature # 8: Length
	features[8] = Length;
	//feature # 9: Volume
	features[9] = Volume;
	//feature # 10: Surface
	features[10] = Surface;	
	//feature # 11: Contraction
	features[11] = Contraction;
	//feature # 12: Fragmentation
	features[12] = Fragmentation;
	//feature # 13: Pd_ratio
	features[13] = Pd_ratio;
	//feature # 14: Hausdorff
	features[14] = Hausdorff;
	//feature # 15: Fractal_Dim
	features[15] = Fractal_Dim;
}

int getParent(int n, QList<NeuronSWC> & list, QHash<int,int> & LUT)
{
	int pid = list.at(n).pn;
	if (pid==-1) return VOID;
	return (LUT.value(pid));
}

QList<int> getChild(int t, QList <NeuronSWC> & list, QHash<int,int> & LUT)
{
	QList<int> childlist = QList<int>();
	for (int i=0;i<list.size();i++)
	{	int pan = list.at(i).pn;
		if (pan==-1) continue;
		if (LUT.value(pan)==t)
			childlist.append(i);
	}
	return childlist;
}

//do a search along the list to compute total width, height, depth, length, surface and volume.
void computeLinear(QList<NeuronSWC> & list, QHash<int,int> & LUT)
{
	double xmin,ymin,zmin;
	xmin = ymin = zmin = VOID;
	double xmax,ymax,zmax;
	xmax = ymax = zmax = 0;
	
	for (int i=0;i<list.size();i++)
	{
		NeuronSWC curr = list.at(i);
		xmin = min(xmin,curr.x); ymin = min(ymin,curr.y); zmin = min(zmin,curr.z);
		xmax = max(xmax,curr.x); ymax = max(ymax,curr.y); zmax = max(zmax,curr.z);
		int parent = getParent(i,list,LUT);
		if (parent==VOID) continue;
		double l = dist(curr,list.at(parent));
		Length += l;
		Surface += 2*PI*curr.r*l;
		Volume += PI*curr.r*curr.r*l;
	}
	Width = xmax-xmin;
	Height = ymax-ymin;
	Depth = zmax-zmin;
}

//do a search along the tree to compute N_stems, N_bif, N_tips, Contraction, Fragmentation and Fractal_Dim
void computeTree(QList<NeuronSWC> &list, QHash<int,int> & LUT)
{
	//find the root
	int rootidx = VOID;
	for (int i=0;i<list.size();i++)
		if (list.at(i).pn==-1) rootidx = i;
	if (rootidx==VOID){
		cerr<<"the input neuron tree does not have a root, please check your data"<<endl;
		return;
	}
	N_stem = getChild(rootidx,list,LUT).size();
	QList<double> path = QList<double>();
	QList<double> eud = QList<double>();
	QList<int> frag = QList<int>();
	QList<double> frac_d = QList<double>();


	QStack<int> stack = QStack<int>();
	stack.push(rootidx);
	while (!stack.isEmpty())
	{
		int t = stack.pop();
		QList<int> child = getChild(t,list,LUT);
		int tmp;
		for (int i=0;i<child.size();i++)
		{
			N_branch++;
			tmp = child.at(i);
			Pd_ratio += list.at(tmp).r/list.at(t).r;
			double pathlength = dist(list.at(tmp),list.at(t));
			int fragment = 1;
			QList<double> pathfr = QList<double>();
			QList<double> eufr = QList<double>();
			pathfr.append(pathlength);
			eufr.append(pathlength);

			while (getChild(tmp,list,LUT).size()==1)
			{
				int ch = getChild(tmp,list,LUT).at(0);
				pathlength += dist(list.at(ch),list.at(tmp));
				pathfr.append(pathlength);
				eufr.append(dist(list.at(t),list.at(ch)));
				fragment++;
				tmp = ch;
			}
			//we are reaching a tip point or another branch point, computation for this branch is over
			if (pathlength!=0)
			{
				path.append(pathlength);
				eud.append(eufr.last());
			}
			frag.append(fragment);
			if (fragment>1)
			{
			double ll = loglog(eufr,pathfr,fragment);
			if (ll!=VOID)
				frac_d.append(ll);
			}
			if (getChild(tmp,list,LUT).size()==0)//tip
				N_tips++;
			else if (getChild(tmp,list,LUT).size()>1)//another branch
			{
				N_bifs++;
				stack.push(tmp);
			}
		}
	}

	Fragmentation = Contraction = Fractal_Dim = 0;
	for (int i=0;i<N_branch;i++)
		Fragmentation += frag.at(i);
	Fragmentation /= N_branch;

	Pd_ratio /= N_branch;

	for (int i=0;i<path.size();i++)
		Contraction += eud.at(i)/path.at(i);
	Contraction /= path.size();

	for (int i=0;i<frac_d.size();i++)
	{
		Fractal_Dim += frac_d.at(i);
	}
	Fractal_Dim /= frac_d.size();
}


//compute Hausdorff dimension
double computeHausdorff(QList <NeuronSWC> & list, QHash<int,int> & LUT)
{
#define LMINMAX 2
#define LMAXMIN 1
#define NCELL 5000        // max# nonempty lattice cells in Hausdorff analysis

	int n;

	short **r1, **r2;

	n = list.size();
	r1 = matrix(3,n);
	r2 = matrix(3,n);

	n = fillArray(list, LUT, r1,  r2);

	int i,  k, k1, l, m, cnt, dl, lmin , lmax;
	short r[3], rr[3], **cell;

	int scale;
	float dr[3], rt[3], total;
	float hd, measure[25], length;

	length=0;
	lmin=0;
	lmax=0;
	for (i=1; i<n; i++) 
		for (k=0; k<3; k++)
		{
			lmin += abs(r1[k][i]-r2[k][i]);
			if (lmax<abs(r2[k][i]-r1[k][1])) lmax=abs(r2[k][i]-r1[k][1]);
		}
	lmin /= LMINMAX*n;
	lmax /= 2;
	/*------------start with lattice cell >= lmin ------------*/
	if (lmin<1) lmin=1;
	else if (lmin>1)
	{
		lmax /= lmin;
		for (i=1; i<n; i++) for (k=0; k<3; k++)
		{
			r1[k][i] /= lmin;
			r2[k][i] /= lmin;
		}
	}
	if (lmax<=1) return(0.0);
	scale=lmin;
	cnt=0;

	cell = matrix(NCELL,3);
	/*-----------------------------------------------------main loop begin----------------------*/
	while (lmax>LMAXMIN)
	{
		for (k=0; k<3; k++) r[k]=r1[k][1];
		m = mark(0, r, cell);
		for (i=1; i<n; i++) if ((r1[0][i]!=r2[0][i]) ||
				(r1[1][i]!=r2[1][i]) ||
				(r1[2][i]!=r2[2][i]))
		{
			/*-------------------------tracing link-------*/
			total=0.0;
			for (k=0; k<3; k++) total += abs(r2[k][i]-r1[k][i]);
			for (k=0; k<3; k++) 
			{
				r[k]=r1[k][i];
				dr[k]=(r2[k][i]-r[k])/total;
				rt[k]=dr[k];
			}
			m=mark(m, r, cell);
			while((r[0]!=r2[0][i]) ||
					(r[1]!=r2[1][i]) ||
					(r[2]!=r2[2][i]))
			{
				l=0;
				k1=-1;
				for (k=0; k<3; k++) rr[k]=r2[k][i]-r[k];
				for (k=0; k<3; k++)
				{
					if ((rt[k]*rr[k]>0) && (abs(l)<abs(rr[k])))
					{
						l=rr[k];
						k1=k;
					}
				}
				if (l>0) 
				{
					r[k1]++;
					rt[k1]--;
				}
				else
				{
					r[k1]--;
					rt[k1]++;
				}
				for (k=0; k<3; k++) rt[k] += dr[k];
				m=mark(m, r, cell);
				if (m>=NCELL) cerr<<"maximal cell number reached"<<endl;;
				if (m>=NCELL) exit(1);
			}

		}
		measure[cnt]=m;
		cnt++;

		for (i=1; i<n; i++) for (k=0; k<3; k++)
		{
			r1[k][i] /= 2;
			r2[k][i] /= 2;
		}
		lmax /= 2;
		scale *=2;
	}
	/*-----------------------------main loop end-------------------------*/
	free_matrix(r1,3,n);
	free_matrix(r2,3,n);
	free_matrix(cell,NCELL,3);
	/*-----------------------------computing Hausdorff dimension---------*/
	hd=0;
	for (i=0; i<cnt; i++) hd += (i-0.5*(cnt-1))*log(measure[i]);
	hd *= -12.0/(cnt*(cnt*cnt-1.0))/log(2.0);
	return(hd);
}


int fillArray(QList<NeuronSWC> & list, QHash<int,int> & LUT, short** r1, short** r2){

	int siz = list.size();
	for (int t=0;t<siz;t++)
	{
		int s = getParent(t,list,LUT);
		if(s==VOID) s = t;
		int cst=1;
		r2[0][t]=(short)list.at(s).x+cst;
		r2[1][t]=(short)list.at(s).y+cst;
		r2[2][t]=(short)list.at(s).z+1;
		r1[0][t]=(short)list.at(t).x+cst;
		r1[1][t]=(short)list.at(t).y+cst;
		r1[2][t]=(short)list.at(t).z+1;

	}
	return siz;
}
short **matrix(int n,int m)
{
	int i;
	short **mat = new short*[n];
	for (int i=0;i<n;i++)
	{
		mat[i] = new short[m];
		for (int j=0;j<m;j++)
			mat[i][j] = 0;
	}
	/* Return pointer to array of pointers to rows. */
	return mat;

}

void free_matrix(short **mat,int n,int m)

	/* Free a float matrix allocated by matrix(). */
{
	int i;

	for (i = 0; i<n ; i++)
	{
		delete(mat[i]);
		mat[i] = NULL;
	}
	delete(mat);
	mat = NULL;
}
/*********************** mark lattice cell r, keep marked set ordered */
int mark(int m, short r[3], short ** c)
{
	int i, j, k;
	if (m<=0)
		for (k=0; k<3; k++) c[0][k]=r[k]; /*--initialize the pool of marked cells--*/
	else
	{
		for (i=0; i<m; i++) 
		{
			if (c[i][0]==r[0] &&
					c[i][1]==r[1] &&
					c[i][2]==r[2]) return(m); /*--- already marked ---*/
			if (c[i][0]>=r[0] &&
					c[i][1]>=r[1] &&
					c[i][2]>r[2]) break; /*--- insert into ordered set ---*/
		}
		if (i<m) for (j=m; j>i; j--) for (k=0; k<3; k++) c[j][k]=c[j-1][k];
		for (k=0; k<3; k++) c[i][k]=r[k];
	}

	return(m+1);

}


double dist(const NeuronSWC & s1, const NeuronSWC & s2)
{
	double x1 = s1.x, y1 = s1.y, z1 = s1.z;
	double x2 = s2.x, y2 = s2.y, z2 = s2.z;
	return (sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)+(z1-z2)*(z1-z2)));
}

double loglog(QList<double> & x, QList<double> & y, int n)
{
	double sumx=0, sumy=0, sumxy=0, sumx2=0;
	for (int i=0;i<n;i++)
	{
		//cout<<i<<" "<<x.at(i)<<" "<<y.at(i)<<"\n";
		double logx = log(x.at(i));
		double logy = log(y.at(i));
		sumx += logx; sumy += logy;
		sumx2 += logx*logx;
		sumxy += logx*logy;
	}
	if (n*sumx2-sumx*sumx==0) return VOID;
	double b=(n*sumxy-sumx*sumy)/(n*sumx2-sumx*sumx);
	return fabs(b);
}

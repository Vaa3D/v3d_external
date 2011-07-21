#define FNUM 16

#include "Nfmain.h"
#include "compute.h"

#include <vector>
#include <math.h>
#include <iostream>
using namespace std;

void nf_main(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
	char * out = (*(vector<char*>*)(output.at(0).p)).at(0);
	QString outfileName = QString(out);
	QFile file(outfileName);
	file.open(QIODevice::WriteOnly|QIODevice::Text);
	QTextStream myfile(&file);

	myfile <<"id\tFile Name\tN_Node\tN_stem\tN_bifs\tN_branch\tN_tips\tWidth\tHeight\tDepth\tLength\tVolume\tSurface\tContraction\tFragmentation\tPd_ratio\tHausdorff\tFractal_Dim"<<endl;

	int neuronNum = (int)inlist->size();

	for (int i=0;i<neuronNum;i++)
	{
		QString name = QString(inlist->at(i));
		NeuronTree nt = readSWC_file(name);
		
		cout<<"\n--------------Neuron #"<<(i+1)<<"----------------\n";
		myfile<<(i+1)<<"\t"<<name<<"\t";
		double * features = new double[FNUM];
		computeFeature(nt, features);
		printFeature(features);
		for (int jj=0;jj<FNUM;jj++)
			myfile<<features[jj]<<"\t";
		myfile<<endl;
	}
	file.close();
}

void printFeature(double * features)
{

	for (int i=0;i<FNUM;i++)
	{
		switch (i)
		{
			case 0:
				cout<<"Number of Nodes: ";
				break;
			case 1:
				cout<<"Number of Stems: ";
				break;
			case 2:
				cout<<"Number of Bifurcatons: ";
				break;
			case 3:
				cout<<"Number of Branches: ";
				break;
			case 4:
				cout<<"Number of Tips: ";
				break;
			case 5:
				cout<<"Width: ";
				break;
			case 6:
				cout<<"Height: ";
				break;
			case 7:
				cout<<"Depth: ";
				break;
			case 8:
				cout<<"Total Length: ";
				break;
			case 9:
				cout<<"Total Volume: ";
				break;
			case 10:
				cout<<"Surface Area: ";
				break;
			case 11:
				cout<<"Average Contraction: ";
				break;
			case 12:
				cout<<"Average Fragmentation: ";
				break;
			case 13:
				cout<<"Average Parent-daughter Ratio: ";
				break;
			case 14:
				cout<<"Hausdorff Dimension: ";
				break;
			case 15:
				cout<<"Fractal Dimension: ";
		}
		cout<<features[i]<<endl;
	}
}

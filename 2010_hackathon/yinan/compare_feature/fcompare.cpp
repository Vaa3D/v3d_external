#define FNUM 16

#include "fcompare.h"
#include "compute.h"
#include "sim_measure.h"

#include <vector>
#include <math.h>
#include <iostream>
using namespace std;

bool compare_feature(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
	char * out = (*(vector<char*>*)(output.at(0).p)).at(0);

	QString outfileName = QString(out);
	QFile file(outfileName);
	file.open(QIODevice::WriteOnly|QIODevice::Text);
	QTextStream myfile(&file);


	if (inlist->size()==1)
	{
		cerr<<"Please specify query id and number of subjects!"<<endl;
		return false;
	}

	vector<char*>* par = (vector<char*>*)(input.at(1).p);
	if (par->size()!=2)
	{
		cerr<<"Wrong parameter list!"<<endl;
		return false;
	}

	bool ok;
	int queryid = QString(par->at(0)).toInt(&ok, 10);
	if (!ok)
	{
		cerr<<"Error in query id!"<<endl;
		return false;
	}
	cout<<"query id:\t"<<queryid<<endl;

	int sbjnum = QString(par->at(1)).toInt(&ok, 10);
	if (!ok)
	{
		cerr<<"Error in subject number!"<<endl;
		return false;
	}
	cout<<"subject number:\t"<<sbjnum<<endl;
		
	int neuronNum = (int)inlist->size();
	QList<double*> featureList = QList<double*>();
	QList<QString> nameList = QList<QString>();
	double * query;
	QString queryName;

	for (int i=0;i<neuronNum;i++)
	{
		QString name = QString(inlist->at(i));
		NeuronTree nt = readSWC_file(name);
		
		cout<<"\n--------------Neuron #"<<(i+1)<<"----------------\n";
		cout<<inlist->at(i)<<endl;
		double * features = new double[FNUM];
		computeFeature(nt, features);
		for (int jj=0;jj<FNUM;jj++)
			cout<<features[jj]<<"\t";
		featureList.append(features);
		nameList.append(name);
	}
	query = featureList.at(queryid);
	queryName = nameList.at(queryid);
	//featureList.removeAt(queryid);
	//nameList.removeAt(queryid);

	int* sbj = new int[sbjnum];
	bool result = unitRange(query, featureList, sbjnum, sbj, FNUM);
	if (result)
	{
		myfile<<"query id:\t"<<queryid<<endl;
		myfile<<"query name:\t"<<queryName<<endl;
		myfile<<"pick:\t"<<sbjnum<<endl;
		for (int i=0;i<sbjnum;i++)
			myfile<<sbj[i]<<endl;
		for (int i=0;i<sbjnum;i++)
			myfile<<nameList.at(sbj[i])<<" ";
		myfile<<endl;
	}
	
	file.close();
	return result;
}

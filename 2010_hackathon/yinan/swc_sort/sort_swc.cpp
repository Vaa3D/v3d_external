/*
 *  sort_swc.cpp
 *  sort_swc 
 *
 *  Created by Wan, Yinan, on 06/20/11.
 *  Last change: Wan, Yinan, on 06/23/11.
 */

#include <QtGlobal>

#include "sort_swc.h"
#include "v3d_message.h" 
#include "../../../v3d_main/basic_c_fun/basic_surf_objs.h"
#include <unistd.h>
#include <iostream>
using namespace std;


//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(sort_swc, SORT_SWCPlugin);


//plugin funcs
const QString title = "sort_swc";

QStringList SORT_SWCPlugin::menulist() const
{
    return QStringList() 
	<<tr("sort_swc")
	<<tr("Help");
}

QStringList SORT_SWCPlugin::funclist() const
{
	return QStringList()
		<<tr("sort_swc");
}

QHash<V3DLONG, V3DLONG> ChildParent(const NeuronTree &neurons, const QList<V3DLONG> & idlist, const QHash<V3DLONG,V3DLONG> & LUT) 
{
	QHash<V3DLONG, V3DLONG> cp;
	for (V3DLONG i=0;i<neurons.listNeuron.size(); i++)
		if (neurons.listNeuron.at(i).pn==-1) cp.insertMulti(idlist.indexOf(LUT.value(neurons.listNeuron.at(i).n)), -1);
		else cp.insertMulti(idlist.indexOf(LUT.value(neurons.listNeuron.at(i).n)), idlist.indexOf(LUT.value(neurons.listNeuron.at(i).pn))); 
	return cp;
}

QHash<V3DLONG, V3DLONG> getUniqueLUT(const NeuronTree &neurons)
{
	QHash<V3DLONG,V3DLONG> LUT;
	for (V3DLONG i=0;i<neurons.listNeuron.size();i++)
	{
		V3DLONG j;
		for (j=0;j<i;j++)
		{
			if (neurons.listNeuron.at(i).x==neurons.listNeuron.at(j).x && neurons.listNeuron.at(i).y==neurons.listNeuron.at(j).y && neurons.listNeuron.at(i).z==neurons.listNeuron.at(j).z)		break;
		}
		
		LUT.insertMulti(neurons.listNeuron.at(i).n,j);
	}
	return (LUT);
}


void DFS(bool** matrix, V3DLONG* neworder, V3DLONG node, V3DLONG* id, V3DLONG siz, bool* numbered)
{
	numbered[node] = true;
	neworder[*id] = node;
	(*id)++;
	for (V3DLONG v=0;v<siz;v++)
		if (!numbered[v] && matrix[v][node])
		{
			DFS(matrix, neworder, v, id, siz,numbered);
		}
}
	
bool SortSWC(const NeuronTree & neurons, QList<NeuronSWC> & lN, V3DLONG newrootid)
{
	//double check the data to ensure it is correct!
	//for (V3DLONG ii=0; ii<neurons.listNeuron.size(); ii++)
	//{
	//	p_cur = (NeuronSWC *)(&(neurons.listNeuron.at(ii)));
		//v3d_msg(QString("x %1 y %2 z %3 r %4\n").arg(p_cur->x).arg(p_cur->y).arg(p_cur->z).arg(p_cur->r),0);
		
		//if (p_cur->x<0 || p_cur->y<0 || p_cur->z<0 || p_cur->r<0)
		//{
	//		v3d_msg("You have illeagal x,y,z coordinates or radius values. Check your data.");
	//		return;
	//	}
	//}
	//create a LUT, from the original id to the position in the listNeuron, different neurons with the same x,y,z & r are merged into one position
	QHash<V3DLONG, V3DLONG> LUT = getUniqueLUT(neurons);

	//create a new id list to give every different neuron a new id		
	QList<V3DLONG> idlist = ((QSet<V3DLONG>)LUT.values().toSet()).toList();

	//create a child-parent table, both child and parent id refers to the index of idlist
	QHash<V3DLONG, V3DLONG> cp = ChildParent(neurons,idlist,LUT);
	
	
	V3DLONG siz = idlist.size();

	
	bool** matrix = new bool*[siz];
	for (V3DLONG i = 0;i<siz;i++)
	{
		matrix[i] = new bool[siz];
		for (V3DLONG j = 0;j<siz;j++) matrix[i][j] = false;
	}
	

	//generate the adjacent matrix for undirected matrix
	for (V3DLONG i = 0;i<siz;i++)
	{
		QList<V3DLONG> parentSet = cp.values(i); //id of the ith node's parents
		for (V3DLONG j=0;j<parentSet.size();j++)
		{
			V3DLONG v2 = (V3DLONG) (parentSet.at(j));
			if (v2==-1) continue;
			matrix[i][v2] = true;
			matrix[v2][i] = true;
		}
	}
	
	
	//do a DFS for the the matrix and re-allocate ids for all the nodes
	V3DLONG root= idlist.indexOf(LUT.value(newrootid));
	
	if (LUT.keys().indexOf(newrootid)==-1)
	{
		v3d_msg("The new root id you have chosen does not exist in the SWC file.");
		return(false);
	}
	
	V3DLONG* neworder = new V3DLONG[siz];
	bool* numbered = new bool[siz];
	for (V3DLONG i=0;i<siz;i++) numbered[i] = false;
	
	V3DLONG id[] = {0};

	DFS(matrix,neworder,root,id,siz,numbered);
	
	if ((*id)<siz) {
		v3d_msg("The root you have chosen cannot reach all other nodes in neuron tree. Show the connected component only.", 0);
		siz = (*id);
		}
	else if ((*id)==siz)
		v3d_msg("The neuronTree is connected. Show re-sorted result.", 0);

			
		NeuronSWC S;
		S.n = 1;
		S.pn = -1;
		V3DLONG oripos = LUT.value(newrootid);
		S.x = neurons.listNeuron.at(oripos).x;
		S.y = neurons.listNeuron.at(oripos).y;
		S.z = neurons.listNeuron.at(oripos).z;
		S.r = neurons.listNeuron.at(oripos).r;
		S.type = neurons.listNeuron.at(oripos).type;
		lN.append(S);
		
		for (V3DLONG ii = 1;ii<siz;ii++)
		{
			for (V3DLONG jj=0;jj<ii;jj++) //after DFS the id of parent must be less than child's
			{
				if (matrix[neworder[ii]][neworder[jj]]) 
				{
					NeuronSWC S;
					S.n = ii+1;
					S.pn = jj+1;
					V3DLONG oripos = idlist.at(neworder[ii]);
					S.x = neurons.listNeuron.at(oripos).x;
					S.y = neurons.listNeuron.at(oripos).y;
					S.z = neurons.listNeuron.at(oripos).z;
					S.r = neurons.listNeuron.at(oripos).r;
					S.type = neurons.listNeuron.at(oripos).type;
					lN.append(S);
				}
			}
		}
	return(true);
}

void sort_swc(V3DPluginCallback2 &callback, QWidget *parent, int method_code)
{
	NeuronTree neuron;
	QString fileOpenName;
	if (method_code == 1)
	{
		fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
												"",
												QObject::tr("Supported file (*.swc)"
															";;Neuron structure	(*.swc)"
															));
		if(fileOpenName.isEmpty()) 
		{
			v3d_msg("You don't have any SWC file open in the main window.");
			return;
		}
		
		
		if (fileOpenName.size()>0)
		{
			neuron = readSWC_file(fileOpenName);
			QList<NeuronSWC> lN;		
			int rootid;
			bool ok;
			rootid = QInputDialog::getInteger(parent, "input root number","New root number:",1,1,neuron.listNeuron.size(),1,&ok);
		if (ok){
			if (SortSWC(neuron, lN,rootid)){

			//write new SWC to file
			QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
												"",
												QObject::tr("Supported file (*.swc)"
															";;Neuron structure	(*.swc)"
															));
			QFile file(fileSaveName);
			file.open(QIODevice::WriteOnly|QIODevice::Text);
			QTextStream myfile(&file);
			for (V3DLONG i=0;i<lN.size();i++)
				myfile << lN.at(i).n <<" " << lN.at(i).type << " "<< lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn << "\n";
	
			file.close();
			}
		    }
		}
		else 
		{
			v3d_msg("You don't have any SWC file open in the main window.");
			return;
		}
		
	}
}

void SORT_SWCPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("sort_swc"))
	{
    		sort_swc(callback, parent,1 );
    	}
	else if (menu_name == tr("Help"))
	{
		v3d_msg("(version 0.14) Set a new root and sort the SWC file into a new order, where the newly set root has the id of 1, and the parent's id is less than its child's. If the original SWC has more than one connected components, return the sorted result of the brench with newly set root. It also includes a merging process of neuron segments, where neurons with the same x,y,z cooridinates are combined as one.");
		return;
	}
}

bool sort_func(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout<<"==========Welcome to sort_swc function============="<<endl;
	NeuronTree neuron;
	char * infile = (*(vector<char*>*)(input.at(0).p)).at(0);
	char * outfile = (*(vector<char*>*)(output.at(0).p)).at(0);
	char * paras = (*(vector<char*>*)(input.at(1).p)).at(0);
	
	cout<<"infile: "<<infile<<endl;
	cout<<"outfile: "<<outfile<<endl;
	cout<<"new root id: "<<paras<<endl;
	
	QString fileOpenName = QString(infile);
	QString fileSaveName = QString(outfile);
	if(fileOpenName.isEmpty()) 
	{
		cout<<"You don't have any SWC file open in the main window."<<endl;
		return false;
	}
	
	if (fileOpenName.size()>0)
	{
		neuron = readSWC_file(fileOpenName);
		QList<NeuronSWC> lN;			
		int rootid = 0;
		for (int i=0;i<strlen(paras);i++)
		{
			rootid = rootid*10 + paras[i]-'0';
		}
		if (SortSWC(neuron, lN,rootid)){
			//write new SWC to file
			QFile file(fileSaveName);
			file.open(QIODevice::WriteOnly|QIODevice::Text);
			QTextStream myfile(&file);
			for (V3DLONG i=0;i<lN.size();i++)
				myfile << lN.at(i).n <<" "<< lN.at(i).type << " "<< lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn << "\n";
	
			file.close();
			return true;
		}	
	}
	else 
	{
		cout<<"You don't have any SWC file open in the main window."<<endl;
		return false;
	}
		
}

	

bool SORT_SWCPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback, QWidget * parent)
{
	if (func_name==tr("sort_swc"))
	{
		return sort_func(input,output); 
	}
}


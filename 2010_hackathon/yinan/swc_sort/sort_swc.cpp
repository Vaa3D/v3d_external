/*
 *  sort_swc .cpp
 *  sort_swc 
 *
 *  Created by Wan, Yinan, on 06/20/11.
 *  Last change: Wan, Yinan, on 06/21/11.
 */

#include <QtGlobal>

#include "sort_swc.h"
#include "v3d_message.h" 
#include "../../../v3d_main/basic_c_fun/basic_surf_objs.h"
#include <iostream>
#include <fstream>
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

QHash<V3DLONG, V3DLONG> NeuronNextPn(const NeuronTree &neurons) 
{
	QHash<V3DLONG, V3DLONG> neuron_id_table;
	for (V3DLONG i=0;i<neurons.listNeuron.size(); i++)
		neuron_id_table.insert(V3DLONG(neurons.listNeuron.at(i).n), i); 
	return neuron_id_table;
}

QHash<V3DLONG, V3DLONG> ChildParent(const NeuronTree &neurons, QList<V3DLONG> & idlist) 
{
	QHash<V3DLONG, V3DLONG> cp;
	for (V3DLONG i=0;i<neurons.listNeuron.size(); i++)
		if (neurons.listNeuron.at(i).pn==-1) cp.insert(idlist.indexOf(neurons.listNeuron.at(i).n), -1);
		else cp.insert(idlist.indexOf(neurons.listNeuron.at(i).n), idlist.indexOf(neurons.listNeuron.at(i).pn)); 
	return cp;
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
	
void SortSWC(const NeuronTree & neurons, QList<NeuronSWC> & lN, V3DLONG newrootid)
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
	//create a LUT
	QHash<V3DLONG, V3DLONG> neuron_id_table = NeuronNextPn(neurons);
	QList<V3DLONG> idlist = neuron_id_table.uniqueKeys();
	QHash<V3DLONG, V3DLONG> cp = ChildParent(neurons,idlist);
	
	
	V3DLONG siz = idlist.size();

	ofstream myfile;
	myfile.open("result.swc");
	//for (int i=0;i<siz;i++)
	//	myfile<< i <<": "<< idlist.at(i)<<"parent:"<<idlist.at(cp.value(i))<<"\n";
	bool** matrix = new bool*[siz];
	for (V3DLONG i = 0;i<siz;i++)
	{
		matrix[i] = new bool[siz];
		for (V3DLONG j = 0;j<siz;j++) matrix[i][j] = false;
	}
	

	//generate the adjacent matrix for undirected matrix
	for (V3DLONG i = 0;i<siz;i++)
	{
		QList<V3DLONG> tmpVSet = cp.values(i); //id of the ith node's parents
		for (V3DLONG j=0;j<tmpVSet.size();j++)
		{
			V3DLONG v2 = (V3DLONG) (tmpVSet.at(j));
			if (v2==-1) continue;
			matrix[i][v2] = true;
			matrix[v2][i] = true;
		}
	}
	
	
	//do a DFS for the the matrix and re-allocate ids for all the nodes
	V3DLONG root= idlist.indexOf(newrootid);
	
	V3DLONG* neworder = new V3DLONG[siz];
	bool* numbered = new bool[siz];
	for (V3DLONG i=0;i<siz;i++) numbered[i] = false;
	
	V3DLONG id[] = {0};

	DFS(matrix,neworder,root,id,siz,numbered);
	
	if ((*id)<siz) {
		v3d_msg("The root you have chosen cannot reach all the nodes in neuron tree. Show the connected component only.");
		siz = (*id);
		}
	else if ((*id)==siz)
		v3d_msg("The neuronTree is connected. Show re-sorted result.");

		//DFS suceeded, change the swc with hashfunc
		v3d_msg("A new SWC file (result.swc) is generated under your v3d directory");
		NeuronSWC S;
		S.n = 1;
		S.pn = -1;
		V3DLONG oripos = neuron_id_table.value(newrootid);
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
					V3DLONG oripos = neuron_id_table.value(idlist.at(neworder[ii]));
					S.x = neurons.listNeuron.at(oripos).x;
					S.y = neurons.listNeuron.at(oripos).y;
					S.z = neurons.listNeuron.at(oripos).z;
					S.r = neurons.listNeuron.at(oripos).r;
					S.type = neurons.listNeuron.at(oripos).type;
					lN.append(S);
				}
			}
		}
		//write new SWC to file
		
		for (V3DLONG i=0;i<lN.size();i++)
			myfile <<lN.at(i).n <<" " << lN.at(i).type << " " << lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn << "\n";
	
	myfile.close();
}

void sort_swc(V3DPluginCallback &callback, QWidget *parent, int method_code)
{
	NeuronTree neuron;
	QString filename;
	QStringList filenames;
	if (method_code == 1)
	{
		filename = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
												"",
												QObject::tr("Supported file (*.swc)"
															";;Neuron structure	(*.swc)"
															));
		if(filename.isEmpty()) 
		{
			v3d_msg("You don't have any SWC file open in the main window.");
			return;
		}
		
		
		if (filename.size()>0)
		{
			neuron = readSWC_file(filename);
			NeuronTree neurons_new;
			QList<NeuronSWC> listNeuron;
			ofstream tmp;
			//tmp.open("tmp.txt");
			//for (int i=0;i<neuron.listNeuron.size();i++)
			//tmp<< neuron.listNeuron.at(i).n<<"\n";
			//tmp.close();

			int rootid;
			bool ok;
			rootid = QInputDialog::getInteger(parent, "input root number","New root number:",1,1,neuron.listNeuron.size(),1,&ok);
			SortSWC(neuron, listNeuron,rootid);

			
			//v3dhandle newwin;
			//newwin = callback.newImageWindow();
			//callback.setSWC(newwin,neurons_new);		
		}
		else 
		{
			v3d_msg("You don't have any SWC file open in the main window.");
			return;
		}
		
	}
}

void SORT_SWCPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
	if (menu_name == tr("sort_swc"))
	{
    		sort_swc(callback, parent,1 );
    	}
	else if (menu_name == tr("Help"))
	{
		v3d_msg("(version 0.11) Set a new root and sort the SWC file into a new order, where the newly set root has the id of 1, and the parent's id is less than its child's");
		return;
	}
}


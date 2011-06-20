/*
 *  sort_swc .cpp
 *  sort_swc 
 *
 *  Created by Wan, Yinan, on 06/20/11.
 *  Last change: 
 */

#include <QtGlobal>

#include "sort_swc.h"
#include "v3d_message.h" 
#include "../../../v3d_main/basic_c_fun/basic_surf_objs.h"

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

void SORT_SWCPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
	if (menu_name == tr("sort_swc"))
	{
    	sort_swc(callback, parent,1 );
    	}
	else if (menu_name == tr("Help"))
	{
		v3d_msg("(version 0.11) Convert a SWC file to a mask image, where the area of the swc tubes will have non-zero values, and other area will have 0s; Use mask imge to filter tif image, where the area of the swc non-zero will tif image values not change, and other area will have 0");
		return;
	}
}

QHash<V3DLONG, V3DLONG> NeuronNextPn(const NeuronTree &neurons) 
{
	QHash<V3DLONG, V3DLONG> neuron_id_table;
	for (V3DLONG i=0;i<neurons.listNeuron.size(); i++)
		neuron_id_table.insert(V3DLONG(neurons.listNeuron.at(i).n), i); 
	return neuron_id_table;
}

QList<V3DLONG> findroot(QHash<V3DLONG, V3DLONG> hashneuron)
{
	QList<V3DLONG> rootlist;
	QHashIterator<V3DLONG, V3DLONG> iter(hashneuron);
	while (iter.hasNext()){
		if (iter.value()==-1) {
			rootlist.append(iter.key());
		}
		iter.next();
	}
	return (rootlist);
}

void SortSWC(NeuronTree neurons, NeuronTree neurons_new, //output neuron tree
					  V3DLONG newrootid //input root id
);

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
		
		NeuronSWC *p_t=0;
		if (filename.size()>0)
		{
			neuron = readSWC_file(filename);
			NeuronTree neurons_new = NeuronTree();
			int rootid;
			scanf("%d",&rootid);
			SortSWC(neuron,neurons_new,rootid);
			
			readSWC_file(neurons_new.file);
		}
		else 
		{
			v3d_msg("You don't have any SWC file open in the main window.");
			return;
		}
		
	}
}

void SortSWC(NeuronTree neurons, NeuronTree neurons_new, //output neuron tree
					  V3DLONG newrootid //input root id
)
{
	NeuronSWC *p_cur = 0;
	//double check the data to ensure it is correct!
	for (V3DLONG ii=0; ii<neurons.listNeuron.size(); ii++)
	{
		p_cur = (NeuronSWC *)(&(neurons.listNeuron.at(ii)));
	//	v3d_msg(QString("x %1 y %2 z %3 r %4\n").arg(p_cur->x).arg(p_cur->y).arg(p_cur->z).arg(p_cur->r),0);
		
		if (p_cur->x<0 || p_cur->y<0 || p_cur->z<0 || p_cur->r<0)
		{
			v3d_msg("You have illeagal x,y,z coordinates or radius values. Check your data.");
			return;
		}
	}
	//create a LUT
	QHash<V3DLONG, V3DLONG> neuron_id_table = NeuronNextPn(neurons); 

	//create a root list
	QList<V3DLONG> rootlist = findroot(neuron_id_table);

	neurons_new.copy(neurons);
	V3DLONG p = newrootid;
	int ii;
	for (ii = 0;ii<neurons.listNeuron.size();ii++)
	{
		if (neurons.listNeuron.at(ii).n==p) break;
	}
	NeuronSWC *p_tmp;
	p_tmp->x = neurons.listNeuron.at(ii).x;
	p_tmp->y = neurons.listNeuron.at(ii).y;
	p_tmp->z = neurons.listNeuron.at(ii).z;
	p_tmp->type = neurons.listNeuron.at(ii).type;
	p_tmp->n = p;
	p_tmp->pn = -1;
	neurons_new.listNeuron.removeAt(ii);
	neurons_new.listNeuron.insert(ii,*p_tmp);

	//compute the path from new root to the old, change the edge to the oppsite direction and generate a new tree
	while (rootlist.indexOf(p)!=-1)
	{
		V3DLONG pn = neuron_id_table.value(p);
		for (ii = 0;ii<neurons.listNeuron.size();ii++)
		{
			if (neurons.listNeuron.at(ii).pn==pn) break;
		}
		p_tmp->x = neurons.listNeuron.at(ii).x;
		p_tmp->y = neurons.listNeuron.at(ii).y;
		p_tmp->z = neurons.listNeuron.at(ii).z;
		p_tmp->type = neurons.listNeuron.at(ii).type;
		p_tmp->n = neurons.listNeuron.at(ii).pn;
		p_tmp->pn = neurons.listNeuron.at(ii).n;
		neurons_new.listNeuron.removeAt(ii);
		neurons_new.listNeuron.insert(ii,*p_tmp);
		p = pn;
	}
	neurons_new.file ="~/Desktop/newSWC.swc"; 
}

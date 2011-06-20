/*
 * swc_to_maskimg.h
 *
 *  Created by Yang, Jinzhu on 11/27/10.
 *  Last change: by Hanchuan Peng, 2010-Dec-7. change some writing style of this Plugin header
 *
 */

#ifndef __SWC_SORT_H_
#define __SWC_SORT_H

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include "v3d_interface.h"

class SORT_SWCPlugin: public QObject, public V3DPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface);
	
public:
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent);
	QStringList funclist() const {return QStringList();}
	void dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, QWidget *parent) {}	
	
};

void sort_swc(V3DPluginCallback &callback, QWidget *parent, int method_code);

QHash<V3DLONG, V3DLONG> NeuronNextPn(const NeuronTree &neurons);
QList<V3DLONG> findroot(QHash<V3DLONG, V3DLONG> hashneuron);
void SortSWC(NeuronTree neurons, NeuronTree neurons_new, V3DLONG newrootid);

#endif



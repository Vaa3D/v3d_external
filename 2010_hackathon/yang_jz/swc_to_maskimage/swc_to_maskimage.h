/*
 * swc_to_maskimg.h
 *
 *  Created by Yang, Jinzhu on 11/27/10.
 *  Last change: by Hanchuan Peng, 2010-Dec-7. change some writing style of this plugin header
 *
 */

#ifndef __SWC_TO_MASKIMAGE_H_
#define __SWC_TO_MASKIMAGE_H

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include "v3d_interface.h"

class SWC_TO_MASKIMAGElugin: public QObject, public V3DPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface);
	
public:
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent);
	
	QStringList funclist() const {return QStringList();}
	void dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, QWidget *parent) {}	
};

void swc_to_maskimage(V3DPluginCallback &callback, QWidget *parent, int method_code);

void BoundNeuronCoordinates(NeuronTree & neuron, 
							bool b_subtractMinFromAllNonnegatives,
							double & output_xmin,
							double & output_xmax,
							double & output_ymin,
							double & output_ymax,
							double & output_zmin,
							double & output_zmax
);

void ComputemaskImage(NeuronTree neurons, 
					  unsigned char* pImMask, //output mask image
					  unsigned char* ImMark,  //an indicator image to show whether or not a pixel has been visited/processed
					  V3DLONG sx, 
					  V3DLONG sy, 
					  V3DLONG sz,
					  int method_code
);

QHash<V3DLONG, V3DLONG> NeuronNextPn(const NeuronTree &neurons);


#endif



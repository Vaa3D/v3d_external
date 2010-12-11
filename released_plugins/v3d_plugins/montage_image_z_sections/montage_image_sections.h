/*
 *  montage_image_sections.h
 *  montage_image_sections 
 *
 *  Created by Yang, Jinzhu on 11/22/10.
 *  
 *
 */

#ifndef __MONTAGE_IMAGE_SECTION_H_
#define __MONTAGE_IMAGE_SECTION_H_

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>

#include "v3d_interface.h"

class MONTAGEPlugin: public QObject, public V3DPluginInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2_1);
	
public:
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);
	
	QStringList funclist() const {return QStringList();}
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)
        {return false;}
	
	template <class T> 
	void montage_image_sections(T *apsInput, T * aspOutput, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer);
	//void do_computation (float *apsInput, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer, V3DLONG* label){}
	//void do_computation (unsigned char *apsInput, unsigned char * aspOutput, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer, V3DLONG h){}

        float getPluginVersion() const {return 1.1f;}
};

//define a simple dialog for choose DT parameters

#endif



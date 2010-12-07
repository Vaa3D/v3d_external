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

#endif



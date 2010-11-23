/*
 *  ada_threshold.h
 *  ada_threshold
 *
 *  Created by Yang, Jinzhu on 11/22/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __DISTTRANFORM_H__
#define __DISTTRANFORM_H__

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>

#include "v3d_interface.h"

class ThPlugin : public QObject, public V3DPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface);
	
public:
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent);
	
	QStringList funclist() const {return QStringList();}
	void dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, QWidget *parent) {}
	
	template <class T> 
	void BinaryProcess(T *apsInput, T * aspOutput, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer, V3DLONG h, V3DLONG d){}
	//void BinaryProcess(float *apsInput, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer, V3DLONG* label){}
	//void BinaryProcess(unsigned char *apsInput, unsigned char * aspOutput, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer, V3DLONG h){}
};

//define a simple dialog for choose DT parameters
class DtDialog : public QDialog
{
	Q_OBJECT
	
public:
	int ch;
	bool b_use_1stmarker; //whether or not use the intensity value at the first marker location as the target value
	bool b_rescale; //if rescale the value of the output
	
	QGridLayout *gridLayout;
	QLabel* label_imagename;
	QLabel* label_channel;
	QComboBox* combo_channel;
	QCheckBox* check_marker;
	QCheckBox* check_rescale;

	
	QLabel *labelx;
	QLabel *labely;
	
	
	QSpinBox** nval;
	QLabel** nlabel;
	
	QSpinBox* Ddistance;
	QSpinBox* Dnumber;
	
	QPushButton* ok;
	QPushButton* cancel;
	
	V3DLONG Dn;
	V3DLONG Dh;
	
	V3DLONG pagesz;
	V3DLONG sc;
	
	///
public:
	DtDialog(V3DPluginCallback &cb, QWidget *parent)
	{
		Image4DSimple* image = cb.getImage(cb.currentImageWindow());
		QString imageName = cb.getImageName(cb.currentImageWindow());		
		sc = image->getCDim();
		//create a dialog
		Ddistance= new QSpinBox();
		Dnumber = new QSpinBox();
	
		
		Dnumber->setMaximum(255); Dnumber->setMinimum(1); Dnumber->setValue(3);
		Ddistance ->setMaximum(255); Ddistance ->setMinimum(1); Ddistance ->setValue(6);
	
		
		ok     = new QPushButton("OK");
		cancel = new QPushButton("Cancel");
		
		gridLayout = new QGridLayout();
		
		labelx = new QLabel(QObject::tr("Ddistance: "));
		labely = new QLabel(QObject::tr("Dnumber: "));

		
		gridLayout->addWidget(labelx, 0,0); gridLayout->addWidget(Ddistance, 0,1);
		gridLayout->addWidget(labely, 1,0); gridLayout->addWidget(Dnumber, 1,1);
		
		
		gridLayout->addWidget(cancel, sc+3,0); gridLayout->addWidget(ok, sc+3,1);
		setLayout(gridLayout);
		setWindowTitle(QString("Change parameters"));
		
		connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
		
		//slot interface
		connect(Ddistance, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(Dnumber,SIGNAL(valueChanged(int)), this, SLOT(update()));
		
	}

	
	~DtDialog(){}
	
	public slots:
	void update();
};



#endif



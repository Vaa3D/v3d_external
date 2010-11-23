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
	//QPushButton* ok;
	//QPushButton* cancel;
	//
	
	QGridLayout *gridLayout;
	
	QLabel *labelx;
	QLabel *labely;
	QLabel *labelz;
	
	QSpinBox** nval;
	QLabel** nlabel;
	
	QSpinBox* coord_x;
	QSpinBox* coord_y;
	QSpinBox* coord_z;
	
	QPushButton* ok;
	QPushButton* cancel;
	///
public:
	DtDialog(V3DPluginCallback &cb, QWidget *parent)
	{
		
		
		//create a dialog
		coord_x = new QSpinBox();
		coord_y = new QSpinBox();
		coord_z = new QSpinBox();
		
		coord_x->setMaximum(N); coord_x->setMinimum(1); coord_x->setValue(1);
		coord_y->setMaximum(M); coord_y->setMinimum(1); coord_y->setValue(1);
		coord_z->setMaximum(P); coord_z->setMinimum(1); coord_z->setValue(1);
		
		nlabel = new QLabel *[sc];
		for(V3DLONG c=0; c<sc; c++)
		{
			nlabel[c] = new QLabel(QObject::tr("Set new intensity value at channel %1: ").arg(c+1));
		}
		
		nval = new QSpinBox *[sc];
		for(V3DLONG c=0; c<sc; c++)
		{
			nval[c] = new QSpinBox();
		}
		
		ok     = new QPushButton("OK");
		cancel = new QPushButton("Cancel");
		
		gridLayout = new QGridLayout();
		
		labelx = new QLabel(QObject::tr("image x: "));
		labely = new QLabel(QObject::tr("image y: "));
		labelz = new QLabel(QObject::tr("image z: "));
		
		gridLayout->addWidget(labelx, 0,0); gridLayout->addWidget(coord_x, 0,1);
		gridLayout->addWidget(labely, 1,0); gridLayout->addWidget(coord_y, 1,1);
		gridLayout->addWidget(labelz, 2,0); gridLayout->addWidget(coord_z, 2,1);
		
		for(V3DLONG c=0; c<sc; c++)
		{
			gridLayout->addWidget(nlabel[c], c+3,0);
			gridLayout->addWidget(nval[c], c+3,1);
			
			nval[c]->setMaximum(255);
			nval[c]->setMinimum(0);
		}
		
		gridLayout->addWidget(cancel, sc+3,0); gridLayout->addWidget(ok, sc+3,1);
		setLayout(gridLayout);
		setWindowTitle(QString("Change Pixel Value"));
		
		connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
		
		//slot interface
		connect(coord_x, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(coord_y, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(coord_z, SIGNAL(valueChanged(int)), this, SLOT(update()));
	}
	
		
		
	/*	gridLayout = new QGridLayout();
		
		Image4DSimple* image = cb.getImage(cb.currentImageWindow());
		QString imageName = cb.getImageName(cb.currentImageWindow());
		label_imagename = new QLabel(imageName.prepend("Your have selected the image [").append("]"));
		gridLayout->addWidget(label_imagename, 1,0,1,2); 
		
		int c = image->getCDim();
		label_channel = new QLabel(QObject::tr("Choose a channel: ")); 
		gridLayout->addWidget(label_channel, 2,0,1,2); 
		
		// channel info
		QStringList chList;
		if(c>=3)
			chList << "red" << "green" << "blue";
		else if(c==2)
			chList << "red" << "green";
		else if(c==1)
			chList << "red";
		combo_channel = new QComboBox(); combo_channel->addItems(chList);
		gridLayout->addWidget(combo_channel, 3,0,1,2);
		
		check_marker = new QCheckBox();
		check_marker->setText(QObject::tr("Use the 1st marker's intensity to define the 'img background' (if checked OFF, then use intensity 0 to indicate background)"));
		check_marker->setChecked(false);
		gridLayout->addWidget(check_marker, 4,0,1,2); 
		
		check_rescale = new QCheckBox();
		check_rescale->setText(QObject::tr("Rescale output image intensity to [0,255]"));
		check_rescale->setChecked(false);
		gridLayout->addWidget(check_rescale, 5,0,1,1); 
		
		ok     = new QPushButton("OK");
		cancel = new QPushButton("Cancel");
		gridLayout->addWidget(cancel, 6,0,1,1); 
		gridLayout->addWidget(ok,     6,1,1,1);
		
		setLayout(gridLayout);
		setWindowTitle(QString("Distance transform"));
		
		//slot interface
		connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
		
		connect(combo_channel, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(check_marker, SIGNAL(stateChanged(int)), this, SLOT(update()));
		connect(check_rescale, SIGNAL(stateChanged(int)), this, SLOT(update()));*/
	}
	
	~DtDialog(){}
	
	public slots:
	void update()
	{
		ch = combo_channel->currentIndex();
		b_use_1stmarker = (check_marker->isChecked()) ?  true : false;
		b_rescale = (check_rescale->isChecked())? true : false;
	}
};



#endif



/*
 *  add_roi.h
 *  add_roi
 *
 *  Created by Yang, Jinzhu on 11/24/10.
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
};

//define a simple dialog for choose DT parameters
class AdaTDialog : public QDialog
{
	Q_OBJECT
	
public:
	
	QGridLayout *gridLayout;
	QLabel* label_imagename;
	QLabel* label_channel;
	QComboBox* combo_channel;

	
	
	QLabel *labelx;
	
	QPushButton* ok;
	QPushButton* cancel;
	
	
	
	V3DLONG pagesz;
	V3DLONG sc;
	
	///
	
	///
public:
	AdaTDialog(V3DPluginCallback &cb, QWidget *parent)
	{
		Image4DSimple* image = cb.getImage(cb.currentImageWindow());
		QString imageName = cb.getImageName(cb.currentImageWindow());		
		sc = image->getCDim();
		v3dhandleList win_list = cb.getImageWindowList();
		//create a dialog
		ok     = new QPushButton("OK");
		cancel = new QPushButton("Cancel");
		gridLayout = new QGridLayout();
		
		labelx = new QLabel(QObject::tr("Target imange"));
		gridLayout->addWidget(labelx, 0,0);

		
		int c = image->getCDim();

		// channel info
		QStringList chList;
		
		
		for (int i = 0; i < win_list.size() ;i++ )
		{
			chList<< cb.getImageName(win_list[i]);
			
		}
		
		combo_channel = new QComboBox(); 
		
		combo_channel->addItems(chList);
		
		gridLayout->addWidget(combo_channel, 2,0);
		
		
		gridLayout->addWidget(cancel, 6,1); gridLayout->addWidget(ok, 6,0);
		setLayout(gridLayout);
		setWindowTitle(QString("Paste and Delete ROI"));
		
		connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

		
	}
	
	
	~AdaTDialog(){}
	
	public slots:
	void update();
};

#endif



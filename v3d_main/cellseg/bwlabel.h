/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).  
 * All rights reserved.
 */


/************
                                            ********* LICENSE NOTICE ************

This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it. 

You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.

1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.

2. You agree to appropriately cite this work in your related studies and publications.

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




/* bwlabeln.h
 * label all the N objects in a BW image: by Hanchuan Peng
 * a wrapper program for Fuhui Long's bwlabeln function
 */


#ifndef __BWLABELN_H__
#define __BWLABELN_H__

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>

#include "v3d_interface.h"

class BWLabelNPlugin : public QObject, public V3DPluginInterface
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
class BWLabelNDialog : public QDialog
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
	QPushButton* ok;
	QPushButton* cancel;
	
public:
	BWLabelNDialog(V3DPluginCallback &cb, QWidget *parent)
	{
		//create a dialog
		gridLayout = new QGridLayout();
		
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
		setWindowTitle(QString("Label all image objects"));
		
		//slot interface
		connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
		
		connect(combo_channel, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(check_marker, SIGNAL(stateChanged(int)), this, SLOT(update()));
		connect(check_rescale, SIGNAL(stateChanged(int)), this, SLOT(update()));
	}
	
	~BWLabelNDialog(){}
	
	public slots:
	void update()
	{
		ch = combo_channel->currentIndex();
		b_use_1stmarker = (check_marker->isChecked()) ?  true : false;
		b_rescale = (check_rescale->isChecked())? true : false;
	}
};



#endif


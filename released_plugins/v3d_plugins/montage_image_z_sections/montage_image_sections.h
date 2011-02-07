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
	template <class T> 
	void montage_image_stack(T *apsInput, T * aspOutput, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer,V3DLONG column,V3DLONG row, V3DLONG slice,V3DLONG n);	
	//void do_computation (float *apsInput, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer, V3DLONG* label){}
	//void do_computation (unsigned char *apsInput, unsigned char * aspOutput, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer, V3DLONG h){}

        float getPluginVersion() const {return 1.1f;}
};
class SetsizeDialog : public QDialog
{
	Q_OBJECT
	
public:
	QGridLayout *gridLayout;
	
	QLabel *labelx;
	QLabel *labely;
	QLabel *labelz;
    QSpinBox* coord_x; 
	QSpinBox* coord_y;
	QSpinBox* coord_z;
	
	QPushButton* ok;
	QPushButton* cancel;
	
	V3DLONG NX;
	V3DLONG NY;
	V3DLONG NZ;
	
public:
	SetsizeDialog(V3DPluginCallback &cb, QWidget *parent)
	{
		Image4DSimple* image = cb.getImage(cb.currentImageWindow());
		QString imageName = cb.getImageName(cb.currentImageWindow());		
		//create a dialog
		coord_z = new QSpinBox();
		coord_z->setMaximum(1500); coord_z->setMinimum(2); coord_z->setValue(7);
		ok     = new QPushButton("OK");
		cancel = new QPushButton("Cancel");
		gridLayout = new QGridLayout();
		labelz = new QLabel(QObject::tr("Compartment number"));
		gridLayout->addWidget(labelz, 0,0); gridLayout->addWidget(coord_z, 0,1);
		gridLayout->addWidget(cancel, 6,1); gridLayout->addWidget(ok, 6,0);
		setLayout(gridLayout);
		setWindowTitle(QString("Change parameters"));
		connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
		connect(coord_z,SIGNAL(valueChanged(int)), this, SLOT(update()));
	}
	
	~SetsizeDialog(){}
	
	public slots:
	void update();
};
//define a simple dialog for choose DT parameters

#endif



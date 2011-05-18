/* cellcounter.h
 * 2010-01-14: create this program by Yang Yu
 */


#ifndef __CELLCOUNTER_H__
#define __CELLCOUNTER_H__


//
#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>

class CellCounterPlugin : public QObject, public V3DPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface);
	
public:
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent);
	
	QStringList funclist() const {return QStringList();}
	void dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, QWidget *parent) {}

};


class CellCounterDialog : public QDialog
{
	Q_OBJECT
	
public:
	CellCounterDialog(V3DPluginCallback &callback, QWidget *parent)
	{
		v3dhandleList win_list = callback.getImageWindowList();
		
		QStringList items;
		for (int i=0; i<win_list.size(); i++) items << callback.getImageName(win_list[i]);
		
		//create a dialog
		combo_subject =  new QComboBox(); combo_subject->addItems(items);
		
		ok     = new QPushButton("OK");
		cancel = new QPushButton("Cancel");
		
		gridLayout = new QGridLayout();
		
		label_subject = new QLabel(QObject::tr("Image (ROI): ")); 
		
		gridLayout->addWidget(label_subject, 0,0,1,1); gridLayout->addWidget(combo_subject, 0,1,1,4);

		i1 = combo_subject->currentIndex();
		
		Image4DSimple* subject = callback.getImage(win_list[i1]);
		
		// channel info
		QStringList chList;
		chList << "red" << "green" << "blue";
		combo_channel = new QComboBox(); combo_channel->addItems(chList);
		
		label_channel = new QLabel(QObject::tr("Searching Channel: ")); 
		
		gridLayout->addWidget(label_channel, 3,0,1,1); gridLayout->addWidget(combo_channel, 3,1,1,4);
		
		ch_rgb = combo_channel->currentIndex();
		
		//parameters
		//ds
		label_ds = new QLabel(QObject::tr("-- Down Sampling -- "));
		
		gridLayout->addWidget(label_ds, 5,0);
		
		qsb_ds_scale_x = new QDoubleSpinBox(); 
		qsb_ds_scale_y = new QDoubleSpinBox();
		qsb_ds_scale_z = new QDoubleSpinBox();
		
		qsb_ds_scale_x->setMaximum(1); qsb_ds_scale_x->setMinimum(0); qsb_ds_scale_x->setValue(0.5); qsb_ds_scale_x->setSingleStep(0.01);
		qsb_ds_scale_y->setMaximum(1); qsb_ds_scale_y->setMinimum(0); qsb_ds_scale_y->setValue(0.5); qsb_ds_scale_y->setSingleStep(0.01);
		qsb_ds_scale_z->setMaximum(1); qsb_ds_scale_z->setMinimum(0); qsb_ds_scale_z->setValue(0.5); qsb_ds_scale_z->setSingleStep(0.01);
		
		label_ds_scale_x = new QLabel(QObject::tr("scaling factor along x: "));
		label_ds_scale_y = new QLabel(QObject::tr("scaling factor along y: "));
		label_ds_scale_z = new QLabel(QObject::tr("scaling factor along z: "));
		
		gridLayout->addWidget(label_ds_scale_x, 7,0,Qt::AlignLeft); gridLayout->addWidget(qsb_ds_scale_x, 7,1,Qt::AlignLeft);
		gridLayout->addWidget(label_ds_scale_y, 8,0,Qt::AlignLeft); gridLayout->addWidget(qsb_ds_scale_y, 8,1,Qt::AlignLeft);
		gridLayout->addWidget(label_ds_scale_z, 9,0,Qt::AlignLeft); gridLayout->addWidget(qsb_ds_scale_z, 9,1,Qt::AlignLeft);
		
		//gf
		label_gf = new QLabel(QObject::tr("-- Gaussian Filtering -- "));
		
		gridLayout->addWidget(label_gf, 11,0);
		
		qsb_gf_r_x = new QSpinBox(); 
		qsb_gf_r_y = new QSpinBox();
		qsb_gf_r_z = new QSpinBox();
		
		qsb_gf_r_x->setMaximum(subject->getXDim()); qsb_gf_r_x->setMinimum(1); qsb_gf_r_x->setValue(10); qsb_gf_r_x->setSingleStep(1);
		qsb_gf_r_y->setMaximum(subject->getYDim()); qsb_gf_r_y->setMinimum(1); qsb_gf_r_y->setValue(10); qsb_gf_r_y->setSingleStep(1);
		qsb_gf_r_z->setMaximum(subject->getZDim()); qsb_gf_r_z->setMinimum(1); qsb_gf_r_z->setValue(10); qsb_gf_r_z->setSingleStep(1);
		
		label_gf_r_x = new QLabel(QObject::tr("filtering window radius along x (pixels): "));
		label_gf_r_y = new QLabel(QObject::tr("filtering window radius along y (pixels): "));
		label_gf_r_z = new QLabel(QObject::tr("filtering window radius along z (pixels): "));
		
		gridLayout->addWidget(label_gf_r_x, 13,0,Qt::AlignLeft); gridLayout->addWidget(qsb_gf_r_x, 13,1,Qt::AlignLeft);
		gridLayout->addWidget(label_gf_r_y, 14,0,Qt::AlignLeft); gridLayout->addWidget(qsb_gf_r_y, 14,1,Qt::AlignLeft);
		gridLayout->addWidget(label_gf_r_z, 15,0,Qt::AlignLeft); gridLayout->addWidget(qsb_gf_r_z, 15,1,Qt::AlignLeft);
		
		//tm
		label_tm = new QLabel(QObject::tr("-- Template Matching -- "));
		
		gridLayout->addWidget(label_tm, 17,0);
		
		r_x = 4; r_y = 4; r_z = 4;
		qsb_r_x = new QDoubleSpinBox(); 
		qsb_r_y = new QDoubleSpinBox();
		qsb_r_z = new QDoubleSpinBox();
		
		qsb_r_x->setMaximum(50); qsb_r_x->setMinimum(0); qsb_r_x->setValue(r_x); qsb_r_x->setSingleStep(0.01);
		qsb_r_y->setMaximum(50); qsb_r_y->setMinimum(0); qsb_r_y->setValue(r_y); qsb_r_y->setSingleStep(0.01);
		qsb_r_z->setMaximum(50); qsb_r_z->setMinimum(0); qsb_r_z->setValue(r_z); qsb_r_z->setSingleStep(0.01);
		
		label_r_x = new QLabel(QObject::tr("radius along x (pixel): "));
		label_r_y = new QLabel(QObject::tr("radius along y (pixel): "));
		label_r_z = new QLabel(QObject::tr("radius along z (pixel): "));
		
		gridLayout->addWidget(label_r_x, 19,0,Qt::AlignLeft); gridLayout->addWidget(qsb_r_x, 19,1,Qt::AlignLeft);
		gridLayout->addWidget(label_r_y, 20,0,Qt::AlignLeft); gridLayout->addWidget(qsb_r_y, 20,1,Qt::AlignLeft);
		gridLayout->addWidget(label_r_z, 21,0,Qt::AlignLeft); gridLayout->addWidget(qsb_r_z, 21,1,Qt::AlignLeft);
		
		sigma_x = 2; sigma_y = 2; sigma_z = 2;
		qsb_sigma_x = new QDoubleSpinBox(); 
		qsb_sigma_y = new QDoubleSpinBox();
		qsb_sigma_z = new QDoubleSpinBox();
		
		qsb_sigma_x->setMaximum(50); qsb_sigma_x->setMinimum(0); qsb_sigma_x->setValue(sigma_x); qsb_sigma_x->setSingleStep(0.01);
		qsb_sigma_y->setMaximum(50); qsb_sigma_y->setMinimum(0); qsb_sigma_y->setValue(sigma_y); qsb_sigma_y->setSingleStep(0.01);
		qsb_sigma_z->setMaximum(50); qsb_sigma_z->setMinimum(0); qsb_sigma_z->setValue(sigma_z); qsb_sigma_z->setSingleStep(0.01);
		
		label_sigma_x = new QLabel(QObject::tr("Gaussian sigma x: "));
		label_sigma_y = new QLabel(QObject::tr("Gaussian sigma y: "));
		label_sigma_z = new QLabel(QObject::tr("Gaussian sigma z: "));
		
		gridLayout->addWidget(label_sigma_x, 19,2,Qt::AlignRight); gridLayout->addWidget(qsb_sigma_x, 19,4,Qt::AlignLeft);
		gridLayout->addWidget(label_sigma_y, 20,2,Qt::AlignRight); gridLayout->addWidget(qsb_sigma_y, 20,4,Qt::AlignLeft);
		gridLayout->addWidget(label_sigma_z, 21,2,Qt::AlignRight); gridLayout->addWidget(qsb_sigma_z, 21,4,Qt::AlignLeft);
		
		correlation_coeff = 0.75;
		qsb_ccoeff = new QDoubleSpinBox(); 
		
		qsb_ccoeff->setMaximum(1); qsb_ccoeff->setMinimum(0); qsb_ccoeff->setValue(correlation_coeff); qsb_ccoeff->setSingleStep(0.01);

		label_ccoeff = new QLabel(QObject::tr("correlation coefficient: "));
		
		gridLayout->addWidget(label_ccoeff, 23,0,Qt::AlignLeft); gridLayout->addWidget(qsb_ccoeff, 23,1,Qt::AlignLeft);
		
		high_th = 50; low_th = 5; delta_th = 1;
		qsb_high_th = new QDoubleSpinBox(); 
		qsb_low_th = new QDoubleSpinBox();
		qsb_delta_th = new QDoubleSpinBox();
		
		qsb_high_th->setMaximum(255); qsb_high_th->setMinimum(0); qsb_high_th->setValue(high_th); qsb_high_th->setSingleStep(0.01);
		qsb_low_th->setMaximum(255); qsb_low_th->setMinimum(0); qsb_low_th->setValue(low_th); qsb_low_th->setSingleStep(0.01);
		qsb_delta_th->setMaximum(255); qsb_delta_th->setMinimum(0); qsb_delta_th->setValue(delta_th); qsb_delta_th->setSingleStep(0.01);
		
		label_high_th = new QLabel(QObject::tr("local threshlod (higher bound): "));
		label_low_th = new QLabel(QObject::tr("local threshlod (lower bound): "));
		label_delta_th = new QLabel(QObject::tr("local threshlod step: "));
		
		gridLayout->addWidget(label_high_th, 29,0,Qt::AlignLeft); gridLayout->addWidget(qsb_high_th, 29,1,Qt::AlignLeft);
		gridLayout->addWidget(label_low_th, 30,0,Qt::AlignLeft); gridLayout->addWidget(qsb_low_th, 30,1,Qt::AlignLeft);
		gridLayout->addWidget(label_delta_th, 31,0,Qt::AlignLeft); gridLayout->addWidget(qsb_delta_th, 31,1,Qt::AlignLeft);
		
		global_th = 4;
		qsb_global_th = new QDoubleSpinBox(); 
		
		qsb_global_th->setMaximum(255); qsb_global_th->setMinimum(0); qsb_global_th->setValue(global_th); qsb_global_th->setSingleStep(0.01);
		
		label_global_th = new QLabel(QObject::tr("global threshold: "));
		
		gridLayout->addWidget(label_global_th, 33,0,Qt::AlignLeft); gridLayout->addWidget(qsb_global_th, 33,1,Qt::AlignLeft);
		

		//gridlayout
		gridLayout->addWidget(cancel, 35,3,Qt::AlignRight); gridLayout->addWidget(ok, 35,4,Qt::AlignRight);
		
		setLayout(gridLayout);
		setWindowTitle(QString("Cell Counter"));
		
		connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
		
		//slot interface
		connect(qsb_ds_scale_x, SIGNAL(valueChanged(double)), this, SLOT(update()));
		connect(qsb_ds_scale_y, SIGNAL(valueChanged(double)), this, SLOT(update()));
		connect(qsb_ds_scale_z, SIGNAL(valueChanged(double)), this, SLOT(update()));
		
		connect(qsb_gf_r_x, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(qsb_gf_r_y, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(qsb_gf_r_z, SIGNAL(valueChanged(int)), this, SLOT(update()));
		
		connect(qsb_r_x, SIGNAL(valueChanged(double)), this, SLOT(update()));
		connect(qsb_r_y, SIGNAL(valueChanged(double)), this, SLOT(update()));
		connect(qsb_r_z, SIGNAL(valueChanged(double)), this, SLOT(update()));
		
		connect(qsb_sigma_x, SIGNAL(valueChanged(double)), this, SLOT(update()));
		connect(qsb_sigma_y, SIGNAL(valueChanged(double)), this, SLOT(update()));
		connect(qsb_sigma_z, SIGNAL(valueChanged(double)), this, SLOT(update()));
		
		connect(qsb_ccoeff, SIGNAL(valueChanged(double)), this, SLOT(update()));
		
		connect(qsb_high_th, SIGNAL(valueChanged(double)), this, SLOT(update()));
		connect(qsb_low_th, SIGNAL(valueChanged(double)), this, SLOT(update()));
		connect(qsb_delta_th, SIGNAL(valueChanged(double)), this, SLOT(update()));
		
		connect(qsb_global_th, SIGNAL(valueChanged(double)), this, SLOT(update()));
		
		connect(combo_subject, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));
		
		connect(combo_channel, SIGNAL(valueChanged(int)), this, SLOT(update()));
		
	}
	
	~CellCounterDialog(){}

	
public slots:
	void update()
	{
		scale_x = qsb_ds_scale_x->text().toDouble();
		scale_y = qsb_ds_scale_y->text().toDouble();
		scale_z = qsb_ds_scale_z->text().toDouble();
		
		w_x = qsb_gf_r_x->text().toInt();
		w_y = qsb_gf_r_y->text().toInt();
		w_z = qsb_gf_r_z->text().toInt();
		
		r_x = qsb_r_x->text().toDouble();
		r_y = qsb_r_y->text().toDouble();
		r_z = qsb_r_z->text().toDouble();
		
		sigma_x = qsb_sigma_x->text().toDouble();
		sigma_y = qsb_sigma_y->text().toDouble();
		sigma_z = qsb_sigma_z->text().toDouble();
		
		correlation_coeff = qsb_ccoeff->text().toDouble();		
		
		high_th = qsb_high_th->text().toDouble();
		low_th = qsb_low_th->text().toDouble();
		delta_th = qsb_delta_th->text().toDouble();
		
		global_th = qsb_global_th->text().toDouble();
		
		ch_rgb = combo_channel->currentIndex();
		
		i1 = combo_subject->currentIndex();
	}
	
	
public:
	double r_x, r_y, r_z;
	double sigma_x, sigma_y, sigma_z;
	
	double scale_x, scale_y, scale_z;
	
	int w_x, w_y, w_z;
	
	double correlation_coeff;
	
	double high_th, low_th, delta_th;
	
	double global_th;
	
	int i1,i2;
	
	int ch_rgb;
	
	QGridLayout *gridLayout;
	
	// down sampling
	
	QLabel *label_ds;
	
	QLabel *label_ds_scale_x;
	QLabel *label_ds_scale_y;
	QLabel *label_ds_scale_z;
	
	QDoubleSpinBox *qsb_ds_scale_x;
	QDoubleSpinBox *qsb_ds_scale_y;
	QDoubleSpinBox *qsb_ds_scale_z;
	
	// gaussian filtering
	
	QLabel *label_gf;
	
	QLabel *label_gf_r_x;
	QLabel *label_gf_r_y;
	QLabel *label_gf_r_z;
	
	QSpinBox *qsb_gf_r_x;
	QSpinBox *qsb_gf_r_y;
	QSpinBox *qsb_gf_r_z;
	
	// template matching
	
	QLabel *label_tm;
	
	QLabel *label_r_x;
	QLabel *label_r_y;
	QLabel *label_r_z;
	
	QLabel *label_sigma_x;
	QLabel *label_sigma_y;
	QLabel *label_sigma_z;
	
	QLabel *label_ccoeff;
	
	QLabel *label_high_th;
	QLabel *label_low_th;
	QLabel *label_delta_th;
	
	QLabel *label_global_th;
	
	QLabel *label_subject;
	
	QLabel *label_channel;
	QComboBox* combo_channel;

	QDoubleSpinBox *qsb_r_x;
	QDoubleSpinBox *qsb_r_y;
	QDoubleSpinBox *qsb_r_z;
	
	QDoubleSpinBox *qsb_sigma_x;
	QDoubleSpinBox *qsb_sigma_y;
	QDoubleSpinBox *qsb_sigma_z;
	
	QDoubleSpinBox *qsb_ccoeff;
	
	QDoubleSpinBox *qsb_high_th;	
	QDoubleSpinBox *qsb_low_th;
	QDoubleSpinBox *qsb_delta_th;
	
	QDoubleSpinBox *qsb_global_th;
	
	QComboBox* combo_subject;
	
	QPushButton* ok;
	QPushButton* cancel;
	
};


#endif




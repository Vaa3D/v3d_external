/*
 *  TIP_DETECTION.h
 *
 *  Created by Yang, Jinzhu on 12/15/10.
 *
 */

#ifndef __CANNY__H__
#define __CANNY__H__

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>

#include "v3d_interface.h"
#include <vector>
using namespace std;

class CannyPlugin : public QObject, public V3DPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface);
	
public:
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent);
	QStringList funclist() const {return QStringList();}
	void dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, QWidget *parent) {}
	void Canny(V3DPluginCallback &callback, QWidget *parent, int method_code);
	template <class T> 
	void BinaryProcess(T *apsInput, T * aspOutput, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer, V3DLONG h, V3DLONG d){}
	
	void BinaryProcess(unsigned char*apsInput, unsigned char * aspOutput, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer, V3DLONG h, V3DLONG d);
	
	void IterateSeg(unsigned char *apsInput, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer, unsigned char *apsOutput);
	
	void Doublelinear_inserting (unsigned char*apsInput,V3DLONG &polValue,float x0,float y0,V3DLONG z0,V3DLONG currX,V3DLONG currY);
	
	void Thinning(unsigned char *apsInput,unsigned char *apsOutput,V3DLONG m_iWid, V3DLONG m_iHei, V3DLONG m_iCount);

	void GaussFilter2D(unsigned char *apsImgInput, unsigned char *apfImgOutput,V3DLONG m_iWid, V3DLONG m_iHei, V3DLONG m_iCount, float *apfTemplate,int iTmpLen);
	
	void CreateGaussFilterTemplet2D(float *apfTemplet, int iSize, float fLamb);
	
    void CreateGaussFilterTemplet3D(float *apfTemplet, int iSize, float fLamb);
	
	void GaussFilter3D(unsigned char *apsImgInput, unsigned char *apfImgOutput,V3DLONG m_iWid, V3DLONG m_iHei, V3DLONG m_iCount, float *apfTemplate,int iTmpLen);
	
	void Getgrad(V3DLONG m_iWid, V3DLONG m_iHei, V3DLONG m_iCount, unsigned char*apsInput,V3DLONG *pGradX, V3DLONG *pGradY, unsigned char*Gradient, unsigned char *NonMax);
	
	void DetectCannyEdges(V3DLONG m_iWid, V3DLONG m_iHei, V3DLONG m_iCount, unsigned char*apsInput,V3DLONG *pGradX, V3DLONG *pGradY, unsigned char *Gradient,unsigned char *NonMax);
	
	void TraceEdge(V3DLONG k, V3DLONG y, V3DLONG x, V3DLONG nThrLow,unsigned char*apsInput,unsigned char*Gradient,V3DLONG m_iWid, V3DLONG m_iHei, V3DLONG m_iCount);
	

public:
	
    V3DLONG                           m_iMinLength;
	
	V3DLONG                           m_iImgWidth;
	
	V3DLONG                           m_iImgHeight;
	
	V3DLONG                           m_OiImgWidth;
	
	V3DLONG                           m_OiImgHeight;
	
	V3DLONG                           m_iImgSize;
	
	V3DLONG                           m_iImgCount;
	
	V3DLONG                         m_ulVolumeSize;
	
};


class CannyDialog : public QDialog
{
	Q_OBJECT
	
public:
	QGridLayout *gridLayout;
	
	QLabel *labelx;
	QLabel *labely;
    QSpinBox* Ddistance; 
	QSpinBox* Dnumber;
	
	QPushButton* ok;
	QPushButton* cancel;
	
	V3DLONG Dn;
	V3DLONG Dh;
	
public:
	CannyDialog(V3DPluginCallback &cb, QWidget *parent)
	{
		Image4DSimple* image = cb.getImage(cb.currentImageWindow());
		QString imageName = cb.getImageName(cb.currentImageWindow());		
		//create a dialog
		Ddistance= new QSpinBox();
		Dnumber = new QSpinBox();
	
		
		Dnumber->setMaximum(255); Dnumber->setMinimum(1); Dnumber->setValue(3);
		Ddistance->setMaximum(255); Ddistance->setMinimum(1); Ddistance->setValue(5);
		
		ok     = new QPushButton("OK");
		cancel = new QPushButton("Cancel");
		gridLayout = new QGridLayout();
		
		labelx = new QLabel(QObject::tr("sampling interval"));
		labely = new QLabel(QObject::tr("number of sampling points"));
		
		gridLayout->addWidget(labelx, 0,0); gridLayout->addWidget(Ddistance, 0,1);
		gridLayout->addWidget(labely, 1,0); gridLayout->addWidget(Dnumber, 1,1);
		
		gridLayout->addWidget(cancel, 6,1); gridLayout->addWidget(ok, 6,0);
		setLayout(gridLayout);
		setWindowTitle(QString("Change parameters"));
		
		connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
		
		//slot interface
		connect(Ddistance, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(Dnumber,SIGNAL(valueChanged(int)), this, SLOT(update()));
	}
	
	~CannyDialog(){}
	
	public slots:
	void update();
};

#endif



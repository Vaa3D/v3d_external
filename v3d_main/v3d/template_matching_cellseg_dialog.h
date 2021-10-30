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




/****************************************************************************
**
template_matching_cellseg_dialog.h
 by Hanchuan Peng
 2009-April-28
**
****************************************************************************/

#ifndef __TEMPLATE_MATCHING_CELLSEG_DIALOG_H__
#define __TEMPLATE_MATCHING_CELLSEG_DIALOG_H__

#include <string>
#include <vector>
using namespace std;

#include <QDialog>
#include <QFileDialog>

#include "ui_template_matching_cellseg.h"

struct para_template_matching_cellseg
{
	int channo;
	int szx, szy, szz; //dim
	float stdx, stdy, stdz; //std
	float t_pixval, t_rgnval, t_corrcoef; //thresholds
	int kernel; //use Gaussian kernel or sth else
	float merge_radius;

	para_template_matching_cellseg() 
	{
		channo=0;
		szx=5; szy=5; szz=1; //dim
		stdx=3.0; stdy=3.0; stdz=0.5; //std
		t_pixval=50; t_rgnval=20; t_corrcoef=0.3f; //thresholds
		merge_radius = 20;
		kernel=0; //0 - Gaussian
	}
};

class para_template_matching_cellseg_dialog : public QDialog, private Ui_Dialog_TemplateMatchingSegPara
{
    Q_OBJECT

public:
    para_template_matching_cellseg_dialog(int chan_id_max, para_template_matching_cellseg * p)
	{
		setupUi(this);
	
		connect(pushButton_ok, SIGNAL(clicked()), this, SLOT(accept()));
		connect(pushButton_cancel, SIGNAL(clicked()), this, SLOT(reject()));

		spinBox_Channel->setRange(1, chan_id_max);
		spinBox_Channel->setValue(p->channo+1);

		comboBox_Kernel->setCurrentIndex(p->kernel);

		spinBox_szx->setValue(p->szx);
		spinBox_szy->setValue(p->szy);
		spinBox_szz->setValue(p->szz);

		doubleSpinBox_stdx->setValue(p->stdx);
		doubleSpinBox_stdy->setValue(p->stdy);
		doubleSpinBox_stdz->setValue(p->stdz);

		doubleSpinBox_pixelval->setValue(p->t_pixval);
		doubleSpinBox_rgnval->setValue(p->t_rgnval);
		doubleSpinBox_corrcoef->setValue(p->t_corrcoef);
		
		doubleSpinBox_rgn_merge_radius->setValue(p->merge_radius);
	}
	void fetchData(para_template_matching_cellseg * p)
	{
		p->channo=spinBox_Channel->value()-1;
		p->szx=spinBox_szx->value(); 
		p->szy=spinBox_szy->value(); 
		p->szz=spinBox_szz->value(); 
		p->stdx=doubleSpinBox_stdx->value(); 
		p->stdy=doubleSpinBox_stdy->value(); 
		p->stdz=doubleSpinBox_stdz->value();
		p->t_pixval=doubleSpinBox_pixelval->value(); 
		p->t_rgnval=doubleSpinBox_rgnval->value(); 
		p->t_corrcoef=doubleSpinBox_corrcoef->value();
		p->merge_radius=doubleSpinBox_rgn_merge_radius->value();
		p->kernel=(comboBox_Kernel->currentText()=="Gaussian")?0:0;
	}
	void setEnabledChannelField(bool v) {if (spinBox_Channel) spinBox_Channel->setEnabled(v);}
	
private:
	
public slots:
};

#endif

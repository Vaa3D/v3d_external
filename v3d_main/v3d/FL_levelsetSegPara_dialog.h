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
FL_levelsetSegPara_dialog.h
 by Hanchuan Peng
 2009-Jan
**
****************************************************************************/

#ifndef __FL_levelsetSegPara_dialog_H__
#define __FL_levelsetSegPara_dialog_H__

#include <string>
#include <vector>
using namespace std;

#include <QDialog>
#include <QFileDialog>

#include "ui_FL_levelsetSegPara.h"

struct FL_levelsetSegPara
{
	int channo;
	int dim;
	int startslice;
	int endslice;
	double lambda;
	double alpha;
	double epsilon;
	double delta;
	vector <int> rgns;
	bool b_useModel;
	string modelFilename;
};

class FL_levelsetSegPara_dialog : public QDialog, private Ui_Dialog_levelset_para
{
    Q_OBJECT

public:
    FL_levelsetSegPara_dialog(int chan_id_max, int slice_num_max, FL_levelsetSegPara * p)
	{
		setupUi(this);
	
		connect(pushButton_ok, SIGNAL(clicked()), this, SLOT(accept()));
		connect(pushButton_cancel, SIGNAL(clicked()), this, SLOT(reject()));
		connect(pushButton_file, SIGNAL(clicked()), this, SLOT(selectfile()));

		spinBox_Channel->setRange(1, chan_id_max);
		spinBox_Channel->setValue(p->channo);

		comboBox_Dimension->setCurrentIndex((p->dim==2)?0:1);

		spinBox_StartSlice->setRange(1, slice_num_max);
		spinBox_EndSlice->setRange(1, slice_num_max);

		doubleSpinBox_lamda->setMinimum(0);
		doubleSpinBox_lamda->setValue(p->lambda);

		doubleSpinBox_alpha->setMinimum(0);
		doubleSpinBox_alpha->setValue(p->alpha);

		doubleSpinBox_epsilon->setMinimum(0);
		doubleSpinBox_epsilon->setValue(p->epsilon);
		
		doubleSpinBox_delta->setMinimum(0);
		doubleSpinBox_delta->setValue(p->delta);

		QString tmp, tmp1; 
		for (int i=0;i<p->rgns.size();i++)
		{
			tmp1.setNum(p->rgns.at(i));
			if (i==0) {tmp = tmp1; } 
			else {tmp1.prepend(","); tmp.append(tmp1);}
		}
		lineEdit_regions->setText(tmp);
		
		checkBox_usemodel->setChecked(p->b_useModel);

		lineEdit_ModelFilename->setText(p->modelFilename.c_str());
	}
	void fetchData(FL_levelsetSegPara * p)
	{
		p->channo = spinBox_Channel->value()-1;
		p->dim = (comboBox_Dimension->currentText()=="2D")?2:3;
		p->startslice = spinBox_StartSlice->value()-1;
		p->endslice = spinBox_EndSlice->value()-1;
		p->lambda = doubleSpinBox_lamda->value();
		p->alpha = doubleSpinBox_alpha->value();
		p->epsilon = doubleSpinBox_epsilon->value();
		p->delta = doubleSpinBox_delta->value();
		p->rgns.clear();
		QStringList list1 = lineEdit_regions->text().split(",");
		for (int i=0;i<list1.size();i++)
		{
			QString tmp = list1.at(i).trimmed();
			if (!tmp.isEmpty()) p->rgns.push_back(tmp.toUInt());
		}
		p->b_useModel = checkBox_usemodel->isChecked();
		p->modelFilename = qPrintable(lineEdit_ModelFilename->text());
	}
	
private:
	
public slots:
	void selectfile() 
	{
		QString fileName = QFileDialog::getOpenFileName(this, tr("Select a model file"), "", tr("Model Files (*.tif *.raw)"));
		if (!fileName.isEmpty()) lineEdit_ModelFilename->setText(fileName);
	}
};

#endif

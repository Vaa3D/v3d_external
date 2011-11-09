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
curve_trace_para_dialog.h
 by Hanchuan Peng
 2009-Jan
**
****************************************************************************/

#ifndef __CURVE_TRACE_PARA_DIALOG_H__
#define __CURVE_TRACE_PARA_DIALOG_H__

#include "../basic_c_fun/v3d_curvetracepara.h"

#include <QDialog>

#include "ui_dialog_curve_trace.h"


class CurveTraceParaDialog : public QDialog, private Ui_curve_trace_dialog
{
    Q_OBJECT

public:
    CurveTraceParaDialog(int chan_id_max, V3DLONG n_landmark_max, CurveTracePara & trace_para)
	{
		setupUi(this);

		connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

		spinBox_datachannel->setMaximum((chan_id_max<1)?1:chan_id_max);
		spinBox_datachannel->setValue(trace_para.channo+1);
		spinBox_landmark_start->setRange(1, n_landmark_max);
		spinBox_landmark_start->setValue(trace_para.landmark_id_start+1);
		spinBox_landmark_end->setRange(1, n_landmark_max);
		spinBox_landmark_end->setValue(trace_para.landmark_id_end+1);

		if (trace_para.sp_num_end_nodes==0)
		{
			label_end_landmark->setText(tr("through entire image"));
			spinBox_landmark_end->hide();
		}
		if (trace_para.sp_num_end_nodes>=2)
		{
			label_end_landmark->setText(tr("to all other landmarks"));
			spinBox_landmark_end->hide();
		}

		//checkBox_b_curve->setCheckable(false); //disable shortest path method as of now
		checkBox_b_curve->setChecked(trace_para.b_deformcurve==0);
		checkBox_b_6connect->setChecked(trace_para.sp_graph_connect==0);
		checkBox_b_fullimage->setChecked(trace_para.sp_graph_background==0);

		spinBox_sp_nodestep->setValue(trace_para.sp_graph_resolution_step);
		spinBox_sp_nodestep->setRange(1, 20);
		spinBox_sp_downsamplestep->setValue(trace_para.sp_downsample_step);
		spinBox_sp_downsamplestep->setRange(1, 20);
		spinBox_sp_smoothwinsz->setValue(trace_para.sp_smoothing_win_sz);
		spinBox_sp_smoothwinsz->setRange(1, 20);

		spinBox_nloops->setValue(trace_para.nloops);
		spinBox_n_controlpts->setRange(1, 100);
		spinBox_n_controlpts->setValue(trace_para.n_points);
		checkBox_adaptiveCtrlPoints->setChecked(trace_para.b_adaptiveCtrlPoints);

		doubleSpinBox_1->setRange(0, 10);
		doubleSpinBox_1->setSingleStep(0.1);
		doubleSpinBox_1->setValue(trace_para.internal_force_weight);
		doubleSpinBox_2->setRange(0, 10);
		doubleSpinBox_2->setSingleStep(0.1);
		doubleSpinBox_2->setValue(trace_para.internal_force2_weight);
		doubleSpinBox_3->setRange(0, 10);
		doubleSpinBox_3->setSingleStep(0.1);
		doubleSpinBox_3->setValue(trace_para.prior_force_weight);

		checkBox_estimateRadii->setChecked(trace_para.b_estRadii);
		checkBox_postMergeClosebyBranches->setChecked(trace_para.b_postMergeClosebyBranches);
	}
	void fetchData(CurveTracePara * p)
	{
		p->channo = spinBox_datachannel->value()-1;
		p->landmark_id_start = spinBox_landmark_start->value()-1;
		p->landmark_id_end = spinBox_landmark_end->value()-1;

		p->b_deformcurve = !(checkBox_b_curve->isChecked());
		p->sp_graph_connect = checkBox_b_6connect->isChecked()? 0:1;
		p->sp_graph_background = checkBox_b_fullimage->isChecked()? 0:1;

		p->sp_graph_resolution_step = spinBox_sp_nodestep->value();
		p->sp_downsample_step = spinBox_sp_downsamplestep->value();
		p->sp_smoothing_win_sz = spinBox_sp_smoothwinsz->value();

		p->nloops = spinBox_nloops->value();
		p->n_points = spinBox_n_controlpts->value();
		p->b_adaptiveCtrlPoints = checkBox_adaptiveCtrlPoints->isChecked();

		p->internal_force_weight  = doubleSpinBox_1->value();
		p->internal_force2_weight = doubleSpinBox_2->value();
		p->prior_force_weight = doubleSpinBox_3->value();
		
		p->b_estRadii = checkBox_estimateRadii->isChecked();
		p->b_postMergeClosebyBranches = checkBox_postMergeClosebyBranches->isChecked();
	}
	void setHideState_spinBox_landmark_start(bool b_hide) {if (spinBox_landmark_start) if (b_hide){spinBox_landmark_start->hide(); label_end_landmark->hide();} else {spinBox_landmark_start->show();label_start_landmark->show();}}
	void setHideState_spinBox_landmark_end(bool b_hide) {if (spinBox_landmark_end) if (b_hide) {spinBox_landmark_end->hide(); label_end_landmark->hide();} else {spinBox_landmark_end->show();label_end_landmark->show();}}
	void setEnabledState_checkBox_b_curve(bool b_enabled) {if (checkBox_b_curve) {checkBox_b_curve->setCheckable(b_enabled); checkBox_b_curve->setText("Hybrid curve parameters (DISABLED)");}}

private:

public slots:

};

#endif

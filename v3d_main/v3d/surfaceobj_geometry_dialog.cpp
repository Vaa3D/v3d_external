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
surfaceobj_geometry_dialog.cpp

by Hanchuan Peng
2009-Feb-22
 last edit: try to added negative scaling, but it seem not meaningful idea
2009-0303: add G-scaling, and revise back the original scaling parts
 **
****************************************************************************/

#include "surfaceobj_geometry_dialog.h"

#include "../3drenderer/v3dr_glwidget.h"
#include "../3drenderer/renderer_tex2.h"

#include "../neuron_editing/neuron_xforms.h"
#include "../neuron_editing/apo_xforms.h"

#define POST_3DVIEWER_updateGL {\
	if (glwidget) glwidget->update();\
}

#define MY_ANGLE_TICK 16

#define B_USE_NEGATIVE_SCALING	1

//#define __SPECIAL_CHANGE_NEURON_SECOND__		1 //just for s epcial experiment to shift/mirror a second neuron use the same transform


double NORMALIZE_ROTATION_AngleStep( double angle )
{
    while (angle < -180 * MY_ANGLE_TICK)   angle += 360 * MY_ANGLE_TICK;
    while (angle >  180 * MY_ANGLE_TICK)   angle -= 360 * MY_ANGLE_TICK;
	return angle;
}

#define MY_PI 3.141592635

SurfaceObjGeometryDialog::SurfaceObjGeometryDialog(V3dR_GLWidget *w, Renderer_tex2 *r, int dc, int st, int i)
{
	glwidget = w;
	renderer = r;
	dataclass = dc;
	surfaceobjtype = st;
	objectindex = i;

	p_tree = 0;
	p_apo = 0;
	if (w && r)
	{
		if (dc==dcSurface && st==stNeuronStructure)
		{
			tree0.copy(r->getHandleNeuronTrees()->at(i));
			p_tree = (NeuronTree *)(&(r->getHandleNeuronTrees()->at(i)));
#ifdef __SPECIAL_CHANGE_NEURON_SECOND__
			int myii=(i==0)?1:0;
			p_tree1 = (NeuronTree *)(&(r->getHandleNeuronTrees()->at(myii)));
#endif
		}
		if (dc==dcSurface && st==stPointCloud)
		{
			apo0 = *(r->getHandleAPOCellList());
			p_apo = r->getHandleAPOCellList();
		}
	}

	//set default
	resetInternalStates();

	updateContent();
}

void SurfaceObjGeometryDialog::resetInternalStates()
{
	cur_shift_x = cur_shift_y = cur_shift_z = 0;
	cur_scale_x = cur_scale_y = cur_scale_z = 1;
	cur_gscale_x = cur_gscale_y = cur_gscale_z = 1;
	cur_scale_r = 1;
	cur_rotate_x = cur_rotate_y = cur_rotate_z = 0;


	cur_cx=cur_cy=cur_cz=0;
	if (p_tree)
		getNeuronTreeCenter(p_tree, cur_cx, cur_cy, cur_cz);
	if (p_apo) //note that as only one of p_tree or p_apo will be no-zero, this operation should be safe
		getAPOCellListCenter(p_apo, cur_cx, cur_cy, cur_cz);

	cur_use_rotation=false;
	cur_flip_x = cur_flip_y = cur_flip_z = false;
}

void SurfaceObjGeometryDialog::updateContent()
{
	create();
}

void SurfaceObjGeometryDialog::fetchData()
{
}

void highlight_dial(QDial *d)
{
	QPalette pe = d->palette();
	pe.setBrush(QPalette::Button, pe.highlight());
	d->setPalette(pe);

	d->setRange(0, 360 * MY_ANGLE_TICK);
	d->setSingleStep(1 * MY_ANGLE_TICK);
	d->setPageStep(10 * MY_ANGLE_TICK);
	d->setWrapping(true);
	d->setNotchesVisible(true);
}

void SurfaceObjGeometryDialog::create()
{
	setupUi(this);

	connect(pushButton_ok, SIGNAL(clicked()), this, SLOT(accept()));
	connect(pushButton_cancel, SIGNAL(clicked()), this, SLOT(reject()));
	connect(pushButton_reset, SIGNAL(clicked()), this, SLOT(reset()));
	connect(pushButton_undo_last, SIGNAL(clicked()), this, SLOT(undo()));

	//default values and events
	doubleSpinBox_shift_x->setRange(-100000,100000); doubleSpinBox_shift_x->setValue(cur_shift_x);
	doubleSpinBox_shift_y->setRange(-100000,100000); doubleSpinBox_shift_y->setValue(cur_shift_y);
	doubleSpinBox_shift_z->setRange(-100000,100000); doubleSpinBox_shift_z->setValue(cur_shift_z);

	connect(doubleSpinBox_shift_x, SIGNAL(valueChanged(double)), this, SLOT(shift_x(double)));
	connect(doubleSpinBox_shift_y, SIGNAL(valueChanged(double)), this, SLOT(shift_y(double)));
	connect(doubleSpinBox_shift_z, SIGNAL(valueChanged(double)), this, SLOT(shift_z(double)));

	doubleSpinBox_scale_x->setRange(1,100000); doubleSpinBox_scale_x->setValue(cur_scale_x*1000);
	doubleSpinBox_scale_y->setRange(1,100000); doubleSpinBox_scale_y->setValue(cur_scale_y*1000);
	doubleSpinBox_scale_z->setRange(1,100000); doubleSpinBox_scale_z->setValue(cur_scale_z*1000);
	doubleSpinBox_scale_r->setRange(1,100000); doubleSpinBox_scale_r->setValue(cur_scale_r*1000);

	connect(doubleSpinBox_scale_x, SIGNAL(valueChanged(double)), this, SLOT(scale_x(double)));
	connect(doubleSpinBox_scale_y, SIGNAL(valueChanged(double)), this, SLOT(scale_y(double)));
	connect(doubleSpinBox_scale_z, SIGNAL(valueChanged(double)), this, SLOT(scale_z(double)));
	connect(doubleSpinBox_scale_r, SIGNAL(valueChanged(double)), this, SLOT(scale_r(double)));

#ifdef B_USE_NEGATIVE_SCALING
	double scale_lower_limit=-100000;
#else
	double scale_lower_limit=1;
#endif

	doubleSpinBox_gscale_x->setRange(scale_lower_limit,100000); doubleSpinBox_gscale_x->setValue(cur_gscale_x*1000);
	doubleSpinBox_gscale_y->setRange(scale_lower_limit,100000); doubleSpinBox_gscale_y->setValue(cur_gscale_y*1000);
	doubleSpinBox_gscale_z->setRange(scale_lower_limit,100000); doubleSpinBox_gscale_z->setValue(cur_gscale_z*1000);

	connect(doubleSpinBox_gscale_x, SIGNAL(valueChanged(double)), this, SLOT(gscale_x(double)));
	connect(doubleSpinBox_gscale_y, SIGNAL(valueChanged(double)), this, SLOT(gscale_y(double)));
	connect(doubleSpinBox_gscale_z, SIGNAL(valueChanged(double)), this, SLOT(gscale_z(double)));

	highlight_dial(dial_x);
	highlight_dial(dial_y);
	highlight_dial(dial_z);

	dial_x->setValue(cur_rotate_x);
	dial_y->setValue(cur_rotate_y);
	dial_z->setValue(cur_rotate_z);
	checkBox_enable_rotation->setChecked(cur_use_rotation);

	connect(dial_x, SIGNAL(valueChanged(int)), this, SLOT(rotate_around_x(int)));
	connect(dial_y, SIGNAL(valueChanged(int)), this, SLOT(rotate_around_y(int)));
	connect(dial_z, SIGNAL(valueChanged(int)), this, SLOT(rotate_around_z(int)));
	connect(checkBox_enable_rotation, SIGNAL(stateChanged(int)), this, SLOT(change_use_rotation(int)));

	dial_x->setDisabled(!cur_use_rotation);
	dial_y->setDisabled(!cur_use_rotation);
	dial_z->setDisabled(!cur_use_rotation);

	checkBox_flip_x->setChecked(cur_flip_x);
	checkBox_flip_y->setChecked(cur_flip_y);
	checkBox_flip_z->setChecked(cur_flip_z);

	connect(checkBox_flip_x, SIGNAL(stateChanged(int)), this, SLOT(flip_x(int)));
	connect(checkBox_flip_y, SIGNAL(stateChanged(int)), this, SLOT(flip_y(int)));
	connect(checkBox_flip_z, SIGNAL(stateChanged(int)), this, SLOT(flip_z(int)));
}

void SurfaceObjGeometryDialog::reset()
{
	if (p_tree || p_apo)
	{
		resetInternalStates();

		doubleSpinBox_shift_x->setValue(cur_shift_x);
		doubleSpinBox_shift_y->setValue(cur_shift_y);
		doubleSpinBox_shift_z->setValue(cur_shift_z);

		doubleSpinBox_scale_x->setValue(cur_scale_x*1000);
		doubleSpinBox_scale_y->setValue(cur_scale_y*1000);
		doubleSpinBox_scale_z->setValue(cur_scale_z*1000);
		doubleSpinBox_scale_r->setValue(cur_scale_r*1000);

		dial_x->setValue(cur_rotate_x);
		dial_y->setValue(cur_rotate_y);
		dial_z->setValue(cur_rotate_z);

		checkBox_flip_x->setChecked(cur_flip_x);
		checkBox_flip_y->setChecked(cur_flip_y);
		checkBox_flip_z->setChecked(cur_flip_z);

		doubleSpinBox_gscale_x->setValue(cur_gscale_x*1000);
		doubleSpinBox_gscale_y->setValue(cur_gscale_y*1000);
		doubleSpinBox_gscale_z->setValue(cur_gscale_z*1000);

		if (glwidget && renderer)
		{
			if (dataclass==dcSurface && surfaceobjtype==stNeuronStructure)
			{
				p_tree->copyGeometry(tree0);
				qDebug()<<"copied back the default in reset()";
			}
		}
	}
    POST_3DVIEWER_updateGL
}

void SurfaceObjGeometryDialog::undo()
{
	if (p_tree || p_apo)
	{
		switch(cur_op)
		{
			case SOG_SHIFT_X:
				doubleSpinBox_shift_x->setValue(last_val);
				break;
			case SOG_SHIFT_Y:
				doubleSpinBox_shift_y->setValue(last_val);
				break;
			case SOG_SHIFT_Z:
				doubleSpinBox_shift_z->setValue(last_val);
				break;
			case SOG_SCALE_X:
				doubleSpinBox_scale_x->setValue(last_val);
				break;
			case SOG_SCALE_Y:
				doubleSpinBox_scale_y->setValue(last_val);
				break;
			case SOG_SCALE_Z:
				doubleSpinBox_scale_z->setValue(last_val);
				break;
			case SOG_GSCALE_X:
				doubleSpinBox_gscale_x->setValue(last_val);
				break;
			case SOG_GSCALE_Y:
				doubleSpinBox_gscale_y->setValue(last_val);
				break;
			case SOG_GSCALE_Z:
				doubleSpinBox_gscale_z->setValue(last_val);
				break;
			case SOG_SCALE_R:
				doubleSpinBox_scale_r->setValue(last_val);
				break;
			case SOG_FLIP_X:
				checkBox_flip_x->setChecked(last_val);
				break;
			case SOG_FLIP_Y:
				checkBox_flip_y->setChecked(last_val);
				break;
			case SOG_FLIP_Z:
				checkBox_flip_z->setChecked(last_val);
				break;
			case SOG_ROT_X:
				dial_x->setValue(last_val);
				break;
			case SOG_ROT_Y:
				dial_y->setValue(last_val);
				break;
			case SOG_ROT_Z:
				dial_z->setValue(last_val);
				break;
			default:
				return;
		}

//		if (pushButton_undo_last->text()=="Undo one step")
//			pushButton_undo_last->setText("Redo one step");
//		else
//			pushButton_undo_last->setText("Undo one step");
//
	}
    POST_3DVIEWER_updateGL
}

void SurfaceObjGeometryDialog::shift_x(double s)
{
	if (p_tree)
	{
		last_val = cur_shift_x; cur_op = SOG_SHIFT_X;
//		if (pushButton_undo_last->text()=="Redo one step") pushButton_undo_last->setText("Undo one step");

		proc_neuron_add_offset(p_tree, s-cur_shift_x, 0, 0);
#ifdef __SPECIAL_CHANGE_NEURON_SECOND__		
		proc_neuron_add_offset(p_tree1, s-cur_shift_x, 0, 0);
#endif
		cur_cx += s-cur_shift_x;
		cur_shift_x = s;
	}
	if (p_apo)
	{
		last_val = cur_shift_x; cur_op = SOG_SHIFT_X;
		proc_apo_add_offset(p_apo, s-cur_shift_x, 0, 0);
		cur_cx += s-cur_shift_x;
		cur_shift_x = s;
	}

	POST_3DVIEWER_updateGL
}

void SurfaceObjGeometryDialog::shift_y(double s)
{
	if (p_tree)
	{
		last_val = cur_shift_y; cur_op = SOG_SHIFT_Y;
//		if (pushButton_undo_last->text()=="Redo one step") pushButton_undo_last->setText("Undo one step");

		proc_neuron_add_offset(p_tree, 0, s-cur_shift_y, 0);
#ifdef __SPECIAL_CHANGE_NEURON_SECOND__		
		proc_neuron_add_offset(p_tree1, 0, s-cur_shift_y, 0);
#endif
		cur_cy += s-cur_shift_y;
	    cur_shift_y = s;
	}
	if (p_apo)
	{
		last_val = cur_shift_y; cur_op = SOG_SHIFT_Y;
		proc_apo_add_offset(p_apo, 0, s-cur_shift_y, 0);
		cur_cy += s-cur_shift_y;
	    cur_shift_y = s;
	}
	POST_3DVIEWER_updateGL
}

void SurfaceObjGeometryDialog::shift_z(double s)
{
	if (p_tree)
	{
		last_val = cur_shift_z; cur_op = SOG_SHIFT_Z;
//		if (pushButton_undo_last->text()=="Redo one step") pushButton_undo_last->setText("Undo one step");

		proc_neuron_add_offset(p_tree, 0, 0, s-cur_shift_z);
#ifdef __SPECIAL_CHANGE_NEURON_SECOND__		
		proc_neuron_add_offset(p_tree1, 0, 0, s-cur_shift_z);
#endif
		cur_cz += s-cur_shift_z;
	    cur_shift_z = s;
	}
	if (p_apo)
	{
		last_val = cur_shift_z; cur_op = SOG_SHIFT_Z;
		proc_apo_add_offset(p_apo, 0, 0, s-cur_shift_z);
		cur_cz += s-cur_shift_z;
	    cur_shift_z = s;
	}
	POST_3DVIEWER_updateGL
}

void SurfaceObjGeometryDialog::scale_x(double s)
{
	if (p_tree)
	{
		last_val = cur_scale_x*1000; cur_op = SOG_SCALE_X;
//		if (pushButton_undo_last->text()=="Redo one step") pushButton_undo_last->setText("Undo one step");

		proc_neuron_multiply_factor(p_tree, s/(cur_scale_x*1000), 1, 1);
	    cur_scale_x = s/1000.0;
	}
	if (p_apo)
	{
		last_val = cur_scale_x*1000; cur_op = SOG_SCALE_X;
		proc_apo_multiply_factor(p_apo, s/(cur_scale_x*1000), 1, 1);
	    cur_scale_x = s/1000.0;
	}
	POST_3DVIEWER_updateGL
}

void SurfaceObjGeometryDialog::scale_y(double s)
{
	if (p_tree)
	{
		last_val = cur_scale_y*1000; cur_op = SOG_SCALE_Y;
//		if (pushButton_undo_last->text()=="Redo one step") pushButton_undo_last->setText("Undo one step");

		proc_neuron_multiply_factor(p_tree, 1, s/(cur_scale_y*1000), 1);
	    cur_scale_y = s/1000.0;
	}
	if (p_apo)
	{
		last_val = cur_scale_y*1000; cur_op = SOG_SCALE_Y;
		proc_apo_multiply_factor(p_apo, 1, s/(cur_scale_y*1000), 1);
	    cur_scale_y = s/1000.0;
	}
	POST_3DVIEWER_updateGL
}

void SurfaceObjGeometryDialog::scale_z(double s)
{
	if (p_tree)
	{
		last_val = cur_scale_z*1000; cur_op = SOG_SCALE_Z;
//		if (pushButton_undo_last->text()=="Redo one step") pushButton_undo_last->setText("Undo one step");

		proc_neuron_multiply_factor(p_tree, 1, 1, s/(cur_scale_z*1000));
        cur_scale_z = s/1000.0;
	}
	if (p_apo)
	{
		last_val = cur_scale_z*1000; cur_op = SOG_SCALE_Z;
		proc_apo_multiply_factor(p_apo, 1, 1, s/(cur_scale_z*1000));
        cur_scale_z = s/1000.0;
	}
	POST_3DVIEWER_updateGL
}

void SurfaceObjGeometryDialog::gscale_x(double s)
{
	if (p_tree)
	{
		last_val = cur_gscale_x*1000; cur_op = SOG_GSCALE_X;
		//		if (pushButton_undo_last->text()=="Redo one step") pushButton_undo_last->setText("Undo one step");

#ifdef B_USE_NEGATIVE_SCALING
		if (s<0.0001 && s>=0) {s=0.0001; QMessageBox::warning(0, "invalid value input", "You have entered a very small value which is not allowed. V3D will just use 0.0001 to replace it.");}
		else if (s<0 && s>-0.0001) {s=-0.0001; QMessageBox::warning(0, "invalid value input", "You have entered a very small value which is not allowed. V3D will just use -0.0001 to replace it.");}
#endif

		proc_neuron_gmultiply_factor(p_tree, s/(cur_gscale_x*1000), 1, 1);
	    cur_gscale_x = s/1000.0;
	}
	if (p_apo)
	{
		last_val = cur_gscale_x*1000; cur_op = SOG_GSCALE_X;
		//		if (pushButton_undo_last->text()=="Redo one step") pushButton_undo_last->setText("Undo one step");

#ifdef B_USE_NEGATIVE_SCALING
		if (s<0.0001 && s>=0) {s=0.0001; QMessageBox::warning(0, "invalid value input", "You have entered a very small value which is not allowed. V3D will just use 0.0001 to replace it.");}
		else if (s<0 && s>-0.0001) {s=-0.0001; QMessageBox::warning(0, "invalid value input", "You have entered a very small value which is not allowed. V3D will just use -0.0001 to replace it.");}
#endif

		proc_apo_gmultiply_factor(p_apo, s/(cur_gscale_x*1000), 1, 1);
	    cur_gscale_x = s/1000.0;
	}
	POST_3DVIEWER_updateGL
}

void SurfaceObjGeometryDialog::gscale_y(double s)
{
	if (p_tree)
	{
		last_val = cur_gscale_y*1000; cur_op = SOG_GSCALE_Y;
		//		if (pushButton_undo_last->text()=="Redo one step") pushButton_undo_last->setText("Undo one step");

#ifdef B_USE_NEGATIVE_SCALING
		if (s<0.0001 && s>=0) {s=0.0001; QMessageBox::warning(0, "invalid value input", "You have entered a very small value which is not allowed. V3D will just use 0.0001 to replace it.");}
		else if (s<0 && s>-0.0001) {s=-0.0001; QMessageBox::warning(0, "invalid value input", "You have entered a very small value which is not allowed. V3D will just use -0.0001 to replace it.");}
#endif

		proc_neuron_gmultiply_factor(p_tree, 1, s/(cur_gscale_y*1000), 1);
	    cur_gscale_y = s/1000.0;
	}
	if (p_apo)
	{
		last_val = cur_gscale_y*1000; cur_op = SOG_GSCALE_Y;
		//		if (pushButton_undo_last->text()=="Redo one step") pushButton_undo_last->setText("Undo one step");

#ifdef B_USE_NEGATIVE_SCALING
		if (s<0.0001 && s>=0) {s=0.0001; QMessageBox::warning(0, "invalid value input", "You have entered a very small value which is not allowed. V3D will just use 0.0001 to replace it.");}
		else if (s<0 && s>-0.0001) {s=-0.0001; QMessageBox::warning(0, "invalid value input", "You have entered a very small value which is not allowed. V3D will just use -0.0001 to replace it.");}
#endif

		proc_apo_gmultiply_factor(p_apo, 1, s/(cur_gscale_y*1000), 1);
	    cur_gscale_y = s/1000.0;
	}
	POST_3DVIEWER_updateGL
}

void SurfaceObjGeometryDialog::gscale_z(double s)
{
	if (p_tree)
	{
		last_val = cur_gscale_z*1000; cur_op = SOG_GSCALE_Z;
		//		if (pushButton_undo_last->text()=="Redo one step") pushButton_undo_last->setText("Undo one step");

#ifdef B_USE_NEGATIVE_SCALING
		if (s<0.0001 && s>=0) {s=0.0001; QMessageBox::warning(0, "invalid value input", "You have entered a very small value which is not allowed. V3D will just use 0.0001 to replace it.");}
		else if (s<0 && s>-0.0001) {s=-0.0001; QMessageBox::warning(0, "invalid value input", "You have entered a very small value which is not allowed. V3D will just use -0.0001 to replace it.");}
#endif

		proc_neuron_gmultiply_factor(p_tree, 1, 1, s/(cur_gscale_z*1000));
        cur_gscale_z = s/1000.0;
	}
	if (p_apo)
	{
		last_val = cur_gscale_z*1000; cur_op = SOG_GSCALE_Z;
		//		if (pushButton_undo_last->text()=="Redo one step") pushButton_undo_last->setText("Undo one step");

#ifdef B_USE_NEGATIVE_SCALING
		if (s<0.0001 && s>=0) {s=0.0001; QMessageBox::warning(0, "invalid value input", "You have entered a very small value which is not allowed. V3D will just use 0.0001 to replace it.");}
		else if (s<0 && s>-0.0001) {s=-0.0001; QMessageBox::warning(0, "invalid value input", "You have entered a very small value which is not allowed. V3D will just use -0.0001 to replace it.");}
#endif

		proc_apo_gmultiply_factor(p_apo, 1, 1, s/(cur_gscale_z*1000));
        cur_gscale_z = s/1000.0;
	}
	POST_3DVIEWER_updateGL
}

void SurfaceObjGeometryDialog::scale_r(double s)
{
	if (p_tree)
	{
		last_val = cur_scale_r*1000; cur_op = SOG_SCALE_R;
//		if (pushButton_undo_last->text()=="Redo one step") pushButton_undo_last->setText("Undo one step");

		proc_neuron_multiply_factor_radius(p_tree, s/(cur_scale_r*1000));
        cur_scale_r = s/1000.0;
	}
	if (p_apo)
	{
		last_val = cur_scale_r*1000; cur_op = SOG_SCALE_R;
		//		if (pushButton_undo_last->text()=="Redo one step") pushButton_undo_last->setText("Undo one step");

		proc_apo_multiply_factor_radius(p_apo, s/(cur_scale_r*1000));
        cur_scale_r = s/1000.0;
	}
	POST_3DVIEWER_updateGL
}

void SurfaceObjGeometryDialog::rotate_around_x(int v)
{
	if (p_tree)
	{
		last_val = cur_rotate_x; cur_op = SOG_ROT_X;
//		if (pushButton_undo_last->text()=="Redo one step") pushButton_undo_last->setText("Undo one step");

		double afmatrix[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
		double a = NORMALIZE_ROTATION_AngleStep( v-cur_rotate_x ) / 180.0 * MY_PI;
		afmatrix[5] = cos(a); afmatrix[6] = -sin(a);
		afmatrix[9] = -afmatrix[6]; afmatrix[10] = afmatrix[5];
		proc_neuron_affine_around_center(p_tree, afmatrix, cur_cx, cur_cy, cur_cz);
        cur_rotate_x = v;
	}
	if (p_apo)
	{
		last_val = cur_rotate_x; cur_op = SOG_ROT_X;
		//		if (pushButton_undo_last->text()=="Redo one step") pushButton_undo_last->setText("Undo one step");

		double afmatrix[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
		double a = NORMALIZE_ROTATION_AngleStep( v-cur_rotate_x ) / 180.0 * MY_PI;
		afmatrix[5] = cos(a); afmatrix[6] = -sin(a);
		afmatrix[9] = -afmatrix[6]; afmatrix[10] = afmatrix[5];
		proc_apo_affine_around_center(p_apo, afmatrix, cur_cx, cur_cy, cur_cz);
        cur_rotate_x = v;
	}
	POST_3DVIEWER_updateGL
}

void SurfaceObjGeometryDialog::rotate_around_y(int v)
{
	if (p_tree)
	{
		last_val = cur_rotate_y; cur_op = SOG_ROT_Y;
//		if (pushButton_undo_last->text()=="Redo one step") pushButton_undo_last->setText("Undo one step");

		double afmatrix[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
		double a = NORMALIZE_ROTATION_AngleStep( v-cur_rotate_y ) / 180.0 * MY_PI;
		afmatrix[0] = cos(a); afmatrix[2] = sin(a);
		afmatrix[8] = -afmatrix[2]; afmatrix[10] = afmatrix[0];
		proc_neuron_affine_around_center(p_tree, afmatrix, cur_cx, cur_cy, cur_cz);
        cur_rotate_y = v;
	}
	if (p_apo)
	{
		last_val = cur_rotate_y; cur_op = SOG_ROT_Y;
		//		if (pushButton_undo_last->text()=="Redo one step") pushButton_undo_last->setText("Undo one step");

		double afmatrix[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
		double a = NORMALIZE_ROTATION_AngleStep( v-cur_rotate_y ) / 180.0 * MY_PI;
		afmatrix[0] = cos(a); afmatrix[2] = sin(a);
		afmatrix[8] = -afmatrix[2]; afmatrix[10] = afmatrix[0];
		proc_apo_affine_around_center(p_apo, afmatrix, cur_cx, cur_cy, cur_cz);
        cur_rotate_y = v;
	}
	POST_3DVIEWER_updateGL
}

void SurfaceObjGeometryDialog::rotate_around_z(int v)
{
	if (p_tree)
	{
		last_val = cur_rotate_z; cur_op = SOG_ROT_Z;
//		if (pushButton_undo_last->text()=="Redo one step") pushButton_undo_last->setText("Undo one step");

		double afmatrix[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
		double a = NORMALIZE_ROTATION_AngleStep( v-cur_rotate_z ) / 180.0 * MY_PI;
		afmatrix[0] = cos(a); afmatrix[1] = -sin(a);
		afmatrix[4] = -afmatrix[1]; afmatrix[5] = afmatrix[0];
		proc_neuron_affine_around_center(p_tree, afmatrix, cur_cx, cur_cy, cur_cz);
        cur_rotate_z = v;
	}
	if (p_apo)
	{
		last_val = cur_rotate_z; cur_op = SOG_ROT_Z;
		//		if (pushButton_undo_last->text()=="Redo one step") pushButton_undo_last->setText("Undo one step");

		double afmatrix[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
		double a = NORMALIZE_ROTATION_AngleStep( v-cur_rotate_z ) / 180.0 * MY_PI;
		afmatrix[0] = cos(a); afmatrix[1] = -sin(a);
		afmatrix[4] = -afmatrix[1]; afmatrix[5] = afmatrix[0];
		proc_apo_affine_around_center(p_apo, afmatrix, cur_cx, cur_cy, cur_cz);
        cur_rotate_z = v;
	}
	POST_3DVIEWER_updateGL
}

void SurfaceObjGeometryDialog::change_use_rotation(int v)
{
	cur_use_rotation = !cur_use_rotation;
	dial_x->setDisabled(!cur_use_rotation);
	dial_y->setDisabled(!cur_use_rotation);
	dial_z->setDisabled(!cur_use_rotation);

	POST_3DVIEWER_updateGL
}

void SurfaceObjGeometryDialog::flip_x(int v)
{
	if (p_tree)
	{
		last_val = cur_flip_x; cur_op = SOG_FLIP_X;
//		if (pushButton_undo_last->text()=="Redo one step") pushButton_undo_last->setText("Undo one step");

		proc_neuron_mirror(p_tree, true, false, false);
#ifdef __SPECIAL_CHANGE_NEURON_SECOND__		
		proc_neuron_mirror(p_tree1, true, false, false);
#endif
	    cur_flip_x = checkBox_flip_x->isChecked();
	}
	if (p_apo)
	{
		last_val = cur_flip_x; cur_op = SOG_FLIP_X;
		//		if (pushButton_undo_last->text()=="Redo one step") pushButton_undo_last->setText("Undo one step");

		proc_apo_mirror(p_apo, true, false, false);
	    cur_flip_x = checkBox_flip_x->isChecked();
	}
	POST_3DVIEWER_updateGL
}

void SurfaceObjGeometryDialog::flip_y(int v)
{
	if (p_tree)
	{
		last_val = cur_flip_y; cur_op = SOG_FLIP_Y;
//		if (pushButton_undo_last->text()=="Redo one step") pushButton_undo_last->setText("Undo one step");

		proc_neuron_mirror(p_tree, false, true, false);
#ifdef __SPECIAL_CHANGE_NEURON_SECOND__		
		proc_neuron_mirror(p_tree1, false, true, false);
#endif
		cur_flip_y = checkBox_flip_y->isChecked();
	}
	if (p_apo)
	{
		last_val = cur_flip_y; cur_op = SOG_FLIP_Y;
		//		if (pushButton_undo_last->text()=="Redo one step") pushButton_undo_last->setText("Undo one step");

		proc_apo_mirror(p_apo, false, true, false);
		cur_flip_y = checkBox_flip_y->isChecked();
	}
	POST_3DVIEWER_updateGL
}

void SurfaceObjGeometryDialog::flip_z(int v)
{
	if (p_tree)
	{
		last_val = cur_flip_z; cur_op = SOG_FLIP_Z;
//		if (pushButton_undo_last->text()=="Redo one step") pushButton_undo_last->setText("Undo one step");

		proc_neuron_mirror(p_tree, false, false, true);
#ifdef __SPECIAL_CHANGE_NEURON_SECOND__		
		proc_neuron_mirror(p_tree1, false, false, true);
#endif
	    cur_flip_z = checkBox_flip_z->isChecked();
	}
	if (p_apo)
	{
		last_val = cur_flip_z; cur_op = SOG_FLIP_Z;
		//		if (pushButton_undo_last->text()=="Redo one step") pushButton_undo_last->setText("Undo one step");

		proc_apo_mirror(p_apo, false, false, true);
	    cur_flip_z = checkBox_flip_z->isChecked();
	}
	POST_3DVIEWER_updateGL
}

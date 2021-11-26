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
 surfaceobj_geometry_dialog.h
 by Hanchuan Peng
 2009_feb-21
 2009-March-03: add Gscaling
**
****************************************************************************/

#ifndef __SURFACEOBJ_GEOMETRY_DIALOG_H__
#define __SURFACEOBJ_GEOMETRY_DIALOG_H__

#include "../3drenderer/renderer_gl1.h"
#include "ui_surfaceobj_geometry_dialog.h"

#include <QDialog>

class V3dR_GLWidget;
class Renderer_gl1;
struct NeuronTree;

struct CellAPO;

enum SOG_Operation {SOG_SHIFT_X, SOG_SHIFT_Y, SOG_SHIFT_Z, SOG_SCALE_X, SOG_SCALE_Y, SOG_SCALE_Z, SOG_GSCALE_X, SOG_GSCALE_Y, SOG_GSCALE_Z, SOG_SCALE_R, SOG_FLIP_X, SOG_FLIP_Y, SOG_FLIP_Z, SOG_ROT_X, SOG_ROT_Y, SOG_ROT_Z};

class SurfaceObjGeometryDialog : public QDialog, private Ui_SurfaceObjGeometryDialog
{
    Q_OBJECT

public:
    SurfaceObjGeometryDialog(V3dR_GLWidget *w, Renderer_gl1 *r, int dc, int st, int i);
	void fetchData();

protected:
	V3dR_GLWidget* glwidget;
	Renderer_gl1* renderer;
	int dataclass;
	int surfaceobjtype;
	int objectindex;

	NeuronTree * p_tree;
	NeuronTree * p_tree1;
	NeuronTree tree0; //the original tree

	QList <CellAPO> *p_apo;
	QList <CellAPO> apo0;

	//data
	double cur_shift_x, cur_shift_y, cur_shift_z,
		   cur_scale_x, cur_scale_y, cur_scale_z,
		   cur_gscale_x, cur_gscale_y, cur_gscale_z,
		   cur_scale_r,
		   cur_rotate_x, cur_rotate_y, cur_rotate_z;
	double cur_cx, cur_cy, cur_cz; //center of the current object, which should NOT affected by a rotation, but should be updated if a shift happen
    bool cur_use_rotation;
	bool cur_flip_x, cur_flip_y, cur_flip_z;

	double last_val;
	SOG_Operation cur_op;

private:
	void resetInternalStates();
	void create();
	void updateContent();

public slots:
	void reset();
	void undo();
	void shift_x(double s);
	void shift_y(double s);
	void shift_z(double s);
	void scale_x(double s);
	void scale_y(double s);
	void scale_z(double s);
	void gscale_x(double s);
	void gscale_y(double s);
	void gscale_z(double s);
	void scale_r(double s);
	void rotate_around_x(int v);
	void rotate_around_y(int v);
	void rotate_around_z(int v);
	void change_use_rotation(int v);
	void flip_x(int v);
	void flip_y(int v);
	void flip_z(int v);
};

void highlight_dial(QDial *d);
double NORMALIZE_ROTATION_AngleStep( double  angle );

#endif

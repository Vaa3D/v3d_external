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
 dialog_imagecrop_bbox.h
 by Yang Yu
 2009-05-08

 090512 RZC: update

**
****************************************************************************/

#ifndef __DIALOG_IMAGECROP_BBOX_H__
#define __DIALOG_IMAGECROP_BBOX_H__

#include <QDialog>

#include "ui_dialog_imagecrop_bbox.h"

struct cropImgPara
{
	int xmin;
	int xmax;
	int ymin;
	int ymax;
	int zmin;
	int zmax;
	int colormin;
	int colormax;
	int landmark_opt;
};


class Dialog_imagecrop_bbox : public QDialog, public Ui_Dialog_ImageCrop_BBox
{
    Q_OBJECT

public:
    Dialog_imagecrop_bbox(cropImgPara *p)
	{
		setupUi(this);

		connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

		//initialization
		xminBox->setMinimum(p->xmin);	xminBox->setMaximum(p->xmax);
		xmaxBox->setMinimum(p->xmin);	xmaxBox->setMaximum(p->xmax);
		xminBox->setValue(p->xmin);
		xmaxBox->setValue(p->xmax);

		yminBox->setMinimum(p->ymin);	yminBox->setMaximum(p->ymax);
		ymaxBox->setMinimum(p->ymin);	ymaxBox->setMaximum(p->ymax);
		yminBox->setValue(p->ymin);
		ymaxBox->setValue(p->ymax);

		zminBox->setMinimum(p->zmin);	zminBox->setMaximum(p->zmax);
		zmaxBox->setMinimum(p->zmin);	zmaxBox->setMaximum(p->zmax);
		zminBox->setValue(p->zmin);
		zmaxBox->setValue(p->zmax);

		colorminBox->setMinimum(p->colormin);	colorminBox->setMaximum(p->colormax);
		colormaxBox->setMinimum(p->colormin);	colormaxBox->setMaximum(p->colormax);
		colorminBox->setValue(p->colormin);
		colormaxBox->setValue(p->colormax);

		landmarksBox->setCurrentIndex(p->landmark_opt);
	}

	void fetchData(cropImgPara *p)
	{
		if (!p) return;
		p->xmin = xminBox->value();
		p->xmax = xmaxBox->value();
		p->ymin = yminBox->value();
		p->ymax = ymaxBox->value();
		p->zmin = zminBox->value();
		p->zmax = zmaxBox->value();
		p->colormin = colorminBox->value();
		p->colormax = colormaxBox->value();
		p->landmark_opt = landmarksBox->currentIndex();
	}

private:


};

#endif

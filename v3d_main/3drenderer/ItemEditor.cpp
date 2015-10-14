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




/*
 * ItemEditor.cpp
 *
 *  Created on: Nov 23, 2008
 *      Author: ruanzongcai
 */

#include "v3dr_common.h" // for v3dr_getColorDialog
#include "ItemEditor.h"


void setItemEditor()
{
	static bool registered =false;
	if (registered) return;
	else registered = true;


	QItemEditorFactory *factory = new QItemEditorFactory(*QItemEditorFactory::defaultFactory());

	//QItemEditorCreatorBase *spinCreator = new QStandardItemEditorCreator<QSpinBox>();
	//QItemEditorCreatorBase *comboCreator = new QStandardItemEditorCreator<QComboBox>();
	QItemEditorCreatorBase *colorCreator = new QStandardItemEditorCreator<ColorEditor>();

	factory->registerEditor(QVariant::String, 0);
	factory->registerEditor(QVariant::Color, colorCreator);

	QItemEditorFactory::setDefaultFactory(factory);
}


ColorEditor::ColorEditor(QWidget* w)
	: QWidget(w)
{
	this->w = w;
}

QColor ColorEditor::color()
{
	//QColor c = QColorDialog::getColor(qcolor, this);
	//QColor c = QColor::fromRgba( QColorDialog::getRgba(qcolor.rgba(), 0, this) );
	//if (c.isValid())  qcolor = c;
	v3dr_getColorDialog( &qcolor, this); //090424 RZC

	return (qcolor);
}

void ColorEditor::setColor(QColor c)
{
	qcolor = c;
}

//void ColorEditor::mousePressEvent(QMouseEvent* event)
//{
//	qDebug("ColorEditor::mousePressEvent");
//	if (event->button()==Qt::LeftButton)
//		color();
//}

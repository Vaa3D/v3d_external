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
 * barFigureDialog.cpp
 *
 *  Created on: May 1, 2009
 *      Author: ruanzongcai
 */

#include "barFigureDialog.h"

#include "../basic_c_fun/volimg_proc.h"

#include <QLayout>
#include <QLabel>
barFigureDialog::barFigureDialog(QVector< QVector<int> >& vvec, QStringList labelsOfLeftTop, QString labelOfRightBottom,
		QWidget *parent, QSize figSize, QColor barColor)
: QDialog(parent)
{
	setWindowFlags(Qt::Popup /*| Qt::WindowStaysOnTopHint*/ | Qt::Tool); setAttribute(Qt::WA_MacAlwaysShowToolWindow);

	QGridLayout *layout = new QGridLayout;

	int nChannel = vvec.size();
	QLabel ** labelPicture = new QLabel * [nChannel]; //revised by PHC, 2010-05-20
	int row = 1;
	for (int i=0; i<nChannel; i++)
	{
		labelPicture[i] = new QLabel;
		labelPicture[i]->setFrameStyle(QFrame::Box | QFrame::Plain);	labelPicture[i]->setLineWidth(1);

		labelPicture[i]->resize(figSize);
		QVector<int> & vec = vvec[i];
		QImage fig = drawBarFigure( vec, barColor);

		V3DLONG imin, imax;
		int vmin, vmax;
		minMaxInVector(vec.data(), vec.size(), imin, vmin, imax, vmax);
		float ave, std;
		mean_and_std(vec.data(), vec.size(), ave, std);
		QString desc = QString("min=%1 max=%2  mean=%3 std=%4").arg(vmin).arg(vmax).arg(ave).arg(std);

		layout->addWidget(new QLabel(labelsOfLeftTop[i]), row, 0);
		layout->addWidget(new QLabel(desc),               row++, 0, Qt::AlignRight);

		layout->addWidget( labelPicture[i],  row++, 0);

		QPicture pic;
		QPainter p;
		p.begin( &pic );
			p.drawImage( labelPicture[i]->rect(), fig );
		p.end();
		labelPicture[i]->setPicture( pic );
	}
	layout->addWidget(new QLabel(QString("%1").arg(0)),  row, 0);
	layout->addWidget(new QLabel(labelOfRightBottom),    row++, 0, Qt::AlignRight);
	layout->setSizeConstraint(QLayout::SetFixedSize); //same as dlg->setFixedSize(dlg->size()); // fixed layout after show

	setLayout(layout);
	if (labelPicture) {delete []labelPicture; labelPicture=0;} //added by PHC, 2010-05-20
}

barFigureDialog::~barFigureDialog()
{
	// TODO Auto-generated destructor stub
}

void barFigureDialog::closeEvent(QCloseEvent* e)
{
	this->deleteLater();
}


QImage drawBarFigure(QVector<int>& vec, QColor barColor)
{
	//find bound
	int x1,x2, y1,y2;
	x1=x2= 0;
	y1=y2= 0;
	for (int i=0; i<vec.size(); i++)
	{
		int value = vec[i];
		if (value > y2) y2 = value;
	}
	x2 = vec.size()-1;

	QSize size(x2-x1+1, y2-y1+1);
	QImage img(size, QImage::Format_RGB32);

	//paint bar figure
	QPainter p(&img);
	QRect rect(0, 0, size.width(), size.height());
	QBrush shade(QColor(255,255,255));
	p.fillRect(rect, shade);
	for (int i=0; i<vec.size(); i++)
	{
		int value = vec[i];
		QRect rect(i, 0, 1, value);
		QBrush shade( barColor );
		p.fillRect(rect, shade);
	}

	return img.mirrored(); // flip y-direction
}

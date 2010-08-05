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




//dialog_rotate.cpp
//by Hanchuan Peng
//080322

#include <QtGui>

#include "dialog_rotate.h"

Dialog_Rotate::Dialog_Rotate(QWidget *parent)
	: QDialog(parent)
{
	label = new QLabel(tr("Rotation &degrees: \n* >0 -- clockwise \n* <0 -- counter-clock-wise"));
	label->setWordWrap(true);
	degreeEdit = new QSpinBox;
    degreeEdit->setRange(-180, 180);
    degreeEdit->setSingleStep(5);
	label->setBuddy(degreeEdit);

	sizeCheckBox = new QCheckBox(tr("Keep same &size"));

	rotateButton = new QPushButton(tr("&Rotate"));
	rotateButton->setDefault(true);

	cancelButton = new QPushButton(tr("Cancel"));

	moreButton = new QPushButton(tr("&More"));
	moreButton->setCheckable(true);
	moreButton->setAutoDefault(false);

	extension = new QWidget;

	QString tmpstr;
	
	xlabel = new QLabel(tr("rotation center &X:"));
	centerXEdit = new QLineEdit;
	centerXEdit->setInputMask("9999.99");
	tmpstr.setNum(256);
	centerXEdit->setText(tmpstr);
	xlabel->setBuddy(centerXEdit);

	ylabel = new QLabel(tr("rotation center &Y:"));
	centerYEdit = new QLineEdit;
	centerYEdit->setInputMask("9999.99");
	tmpstr.setNum(256);
	centerYEdit->setText(tmpstr);
	ylabel->setBuddy(centerYEdit);

	zlabel = new QLabel(tr("rotation center &Z:"));
	centerZEdit = new QLineEdit;
	centerZEdit->setInputMask("9999.99");
	tmpstr.setNum(256);
	centerZEdit->setText(tmpstr);
	zlabel->setBuddy(centerZEdit);

	fillcolorlabel = new QLabel(tr("fill background gray-value:"));
	fillColorEdit = new QSpinBox;
    fillColorEdit->setRange(0, 255);
    fillColorEdit->setSingleStep(30);
	fillcolorlabel->setBuddy(fillColorEdit);

	connect(rotateButton, SIGNAL(clicked()), this, SLOT(accept()));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	connect(moreButton, SIGNAL(toggled(bool)), extension, SLOT(setVisible(bool)));

	QGridLayout *extensionLayout = new QGridLayout;
	extensionLayout->setMargin(0);
	extensionLayout->addWidget(xlabel, 0, 0, 1, 1);
	extensionLayout->addWidget(centerXEdit, 0, 1, 1, 2);
	extensionLayout->addWidget(ylabel, 1, 0, 1, 1);
	extensionLayout->addWidget(centerYEdit, 1, 1, 1, 2);
	extensionLayout->addWidget(zlabel, 2, 0, 1, 1);
	extensionLayout->addWidget(centerZEdit, 2, 1, 1, 2);
	extensionLayout->addWidget(fillcolorlabel, 3, 0, 1, 1);
	extensionLayout->addWidget(fillColorEdit, 3, 1, 1, 2);
	extension->setLayout(extensionLayout);

	QHBoxLayout *topLeftLayout = new QHBoxLayout;
	topLeftLayout->addWidget(label);
	topLeftLayout->addWidget(degreeEdit);

	QVBoxLayout *leftLayout = new QVBoxLayout;
	leftLayout->addLayout(topLeftLayout);
	leftLayout->addWidget(sizeCheckBox);
	leftLayout->addStretch(1);

	QVBoxLayout *rightLayout = new QVBoxLayout;
	rightLayout->addWidget(rotateButton);
	rightLayout->addWidget(cancelButton);
	rightLayout->addWidget(moreButton);
	rightLayout->addStretch(1);

	QGridLayout *mainLayout = new QGridLayout;
	mainLayout->setSizeConstraint(QLayout::SetFixedSize);
	mainLayout->addLayout(leftLayout, 0, 0);
	mainLayout->addLayout(rightLayout, 0, 1);
	mainLayout->addWidget(extension, 1, 0, 1, 2);
	setLayout(mainLayout);

	setWindowTitle(tr("Rotation options"));
	extension->hide();
}

void Dialog_Rotate::getContents(Options_Rotate & res)
{
	res.degree = -degreeEdit->value()/180.0*3.141592635;
	res.fillcolor = fillColorEdit->value();
	res.b_keepSameSize = (sizeCheckBox->checkState()==Qt::Checked)?true:false;
	QString tmpstr;
	tmpstr = centerXEdit->text(); res.center_x = tmpstr.toDouble()-0.5;
	tmpstr = centerYEdit->text(); res.center_y = tmpstr.toDouble()-0.5;
	tmpstr = centerZEdit->text(); res.center_z = tmpstr.toDouble()-0.5;
}

void Dialog_Rotate::setContents(const Options_Rotate & res)
{
	degreeEdit->setValue(-res.degree/3.141592635*180.0);
	fillColorEdit->setValue(res.fillcolor);
	sizeCheckBox->setCheckState((res.b_keepSameSize)?Qt::Checked:Qt::Unchecked);
	QString tmpstr;
	tmpstr.setNum(res.center_x+0.5); centerXEdit->setText(tmpstr);
	tmpstr.setNum(res.center_y+0.5); centerYEdit->setText(tmpstr);
	tmpstr.setNum(res.center_z+0.5); centerZEdit->setText(tmpstr);
}

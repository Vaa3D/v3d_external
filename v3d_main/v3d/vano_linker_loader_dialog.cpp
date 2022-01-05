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
 vano_linker_loader_dialog.cpp
 by Hanchuan Peng
 2009_March-6
 **
 ****************************************************************************/

#include "vano_linker_loader_dialog.h"

//#include <QtGui>

#include <QFileInfo>
#include <QFile>
#include <QFileDialog>

VANO_LinkerLoadDialog::VANO_LinkerLoadDialog(const QString & linker_file, const QString & raw_file, const QString & mask_file, const QString & apo_file)
{
	_linkerfile = linker_file;
	_rawimgfile = raw_file;
	_maskimgfile = mask_file;
	_apofile = apo_file;

	updateContent();
}

void VANO_LinkerLoadDialog::updateContent()
{
	create();
}

void VANO_LinkerLoadDialog::fetchData(QString & raw_file, QString & mask_file, QString & apo_file)
{
	raw_file = _rawimgfile;
	mask_file = _maskimgfile;
	apo_file = _apofile;
}

void VANO_LinkerLoadDialog::create()
{
	setupUi(this);
	
	connect(pushButton_loaddata, SIGNAL(clicked()), this, SLOT(accept()));
	connect(pushButton_cancel, SIGNAL(clicked()), this, SLOT(reject()));

	//default values and events
	lineEdit_rawimg->setText(_rawimgfile);
	lineEdit_maskimg->setText(_maskimgfile);
	lineEdit_apofile->setText(_apofile);
	
	connect(lineEdit_rawimg, SIGNAL(textChanged(const QString &)), this, SLOT(change_rawfile(const QString &)));
	connect(lineEdit_maskimg, SIGNAL(textChanged(const QString &)), this, SLOT(change_maskfile(const QString &)));
	connect(lineEdit_apofile, SIGNAL(textChanged(const QString &)), this, SLOT(change_apofile(const QString &)));

	connect(pushButton_rawimg, SIGNAL(clicked()), this, SLOT(select_rawfile()));
	connect(pushButton_maskimg, SIGNAL(clicked()), this, SLOT(select_maskfile()));
	connect(pushButton_apofile, SIGNAL(clicked()), this, SLOT(select_apofile()));
	
	if (!QFile::exists(_rawimgfile))
		label_rawimg->setText("<font color=\"red\">Raw image</font>");
	else
		label_rawimg->setText("<font color=\"black\">Raw image</font>");

	if (!QFile::exists(_maskimgfile))
		label_maskimg->setText("<font color=\"red\">Mask image</font>");
	else
		label_maskimg->setText("<font color=\"black\">Mask image</font>");

	if (!QFile::exists(_apofile))
		label_apofile->setText("<font color=\"red\">Annotation</font>");
	else
		label_apofile->setText("<font color=\"black\">Annotation</font>");
}

void VANO_LinkerLoadDialog::change_rawfile(const QString & s) 
{
	_rawimgfile = s.trimmed();
	if (!QFile::exists(_rawimgfile))
		label_rawimg->setText("<font color=\"red\">Raw image</font>");
	else
		label_rawimg->setText("<font color=\"black\">Raw image</font>");
}

void VANO_LinkerLoadDialog::change_maskfile(const QString & s) 
{
	_maskimgfile = s.trimmed();
	if (!QFile::exists(_maskimgfile))
		label_maskimg->setText("<font color=\"red\">Mask image</font>");
	else
		label_maskimg->setText("<font color=\"black\">Mask image</font>");
}

void VANO_LinkerLoadDialog::change_apofile(const QString & s) 
{
	_apofile = s.trimmed();
	if (!QFile::exists(_apofile))
		label_apofile->setText("<font color=\"red\">Annotation</font>");
	else
		label_apofile->setText("<font color=\"black\">Annotation</font>");
}

void VANO_LinkerLoadDialog::select_rawfile() 
{
	QString s;
	QFileInfo info(_linkerfile);
	//qDebug() << info.dir().path();
	s = QFileDialog::getOpenFileName(0, tr("select RAW image file"), info.dir().path(), tr("Images (*.tif *tiff *raw)"));
	if (!s.isEmpty())
	{
		lineEdit_rawimg->setText(s);//this should call change_rawfile automaically
		//change_rawfile(s);
	}
}

void VANO_LinkerLoadDialog::select_maskfile() 
{
	QString s;
	QFileInfo info(_linkerfile);
	s = QFileDialog::getOpenFileName(0, "select MASK image file", info.dir().path(), "Images (*.tif *tiff *raw)");
	if (!s.isEmpty())
	{
		lineEdit_maskimg->setText(s);//this should call change_maskfile automaically
		//change_maskfile(s);
	}
}

void VANO_LinkerLoadDialog::select_apofile() 
{
	QString s;
	QFileInfo info(_linkerfile);
	s = QFileDialog::getOpenFileName(0, "select Annotation file", info.dir().path(), "Annotation spread sheet file (*.apo *.csv)");
	if (!s.isEmpty())
	{
		lineEdit_apofile->setText(s);//this should call change_apofile automaically
		//change_apofile(s);
	}
}

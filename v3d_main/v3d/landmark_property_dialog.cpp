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
landmarkproperty_dialog.cpp

by Hanchuan Peng
2008, Dec-08
2009-05-19: add principal vector values 
**
****************************************************************************/

#include "landmark_property_dialog.h"

#include "v3d_core.h"

#ifndef _MSC_VER

LandmarkPropertyDialog::LandmarkPropertyDialog(const QList <LocationSimple> *p_anoTable, int curRowNum, My4DImage *p_imgdata=0)
{
	if (!p_anoTable)
		return;
	else
		updateContent(p_anoTable, curRowNum, p_imgdata);
}

void LandmarkPropertyDialog::updateContent(const QList <LocationSimple> *p_anoTable, int curRowNum, My4DImage *p_imgdata=0)
{
	if (!p_anoTable)
	{
		printf("anoTable is not valid in LandmarkPropertyDialog::fetchData().\n");
		return;
	}
	
	if (curRowNum>=p_anoTable->count())
	{
		printf("The index [=%d] is bigger than the size of the list [=%d].\n", curRow, anoTable->count());
		return;
	}


	imgdata = p_imgdata;
	anoTable = (QList <LocationSimple> *)p_anoTable;
	curRow = curRowNum;
	create();
	
	QString tmp;
	
	const LocationSimple *p_landmark = &(anoTable->at(curRow));
	
	//landmark name/comments/annoattions
	
	if (curRow==0 && anoTable->size()==1)
		order->setText(tmp.setNum(curRow+1).prepend("No.") + " or new landmark"); //curRow
	else
		order->setText(tmp.setNum(curRow+1)); //curRow
		
	name->setText(p_landmark->name.c_str());
	comment->setText(p_landmark->comments.c_str());
	
	//landmark geometry
	
	coord_z->setText(tmp.setNum(int(p_landmark->z)));
	coord_x->setText(tmp.setNum(int(p_landmark->x)));
	coord_y->setText(tmp.setNum(int(p_landmark->y)));
	radius->setText(tmp.setNum(int(p_landmark->radius)));
	
	if (int(p_landmark->shape)>=landmark_shape->count())
	{
		qDebug("Warning: your landmark shape type is not compatible with the combobox list. Unset it.\n");
		landmark_shape->setCurrentIndex(0); //the first one is call "unset" 
	}
	else
		landmark_shape->setCurrentIndex(int(p_landmark->shape)); 
	
	
	//pixel intensity
	
	if (imgdata)
	{	
		int nc = imgdata->getCDim();
		pix_val_red->setText(tmp.setNum(int(imgdata->at(int(p_landmark->x-1), int(p_landmark->y-1), int(p_landmark->z-1), 0))));
		if (nc>=2) 
			pix_val_green->setText(tmp.setNum(int(imgdata->at(int(p_landmark->x-1), int(p_landmark->y-1), int(p_landmark->z-1), 1))));
		else
			pix_val_green->setText(tmp.setNum(0));
		if (nc>=3) 
			pix_val_blue->setText(tmp.setNum(int(imgdata->at(int(p_landmark->x-1), int(p_landmark->y-1), int(p_landmark->z-1), 2))));
		else
			pix_val_blue->setText(tmp.setNum(0));
		if (nc>=4) 
			pix_val_ch4->setText(tmp.setNum(int(imgdata->at(int(p_landmark->x-1), int(p_landmark->y-1), int(p_landmark->z-1), 3))));
		else
			pix_val_ch4->setText(tmp.setNum(0));
		if (nc>=5) 
			pix_val_ch5->setText(tmp.setNum(int(imgdata->at(int(p_landmark->x-1), int(p_landmark->y-1), int(p_landmark->z-1), 4))));
		else
			pix_val_ch5->setText(tmp.setNum(0));
		
		//landmark surrounding area statistics

		statistics_channel->setRange(1, nc);
		//compute the stat of surrounding rgn
		
		int tmp_vv =int(imgdata->at(int(p_landmark->x-1), int(p_landmark->y-1), int(p_landmark->z-1), 0));
		
		val_peak->setText(tmp.setNum(tmp_vv));
		val_mean->setText(tmp.setNum(tmp_vv));
		val_stddev->setText(tmp.setNum(0));
		val_size->setText(tmp.setNum(1));
		val_mass->setText(tmp.setNum(tmp_vv));
		
		//now do computation
		compute_rgn_stat();
	}
	else
	{
		pix_val_red->setText("Unset");
		pix_val_green->setText("Unset");
		pix_val_blue->setText("Unset");
		pix_val_ch4->setText("Unset");
		pix_val_ch5->setText("Unset");
		
		//landmark surrounding area statistics
		
		statistics_channel->setRange(0, 0);
		//compute the stat of surrounding rgn
		val_peak->setText("Unset");
		val_mean->setText("Unset");
		val_stddev->setText("Unset");
		val_size->setText("Unset");
		val_mass->setText("Unset");
	}

	//set read/write property
	
	order->setReadOnly(true);
	coord_z->setReadOnly(true);
	coord_x->setReadOnly(true);
	coord_y->setReadOnly(true);
	pix_val_red->setReadOnly(true);
	pix_val_green->setReadOnly(true);
	pix_val_blue->setReadOnly(true);
	pix_val_ch4->setReadOnly(true);
	pix_val_ch5->setReadOnly(true);
	val_peak->setReadOnly(true);
	val_mean->setReadOnly(true);
	val_stddev->setReadOnly(true);
	val_size->setReadOnly(true);
	val_mass->setReadOnly(true);
}

void LandmarkPropertyDialog::fetchData(QList <LocationSimple>  *anoTable, int curRow)
{
	if (!anoTable)
	{
		printf("anoTable is not valid in LandmarkPropertyDialog::fetchData().\n");
		return;
	}

	if (curRow>=anoTable->count())
	{
		printf("The index [=%d] is bigger than the size of the list [=%d].\n", curRow, anoTable->count());
		return;
	}
	
	LocationSimple *p_landmark = (LocationSimple *) &(anoTable->at(curRow));
	
	//landmark name/comments/annoattions
	
	//order->setText(tmp.setNum(curRow));
	p_landmark->name = qPrintable(name->text());
	p_landmark->comments = qPrintable(comment->text());
	
	//landmark geometry
	
	p_landmark->x = coord_x->text().toDouble();
	p_landmark->y = coord_y->text().toDouble();
	p_landmark->z = coord_z->text().toDouble();
	p_landmark->radius = (radius->text().toDouble()>=0)?radius->text().toDouble():0;
	p_landmark->shape = PxLocationMarkerShape(landmark_shape->currentIndex());
}

void LandmarkPropertyDialog::create()
{
	setupUi(this);
	
	connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	
//	radius->setInputMask(tr("999")); //only allow three degits, each is 0~9
	connect(radius, SIGNAL(editingFinished()), this, SLOT(compute_rgn_stat()));
	connect(landmark_shape, SIGNAL(currentIndexChanged(int)), this, SLOT(compute_rgn_stat(int)));
	
	connect(statistics_channel, SIGNAL(valueChanged(int)), this, SLOT(compute_rgn_stat(int)));
}

void LandmarkPropertyDialog::compute_rgn_stat(int c) //overload for convenience
{
	compute_rgn_stat(); 
}

void LandmarkPropertyDialog::compute_rgn_stat()
{
	if (!imgdata || !imgdata->valid()) return;
	
	LocationSimple pt;

	pt.x = coord_x->text().toInt()-1;
	pt.y = coord_y->text().toInt()-1;
	pt.z = coord_z->text().toInt()-1;
	V3DLONG cc = statistics_channel->value()-1; if (cc<0) cc=0; if (cc>=imgdata->getCDim()) cc=imgdata->getCDim()-1;
	pt.radius = (radius->text().toDouble()>=0)?radius->text().toDouble():0;
	pt.shape = PxLocationMarkerShape(landmark_shape->currentIndex());
	
	//now do the computation
	if (imgdata->compute_rgn_stat(pt, cc)==true)
	{
		//now update the value of the respective 
		QString tmp;
		val_peak->setText(tmp.setNum(pt.pixmax));
		val_mean->setText(tmp.setNum(pt.ave));
		val_stddev->setText(tmp.setNum(pt.sdev));
		val_size->setText(tmp.setNum(pt.size));
		val_mass->setText(tmp.setNum(pt.mass));	
		
		if (pt.ev_pc1==VAL_INVALID && pt.ev_pc2==VAL_INVALID && pt.ev_pc3==VAL_INVALID)
		{
			val_pc1_d->setText("uncomputed");
			val_pc2_d->setText("uncomputed");
			val_pc3_d->setText("uncomputed");
		}
		else
		{	
			val_pc1_d->setText(tmp.setNum(sqrt(pt.ev_pc1)));
			val_pc2_d->setText(tmp.setNum(sqrt(pt.ev_pc2)));
			val_pc3_d->setText(tmp.setNum(sqrt(pt.ev_pc3)));
		}
	}
}

#endif


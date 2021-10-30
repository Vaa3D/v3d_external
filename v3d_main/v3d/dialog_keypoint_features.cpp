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
keypoint_features_dialog.cpp
 by Yang Yu
 2009-06-20
 changed by Yang Yu 2009-07-07
**
****************************************************************************/
#include <cmath>
#include <QTime>

#include "dialog_keypoint_features.h"

#include "../basic_c_fun/volimg_proc.h"

#include "mainwindow.h"

#define INF 1E20

KeypointFeaturesDialog::KeypointFeaturesDialog(const QList <ImgPixelFea> *p_anoTable, int curRowNum, My4DImage *p_imgdata=0)
{
	if (!p_anoTable)
		return;
	else
		updateContent(p_anoTable, curRowNum, p_imgdata);
}

void KeypointFeaturesDialog::updateContent(const QList <ImgPixelFea> *p_anoTable, int curRowNum, My4DImage *p_imgdata=0)
{
	if (!p_anoTable)
	{
		printf("anoTable is not valid in KeypointFeaturesDialog::fetchData().\n");
		return;
	}
	
	if (curRowNum>=p_anoTable->count())
	{
		printf("The index [=%d] is bigger than the size of the list [=%d].\n", curRow, anoTable->count());
		return;
	}


	imgdata = p_imgdata;
	anoTable = (QList <ImgPixelFea> *)p_anoTable;
	curRow = curRowNum;
	create();
	
	QString tmp;
	
	const ImgPixelFea *p_keypoint = &(anoTable->at(curRow));
	
	//landmark name/comments/annoattions
	if (curRow==0 && anoTable->size()==1)
		order->setText(tmp.setNum(curRow+1).prepend("No.") + " or new keypoint"); //curRow
	else
		order->setText(tmp.setNum(curRow+1)); //curRow
		
	name->setText(p_keypoint->name.c_str());
	comment->setText(p_keypoint->comments.c_str());
	
	//landmark geometry
	coord_z->setText(tmp.setNum(int(p_keypoint->z)));
	coord_x->setText(tmp.setNum(int(p_keypoint->x)));
	coord_y->setText(tmp.setNum(int(p_keypoint->y)));
	radius->setText(tmp.setNum(int(p_keypoint->radius)));

	//pixel intensity
	if (imgdata)
	{	
		//which channel is interest
		int nc = imgdata->getCDim();
		statistics_channel->setRange(1, nc);
		
		//normalized when value=1
		normalized->setRange(0, 1);
		normalized->setValue(1);
		
		//thresholding get the foreground region
		V3DLONG cc = statistics_channel->value()-1; if (cc<0) cc=0; if (cc>=imgdata->getCDim()) cc=imgdata->getCDim()-1;
		float meanVal=0, stdVal=0;
		V3DLONG channel_bytes = imgdata->getZDim() * imgdata->getYDim() *imgdata->getZDim();
		
		switch ( imgdata->getDatatype() )
		{
			case V3D_UINT8:

				mean_and_std(imgdata->getRawData() + cc*channel_bytes, channel_bytes, meanVal, stdVal);
				threshold->setText(tmp.setNum( meanVal + 1*stdVal ));

				break;
				
			case V3D_UINT16:

				mean_and_std(imgdata->getRawData() + cc*channel_bytes, channel_bytes, meanVal, stdVal);
				threshold->setText(tmp.setNum( meanVal + 1*stdVal ));
				
				break;
				
			case V3D_FLOAT32:

				mean_and_std(imgdata->getRawData() + cc*channel_bytes, channel_bytes, meanVal, stdVal);
				threshold->setText(tmp.setNum( meanVal + 1*stdVal ));
				
				break;
		}
		
		//set initial multiple resolution checkstates Qt::Checked 2
		checkBox_ori_scale1->setCheckState(Qt::Checked);
		checkBox_ori_scale2->setCheckState(Qt::Checked);
		checkBox_ori_scale3->setCheckState(Qt::Checked);
		checkBox_ori_scale4->setCheckState(Qt::Checked);
		
		checkBox_diff_scale1->setCheckState(Qt::Checked);
		checkBox_diff_scale2->setCheckState(Qt::Checked);
		checkBox_diff_scale3->setCheckState(Qt::Checked);
		checkBox_diff_scale4->setCheckState(Qt::Checked);
	}
	else
	{
		lambda1_ori_r1->setText("Unset");
		lambda2_ori_r1->setText("Unset");
		lambda3_ori_r1->setText("Unset");
		lambda1_diff_r1->setText("Unset");
		lambda2_diff_r1->setText("Unset");
		lambda3_diff_r1->setText("Unset");
		
		lambda1_ori_r2->setText("Unset");
		lambda2_ori_r2->setText("Unset");
		lambda3_ori_r2->setText("Unset");
		lambda1_diff_r2->setText("Unset");
		lambda2_diff_r2->setText("Unset");
		lambda3_diff_r2->setText("Unset");
		
		lambda1_ori_r3->setText("Unset");
		lambda2_ori_r3->setText("Unset");
		lambda3_ori_r3->setText("Unset");
		lambda1_diff_r3->setText("Unset");
		lambda2_diff_r3->setText("Unset");
		lambda3_diff_r3->setText("Unset");
		
		lambda1_ori_r4->setText("Unset");
		lambda2_ori_r4->setText("Unset");
		lambda3_ori_r4->setText("Unset");
		lambda1_diff_r4->setText("Unset");
		lambda2_diff_r4->setText("Unset");
		lambda3_diff_r4->setText("Unset");
		
	}

	//set read/write property
	
	order->setReadOnly(true);
	coord_z->setReadOnly(true);
	coord_x->setReadOnly(true);
	coord_y->setReadOnly(true);

}

void KeypointFeaturesDialog::fetchData(QList <ImgPixelFea>  *anoTable, int curRow)
{
	if (!anoTable)
	{
		printf("anoTable is not valid in KeypointFeaturesDialog::fetchData().\n");
		return;
	}

	if (curRow>=anoTable->count())
	{
		printf("The index [=%d] is bigger than the size of the list [=%d].\n", curRow, anoTable->count());
		return;
	}
	
	ImgPixelFea *p_keypoint = (ImgPixelFea *) &(anoTable->at(curRow));
	
	//keypoint name/comments/annoattions
	
	p_keypoint->name = qPrintable(name->text());
	p_keypoint->comments = qPrintable(comment->text());
	
	//keypoint geometry
	
	p_keypoint->x = coord_x->text().toDouble();
	p_keypoint->y = coord_y->text().toDouble();
	p_keypoint->z = coord_z->text().toDouble();
	p_keypoint->radius = (radius->text().toDouble()>=0)?radius->text().toDouble():0;

}

void KeypointFeaturesDialog::create()
{
	setupUi(this);
	
	connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	
	connect(radius, SIGNAL(editingFinished()), this, SLOT(compute_pointfea()));
	connect(statistics_channel, SIGNAL(valueChanged(int)), this, SLOT(compute_pointfea()));
	connect(normalized, SIGNAL(valueChanged(int)), this, SLOT(compute_pointfea()));

	connect(GenSimMap, SIGNAL(clicked()), this, SLOT(compute_similarmap()));

}

void KeypointFeaturesDialog::compute_pointfea()
{
	if (!imgdata || !imgdata->valid()) return;
	
	QTime t;
	t.start();

	pt.x = coord_x->text().toInt()-1;
	pt.y = coord_y->text().toInt()-1;
	pt.z = coord_z->text().toInt()-1;
	V3DLONG cc = statistics_channel->value()-1; if (cc<0) cc=0; if (cc>=imgdata->getCDim()) cc=imgdata->getCDim()-1;
	V3DLONG normalized_flag = normalized->value(); 
	pt.radius = (radius->text().toDouble()>=0)?radius->text().toDouble():0;

	//now do the computation

	V3DLONG xx = V3DLONG(pt.x+0.5);
	V3DLONG yy = V3DLONG(pt.y+0.5);
	V3DLONG zz = V3DLONG(pt.z+0.5);
	V3DLONG rr = pt.radius; if (rr<0) rr=0;
	
	V3DLONG sz0 = imgdata->getXDim();
  V3DLONG sz1 = imgdata->getYDim();
  V3DLONG sz2 = imgdata->getZDim();
  V3DLONG sz3 = imgdata->getCDim();

	
	pt.n = 4;

	//computing	
	pt.setPCAscore(imgdata, pt.n, rr, cc, normalized_flag);
	
	//display
	QString tmp;

	if(pt.fpa[0].imgpca.lambda1==VAL_INVALID && pt.fpa[0].imgpca.lambda2==VAL_INVALID && pt.fpa[0].imgpca.lambda3==VAL_INVALID &&
	   pt.fpa[1].imgpca.lambda1==VAL_INVALID && pt.fpa[1].imgpca.lambda2==VAL_INVALID && pt.fpa[1].imgpca.lambda3==VAL_INVALID &&
	   pt.fpa[2].imgpca.lambda1==VAL_INVALID && pt.fpa[2].imgpca.lambda2==VAL_INVALID && pt.fpa[2].imgpca.lambda3==VAL_INVALID &&
	   pt.fpa[3].imgpca.lambda1==VAL_INVALID && pt.fpa[3].imgpca.lambda2==VAL_INVALID && pt.fpa[3].imgpca.lambda3==VAL_INVALID &&
		pt.fpa[0].diffpca.lambda1==VAL_INVALID && pt.fpa[0].diffpca.lambda2==VAL_INVALID && pt.fpa[0].diffpca.lambda3==VAL_INVALID &&
		pt.fpa[1].diffpca.lambda1==VAL_INVALID && pt.fpa[1].diffpca.lambda2==VAL_INVALID && pt.fpa[1].diffpca.lambda3==VAL_INVALID &&
		pt.fpa[2].diffpca.lambda1==VAL_INVALID && pt.fpa[2].diffpca.lambda2==VAL_INVALID && pt.fpa[2].diffpca.lambda3==VAL_INVALID &&
		pt.fpa[3].diffpca.lambda1==VAL_INVALID && pt.fpa[3].diffpca.lambda2==VAL_INVALID && pt.fpa[3].diffpca.lambda3==VAL_INVALID)
	{
		lambda1_ori_r1->setText("uncomputed");
		lambda2_ori_r1->setText("uncomputed");
		lambda3_ori_r1->setText("uncomputed");
		lambda1_diff_r1->setText("uncomputed");
		lambda2_diff_r1->setText("uncomputed");
		lambda3_diff_r1->setText("uncomputed");
		
		lambda1_ori_r2->setText("uncomputed");
		lambda2_ori_r2->setText("uncomputed");
		lambda3_ori_r2->setText("uncomputed");
		lambda1_diff_r2->setText("uncomputed");
		lambda2_diff_r2->setText("uncomputed");
		lambda3_diff_r2->setText("uncomputed");
		
		lambda1_ori_r3->setText("uncomputed");
		lambda2_ori_r3->setText("uncomputed");
		lambda3_ori_r3->setText("uncomputed");
		lambda1_diff_r3->setText("uncomputed");
		lambda2_diff_r3->setText("uncomputed");
		lambda3_diff_r3->setText("uncomputed");
		
		lambda1_ori_r4->setText("uncomputed");
		lambda2_ori_r4->setText("uncomputed");
		lambda3_ori_r4->setText("uncomputed");
		lambda1_diff_r4->setText("uncomputed");
		lambda2_diff_r4->setText("uncomputed");
		lambda3_diff_r4->setText("uncomputed");

	}
	else
	{	
		lambda1_ori_r1->setText(tmp.setNum(pt.fpa[0].imgpca.lambda1));
		lambda2_ori_r1->setText(tmp.setNum(pt.fpa[0].imgpca.lambda2));
		lambda3_ori_r1->setText(tmp.setNum(pt.fpa[0].imgpca.lambda3));
		lambda1_diff_r1->setText(tmp.setNum(pt.fpa[0].diffpca.lambda1));
		lambda2_diff_r1->setText(tmp.setNum(pt.fpa[0].diffpca.lambda2));
		lambda3_diff_r1->setText(tmp.setNum(pt.fpa[0].diffpca.lambda3));
		
		lambda1_ori_r2->setText(tmp.setNum(pt.fpa[1].imgpca.lambda1));
		lambda2_ori_r2->setText(tmp.setNum(pt.fpa[1].imgpca.lambda2));
		lambda3_ori_r2->setText(tmp.setNum(pt.fpa[1].imgpca.lambda3));
		lambda1_diff_r2->setText(tmp.setNum(pt.fpa[1].diffpca.lambda1));
		lambda2_diff_r2->setText(tmp.setNum(pt.fpa[1].diffpca.lambda2));
		lambda3_diff_r2->setText(tmp.setNum(pt.fpa[1].diffpca.lambda3));
		
		lambda1_ori_r3->setText(tmp.setNum(pt.fpa[2].imgpca.lambda1));
		lambda2_ori_r3->setText(tmp.setNum(pt.fpa[2].imgpca.lambda2));
		lambda3_ori_r3->setText(tmp.setNum(pt.fpa[2].imgpca.lambda3));
		lambda1_diff_r3->setText(tmp.setNum(pt.fpa[2].diffpca.lambda1));
		lambda2_diff_r3->setText(tmp.setNum(pt.fpa[2].diffpca.lambda2));
		lambda3_diff_r3->setText(tmp.setNum(pt.fpa[2].diffpca.lambda3));
		
		lambda1_ori_r4->setText(tmp.setNum(pt.fpa[3].imgpca.lambda1));
		lambda2_ori_r4->setText(tmp.setNum(pt.fpa[3].imgpca.lambda2));
		lambda3_ori_r4->setText(tmp.setNum(pt.fpa[3].imgpca.lambda3));
		lambda1_diff_r4->setText(tmp.setNum(pt.fpa[3].diffpca.lambda1));
		lambda2_diff_r4->setText(tmp.setNum(pt.fpa[3].diffpca.lambda2));
		lambda3_diff_r4->setText(tmp.setNum(pt.fpa[3].diffpca.lambda3));
		
	}//end of display


	
	//display to the screen
	qDebug(" current marker: [%d] \nscale (in row), img_pca, imgdiff_pca (in columns):\n %lf %lf %lf %lf %lf %lf \n %lf %lf %lf %lf %lf %lf \n %lf %lf %lf %lf %lf %lf \n %lf %lf %lf %lf %lf %lf \n ",
		   curRow,
		    pt.fpa[0].imgpca.lambda1, pt.fpa[0].imgpca.lambda2, pt.fpa[0].imgpca.lambda3,
		    pt.fpa[0].diffpca.lambda1, pt.fpa[0].diffpca.lambda2, pt.fpa[0].diffpca.lambda3,
		    
		    pt.fpa[1].imgpca.lambda1, pt.fpa[1].imgpca.lambda2, pt.fpa[1].imgpca.lambda3,
		    pt.fpa[1].diffpca.lambda1, pt.fpa[1].diffpca.lambda2, pt.fpa[1].diffpca.lambda3,

		    pt.fpa[2].imgpca.lambda1, pt.fpa[2].imgpca.lambda2, pt.fpa[2].imgpca.lambda3,
		    pt.fpa[2].diffpca.lambda1, pt.fpa[2].diffpca.lambda2, pt.fpa[2].diffpca.lambda3,

		    pt.fpa[3].imgpca.lambda1, pt.fpa[3].imgpca.lambda2, pt.fpa[3].imgpca.lambda3,
		    pt.fpa[3].diffpca.lambda1, pt.fpa[3].diffpca.lambda2, pt.fpa[3].diffpca.lambda3);

	//testing cost time
	qDebug("Time elapsed: %d ms \n", t.elapsed());
}


bool KeypointFeaturesDialog::compute_similarmap()
{
	if (!imgdata || !imgdata->valid()) return false;
	
	QTime t;
	t.start();
	
	pt.x = coord_x->text().toInt()-1;
	pt.y = coord_y->text().toInt()-1;
	pt.z = coord_z->text().toInt()-1;
	V3DLONG channel_no = statistics_channel->value()-1; if (channel_no<0) channel_no=0; if (channel_no>=imgdata->getCDim()) channel_no=imgdata->getCDim()-1;
	V3DLONG normalized_flag = normalized->value(); 
	pt.radius = (radius->text().toDouble()>=0)?radius->text().toDouble():0;
	
	//now do the computation
	
	V3DLONG xx = V3DLONG(pt.x+0.5);
	V3DLONG yy = V3DLONG(pt.y+0.5);
	V3DLONG zz = V3DLONG(pt.z+0.5);
	V3DLONG rr = pt.radius; if (rr<0) rr=0;
	
	//reading updated current inputs
	if(checkBox_ori_scale1->isChecked())
	{
		pt.fpa[0].imgpca.lambda1 = lambda1_ori_r1->text().toDouble();
		pt.fpa[0].imgpca.lambda2 = lambda2_ori_r1->text().toDouble();
		pt.fpa[0].imgpca.lambda3 = lambda3_ori_r1->text().toDouble();		
	}
	else
	{
		pt.fpa[0].imgpca.lambda1 = 0;
		pt.fpa[0].imgpca.lambda2 = 0;
		pt.fpa[0].imgpca.lambda3 = 0;		
	}
	
	if(checkBox_ori_scale2->isChecked())
	{
		pt.fpa[1].imgpca.lambda1 = lambda1_ori_r2->text().toDouble();
		pt.fpa[1].imgpca.lambda2 = lambda2_ori_r2->text().toDouble();
		pt.fpa[1].imgpca.lambda3 = lambda3_ori_r2->text().toDouble();
	}
	else
	{
		pt.fpa[1].imgpca.lambda1 = 0;
		pt.fpa[1].imgpca.lambda2 = 0;
		pt.fpa[1].imgpca.lambda3 = 0;
	}
	
	if(checkBox_ori_scale3->isChecked())
	{
		pt.fpa[2].imgpca.lambda1 = lambda1_ori_r3->text().toDouble();
		pt.fpa[2].imgpca.lambda2 = lambda2_ori_r3->text().toDouble();
		pt.fpa[2].imgpca.lambda3 = lambda3_ori_r3->text().toDouble();
	}
	else
	{
		pt.fpa[2].imgpca.lambda1 = 0;
		pt.fpa[2].imgpca.lambda2 = 0;
		pt.fpa[2].imgpca.lambda3 = 0;
	}
	
	if(checkBox_ori_scale4->isChecked())
	{
		pt.fpa[3].imgpca.lambda1 = lambda1_ori_r4->text().toDouble();
		pt.fpa[3].imgpca.lambda2 = lambda2_ori_r4->text().toDouble();
		pt.fpa[3].imgpca.lambda3 = lambda3_ori_r4->text().toDouble();
	}
	else
	{
		pt.fpa[3].imgpca.lambda1 = 0;
		pt.fpa[3].imgpca.lambda2 = 0;
		pt.fpa[3].imgpca.lambda3 = 0;
	}
	
	if(checkBox_diff_scale1->isChecked())
	{
		pt.fpa[0].diffpca.lambda1 = lambda1_diff_r1->text().toDouble();
		pt.fpa[0].diffpca.lambda2 = lambda2_diff_r1->text().toDouble();
		pt.fpa[0].diffpca.lambda3 = lambda3_diff_r1->text().toDouble();
	}
	else
	{
		pt.fpa[0].diffpca.lambda1 = 0;
		pt.fpa[0].diffpca.lambda2 = 0;
		pt.fpa[0].diffpca.lambda3 = 0;
	}
	
	if(checkBox_diff_scale2->isChecked())
	{
		pt.fpa[1].diffpca.lambda1 = lambda1_diff_r2->text().toDouble();
		pt.fpa[1].diffpca.lambda2 = lambda2_diff_r2->text().toDouble();
		pt.fpa[1].diffpca.lambda3 = lambda3_diff_r2->text().toDouble();
	}
	else
	{
		pt.fpa[1].diffpca.lambda1 = 0;
		pt.fpa[1].diffpca.lambda2 = 0;
		pt.fpa[1].diffpca.lambda3 = 0;
	}
		
	if(checkBox_diff_scale3->isChecked())
	{
		pt.fpa[2].diffpca.lambda1 = lambda1_diff_r3->text().toDouble();
		pt.fpa[2].diffpca.lambda2 = lambda2_diff_r3->text().toDouble();
		pt.fpa[2].diffpca.lambda3 = lambda3_diff_r3->text().toDouble();
	}
	else
	{
		pt.fpa[2].diffpca.lambda1 = 0;
		pt.fpa[2].diffpca.lambda2 = 0;
		pt.fpa[2].diffpca.lambda3 = 0;
	}
	
	if(checkBox_diff_scale4->isChecked())
	{
		pt.fpa[3].diffpca.lambda1 = lambda1_diff_r4->text().toDouble();
		pt.fpa[3].diffpca.lambda2 = lambda2_diff_r4->text().toDouble();
		pt.fpa[3].diffpca.lambda3 = lambda3_diff_r4->text().toDouble();
	}
	else
	{
		pt.fpa[3].diffpca.lambda1 = 0;
		pt.fpa[3].diffpca.lambda2 = 0;
		pt.fpa[3].diffpca.lambda3 = 0;
	}

	//do computation of similarity map
	V3DLONG sz0 = imgdata->getXDim();
  V3DLONG sz1 = imgdata->getYDim();
  V3DLONG sz2 = imgdata->getZDim();
  V3DLONG sz3 = imgdata->getCDim();

	//initial a float type pointer for computing difference
	float **** pDiff = new float *** [sz3];
	if (!pDiff)
	{
		printf("Memory allocate error for pDiff! \n");
		return false;
	}

	for (V3DLONG c=0;c<sz3; c++)
	{
		pDiff[c] = new float ** [sz2];
		if (!pDiff[c])
		{
			for (V3DLONG i=0;i<c;i++) {delete [] (pDiff[c]);}
			delete []pDiff; 
			pDiff=NULL;
			return false;
		}
		else
		{
			for (V3DLONG i=0;i<sz2; i++)
			{
				pDiff[c][i] = new float * [sz1];
				if (!pDiff[c][i])
				{
					for (V3DLONG j=0;j<i;j++) {delete [] (pDiff[c][i]);}
					delete [](pDiff[c]); 
					pDiff[c]=NULL;
					return false;
				}
				else
				{
					for (V3DLONG j=0;j<sz1; j++)
					{
						pDiff[c][i][j] = new float [sz0]; 
						if (!pDiff[c][i][j])
						{
							for (V3DLONG k=0;k<j;k++) {delete [] (pDiff[c][i][j]);}
							delete [](pDiff[c][i]); 
							pDiff[c][i]=NULL;
							return false;
						}
						else
						{
							for(V3DLONG k=0; k<sz0; k++)
							{
								pDiff[c][i][j][k]=0; //initialization
							}
						}
					}
				}
			}
		}
	}
	
	int b_win_shape=1; //0 for cube and 1 for sphere for pca computation
	bool b_disp_CoM_etc=false; //if display center of mass and covariance info
	
	double max_val = -INF, min_val = INF; //for rescaling
	
	V3DLONG channel_bytes = sz2*sz1*sz0;
	
	float **** data4d_float32 = (float ****)imgdata->getData();;
	USHORTINT16 **** data4d_uint16 = (USHORTINT16 ****)imgdata->getData();
	unsigned char **** data4d_uint8 = (unsigned char ****)imgdata->getData();;
	
	float * pSMap = new float [channel_bytes];
	if (!pSMap)
	{
		printf("Fail to allocate memory for pSMap!\n");
		return false;
	}
	for (V3DLONG k=0;k<channel_bytes;k++)
	{
		pSMap[k] = 0;
	}

	//computing similarity map according to point (xx, yy, zz)	
	double thresh = threshold->text().toDouble();
	int m = 0;
	
	switch ( imgdata->getDatatype() )
	{
		case V3D_UINT8:
			//compute distance instead of correlation
			for (int k=0;k<sz2;k++)
			{
				qDebug("k %d ", k);
				for (int j=0;j<sz1;j++)
				{
					for (int i=0;i<sz0;i++)
					{
						if(data4d_uint8[channel_no][k][j][i] > thresh)
						{
							V3DLONG indLoop = k*sz0*sz1 + j*sz0 + i;
							
							//computing	
							ImgPixelFea pt_cmp(i,j,k);
							
							double sum = 0;
							if(checkBox_ori_scale1->isChecked())
							{
								m = 0;

								compute_win3d_pca(data4d_uint8[channel_no], sz0, sz1, sz2,
												  i, j, k,
												  rr, rr, rr,
												  pt_cmp.fpa[m].imgpca.lambda1, pt_cmp.fpa[m].imgpca.lambda2, pt_cmp.fpa[m].imgpca.lambda3, b_win_shape, b_disp_CoM_etc, normalized_flag);
								
								sum += abs(pt.fpa[m].imgpca.lambda1 - pt_cmp.fpa[m].imgpca.lambda1) + abs(pt.fpa[m].imgpca.lambda2 - pt_cmp.fpa[m].imgpca.lambda2) + abs(pt.fpa[m].imgpca.lambda3 - pt_cmp.fpa[m].imgpca.lambda3);
								
							}
							
							if(checkBox_ori_scale2->isChecked())
							{
								m = 1;
								
								compute_win3d_pca(data4d_uint8[channel_no], sz0, sz1, sz2,
												  i, j, k,
												  i*rr, i*rr, i*rr,
												  pt_cmp.fpa[m].imgpca.lambda1, pt_cmp.fpa[m].imgpca.lambda2, pt_cmp.fpa[m].imgpca.lambda3, b_win_shape, b_disp_CoM_etc, normalized_flag);
								
								sum += abs(pt.fpa[m].imgpca.lambda1 - pt_cmp.fpa[m].imgpca.lambda1) + abs(pt.fpa[m].imgpca.lambda2 - pt_cmp.fpa[m].imgpca.lambda2) + abs(pt.fpa[m].imgpca.lambda3 - pt_cmp.fpa[m].imgpca.lambda3);
							}
							
							if(checkBox_ori_scale3->isChecked())
							{
								m = 2;

								compute_win3d_pca(data4d_uint8[channel_no], sz0, sz1, sz2,
												  i, j, k,
												  i*rr, i*rr, i*rr,
												  pt_cmp.fpa[m].imgpca.lambda1, pt_cmp.fpa[m].imgpca.lambda2, pt_cmp.fpa[m].imgpca.lambda3, b_win_shape, b_disp_CoM_etc, normalized_flag);
								
								sum += abs(pt.fpa[m].imgpca.lambda1 - pt_cmp.fpa[m].imgpca.lambda1) + abs(pt.fpa[m].imgpca.lambda2 - pt_cmp.fpa[m].imgpca.lambda2) + abs(pt.fpa[m].imgpca.lambda3 - pt_cmp.fpa[m].imgpca.lambda3);
							}
							
							if(checkBox_ori_scale4->isChecked())
							{
								m = 3;

								compute_win3d_pca(data4d_uint8[channel_no], sz0, sz1, sz2,
												  i, j, k,
												  i*rr, i*rr, i*rr,
												  pt_cmp.fpa[m].imgpca.lambda1, pt_cmp.fpa[m].imgpca.lambda2, pt_cmp.fpa[m].imgpca.lambda3, b_win_shape, b_disp_CoM_etc, normalized_flag);	
								
								sum += abs(pt.fpa[m].imgpca.lambda1 - pt_cmp.fpa[m].imgpca.lambda1) + abs(pt.fpa[m].imgpca.lambda2 - pt_cmp.fpa[m].imgpca.lambda2) + abs(pt.fpa[m].imgpca.lambda3 - pt_cmp.fpa[m].imgpca.lambda3);
							}
							
							if(checkBox_diff_scale1->isChecked())
							{
								m = 1;
								
								compute_win3d_diff(data4d_uint8[channel_no], pDiff[channel_no], sz0, sz1, sz2,
												   i,j,k,
												   rr,rr,rr);
								
								compute_win3d_pca(pDiff[channel_no], sz0, sz1, sz2,
												  i, j, k,
												  rr, rr, rr,
												  pt_cmp.fpa[m].diffpca.lambda1, pt_cmp.fpa[m].diffpca.lambda2, pt_cmp.fpa[m].diffpca.lambda3, b_win_shape, b_disp_CoM_etc, normalized_flag);
								
								sum += abs(pt.fpa[m].diffpca.lambda1 - pt_cmp.fpa[m].diffpca.lambda1) + abs(pt.fpa[m].diffpca.lambda2 - pt_cmp.fpa[m].diffpca.lambda2) + abs(pt.fpa[m].diffpca.lambda3 - pt_cmp.fpa[m].diffpca.lambda3);
							}
							
							if(checkBox_diff_scale2->isChecked())
							{
								m = 1;
								
								compute_win3d_diff(data4d_uint8[channel_no], pDiff[channel_no], sz0, sz1, sz2,
												   i,j,k,
												   i*rr, i*rr, i*rr);
								
								compute_win3d_pca(pDiff[channel_no], sz0, sz1, sz2,
												  i, j, k,
												  i*rr, i*rr, i*rr,
												  pt_cmp.fpa[m].diffpca.lambda1, pt_cmp.fpa[m].diffpca.lambda2, pt_cmp.fpa[m].diffpca.lambda3, b_win_shape, b_disp_CoM_etc, normalized_flag);
								
								sum += abs(pt.fpa[m].diffpca.lambda1 - pt_cmp.fpa[m].diffpca.lambda1) + abs(pt.fpa[m].diffpca.lambda2 - pt_cmp.fpa[m].diffpca.lambda2) + abs(pt.fpa[m].diffpca.lambda3 - pt_cmp.fpa[m].diffpca.lambda3);
							}
							
							if(checkBox_diff_scale3->isChecked())
							{
								m = 2;
								
								compute_win3d_diff(data4d_uint8[channel_no], pDiff[channel_no], sz0, sz1, sz2,
												   i,j,k,
												   i*rr, i*rr, i*rr);
								
								compute_win3d_pca(pDiff[channel_no], sz0, sz1, sz2,
												  i, j, k,
												  i*rr, i*rr, i*rr,
												  pt_cmp.fpa[m].diffpca.lambda1, pt_cmp.fpa[m].diffpca.lambda2, pt_cmp.fpa[m].diffpca.lambda3, b_win_shape, b_disp_CoM_etc, normalized_flag);
								
								sum += abs(pt.fpa[m].diffpca.lambda1 - pt_cmp.fpa[m].diffpca.lambda1) + abs(pt.fpa[m].diffpca.lambda2 - pt_cmp.fpa[m].diffpca.lambda2) + abs(pt.fpa[m].diffpca.lambda3 - pt_cmp.fpa[m].diffpca.lambda3);
								
							}
							
							if(checkBox_diff_scale4->isChecked())
							{
								m = 3;
								
								compute_win3d_diff(data4d_uint8[channel_no], pDiff[channel_no], sz0, sz1, sz2,
												   i,j,k,
												   i*rr, i*rr, i*rr);
								
								compute_win3d_pca(pDiff[channel_no], sz0, sz1, sz2,
												  i, j, k,
												  i*rr, i*rr, i*rr,
												  pt_cmp.fpa[m].diffpca.lambda1, pt_cmp.fpa[m].diffpca.lambda2, pt_cmp.fpa[m].diffpca.lambda3, b_win_shape, b_disp_CoM_etc, normalized_flag);
								
								sum += abs(pt.fpa[m].diffpca.lambda1 - pt_cmp.fpa[m].diffpca.lambda1) + abs(pt.fpa[m].diffpca.lambda2 - pt_cmp.fpa[m].diffpca.lambda2) + abs(pt.fpa[m].diffpca.lambda3 - pt_cmp.fpa[m].diffpca.lambda3);
							}

							//compute distance
							pSMap[indLoop] = exp(-sum);
							
							max_val = (max_val<pSMap[indLoop])? pSMap[indLoop] : max_val;
							min_val = (min_val>pSMap[indLoop])? pSMap[indLoop] : min_val;
							
						}

					}
				}
			}
			
			break;
			
		case V3D_UINT16:
			//compute distance instead of correlation
			for (int k=0;k<sz2;k++)
			{
				for (int j=0;j<sz1;j++)
				{
					for (int i=0;i<sz0;i++)
					{
						if(data4d_uint16[channel_no][k][j][i] > thresh)
						{
							V3DLONG indLoop = k*sz0*sz1 + j*sz0 + i;
							
							//computing	
							ImgPixelFea pt_cmp(i,j,k);
							
							double sum = 0;
							if(checkBox_ori_scale1->isChecked())
							{
								m = 0;

								compute_win3d_pca(data4d_uint16[channel_no], sz0, sz1, sz2,
												  i, j, k,
												  rr, rr, rr,
												  pt_cmp.fpa[m].imgpca.lambda1, pt_cmp.fpa[m].imgpca.lambda2, pt_cmp.fpa[m].imgpca.lambda3, b_win_shape, b_disp_CoM_etc, normalized_flag);
								
								sum += abs(pt.fpa[m].imgpca.lambda1 - pt_cmp.fpa[m].imgpca.lambda1) + abs(pt.fpa[m].imgpca.lambda2 - pt_cmp.fpa[m].imgpca.lambda2) + abs(pt.fpa[m].imgpca.lambda3 - pt_cmp.fpa[m].imgpca.lambda3);
								
							}
							
							if(checkBox_ori_scale2->isChecked())
							{
								m = 1;
								
								compute_win3d_pca(data4d_uint16[channel_no], sz0, sz1, sz2,
												  i, j, k,
												  i*rr, i*rr, i*rr,
												  pt_cmp.fpa[m].imgpca.lambda1, pt_cmp.fpa[m].imgpca.lambda2, pt_cmp.fpa[m].imgpca.lambda3, b_win_shape, b_disp_CoM_etc, normalized_flag);
								
								sum += abs(pt.fpa[m].imgpca.lambda1 - pt_cmp.fpa[m].imgpca.lambda1) + abs(pt.fpa[m].imgpca.lambda2 - pt_cmp.fpa[m].imgpca.lambda2) + abs(pt.fpa[m].imgpca.lambda3 - pt_cmp.fpa[m].imgpca.lambda3);
							}
							
							if(checkBox_ori_scale3->isChecked())
							{
								m = 2;
								
								compute_win3d_pca(data4d_uint16[channel_no], sz0, sz1, sz2,
												  i, j, k,
												  i*rr, i*rr, i*rr,
												  pt_cmp.fpa[m].imgpca.lambda1, pt_cmp.fpa[m].imgpca.lambda2, pt_cmp.fpa[m].imgpca.lambda3, b_win_shape, b_disp_CoM_etc, normalized_flag);
								
								sum += abs(pt.fpa[m].imgpca.lambda1 - pt_cmp.fpa[m].imgpca.lambda1) + abs(pt.fpa[m].imgpca.lambda2 - pt_cmp.fpa[m].imgpca.lambda2) + abs(pt.fpa[m].imgpca.lambda3 - pt_cmp.fpa[m].imgpca.lambda3);
							}
							
							if(checkBox_ori_scale4->isChecked())
							{
								m = 3;
								
								compute_win3d_pca(data4d_uint16[channel_no], sz0, sz1, sz2,
												  i, j, k,
												  i*rr, i*rr, i*rr,
												  pt_cmp.fpa[m].imgpca.lambda1, pt_cmp.fpa[m].imgpca.lambda2, pt_cmp.fpa[m].imgpca.lambda3, b_win_shape, b_disp_CoM_etc, normalized_flag);	
								
								sum += abs(pt.fpa[m].imgpca.lambda1 - pt_cmp.fpa[m].imgpca.lambda1) + abs(pt.fpa[m].imgpca.lambda2 - pt_cmp.fpa[m].imgpca.lambda2) + abs(pt.fpa[m].imgpca.lambda3 - pt_cmp.fpa[m].imgpca.lambda3);
							}
							
							if(checkBox_diff_scale1->isChecked())
							{
								m = 0;
								
								compute_win3d_diff(data4d_uint16[channel_no], pDiff[channel_no], sz0, sz1, sz2,
												   i,j,k,
												   rr,rr,rr);	
								
								compute_win3d_pca(pDiff[channel_no], sz0, sz1, sz2,
												  i, j, k,
												  rr, rr, rr,
												  pt_cmp.fpa[m].diffpca.lambda1, pt_cmp.fpa[m].diffpca.lambda2, pt_cmp.fpa[m].diffpca.lambda3, b_win_shape, b_disp_CoM_etc, normalized_flag);
								
								sum += abs(pt.fpa[m].diffpca.lambda1 - pt_cmp.fpa[m].diffpca.lambda1) + abs(pt.fpa[m].diffpca.lambda2 - pt_cmp.fpa[m].diffpca.lambda2) + abs(pt.fpa[m].diffpca.lambda3 - pt_cmp.fpa[m].diffpca.lambda3);
							}
							
							if(checkBox_diff_scale2->isChecked())
							{
								m = 1;
								
								compute_win3d_diff(data4d_uint16[channel_no], pDiff[channel_no], sz0, sz1, sz2,
												   i,j,k,
												   i*rr, i*rr, i*rr);	
								
								compute_win3d_pca(pDiff[channel_no], sz0, sz1, sz2,
												  i, j, k,
												  i*rr, i*rr, i*rr,
												  pt_cmp.fpa[m].diffpca.lambda1, pt_cmp.fpa[m].diffpca.lambda2, pt_cmp.fpa[m].diffpca.lambda3, b_win_shape, b_disp_CoM_etc, normalized_flag);
								
								sum += abs(pt.fpa[m].diffpca.lambda1 - pt_cmp.fpa[m].diffpca.lambda1) + abs(pt.fpa[m].diffpca.lambda2 - pt_cmp.fpa[m].diffpca.lambda2) + abs(pt.fpa[m].diffpca.lambda3 - pt_cmp.fpa[m].diffpca.lambda3);
							}
							
							if(checkBox_diff_scale3->isChecked())
							{
								m = 2;
								
								compute_win3d_diff(data4d_uint16[channel_no], pDiff[channel_no], sz0, sz1, sz2,
												   i,j,k,
												   i*rr, i*rr, i*rr);	
								
								compute_win3d_pca(pDiff[channel_no], sz0, sz1, sz2,
												  i, j, k,
												  i*rr, i*rr, i*rr,
												  pt_cmp.fpa[m].diffpca.lambda1, pt_cmp.fpa[m].diffpca.lambda2, pt_cmp.fpa[m].diffpca.lambda3, b_win_shape, b_disp_CoM_etc, normalized_flag);
								
								sum += abs(pt.fpa[m].diffpca.lambda1 - pt_cmp.fpa[m].diffpca.lambda1) + abs(pt.fpa[m].diffpca.lambda2 - pt_cmp.fpa[m].diffpca.lambda2) + abs(pt.fpa[m].diffpca.lambda3 - pt_cmp.fpa[m].diffpca.lambda3);
								
							}
							
							if(checkBox_diff_scale4->isChecked())
							{
								m = 3;
								
								compute_win3d_diff(data4d_uint16[channel_no], pDiff[channel_no], sz0, sz1, sz2,
												   i,j,k,
												   i*rr, i*rr, i*rr);	
								
								compute_win3d_pca(pDiff[channel_no], sz0, sz1, sz2,
												  i, j, k,
												  i*rr, i*rr, i*rr,
												  pt_cmp.fpa[m].diffpca.lambda1, pt_cmp.fpa[m].diffpca.lambda2, pt_cmp.fpa[m].diffpca.lambda3, b_win_shape, b_disp_CoM_etc, normalized_flag);
								
								sum += abs(pt.fpa[m].diffpca.lambda1 - pt_cmp.fpa[m].diffpca.lambda1) + abs(pt.fpa[m].diffpca.lambda2 - pt_cmp.fpa[m].diffpca.lambda2) + abs(pt.fpa[m].diffpca.lambda3 - pt_cmp.fpa[m].diffpca.lambda3);
							}
							
							
							
							//compute distance
							
							pSMap[indLoop] = exp(-sum);
							
							max_val = (max_val<pSMap[indLoop])? pSMap[indLoop] : max_val;
							min_val = (min_val>pSMap[indLoop])? pSMap[indLoop] : min_val;
							
						}
						
					}
				}
			}
			
			break;
			
		case V3D_FLOAT32:
			//compute distance instead of correlation
			for (int k=0;k<sz2;k++)
			{
				for (int j=0;j<sz1;j++)
				{
					for (int i=0;i<sz0;i++)
					{
						if(data4d_float32[channel_no][k][j][i] > thresh)
						{
							V3DLONG indLoop = k*sz0*sz1 + j*sz0 + i;
							
							//computing	
							ImgPixelFea pt_cmp(i,j,k);
							
							double sum = 0;
							if(checkBox_ori_scale1->isChecked())
							{
								m = 0;
								
								compute_win3d_pca(data4d_float32[channel_no], sz0, sz1, sz2,
												  i, j, k,
												  rr, rr, rr,
												  pt_cmp.fpa[m].imgpca.lambda1, pt_cmp.fpa[m].imgpca.lambda2, pt_cmp.fpa[m].imgpca.lambda3, b_win_shape, b_disp_CoM_etc, normalized_flag);
								
								sum += abs(pt.fpa[m].imgpca.lambda1 - pt_cmp.fpa[m].imgpca.lambda1) + abs(pt.fpa[m].imgpca.lambda2 - pt_cmp.fpa[m].imgpca.lambda2) + abs(pt.fpa[m].imgpca.lambda3 - pt_cmp.fpa[m].imgpca.lambda3);
								
							}
							
							if(checkBox_ori_scale2->isChecked())
							{
								m = 1;
								
								compute_win3d_pca(data4d_float32[channel_no], sz0, sz1, sz2,
												  i, j, k,
												  i*rr, i*rr, i*rr,
												  pt_cmp.fpa[m].imgpca.lambda1, pt_cmp.fpa[m].imgpca.lambda2, pt_cmp.fpa[m].imgpca.lambda3, b_win_shape, b_disp_CoM_etc, normalized_flag);
								
								sum += abs(pt.fpa[m].imgpca.lambda1 - pt_cmp.fpa[m].imgpca.lambda1) + abs(pt.fpa[m].imgpca.lambda2 - pt_cmp.fpa[m].imgpca.lambda2) + abs(pt.fpa[m].imgpca.lambda3 - pt_cmp.fpa[m].imgpca.lambda3);
							}
							
							if(checkBox_ori_scale3->isChecked())
							{
								m = 2;
								
								compute_win3d_pca(data4d_float32[channel_no], sz0, sz1, sz2,
												  i, j, k,
												  i*rr, i*rr, i*rr,
												  pt_cmp.fpa[m].imgpca.lambda1, pt_cmp.fpa[m].imgpca.lambda2, pt_cmp.fpa[m].imgpca.lambda3, b_win_shape, b_disp_CoM_etc, normalized_flag);
								
								sum += abs(pt.fpa[m].imgpca.lambda1 - pt_cmp.fpa[m].imgpca.lambda1) + abs(pt.fpa[m].imgpca.lambda2 - pt_cmp.fpa[m].imgpca.lambda2) + abs(pt.fpa[m].imgpca.lambda3 - pt_cmp.fpa[m].imgpca.lambda3);
							}
							
							if(checkBox_ori_scale4->isChecked())
							{
								m = 3;
								
								compute_win3d_pca(data4d_float32[channel_no], sz0, sz1, sz2,
												  i, j, k,
												  i*rr, i*rr, i*rr,
												  pt_cmp.fpa[m].imgpca.lambda1, pt_cmp.fpa[m].imgpca.lambda2, pt_cmp.fpa[m].imgpca.lambda3, b_win_shape, b_disp_CoM_etc, normalized_flag);	
								
								sum += abs(pt.fpa[m].imgpca.lambda1 - pt_cmp.fpa[m].imgpca.lambda1) + abs(pt.fpa[m].imgpca.lambda2 - pt_cmp.fpa[m].imgpca.lambda2) + abs(pt.fpa[m].imgpca.lambda3 - pt_cmp.fpa[m].imgpca.lambda3);
							}
							
							if(checkBox_diff_scale1->isChecked())
							{
								m = 0;
								
								compute_win3d_diff(data4d_float32[channel_no], pDiff[channel_no], sz0, sz1, sz2,
												   i,j,k,
												   rr,rr,rr);
								
								compute_win3d_pca(pDiff[channel_no], sz0, sz1, sz2,
												  i, j, k,
												  rr, rr, rr,
												  pt_cmp.fpa[m].diffpca.lambda1, pt_cmp.fpa[m].diffpca.lambda2, pt_cmp.fpa[m].diffpca.lambda3, b_win_shape, b_disp_CoM_etc, normalized_flag);
								
								sum += abs(pt.fpa[m].diffpca.lambda1 - pt_cmp.fpa[m].diffpca.lambda1) + abs(pt.fpa[m].diffpca.lambda2 - pt_cmp.fpa[m].diffpca.lambda2) + abs(pt.fpa[m].diffpca.lambda3 - pt_cmp.fpa[m].diffpca.lambda3);
							}
							
							if(checkBox_diff_scale2->isChecked())
							{
								m = 1;
								
								compute_win3d_diff(data4d_float32[channel_no], pDiff[channel_no], sz0, sz1, sz2,
												   i,j,k,
												   i*rr, i*rr, i*rr);
								
								compute_win3d_pca(pDiff[channel_no], sz0, sz1, sz2,
												  i, j, k,
												  i*rr, i*rr, i*rr,
												  pt_cmp.fpa[m].diffpca.lambda1, pt_cmp.fpa[m].diffpca.lambda2, pt_cmp.fpa[m].diffpca.lambda3, b_win_shape, b_disp_CoM_etc, normalized_flag);
								
								sum += abs(pt.fpa[m].diffpca.lambda1 - pt_cmp.fpa[m].diffpca.lambda1) + abs(pt.fpa[m].diffpca.lambda2 - pt_cmp.fpa[m].diffpca.lambda2) + abs(pt.fpa[m].diffpca.lambda3 - pt_cmp.fpa[m].diffpca.lambda3);
							}
							
							if(checkBox_diff_scale3->isChecked())
							{
								m = 2;
								
								compute_win3d_diff(data4d_float32[channel_no], pDiff[channel_no], sz0, sz1, sz2,
												   i,j,k,
												   i*rr, i*rr, i*rr);
								
								compute_win3d_pca(pDiff[channel_no], sz0, sz1, sz2,
												  i, j, k,
												  i*rr, i*rr, i*rr,
												  pt_cmp.fpa[m].diffpca.lambda1, pt_cmp.fpa[m].diffpca.lambda2, pt_cmp.fpa[m].diffpca.lambda3, b_win_shape, b_disp_CoM_etc, normalized_flag);
								
								sum += abs(pt.fpa[m].diffpca.lambda1 - pt_cmp.fpa[m].diffpca.lambda1) + abs(pt.fpa[m].diffpca.lambda2 - pt_cmp.fpa[m].diffpca.lambda2) + abs(pt.fpa[m].diffpca.lambda3 - pt_cmp.fpa[m].diffpca.lambda3);
								
							}
							
							if(checkBox_diff_scale4->isChecked())
							{
								m = 3;
								
								compute_win3d_diff(data4d_float32[channel_no], pDiff[channel_no], sz0, sz1, sz2,
												   i,j,k,
												   i*rr, i*rr, i*rr);
								
								compute_win3d_pca(pDiff[channel_no], sz0, sz1, sz2,
												  i, j, k,
												  i*rr, i*rr, i*rr,
												  pt_cmp.fpa[m].diffpca.lambda1, pt_cmp.fpa[m].diffpca.lambda2, pt_cmp.fpa[m].diffpca.lambda3, b_win_shape, b_disp_CoM_etc, normalized_flag);
								
								sum += abs(pt.fpa[m].diffpca.lambda1 - pt_cmp.fpa[m].diffpca.lambda1) + abs(pt.fpa[m].diffpca.lambda2 - pt_cmp.fpa[m].diffpca.lambda2) + abs(pt.fpa[m].diffpca.lambda3 - pt_cmp.fpa[m].diffpca.lambda3);
							}
							
							
							
							//compute distance
							
							pSMap[indLoop] = exp(-sum);
							
							max_val = (max_val<pSMap[indLoop])? pSMap[indLoop] : max_val;
							min_val = (min_val>pSMap[indLoop])? pSMap[indLoop] : min_val;							
							
						}
						
					}
				}
			}
			
			break;
	}
	qDebug("max value and min value of the point similarity map is %lf and %lf .\n", max_val, min_val);

	//RGB with similarity=1 Blue (255) & similarity=0 Red (0)
	unsigned char * outvol1d = new unsigned char [3*channel_bytes];
	if (!outvol1d)
	{
		printf("Fail to allocate memory for outvol1d!\n");
		return false;
	}
	else
	{
		for (V3DLONG k=0;k<channel_bytes;k++)
		{
			outvol1d[k] = 0;
			outvol1d[k+channel_bytes] = 0;
			outvol1d[k+channel_bytes+channel_bytes] = 0;
		}
	}
	
	XFormWidget * newwin = imgdata->getXWidget()->getMainControlWindow()->createMdiChild();
	newwin->newProcessedImage("Similarity Map ", outvol1d, sz0, sz1, sz2, 3, V3D_UINT8); 
	
	V3DLONG sxy = sz0*sz1;
	V3DLONG offset2 = 2*channel_bytes;
	
	for (V3DLONG k=0;k<sz2;k++)
	{
		V3DLONG indz = k*sxy;
		for (V3DLONG j=0;j<sz1;j++)
		{
			V3DLONG indy = indz+j*sz0;
			for (V3DLONG i=0;i<sz0;i++)
			{
				V3DLONG indLoop = indy + i;
				
				if(pSMap[indLoop])
				{
					float tmp = 255*(pSMap[indLoop]-min_val)/(max_val-min_val);

					outvol1d[indLoop] = 255-tmp;
					outvol1d[indLoop + channel_bytes] = 0;
					outvol1d[indLoop + offset2] = tmp;
				}
				else
				{
					outvol1d[indLoop] = 0;
					outvol1d[indLoop + channel_bytes] = 0;
					outvol1d[indLoop + offset2] = 0;
				}
			}
		}
	}
	
	newwin->show();
	newwin->getImageData()->updateViews();
	

	//de-alloc
	if(pSMap) {delete [] pSMap; pSMap=0;}
	
	if(pDiff) 
	{
		for (V3DLONG i=0;i<sz3; i++)
		{
			if(pDiff[i])
			{
				for (V3DLONG j=0;j<sz2; j++)
				{
					if (pDiff[i][j])
					{
						for(V3DLONG k=0; k<sz1; k++)
						{
							if(pDiff[i][j][k])
							{
								delete [] (pDiff[i][j][k]);
								pDiff[i][j][k]=NULL;
							}
						}
						
						delete [] (pDiff[i][j]);
						pDiff[i][j] = NULL;
					}
				}
			}
			
			delete [] pDiff[i];
			pDiff[i] = NULL;
		}
		
		delete [] pDiff;
		pDiff = NULL;
	}
	
	//testing cost time
	qDebug("Time elapsed: %d ms \n", t.elapsed());
	
	//Exit Normal
	return true;
	
}



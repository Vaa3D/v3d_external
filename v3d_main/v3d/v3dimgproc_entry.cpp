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




//by Hanchuan Peng
// 090706
// separated from v3d_core.cpp to make the codes cleaner
//

#include <stdio.h>
#if defined (_MSC_VER) //added by PHC, 2010-05-20. do not need to include <strings.h> for VC complier (indeed this file does not even exist)
#else
#include <strings.h>
#endif
#include <math.h>

#include <QtGui>

#include "v3d_core.h"
#include "mainwindow.h"

using namespace std;

#include "dialog_rotate.h"
#include "landmark_property_dialog.h"
#include "dialog_keypoint_features.h"
#include "dialog_curve_trace_para.h"
#include "import_images_tool_dialog.h"

#include "dialog_imagecrop_bbox.h"
#include "dialog_imageresample.h"
#include "dialog_maskroi.h"

#include "atlas_viewer.h"

#include "v3d_version_info.h"

#include "../basic_c_fun/volimg_proc.h"


void XFormView::popupImageProcessingDialog()
{
	popupImageProcessingDialog(QString(""));
}

void XFormView::popupImageProcessingDialog(QString item)
{
	QStringList items;
	items << tr("For general 3D images")
		  << tr(" -- Rotate image so that principal axis is horizontal")
		  << tr(" -- Rotate image an arbitrary degree")
		  << tr(" -- Flip image")
		  << tr(" -- Permute image")
		  << tr(" -- clear all landmarks (and of course the connectivity map of landmarks)")
		  << tr(" -- clear the enitre connectivity map of landmarks")
		  << tr(" -- rescale landmarks only (without resampling image)")
		  << tr(" -- landmarks +/- constant")
  		  << tr(" -- split channels")
		  << tr(" -- extract one channel")
		  << tr(" -- crop image using minMax bounding box in 3D (derived from ROIs)")
		  << tr(" -- crop image via input min-max bounds")
	      << tr(" -- masking image using bounding boxes in 3D (derived from ROIs)")
		  << tr(" -- masking image using ROIs in 3D")
		  << tr(" -- masking image using non-ROIs for all XY planes")
		  << tr(" -- masking image using channels")
		  << tr(" -- clear the ROI")
		  << tr(" -- resample image (and also associated landmarks)")
		  << tr(" -- projection (max)")
		  << tr(" -- merge channels of multple images")
		  << tr(" -- stitch two images")
	      << tr(" -- display histogram")
		  << tr(" -- linear adjustment")
		  << tr(" -- histogram equalization")
		  << tr(" -- convert indexed image to RGB")
		  << tr(" -- linear scaling to [0,255] and convert to 8 bit")
		  << tr(" -- convert 16bit image to 8 bit")
		  << tr(" -- convert 32bit (single-precision float) image to 8 bit")
		  << tr(" -- intensity scaling")
		  << tr(" -- intensity thresholding")
		  << tr(" -- intensity binarization")
		  << tr(" -- intensity minmaxvalue recomputing")
		  << tr(" -- invert image color")
		  << tr(" -- rearrange grayscale image as color images")
		  //<< tr(" -- open another image/stack in *THIS* window") //080930: this may crash the software if the new image is too big. thus disabled
		  << tr(" -- save to file")
		  << tr(" -- save to VANO annotation files")
		  << tr(" -- save to movie")
		  << tr(" -- Export landmarks to point cloud (APO) file")
		  << tr(" -- Export landmarks and their relationship to graph (SWC) file")
		  << tr(" -- Export traced neuron or fibrous structures to graph (SWC) file")
/*
#ifdef _ALLOW_IMGSTD_MENU_
		  << tr("For C. elegans or similarly elongated images")
		  << tr(" -- Randomly seed landmarks/control-points")
		  << tr(" -- Find MST (Minimum spanning tree) of landmarks")
		  << tr(" -- Detect diameter graph of MST")
		  << tr(" -- Find cutting plane locations")
		  << tr(" -- Straighten using slice-restacking")
	//<< tr(" -- (available soon) Adjust backbone graph using BDB_minus algorithm")
#endif
#ifdef _ALLOW_IMGREG_MENU_
		  << tr("For 3D image registration")
		  << tr(" -- Seed landmarks/control-points on regular grid")
		  << tr(" -- Seed landmarks/control-points randomly")
		  << tr(" -- Seed landmarks/control-points using a file")
		  << tr(" -- Global affine alignment (using image content)")
		  << tr(" -- Global affine alignment (using xform derived from matching landmarks)")
		  << tr(" -- Segmenting optical lobes of a fly brain (for a globally aligned fly brain)")
		  << tr(" -- Mask blue to 0")
		  << tr(" -- Match landmarks defined for another image (i.e. registration target)")
		  << tr(" -- Match one single landmark in another image")
		  << tr(" -- Warp image using corresponding landmarks")
		  << tr(" -- ** All-in-one: detect corresponding landmarks and then warp")
	//<< tr(" -- (available soon) Seed landmarks/control-points using big gradient (edge) points")
	//<< tr(" -- (available soon) Seed landmarks/control-points using big curvature (corner) points")
#endif
*/
#ifdef _ALLOW_NEURONSEG_MENU_
	<< tr("For 3D fibrous structure segmentation/tracing")
    << tr(" -- top-down skeletonization")
	<< tr(" -- trace one marker to all others")
	<< tr(" -- trace between two locations")
	<< tr(" -- undo last tracing step")
	<< tr(" -- redo last tracing step")
	<< tr(" -- clear traced neuron")
	<< tr(" -- update 3D view of traced neuron")
	<< tr(" -- save traced neuron")
		  //<< tr(" -- (available soon) bottom-up marching")
		  //<< tr(" -- (available soon) global-local (Global) integration of top-down/bottom-up results")
		  //<< tr(" -- (available soon) manual correction of fibrous segmentation")
#endif
	/*
#ifdef _ALLOW_CELLSEG_MENU_
	<< tr("For globular structure (e.g. cells, nuclei) segmentation")
	  << tr(" -- local template matching")
	  << tr(" -- cell counting (Yang Yu)")
	  << tr(" -- watershed segmentation")
	  //<< tr(" -- levelset segmentation")
	  << tr(" -- 1-Gaussian fit of current focus pos")
	  << tr(" -- N-Gaussian fit of current focus pos")
	  //<< tr(" -- (available soon) Gaussian partition")
	  //<< tr(" -- (available soon) manual correction/identification of spherical structures")
#endif
*/
#ifdef _ALLOW_AUTOMARKER_MENU_
	<< tr("For AutoMarker")
	<< tr(" -- automarker for entire image")
	<< tr(" -- automarker for roi")
#endif
	;

	if (!imgData || !imgData->valid())
	{
		printf("The image data is invalid in popupImageProcessingDialog(). Check your data. do nothing.\n");
		return;
	}

	bool ok;
	//QString item;
	if (item.isEmpty()) //if the string is null or contains no character
		item = QInputDialog::getItem(this, tr("Image processing"),
								 tr("Please select the image processing procedure you like to do"), items, 0, false, &ok);
	else
		ok=true; //so that when item has preset value, then directly go to processing

	if (ok && !item.isEmpty())
	{
		try
		{
			if (item==tr(" -- Rotate image so that principal axis is horizontal"))
			{
				try {imgData->proj_general_principal_axis(Ptype);}
				catch (...) {QString tmps=QString("Fail to run [") + item + "]"; QMessageBox::warning(0, "Fail to run", tmps);
					printf("%s.\n", qPrintable(tmps));
				}
			}
			else if (item==tr(" -- Rotate image an arbitrary degree"))
			{
				Options_Rotate tmpopt;
				tmpopt.degree=0.0;
				tmpopt.b_keepSameSize=true;
				tmpopt.fillcolor=0;
				tmpopt.center_x = (imgData->getXDim()-1.0)/2;
				tmpopt.center_y = (imgData->getYDim()-1.0)/2;
				tmpopt.center_z = (imgData->getZDim()-1.0)/2;

				Dialog_Rotate tmpdlg;
				tmpdlg.setContents(tmpopt);

				int dlg_res = tmpdlg.exec();
				if (dlg_res)
				{
					tmpdlg.getContents(tmpopt);
					try {imgData->rotate(Ptype, tmpopt);}
					catch (...) {QString tmps=QString("Fail to run [") + item + "]"; QMessageBox::warning(0, "Fail to run", tmps);
						printf("%s.\n", qPrintable(tmps));
					}
				}
			}
			else if (item==tr(" -- Flip image"))
			{
				bool ok1, ok_opt;
				AxisCode my_axiscode;

				QStringList items_opt;
				QString item_opt;

				items_opt << tr("Y axis") << tr("Z axis") << tr("X axis") <<tr("C axis");
				item_opt = QInputDialog::getItem(this, tr("image flipping option"),	 tr("Which axis to flip"), items_opt, 0, false, &ok_opt);
				if (ok_opt && !item_opt.isEmpty())
				{
					if (item_opt==tr("Y axis"))	{my_axiscode = axis_y; imgData->flip(my_axiscode);}
					else if (item_opt==tr("Z axis")) {my_axiscode = axis_z; imgData->flip(my_axiscode);}
					else if (item_opt==tr("X axis")) {my_axiscode = axis_x; imgData->flip(my_axiscode);}
					else if (item_opt==tr("C axis")) {my_axiscode = axis_c; imgData->flip(my_axiscode);}
					else
					{
						printf("wrong option in flipping image.\n");
					}
				}
			}
			else if (item==tr(" -- Permute image"))
			{
				bool ok1, ok_opt;
				AxisCode my_axiscode;

				QStringList items_opt;
				QString item_opt;

				items_opt << tr("XY") << tr("XZ") << tr("YZ") <<tr("ZC") <<tr("YZX");
				item_opt = QInputDialog::getItem(this, tr("image permutation option"),	 tr("Which axis order you want to change to (the unshown axis will not be influenced)"), items_opt, 0, false, &ok_opt);
				if (ok_opt && !item_opt.isEmpty())
				{
				    V3DLONG dimorder[4]; dimorder[0] = 0; dimorder[1] = 1; dimorder[2]=2; dimorder[3]=3;
				    if (item_opt==tr("XY"))	{dimorder[0] = 1; dimorder[1] = 0; }
					else if (item_opt==tr("XZ"))	{dimorder[2] = 0; dimorder[0] = 2; }
					else if (item_opt==tr("YZ"))	{dimorder[1] = 2; dimorder[2] = 1; }
					else if (item_opt==tr("ZC"))	{dimorder[2] = 3; dimorder[3] = 2; }
					else if (item_opt==tr("YZX"))	{dimorder[0] = 1; dimorder[1] = 2; dimorder[2]= 0;}
					else
					{
						printf("wrong option in permute image.\n");
					}
					imgData->permute(dimorder);
				}
			}
			else if (item==tr(" -- clear all landmarks (and of course the connectivity map of landmarks)"))
			{
				imgData->listLandmarks.clear();
				imgData->listLocationRelationship.clear();
			}
			else if (item==tr(" -- clear the enitre connectivity map of landmarks"))
			{
				imgData->listLocationRelationship.clear();
			}
			else if (item==tr(" -- crop image using minMax bounding box in 3D (derived from ROIs)") || item==tr(" -- crop image via input min-max bounds"))
			{
				bool ok_opt;

				QStringList items_opt;
				QString item_opt;

				int landmark_crop_opt=0;
				if (imgData->listLandmarks.count()>0) //only activate this dialog if there is landmark(s)
				{
					items_opt << tr("Subtract min and delete landmarks outside the bounding box") << tr("Subtract min and keep landmarks outside the bounding box") << tr("Do not alter landmark coordinates");
					item_opt = QInputDialog::getItem(this, tr("Landmark changing option"),
										 tr("You have defined some landmarks for this image; how you would like to alter their coordinates for cropping?"), items_opt, 0, false, &ok_opt);
					if (ok_opt && !item_opt.isEmpty())
					{
						if (item_opt==tr("Subtract min and delete landmarks outside the bounding box"))
							landmark_crop_opt = 1;
						else if (item_opt==tr("Subtract min and keep landmarks outside the bounding box"))
							landmark_crop_opt = 2;
						else if (item_opt==tr("Do not alter landmark coordinates"))
							landmark_crop_opt = 0;
						else
							landmark_crop_opt = 0;
					}
				}

				if (item==tr(" -- crop image using minMax bounding box in 3D (derived from ROIs)"))
				{
					try {imgData->crop(landmark_crop_opt);}
					catch (...) {QString tmps=QString("Fail to run [") + item + "]"; QMessageBox::warning(0, "Fail to run", tmps);
						printf("%s.\n", qPrintable(tmps));
					}
				}
				else //if (item==tr(" -- crop image via input min-max bounds")) // a stupid dialog, need to change later
				{
					bool ok1;
					V3DLONG bpos_x, epos_x, bpos_y, epos_y, bpos_z, epos_z, bpos_c, epos_c;

					cropImgPara p;
					p.xmin  = 1;
					p.xmax  = imgData->getXDim();
					p.ymin  = 1;
					p.ymax  = imgData->getYDim();
					p.zmin  = 1;
					p.zmax  = imgData->getZDim();
					p.colormin = 1;
					p.colormax = imgData->getCDim();
					p.landmark_opt = landmark_crop_opt;

					Dialog_imagecrop_bbox d(&p);
					if (d.exec()==QDialog::Accepted)
					{
						d.fetchData(&p);

						bpos_x = p.xmin;
						epos_x = p.xmax;
						bpos_y = p.ymin;
						epos_y = p.ymax;
						bpos_z = p.zmin;
						epos_z = p.zmax;
						bpos_c = p.colormin;
						epos_c = p.colormax;
						landmark_crop_opt = p.landmark_opt;

						try {imgData->crop(bpos_x-1, epos_x-1, bpos_y-1, epos_y-1, bpos_z-1, epos_z-1, bpos_c-1, epos_c-1, landmark_crop_opt);}
						catch (...) {QString tmps=QString("Fail to run [") + item + "]"; QMessageBox::warning(0, "Fail to run", tmps);
							printf("%s.\n", qPrintable(tmps));
						}
					}
				}
			}
			else if (item==tr(" -- split channels"))
			{
				try {imgData->proj_general_split_channels(true, 0);} //the first parameter=true determines all channels will be split and get kept
				catch (...) {QString tmps=QString("Fail to run [") + item + "]"; QMessageBox::warning(0, "Fail to run", tmps);
					printf("%s.\n", qPrintable(tmps));
				}
			}
			else if (item==tr(" -- extract one channel"))
			{
				bool ok1;
				V3DLONG bpos_c = QInputDialog::getInteger(this, tr("Channel to extract"), tr("Which channel to extract:"), 1, 1, imgData->getCDim(), 1, &ok1);
				if (ok1)
				{
					try {imgData->proj_general_split_channels(false, bpos_c-1);} //the first parameter=false determines one channel will be kept
					catch (...) {QString tmps=QString("Fail to run [") + item + "]"; QMessageBox::warning(0, "Fail to run", tmps);
						printf("%s.\n", qPrintable(tmps));
					}
				}
			}
			else if (item==tr(" -- masking image using bounding boxes in 3D (derived from ROIs)") || item==tr(" -- masking image using ROIs in 3D"))
			{
				int minchannel, maxchannel;
				int tval;
				bool b_inside;
				ImageMaskingCode my_maskcode;

				if (item==tr(" -- masking image using bounding boxes in 3D (derived from ROIs)"))
					my_maskcode = IMC_XYZ_UNION;
				else
					my_maskcode = IMC_XY; //default

				maskROIPara p;
				p.maskregion   = my_maskcode;
				p.firstchannel = 1;
				p.lastchannel  = imgData->getCDim();
				p.fillvalue    = 0;
				p.fillROI = 0; // 1 for inside and 0 for outside ROI

				Dialog_maskroi d(&p);
				if (d.exec()==QDialog::Accepted)
				{
					d.fetchData(&p);

					my_maskcode = ImageMaskingCode(p.maskregion);
					minchannel  = p.firstchannel;
					maxchannel  = p.lastchannel;
					tval        = p.fillvalue;
					b_inside    = p.fillROI > 0;

					if (item==tr(" -- masking image using bounding boxes in 3D (derived from ROIs)"))
						imgData->maskBW_roi_bbox(tval, minchannel-1, maxchannel-1, my_maskcode, b_inside);
					else //	if (item==tr(" -- masking image using ROIs in 3D"))
						imgData->maskBW_roi(tval, minchannel-1, maxchannel-1, my_maskcode, b_inside);
				}
			}
			else if (item==tr(" -- masking image using non-ROIs for all XY planes")) //provided for convenience
			{
				int mincoord=1, maxcoord=imgData->getCDim();
				int tval=0;
				ImageMaskingCode my_maskcode = IMC_XY;
				bool b_inside=false;
				bool ok1;

				tval = QInputDialog::getInteger(this, tr("fill value"), tr("fill value:"), 0, 0, 255, 10, &ok1); //090512 RZC
				if (ok1)
				{
					imgData->maskBW_roi(tval, mincoord-1, maxcoord-1, my_maskcode, b_inside);
				}
			}
			else if (item==tr(" -- masking image using channels"))
			{
				int mask_channel_no;
				//int tval;
				bool ok1;

				mask_channel_no = QInputDialog::getInteger(this, tr("range"), tr("which channel contains the mask (non-zero indicats the mask rgn:"), 3, 1, imgData->getCDim(), 1, &ok1);
				if (ok1)
				{
					try {imgData->maskBW_channel(V3DLONG(mask_channel_no-1));}
					catch (...) {QString tmps=QString("Fail to run [") + item + "]"; QMessageBox::warning(0, "Fail to run", tmps);
						printf("%s.\n", qPrintable(tmps));
					}
				}
			}
			else if (item==tr(" -- clear the ROI"))
			{
				roiPolygon.clear();
			}
			else if (item==tr(" -- resample image (and also associated landmarks)")) //will be extended as x, y, z, xy, xyz resampling
			{
				double cur_rez, target_rez;
				bool ok1, ok_opt;

				Dialog_imageresample d;
				if (d.exec()==QDialog::Accepted)
				{
					imageResamplePara p;
					d.fetchData(&p);

					ImageResamplingCode my_tmpcode;
					switch (p.axes)
					{
						case 0: my_tmpcode = PRS_Z_ONLY; break;
						case 1: my_tmpcode = PRS_XY_SAME; break;
						case 2: my_tmpcode = PRS_XYZ_SAME; break;
						case 3: my_tmpcode = PRS_X_ONLY; break;
						case 4: my_tmpcode = PRS_Y_ONLY; break;
						default: my_tmpcode = PRS_Z_ONLY; break;
					}

					try {imgData->proj_general_resampling(my_tmpcode, p.tarPixelsize, p.curPixelsize, p.b_method_interp);}
					catch (...)
					{
						QString tmps=QString("Fail to run [") + item + "]"; QMessageBox::warning(0, "Fail to run", tmps);
						printf("%s.\n", qPrintable(tmps));
					}
				}
			}
			else if (item==tr(" -- rescale landmarks only (without resampling image)"))
			{
				double cur_rez, target_rez;
				bool ok1, ok_opt;

				QStringList items_opt;
				QString item_opt;

				items_opt << tr("Z axis") << tr("XY axis (in proportion)") << tr("XYZ axis (in proportion)") << tr("X axis") << tr("Y axis");

				item_opt = QInputDialog::getItem(this, tr("Rescaling options"),
									 tr("Which axis/axes you like to rescale the landmarks"), items_opt, 0, false, &ok_opt);
				if (ok_opt && !item_opt.isEmpty())
				{
					cur_rez = QInputDialog::getDouble(this, tr("current pixel resolution"), tr("current pixel size (um):"), 1, 0.0001, 20, 4, &ok1);
					if (ok1)
					{
						target_rez = QInputDialog::getDouble(this, tr("target pixel resolution"), tr("target pixel size (um):"), 1, 0.0001, 20, 4, &ok1);
						if (ok1 && target_rez!=cur_rez)
						{
							ImageResamplingCode my_tmpcode;
							if (item_opt==tr("Z axis"))
								my_tmpcode = PRS_Z_ONLY;
							else if (item_opt==tr("XY axis (in proportion)"))
								my_tmpcode = PRS_XY_SAME;
							else if (item_opt==tr("XYZ axis (in proportion)"))
								my_tmpcode = PRS_XYZ_SAME;
							else if (item_opt==tr("X axis"))
								my_tmpcode = PRS_X_ONLY;
							else if (item_opt==tr("Y axis"))
								my_tmpcode = PRS_Y_ONLY;

							try
							{
								imgData->proj_general_resampling_landmark_only(my_tmpcode, target_rez, cur_rez);
							}
							catch (...)
							{
								QString tmps=QString("Fail to run [") + item + "]"; QMessageBox::warning(0, "Fail to run", tmps);
								printf("%s.\n", qPrintable(tmps));
							}
						}
					}
				}
			}
			else if (item==tr(" -- landmarks +/- constant"))
			{
				double cval;
				bool ok1, ok_opt;

				QStringList items_opt;
				QString item_opt;

				items_opt << tr("Z coordinate") << tr("X coordinate") << tr("Y coordinate");

				item_opt = QInputDialog::getItem(this, tr("+/- constant options"),
									 tr("Which coordinate you like to +/- a constant to ALL landmarks"), items_opt, 0, false, &ok_opt);
				if (ok_opt && !item_opt.isEmpty())
				{
					cval = QInputDialog::getDouble(this, tr("offset value"), tr("the constant offset value (+/- pixels):"), 0, -4096, 4096, 10, &ok1);
					if (ok1)
					{
						if (item_opt==tr("Z coordinate"))
							imgData->proj_general_landmark_plusminus_constant(PRS_Z_ONLY, cval);
						else if (item_opt==tr("X coordinate"))
							imgData->proj_general_landmark_plusminus_constant(PRS_X_ONLY, cval);
						else if (item_opt==tr("Y coordinate"))
							imgData->proj_general_landmark_plusminus_constant(PRS_Y_ONLY, cval);
					}
				}
			}
			else if (item==tr(" -- automarker for entire image") || item==tr(" -- automarker for roi"))
			{
				bool ok1=true;
				V3DLONG chno=1;
				if (imgData->getCDim()>1)
					chno = QInputDialog::getInteger(0, QString("select a channel"), QString("select a channel of image you'd apply AutoMarker to:"), 1, 1, int(imgData->getCDim()), 1, &ok1);

				if (ok1)
				{
					QList <LocationSimple> rlist;
					if (item==tr(" -- automarker for roi"))
					{
						QRect b_xy = imgData->p_xy_view->getRoiBoundingRect();
						QRect b_yz = imgData->p_yz_view->getRoiBoundingRect();
						QRect b_zx = imgData->p_zx_view->getRoiBoundingRect();

						V3DLONG bpos_x = qBound(V3DLONG(0), V3DLONG(qMax(b_xy.left(), b_zx.left())), imgData->getXDim()-1),
							bpos_y = qBound(V3DLONG(0), V3DLONG(qMax(b_xy.top(),  b_yz.top())), imgData->getYDim()-1),
							bpos_z = qBound(V3DLONG(0), V3DLONG(qMax(b_yz.left(), b_zx.top())), imgData->getZDim()-1);
						V3DLONG epos_x = qBound(V3DLONG(0), V3DLONG(qMin(b_xy.right(), b_zx.right())), imgData->getXDim()-1),
							epos_y = qBound(V3DLONG(0), V3DLONG(qMin(b_xy.bottom(), b_yz.bottom())), imgData->getYDim()-1),
							epos_z = qBound(V3DLONG(0), V3DLONG(qMin(b_yz.right(), b_zx.bottom())), imgData->getZDim()-1);

						BoundingBox bbox(bpos_x, bpos_y, bpos_z, epos_x, epos_y, epos_z);
						float zthickness=1.0; //if click from the main menu, then the triview is the currently active window, then of course zthickness has not been defined yet, thus use default 1
						rlist = imgData->autoMarkerFromImg(chno-1, bbox, zthickness);
					}
					else
					{
						rlist = imgData->autoMarkerFromImg(chno-1);
					}
					imgData->listLandmarks <<(rlist);
				}
			}
			else if (item==tr(" -- projection (max)"))
			{
				double mincoord, maxcoord;
				AxisCode myaxis;

				bool ok1, ok_opt;

				QStringList items_opt;
				QString item_opt;

				items_opt << tr("along Z axis (to XY plane)") << tr("along X axis (to YZ plane)") << tr("along Y axis (to XZ plane)");

				item_opt = QInputDialog::getItem(this, tr("projection direction"),
									 tr("Along which axis you want the projection"), items_opt, 0, false, &ok_opt);
				if (ok_opt && !item_opt.isEmpty())
				{
					if (item_opt==tr("along Z axis (to XY plane)"))	{myaxis = axis_z; mincoord = maxcoord = imgData->getZDim();}
					else if (item_opt==tr("along X axis (to YZ plane)")) {myaxis = axis_x; mincoord = maxcoord = imgData->getXDim();}
					else //if (item_opt==tr("along Y axis (to XZ plane)"))
						{myaxis = axis_y; mincoord = maxcoord = imgData->getYDim();}

					mincoord = QInputDialog::getInteger(this, tr("range"), tr("start section:"), 1, 1, mincoord, 10, &ok1);
					if (ok1)
					{
						maxcoord = QInputDialog::getInteger(this, tr("range"), tr("end section:"), maxcoord, 1, maxcoord, 10, &ok1);
						if (ok1)
							imgData->proj_general_projection(myaxis, mincoord-1, maxcoord-1);
					}
				}
			}
			else if (item==tr(" -- merge channels of multple images"))
			{
				imgData->proj_general_blend_channels();
			}
			else if (item==tr(" -- stitch two images"))
			{
				V3DLONG channo;
				bool ok1;
				channo = QInputDialog::getInteger(this, tr("Channel"), tr("Which channel to align:"), 1, 1, imgData->getCDim(), 1, &ok1);
				if (ok1)
					imgData->proj_general_stitchTwoImages(channo-1);
			}
			else if (item==tr(" -- display histogram") )
			{
				bool ok1;

				imgData->proj_general_hist_display();
				v3d_msg("ok",0);

			}
			else if (item==tr(" -- linear adjustment") )
			{
				bool ok1;

				imgData->proj_general_linear_adjustment();
				v3d_msg("ok",0);

			}
			else if (item==tr(" -- histogram equalization") )
			{
				bool ok1;
				int lowerbound, higherbound;
				lowerbound = QInputDialog::getInteger(this, tr("Equalization range"), tr("Lowerbound of the foreground intensity:"), 30, 0, 255, 10, &ok1);
				if (ok1)
				{
					higherbound = QInputDialog::getInteger(this, tr("Equalization range"), tr("Higherbound of the foreground intensity:"), 255, 0, 255, 10, &ok1);
					if (ok1)
						imgData->proj_general_hist_equalization((unsigned char)lowerbound, (unsigned char)higherbound);
				}
			}
			else if (item==tr(" -- convert indexed image to RGB"))
			{
				imgData->proj_general_convertIndexedImg2RGB();
			}
			else if (item==tr(" -- linear scaling to [0,255] and convert to 8 bit"))
			{
				imgData->proj_general_scaleandconvert28bit(0, 255);
			}
			else if (item== tr(" -- convert 16bit image to 8 bit"))
			{
				bool ok1;
				int shiftnbits = QInputDialog::getInteger(this, tr("Dividing factor"), tr("How many bits you would like to shift to the right during the 16-bit to 8-bit conversion?"), 0, 0, 8, 1, &ok1);
				if (ok1)
				{
					imgData->proj_general_convert16bit_to_8bit(shiftnbits);
				}
			}
			else if (item== tr(" -- convert 32bit (single-precision float) image to 8 bit"))
			{
//				bool ok1;
//				int shiftnbits = QInputDialog::getInteger(this, tr("Dividing factor"), tr("How many bits you would like to shift to the right during the 16-bit to 8-bit conversion?"), 0, 0, 8, 1, &ok1);
//				if (ok1)
//				{
				//2010-01-29. no need to ask how many bits, just shift 0 bit as the function indeed will call the proj_general_scaleandconvert28bit()
					imgData->proj_general_convert32bit_to_8bit(0);
//				}
			}
			else if (item== tr(" -- intensity scaling"))
			{
				V3DLONG channo;
				bool ok1;
				channo = QInputDialog::getInteger(this, tr("Channel"), tr("Which channel to scale intensity (select 0 to choose *ALL* channels):"), 1, 0, imgData->getCDim(), 1, &ok1);
				if (ok1)
				{
					double lower_th, higher_th, target_min=0, target_max=255;
					lower_th = QInputDialog::getDouble(this, "range", "input the lower bound for intensity rescaling:", imgData->getChannalMinIntensity(channo-1), -2147483647, 2147483647, 2, &ok1);
					if (ok1)
					{
						higher_th = QInputDialog::getDouble(this, "range", "input the upper bound for intensity rescaling:", imgData->getChannalMaxIntensity(channo-1), -2147483647, 2147483647, 2, &ok1);
						if (ok1)
						{
							target_min = QInputDialog::getDouble(this, "range", "input the target min value for intensity rescaling:", 0, -2147483647, 2147483647, 2, &ok1);
							if (ok1)
							{
								target_max = QInputDialog::getDouble(this, "range", "input the target max value for intensity rescaling:", 255, -2147483647, 2147483647, 2, &ok1);
								if (ok1)
								{
									imgData->scaleintensity(channo-1, lower_th, higher_th, target_min, target_max);
								}
							}
						}
					}
				}
			}
			else if (item==tr(" -- intensity thresholding"))
			{
				V3DLONG channo;
				bool ok1;
				channo = QInputDialog::getInteger(this, tr("Channel"), tr("Which channel to threshold intensity (select 0 to choose *ALL* channels):"), 1, 0, imgData->getCDim(), 1, &ok1);
				if (ok1)
				{
					double th;
					th = QInputDialog::getDouble(this, "range", "input the threshold (any value < t will be set to 0, >=t remains unchanged):", imgData->getChannalMinIntensity(channo-1), -2147483647, 2147483647, 2, &ok1);
					if (ok1)
					{
						imgData->thresholdintensity(channo-1, th);
					}
				}
			}
			else if (item==tr(" -- intensity binarization"))
			{
				V3DLONG channo;
				bool ok1;
				channo = QInputDialog::getInteger(this, tr("Channel"), tr("Which channel to binarize intensity (select 0 to choose *ALL* channels):"), 1, 0, imgData->getCDim(), 1, &ok1);
				if (ok1)
				{
					double th;
					th = QInputDialog::getDouble(this, "range", "input the threshold (any value < t will be set to 0, >=t set to 1):", imgData->getChannalMinIntensity(channo-1), -2147483647, 2147483647, 2, &ok1);
					if (ok1)
					{
						imgData->binarizeintensity(channo-1, th);
					}
				}
			}
			else if (item==tr(" -- intensity minmaxvalue recomputing"))
			{
				imgData->updateminmaxvalues();
				imgData->updateViews();
			}
			else if (item==tr(" -- invert image color"))
			{
				V3DLONG channo;
				bool ok1;
				channo = QInputDialog::getInteger(this, tr("Channel"), tr("Which channel to invert color (select 0 to invert *ALL* channels):"), 1, 0, imgData->getCDim(), 1, &ok1);
				if (ok1)
					if (imgData) imgData->invertcolor(channo-1);
			}
			else if (item==tr(" -- rearrange grayscale image as color images"))
			{
				if (imgData->getCDim()!=1)
				{
					QMessageBox::warning(0, "Not a grayscale image", "Your image is not a grsyscale image. Do nothing.");
					printf("Your image is not a grayscale image. Do nothing.\n");
				}
				else
				{
					bool ok1;
					int nchannels = QInputDialog::getInteger(this, tr("select number of color channels"), tr("How many color channels to want to have:"), 2, 1, 10, 1, &ok1);
					if (ok1)
					{
						if (imgData->getZDim()/nchannels*nchannels!=imgData->getZDim())
						{
							QMessageBox::warning(0, "Unusable number of color channels", "The number of image sections cannot be divided by the color channel number you just input. Do nothing.");
							printf("The number of image sections cannot be divided by the color channel number you just input. Do nothing.\n");
						}
						else
						{
							if(QMessageBox::Yes == QMessageBox::question (0, "", "Are the color channels interlaced (e.g. 1 red section/slice followed by 1 green section/slice) or arranged in trunks (e.g. 30 red slices followed by 30 greens)?", QMessageBox::Yes, QMessageBox::No))
							{
								imgData->reshape(imgData->getXDim(), imgData->getYDim(), nchannels, imgData->getZDim()/nchannels);
								V3DLONG dimorder[4]; dimorder[0]=0; dimorder[1]=1; dimorder[2]=3; dimorder[3]=2;
								imgData->permute(dimorder);
							}
							else
							{
								imgData->reshape(imgData->getXDim(), imgData->getYDim(), imgData->getZDim()/nchannels, nchannels);
							}
						}
					}
				}
			}
			else if (item==tr(" -- open another image/stack in *THIS* window"))
			{
				imgData->getXWidget()->setOpenFileName();
			}
			else if (item==tr(" -- save to file"))
			{
				//imgData->saveFile();
				try	{imgData->getXWidget()->saveData();} catch (...) {QMessageBox::warning(0, "Fail to save image", "Fail to save images. ");printf("Fail to save image.\n");}
			}
			else if (item==tr(" -- save to VANO annotation files"))
			{
				try	{imgData->saveVANO_data();} catch (...) {QMessageBox::warning(0, "Fail to save VANO files", "Fail to save VANO files. ");printf("Fail to save VANO files.\n");}
			}
			else if (item==tr(" -- save to movie"))
			{
				try	{imgData->saveMovie();} catch (...) {QMessageBox::warning(0, "Fail to save to movie", "Fail to save to movie. ");printf("Fail to save movie.\n");}
			}
			else if (item==tr(" -- Export landmarks to point cloud (APO) file"))
			{
				imgData->exportLandmarkToPointCloudAPOFile();
			}
			else if (item==tr(" -- Export landmarks and their relationship to graph (SWC) file"))
			{
				imgData->exportLandmarkandRelationToSWCFile();
			}
			else if (item==tr(" -- Export traced neuron or fibrous structures to graph (SWC) file"))
			{
				imgData->exportNeuronToSWCFile();
			}
/*
			else if (item==tr(" -- Randomly seed landmarks/control-points"))
			{
				bool ok1;
				int kch = QInputDialog::getInteger(this, tr("Channel No. used to determine foreground"), tr("Channel No:"), 1, 1, imgData->getCDim(), 1, &ok1);
				if (ok1)
					imgData->proj_worm_random_landmarking(kch-1, 1, 40);
			}
			else if (item==tr(" -- Find MST (Minimum spanning tree) of landmarks"))
				imgData->proj_worm_mst_diameter(false);
			else if (item==tr(" -- Detect diameter graph of MST"))
				imgData->proj_worm_mst_diameter(true);
			else if (item==tr(" -- Adjust backbone graph using BDB_minus algorithm"))
			{
				imgData->proj_worm_bdb_backbone();
			}
			else if (item==tr(" -- Find cutting plane locations"))
			{
				imgData->proj_worm_straightening(false, 160); //only show cutting plane locations
			}
			else if (item==tr(" -- Straighten using slice-restacking"))
			{
				bool ok1;
				int OutWid = QInputDialog::getInteger(this, tr("Diameter of cutting plane"), tr("Diameter (# pixels) of each cutting plane:"), 160, 1, qMin(imgData->getXDim(),imgData->getYDim()), 20, &ok1);
				if (ok1)
					imgData->proj_worm_straightening(true, OutWid); //directly do straightening
			}
			else if (item==tr(" -- Seed landmarks/control-points on regular grid"))
			{
				bool ok1;
				int kch = QInputDialog::getInteger(this, tr("Channel No. used to determine foreground"), tr("Channel No:"), 1, 1, imgData->getCDim(), 1, &ok1);
				if (ok1)
					imgData->proj_alignment_seed_grid(kch-1);
			}
			else if (item==tr(" -- Seed landmarks/control-points using big gradient (edge) points"))
			{
				bool ok1;
				int kch = QInputDialog::getInteger(this, tr("Channel No. used to determine foreground"), tr("Channel No:"), 1, 1, imgData->getCDim(), 1, &ok1);
				if (ok1)
					imgData->proj_alignment_seed_gradient(kch-1);
			}
			else if (item==tr(" -- Seed landmarks/control-points using big curvature (corner) points"))
			{
				bool ok1;
				int kch = QInputDialog::getInteger(this, tr("Channel No. used to determine foreground"), tr("Channel No:"), 1, 1, imgData->getCDim(), 1, &ok1);
				if (ok1)
					imgData->proj_alignment_seed_curvature(kch-1);
			}
			else if (item==tr(" -- Seed landmarks/control-points randomly"))
			{
				bool ok1;
				int kch = QInputDialog::getInteger(this, tr("Channel No. used to determine foreground"), tr("Channel No:"), 1, 1, imgData->getCDim(), 1, &ok1);
				if (ok1)
					imgData->proj_alignment_seed_random(kch-1, 2.0, 100);
			}
			else if (item==tr(" -- Seed landmarks/control-points using a file"))
			{
				QString curFile = QFileDialog::getOpenFileName(this,
											QObject::tr("Select a text file to save the coordinates of landmark points... "),
											QObject::tr(""),
											QObject::tr("Landmark definition file (*.csv);;Landmark definition file (*.txt);;All Files (*)"));
				imgData->proj_alignment_seed_file(curFile);
			}
			else if (item==tr(" -- Global affine alignment (using image content)"))
			{
				imgData->proj_alignment_global();
			}
			else if (item==tr(" -- Global affine alignment (using xform derived from matching landmarks)"))
			{
				imgData->proj_alignment_affine_matching_landmarks();
			}
			else if (item==tr(" -- Segmenting optical lobes of a fly brain (for a globally aligned fly brain)"))
			{
				imgData->proj_alignment_flybrain_lobeseg();
			}
			else if (item==tr(" -- Mask blue to 0") )
			{
				imgData->proj_general_maskBlue2Zero();
			}
			else if (item==tr(" -- Match landmarks defined for another image (i.e. registration target)"))
			{
				imgData->proj_alignment_matching_point();
			}
			else if (item==tr(" -- Match one single landmark in another image") )
			{
				if (imgData->listLandmarks.count()<=0)
				{
					QMessageBox::warning(0, "Non landmark defined yet", "You have NOT defined any landmark yet. Define at least one before you select this menu.");
				}
				else
				{
					double dmin;
					int cur_pt_ind = imgData->find_closest_control_pt(imgData->curFocusX, imgData->curFocusY, imgData->curFocusZ, dmin);
					cur_pt_ind = (cur_pt_ind<0) ? 1 : (cur_pt_ind + 1);
					bool ok1;
					int pt_ind = QInputDialog::getInteger(this, tr("landmark index"), tr("which landmark you want to find the match:"), cur_pt_ind, 1, imgData->listLandmarks.count(), 1, &ok1);
					if (ok1)
					{
						imgData->proj_alignment_matching_1single_pt(pt_ind-1);
					}
				}
			}
			else if (item==tr(" -- Warp image using corresponding landmarks"))
			{
				bool b_overwrite_subject;
				if(QMessageBox::Yes == QMessageBox::question (0, "", "Do you want to overwrite the subject image?", QMessageBox::Yes, QMessageBox::No))
					b_overwrite_subject = true;
				else
					b_overwrite_subject = false;

				imgData->proj_alignment_warp_using_landmarks(b_overwrite_subject);
			}
			else if (item==tr(" -- ** All-in-one: detect corresponding landmarks and then warp"))
			{
				bool b_overwrite_subject;
				if(QMessageBox::Yes == QMessageBox::question (0, "", "Do you want to overwrite the subject image?", QMessageBox::Yes, QMessageBox::No))
					b_overwrite_subject = true;
				else
					b_overwrite_subject = false;

				imgData->proj_alignment_find_landmark_and_warp(b_overwrite_subject);
			}
			else if (item==tr(" -- top-down skeletonization"))
			{
				bool ok1;
				int kch = QInputDialog::getInteger(this, tr("Channel no"), tr("which channel contains your fibrous structures (e.g. neurons):"), 1, 1, imgData->getCDim(), 1, &ok1);
				if (ok1)
				{
					double kfactor = QInputDialog::getDouble(this, tr("threshold"), tr("threshold factor:"), 1, -1, 10, 1, &ok1);
					if (ok1)
					{
						int npt = QInputDialog::getInteger(this, tr("Number of points"), tr("Number of initial seed points:"), 200, 1, 10000, 50, &ok1);
						if (ok1)
						{
							imgData->proj_alignment_seed_random(kch-1, kfactor, npt);
							imgData->proj_worm_mst_diameter(false); //need to further incorprate all the skeletionzation codes. by Hanchuan Peng, 080424
						}
					}
				}
			}
*/
			else if (item==tr(" -- trace one marker to all others"))
			{
				if (imgData->proj_trace_deformablepath_one_point_to_allotherpoints(imgData->cur_hit_landmark))
					imgData->update_3drenderer_neuron_view();
			}
			else if (item==tr(" -- trace between two locations"))
			{
				if (imgData->proj_trace_deformablepath_two_points(0, 1, true))
					imgData->update_3drenderer_neuron_view();
			}
			else if (item==tr(" -- undo last tracing step"))
			{
				imgData->proj_trace_history_undo();
				imgData->update_3drenderer_neuron_view();
			}
			else if (item==tr(" -- redo last tracing step"))
			{
				imgData->proj_trace_history_redo();
				imgData->update_3drenderer_neuron_view();
			}
			else if (item==tr(" -- clear traced neuron"))
			{
				imgData->tracedNeuron.seg.clear();
				//also try to clear the neuron in the renderer
				imgData->update_3drenderer_neuron_view();
			}
			else if (item==tr(" -- update 3D view of traced neuron"))
			{
				imgData->update_3drenderer_neuron_view();
			}
			else if (item==tr(" -- save traced neuron"))
			{
				imgData->exportNeuronToSWCFile();
			}
/*			
			else if (item==tr(" -- bottom-up marching"))
			{
			//need to incorprate the local searching codes. by Hanchuan Peng, 080424
			}
			else if (item==tr(" -- global-local (Global) integration of top-down/bottom-up results"))
			{
			//need to incorprate the glocal integration codes. by Hanchuan Peng, 080424
			}
			else if (item==tr(" -- manual correction"))
			{
			//need to incorprate the respacing and manual correction codes. by Hanchuan Peng, 080424
			}
			else if (item==tr(" -- local template matching"))
			{
				imgData->proj_cellseg_templatematching();
			}
			else if (item==tr(" -- cell counting (Yang Yu)"))
			{
				imgData->proj_cellseg_cellcounting();
			}
			else if (item==tr(" -- watershed segmentation"))
			{
				imgData->proj_cellseg_watershed();
			}
			else if (item==tr(" -- levelset segmentation"))
			{
				imgData->proj_cellseg_levelset();
			}
			else if (item==tr(" -- 1-Gaussian fit of current focus pos"))
			{
				V3DLONG tmp_channo = 0; //will need to change this later
				imgData->proj_cellseg_GaussianFit_pos(imgData->curFocusX, imgData->curFocusY, imgData->curFocusZ, tmp_channo, 1, true);
			}
			else if (item==tr(" -- N-Gaussian fit of current focus pos"))
			{
				V3DLONG tmp_channo = 0; //will need to change this later
				imgData->proj_cellseg_GaussianFit_pos(imgData->curFocusX, imgData->curFocusY, imgData->curFocusZ, tmp_channo, 4, true);
			}
*/
		}
		catch (...)
		{
			QString tmps=QString("Fail to run [") + item + "]"; QMessageBox::warning(0, "Fail to run", tmps);
			printf("%s.\n", qPrintable(tmps));
		}
	}

	//update all views
	imgData->updateViews();
}




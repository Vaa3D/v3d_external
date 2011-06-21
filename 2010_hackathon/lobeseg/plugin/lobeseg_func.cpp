/* lobeseg_func.cpp
 * This is a lobeseg plugin
 * 2011-06-20 : by Hang Xiao
 */

#include <v3d_interface.h>
#include "v3d_message.h"
#include "lobeseg_func.h"
#include "lobeseg_gui.h"

#include "../lobeseg.h"
#include "../../../v3d_main/worm_straighten_c/bdb_minus.h"

const QString title = QObject::tr("Lobeseg Plugin");
int lobeseg_two_sides(V3DPluginCallback2 &callback, QWidget *parent)
{
	v3dhandleList win_list = callback.getImageWindowList();

	if(win_list.size()<1)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return -1;
	}

	TwoSidesDialog dialog(callback, parent);
	if(dialog.exec() == QDialog::Rejected) return -1;
	dialog.update();
	int i = dialog.i;
	int c = dialog.channel;
	double alpha = dialog.alpha;
	double beta = dialog.beta;
	double gamma = dialog.gamma;
	int nloops = dialog.nloops;
	int radius = dialog.radius;

	Image4DSimple * image = callback.getImage(win_list[i]);
	if(image->getCDim() <= c) {v3d_msg(QObject::tr("The channel isn't existed.")); return -1;}
	unsigned char * inimg1d = image->getRawData();
	V3DLONG sz[4];
	sz[0] = image->getXDim();
	sz[1] = image->getYDim();
	sz[2] = image->getZDim();
	sz[3] = image->getCDim();
	unsigned char * outimg1d = new unsigned char[sz[0] * sz[1] * sz[2] * sz[3]];
	int out_channel_no = 2;

	BDB_Minus_ConfigParameter mypara;
	mypara.f_image = alpha;
	mypara.f_smooth = beta;
	mypara.f_length = gamma;
	mypara.nloops = nloops;
	mypara.radius = radius;
	mypara.radius_x = radius;
	mypara.radius_y = radius;
	mypara.TH = 0.1;

	V3DPluginArgItem arg;
	V3DPluginArgList input;
	V3DPluginArgList output;
	arg.type="image1d"; arg.p = inimg1d; input << arg;
	arg.type="v3dlong1x4"; arg.p = sz; input<<arg;
	arg.type="image1d"; arg.p = outimg1d; input<<arg;
	arg.type="int"; arg.p = &c; input<<arg;
	arg.type="int"; arg.p = &out_channel_no; input<<arg;
	arg.type="BDB_Minus_ConfigParameter"; arg.p = &mypara ; input<<arg;

	if(!lobeseg_two_sides(input, output))
	{
		v3d_msg("lobeseg two sides error!");
		return -1;
	}
	//outimg1d = (unsigned char*)output.at(0).p; 

	Image4DSimple * p4DImage = new Image4DSimple();
	p4DImage->setXDim(sz[0]);
	p4DImage->setYDim(sz[1]);
	p4DImage->setZDim(sz[2]);
	p4DImage->setCDim(sz[3]);
	p4DImage->setRawDataPointer(outimg1d);
	
	v3dhandle newwin;
	if(QMessageBox::Yes == QMessageBox::question(0, "", QString("Do you want to use the existing windows?"), QMessageBox::Yes, QMessageBox::No))
	newwin = callback.currentImageWindow();
	else
		newwin = callback.newImageWindow();

	callback.setImage(newwin, p4DImage);
	callback.setImageName(newwin, QObject::tr("lobeseg_two_sides"));
	callback.updateImageWindow(newwin);

	return 1;
}

bool lobeseg_two_sides(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	unsigned char * inimg1d = (unsigned char *) input.at(0).p;	
	const V3DLONG * sz = (const V3DLONG *)input.at(1).p;
	unsigned char * outimg1d = (unsigned char *) input.at(2).p; 
	int in_channel_no = * ((int *)input.at(3).p);
	int out_channel_no = * ((int *)input.at(4).p);
	BDB_Minus_ConfigParameter* mypara = (BDB_Minus_ConfigParameter *)input.at(5).p;
	return do_lobeseg_bdbminus(inimg1d, sz, outimg1d, in_channel_no, out_channel_no, *mypara);
}

int lobeseg_one_side_only(V3DPluginCallback2 &callback, QWidget *parent)
{
	v3dhandleList win_list = callback.getImageWindowList();

	if(win_list.size()<1)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return -1;
	}

	OneSideOnlyDialog dialog(callback, parent);
	if(dialog.exec() == QDialog::Rejected) return -1;
	dialog.update();
	int i = dialog.i;
	int c = dialog.channel;
	double alpha = dialog.alpha;
	double beta = dialog.beta;
	double gamma = dialog.gamma;
	int nloops = dialog.nloops;
	int radius = dialog.radius;
	int x0 = dialog.x0;
	int y0 = dialog.y0;
	int x1 = dialog.x1;
	int y1 = dialog.y1;
	int keep_which = dialog.keep_which;
	int nctrls = dialog.nctrls;
	bool is_surf = dialog.is_surf;

	Image4DSimple * image = callback.getImage(win_list[i]);
	if(image->getCDim() <= c) {v3d_msg(QObject::tr("The channel isn't existed.")); return -1;}
	unsigned char * inimg1d = image->getRawData();
	V3DLONG sz[4];
	sz[0] = image->getXDim();
	sz[1] = image->getYDim();
	sz[2] = image->getZDim();
	sz[3] = image->getCDim();
	unsigned char * outimg1d = new unsigned char[sz[0] * sz[1] * sz[2] * sz[3]];
	int out_channel_no = 2;

	BDB_Minus_ConfigParameter mypara;
	mypara.f_image = alpha;
	mypara.f_smooth = beta;
	mypara.f_length = gamma;
	mypara.nloops = nloops;
	mypara.radius = radius;
	mypara.radius_x = radius;
	mypara.radius_y = radius;
	mypara.TH = 0.1;

	V3DPluginArgItem arg;
	V3DPluginArgList input;
	V3DPluginArgList output;
	arg.type="image1d"; arg.p = inimg1d; input << arg;
	arg.type="v3dlong1x4"; arg.p = sz; input<<arg;
	arg.type="image1d"; arg.p = outimg1d; input<<arg;
	arg.type="int"; arg.p = &c; input<<arg;
	arg.type="int"; arg.p = &out_channel_no; input<<arg;
	arg.type="BDB_Minus_ConfigParameter"; arg.p = &mypara ; input<<arg;
	arg.type="int"; arg.p = &x0; input<<arg;
	arg.type="int"; arg.p = &y0; input<<arg;
	arg.type="int"; arg.p = &x1; input<<arg;
	arg.type="int"; arg.p = &y1; input<<arg;
	arg.type="int"; arg.p = &keep_which; input<<arg;
	arg.type="int"; arg.p = &nctrls; input<<arg;
	arg.type="bool"; arg.p = &is_surf; input<<arg;

	if(!lobeseg_one_side_only(input, output))
	{
		v3d_msg("lobeseg two sides error!");
		return -1;
	}
	//outimg1d = (unsigned char*)output.at(0).p; 

	Image4DSimple * p4DImage = new Image4DSimple();
	p4DImage->setXDim(sz[0]);
	p4DImage->setYDim(sz[1]);
	p4DImage->setZDim(sz[2]);
	p4DImage->setCDim(sz[3]);
	p4DImage->setRawDataPointer(outimg1d);
	
	v3dhandle newwin;
	if(QMessageBox::Yes == QMessageBox::question(0, "", QString("Do you want to use the existing windows?"), QMessageBox::Yes, QMessageBox::No))
	newwin = callback.currentImageWindow();
	else
		newwin = callback.newImageWindow();

	callback.setImage(newwin, p4DImage);
	callback.setImageName(newwin, QObject::tr("lobeseg_one_side_only"));
	callback.updateImageWindow(newwin);

	return 1;
}

bool lobeseg_one_side_only(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	unsigned char * inimg1d = (unsigned char *) input.at(0).p;	
	const V3DLONG * sz = (const V3DLONG *)input.at(1).p;
	unsigned char * outimg1d = (unsigned char *) input.at(2).p; 
	int in_channel_no = * ((int *)input.at(3).p);
	int out_channel_no = * ((int *)input.at(4).p);
	BDB_Minus_ConfigParameter mypara = *((BDB_Minus_ConfigParameter *)input.at(5).p);
	int x0 = * ((int *) input.at(6).p);
	int y0 = * ((int *) input.at(7).p);
	int x1 = * ((int *) input.at(8).p);
	int y1 = * ((int *) input.at(9).p);
	int keep_which = *((int*) input.at(10).p);
	int nctrls = *((int *) input.at(11).p);
	bool is_surf = *((bool *) input.at(12).p);
	QMessageBox::information(0, "", QObject::tr("sz[0] = %1, sz[1] = %2, sz[2] = %3, sz[3] = %4\n"
				"in_channel = %5, out_channel = %6\n"
				"mypara.f_image = %7, mypara.f_length = %8, mypara.f_smooth = %9\n"
				"mypara.radius = %10, mypara.radius_x = %11, mypara.radius_y = %12\n"
				"mypara.TH = %13, x0 = %14, y0 = %15, x1 = %16, y1 = %17\n"
				"keep_which = %18, nctrls = %19, is_surf = %20")
			.arg(sz[0]).arg(sz[1]).arg(sz[2]).arg(sz[3])
			.arg(in_channel_no).arg(out_channel_no)
			.arg(mypara.f_image).arg(mypara.f_length).arg(mypara.f_smooth)
			.arg(mypara.radius).arg(mypara.radius_x).arg(mypara.radius_y)
			.arg(mypara.TH).arg(x0).arg(y0).arg(x1).arg(y1)
			.arg(keep_which).arg(nctrls).arg(is_surf));
	return do_lobeseg_bdbminus_onesideonly(inimg1d, sz, outimg1d, in_channel_no, out_channel_no, mypara, x0, y0, x1, y1, keep_which, nctrls, is_surf);
}


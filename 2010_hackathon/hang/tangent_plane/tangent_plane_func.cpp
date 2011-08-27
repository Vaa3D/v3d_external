/* tangent_plane_func.cpp
 * This is a tangent plane plugin
 * 2011-08-26 : by Hang Xiao
 */

#include <v3d_interface.h>
#include "v3d_message.h"
#include "tangent_plane_func.h"
#include "tangent_plane_gui.h"

const QString title = QObject::tr("Tangent Plane Plugin");

int start_program(V3DPluginCallback2 &callback, QWidget *parent)
{
	v3dhandleList win_list = callback.getImageWindowList();

	if(win_list.size()<1)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return -1;
	}
	TangentPlaneWidget * widget = new TangentPlaneWidget(callback,parent);
	widget->show();
	//TestDialog dialog(callback, parent);

	//if (dialog.exec()!=QDialog::Accepted) return -1;

	//dialog.update();
	//int i = dialog.i;
	//int c = dialog.channel;
	//Image4DSimple *p4DImage = callback.getImage(win_list[i]);
	//if(p4DImage->getCDim() <= c) {v3d_msg(QObject::tr("The channel isn't existed.")); return -1;}
	//V3DLONG sz[3];
	//sz[0] = p4DImage->getXDim();
	//sz[1] = p4DImage->getYDim();
	//sz[2] = p4DImage->getZDim();

	//unsigned char * inimg1d = p4DImage->getRawDataAtChannel(c);

	//v3dhandle newwin;
	//if(QMessageBox::Yes == QMessageBox::question(0, "", QString("Do you want to use the existing windows?"), QMessageBox::Yes, QMessageBox::No))
		//newwin = callback.currentImageWindow();
	//else
		//newwin = callback.newImageWindow();

	//p4DImage->setData(inimg1d, sz[0], sz[1], sz[2], sz[3]);
	//callback.setImage(newwin, p4DImage);
	//callback.setImageName(newwin, QObject::tr("start_program"));
	//callback.updateImageWindow(newwin);
	return 1;
}



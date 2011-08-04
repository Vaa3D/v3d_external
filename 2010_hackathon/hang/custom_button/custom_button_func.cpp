/* custom_button_func.cpp
 * This plugin is used to define the toolbox by yourself
 * 2011-08-04 : by Hang Xiao
 */

#include <v3d_interface.h>
#include "v3d_message.h"
#include "custom_button_func.h"
#include "custom_button_gui.h"

const QString title = QObject::tr("Quick Button Plugin");

int custom_button(V3DPluginCallback2 &callback, QWidget *parent)
{

	CustomButtonToolBar* toolBar = new CustomButtonToolBar(callback, parent);
	toolBar->show();
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
	//callback.setImageName(newwin, QObject::tr("custom_button"));
	//callback.updateImageWindow(newwin);
	return 1;
}



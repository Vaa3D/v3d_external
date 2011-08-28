/* tangent_plane_func.cpp
 * This is a tangent plane plugin
 * 2011-08-26 : by Hang Xiao
 */

#include <v3d_interface.h>
#include "v3d_message.h"
#include "tangent_plane_func.h"
#include "tangent_plane_gui.h"

const QString title = QObject::tr("Tangent Plane Plugin");

int get_tangent_plane(V3DPluginCallback2 &callback, QWidget *parent)
{
	v3dhandleList win_list = callback.getImageWindowList();

	if(win_list.size()<1)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return -1;
	}
	TangentPlaneWidget * widget = new TangentPlaneWidget(callback,parent);
	widget->show();
	
	return 1;
}

int tracking_without_branch(V3DPluginCallback2 &callback, QWidget *parent)
{
	v3dhandleList win_list = callback.getImageWindowList();

	if(win_list.size()<1)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return -1;
	}
	TrackingWithoutBranchWidget * widget = new TrackingWithoutBranchWidget(callback,parent);
	widget->show();
	
	return 1;
}


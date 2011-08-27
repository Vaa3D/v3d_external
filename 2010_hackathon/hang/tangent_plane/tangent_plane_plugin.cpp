/* tangent_plane_plugin.cpp
 * This is a tangent plane plugin
 * 2011-08-26 : by Hang Xiao
 */
 
#include "v3d_message.h"

#include "tangent_plane_plugin.h"
#include "tangent_plane_func.h"
 
Q_EXPORT_PLUGIN2(tangent_plane, TangentPlanePlugin);
 
QStringList TangentPlanePlugin::menulist() const
{
	return QStringList()
		<<tr("start program")
		<<tr("about");
}

void TangentPlanePlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("start program"))
	{
		start_program(callback,parent);
	}
	else
	{
		v3d_msg(tr("This is a tangent plane plugin. "
			"Developed by Hang Xiao, 2011-08-26"));
	}
}


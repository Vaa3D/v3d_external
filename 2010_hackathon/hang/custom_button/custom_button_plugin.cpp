/* custom_button_plugin.cpp
 * This plugin is used to define the toolbox by yourself
 * 2011-08-04 : by Hang Xiao
 */
 
#include "v3d_message.h"

#include "custom_button_plugin.h"
#include "custom_button_func.h"
 
Q_EXPORT_PLUGIN2(custom_button, CustomButtonPlugin);
 
QStringList CustomButtonPlugin::menulist() const
{
	return QStringList()
		<<tr("custom button")
		<<tr("about");
}

void CustomButtonPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("custom button"))
	{
		custom_button(callback,parent);
	}
	else
	{
		v3d_msg(tr("This plugin is used to define the toolbar by yourself. "
			"Developed by Hang Xiao, 2011-08-04"));
	}
}


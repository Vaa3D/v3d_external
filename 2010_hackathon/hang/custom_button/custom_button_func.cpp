/* custom_button_func.cpp
 * This plugin is used to define the toolbox by yourself
 * 2011-08-04 : by Hang Xiao
 */

#include <v3d_interface.h>
#include "v3d_message.h"
#include "custom_button_func.h"
#include "custom_button_gui.h"

const QString title = QObject::tr("Quick Button Plugin");

static int bar_num = 1;
static bool isFirstLoading = true;
int custom_button(V3DPluginCallback2 &callback, QWidget *parent)
{
	if(isFirstLoading)
	{
		isFirstLoading = false;
		loadToolBarSettings();
	}
	
	QList<CustomButtonSetting*> & settingList = getToolBarSettingList();
	if(!settingList.empty())
	{
		bar_num = settingList.size() + 1;

		foreach(CustomButtonSetting* cbs, settingList)
		{
			CustomButtonSelectWidget * selectWidget = new CustomButtonSelectWidget(callback, parent, cbs);
		}
		return 1;
	}
	else
	{
		QString barTitle = bar_num > 1 ? QObject::tr("Custom Toolbar - %1").arg(bar_num) : QObject::tr("Custom Toolbar");

		QToolBar* toolBar = new QToolBar(barTitle, parent);
		CustomButtonSetting* cbs = new CustomButtonSetting(toolBar);

		CustomButtonSelectWidget * selectWidget = new CustomButtonSelectWidget(callback, parent, cbs);
		settingList.push_back(cbs);

		bar_num++;
		return 1;
	}
}


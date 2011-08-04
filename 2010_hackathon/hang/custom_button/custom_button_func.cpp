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
int custom_button(V3DPluginCallback2 &callback, QWidget *parent)
{

	/* Method 1 */
	//CustomButtonToolBar* toolBar = new CustomButtonToolBar(callback, parent);
	//toolBar->show();

	/* Method 2 */
	QString title = bar_num > 1 ? QObject::tr("Custom Buttons - %1").arg(bar_num) : QObject::tr("Custom Buttons");
	bar_num++;
	QToolBar* toolBar = new QToolBar(title,parent);
	CustomButtonSelectWidget * selectWidget = new CustomButtonSelectWidget(callback, parent, toolBar);
	return 1;
}



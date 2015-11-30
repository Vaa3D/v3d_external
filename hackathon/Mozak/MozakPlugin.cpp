/* MozakPlugin.cpp
 * Simplified Terafly interface for crowdsourcing neuron tracing.
 * 2015-11-23 : by Tim Pavlik
 */
 
#include "v3d_message.h"
#include <vector>
#include "MozakPlugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(Mozak, MozakPlugin);
 
QStringList MozakPlugin::menulist() const
{
	return QStringList() 
		<<tr("open")
		<<tr("about");
}

QStringList MozakPlugin::funclist() const
{
	return QStringList();
}

void MozakPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("open"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("Simplified Terafly interface for crowdsourcing neuron tracing."
			"Developed by the Center for Game Science, University of Washington Nov-2015"));
	}
}

bool MozakPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	return false;
}


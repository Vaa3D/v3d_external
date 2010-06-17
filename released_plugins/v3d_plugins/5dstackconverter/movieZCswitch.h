/* movieZCswitch.h
 * 2009-09-22: create this program by Yang Yu
 */

#ifndef __MOVIEZCSWITCH_H__
#define __MOVIEZCSWITCH_H__

//CHANGES MOVIE STACK STORING

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <v3d_interface.h>

class MovieZCswitchPlugin : public QObject, public V3DPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface)

public:
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent);
	
	QStringList funclist() const {return QStringList();}
	void dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, QWidget *parent) {}
};

#endif




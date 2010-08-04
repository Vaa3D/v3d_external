/* ex_push.h
 * an example program to test the push function in the plugin interface
* 2010-08-3: by Hanchuan Peng
 */


#ifndef __EX_PUSH_H__
#define __EX_PUSH_H__

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>

#include "v3d_interface.h"

class ExPushPlugin : public QObject, public V3DPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface);
	
public:
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent);
	
	QStringList funclist() const {return QStringList();}
	void dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, QWidget *parent) {}
};




#endif


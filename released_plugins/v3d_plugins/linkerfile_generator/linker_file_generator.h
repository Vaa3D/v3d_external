// generate an linker file (.ano) for all swc, apo, etc files in the given directory
// by Lei Qu
// 2009-12-30
//last change: by Hanchuan Peng. 2010-04-21

#ifndef __PLUGIN_ANOGENERATOR_H__
#define __PLUGIN_ANOGENERATOR_H__

#include "v3d_interface.h"

class LinkerPlugin : public QObject, public V3DPluginInterface
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


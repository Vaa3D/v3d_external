
//by Hanchuan Peng
//2010-May-30

#ifndef __EDGE_OF_MASKIMG_PLUGIN_H__
#define __EDGE_OF_MASKIMG_PLUGIN_H__


#include <v3d_interface.h>

class EdgeOfMaskImgPlugin : public QObject, public V3DPluginInterface
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

//plugin_image_registration.h
//by Lei Qu
//2011-04-08

#ifndef __PLUGIN_IMAGE_REGISTRATION_H__
#define __PLUGIN_IMAGE_REGISTRATION_H__

#include <v3d_interface.h>


class ImageRegistrationPlugin : public QObject, public V3DPluginInterface
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

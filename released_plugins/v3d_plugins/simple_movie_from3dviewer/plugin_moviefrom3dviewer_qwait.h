//plugin_moviefrom3dviewer.cpp
//by Lei Qu
//2010-11-08


#ifndef __PLUGIN_MOVIEWFROM3DVIEWER_H__
#define __PLUGIN_MOVIEWFROM3DVIEWER_H__

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>

#include "v3d_interface.h"

class MovieFrom3DviewerPlugin : public QObject, public V3DPluginInterface2
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2);

public:
	QStringList menulist() const;
	void domenu(const QString & menu_name, V3DPluginCallback2 & v3d,  QWidget * parent);

	QStringList funclist() const {return QStringList();}
	bool dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
				 V3DPluginCallback2 & v3d,  QWidget * parent) {return true;}

};

class lookPanel : public QDialog
{

	Q_OBJECT

public:
	lookPanel(V3DPluginCallback2 &_v3d, QWidget *parent);
	~lookPanel();

public:
	bool b_stop;
	long l_frameind;
	QLineEdit *m_pLineEdit_filepath;
	QLineEdit *m_pLineEdit_fps;
	V3DPluginCallback2 &v3d;
	static lookPanel* panel;

private slots:
	void _slot_start();
	void _slot_end();


};

#endif


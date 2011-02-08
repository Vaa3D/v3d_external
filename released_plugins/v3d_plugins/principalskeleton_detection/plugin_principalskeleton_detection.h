//principal skeleton detection plugin
//by Lei Qu
//2009-12-29

#ifndef __PLUGIN_PRINCIPALSKELETON_DETECTION_H__
#define __PLUGIN_PRINCIPALSKELETON_DETECTION_H__

#include <v3d_interface.h>
#include <QDialog>

class QLabel;
class QLineEdit;
class QCheckBox;

//------------------------------------------------------------------------------------------------------------------------------------
class PrincipalSkeletonDetectionPlugin : public QObject, public V3DPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface);

public:
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent);

	QStringList funclist() const {return QStringList();}
	void dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, QWidget *parent) {}
};

//------------------------------------------------------------------------------------------------------------------------------------
class Choose2FileDialog_marker : public QDialog
{
	Q_OBJECT

public:
	Choose2FileDialog_marker(QWidget *parent);
	QString getFilename_1();
	QString getFilename_2();

private slots:
	void openFileDialog_1();
	void openFileDialog_2();

public:
	QLineEdit *filePathLineEdit_1;
	QLineEdit *filePathLineEdit_2;

	QLabel *refChannelLabel;
	QLabel *stopThreshLabel;
	QLabel *foregroundRatioLabel;
	QLineEdit *refChannelLineEdit;
	QLineEdit *stopThreshLineEdit;
	QLineEdit *foregroundRatioLineEdit;
};

#endif

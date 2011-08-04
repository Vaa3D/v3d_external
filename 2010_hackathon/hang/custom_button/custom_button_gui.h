#ifndef __CUSTOM_BUTTON_H__
#define __CUSTOM_BUTTON_H__

#include <QtGui>
#include <v3d_interface.h>

class EmptyClass{};

typedef void (EmptyClass::*VoidFunc)();
typedef void (TriviewControl::*TriviewFunc)();
typedef void (View3DControl::*View3DFunc)();
typedef void (V3DPluginInterface2_1::*PluginMenuFunc)(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

void getAllFiles(QString dirname, QStringList & fileList);
void getObjectList(QStringList & fileList,QList<QObject*> &objectList);
QString v3d_getInterfaceName(QObject *plugin);
QStringList v3d_getInterfaceMenuList(QObject *plugin);
QStringList v3d_getInterfaceFuncList(QObject *plugin);

class CustomButton : public QObject
{
	Q_OBJECT
public:
	QAction * button;
	QObject * slot_class;
	VoidFunc slot_func;
	int bt;

	QString menu_name;
	V3DPluginCallback2 * callback;
	QWidget * parent;

public:
	CustomButton(QIcon * icon, const QString &text, QObject* parent)
	{
		if(!icon) button = new QAction(text, parent);
		else button = new QAction(*icon, text, parent);
		slot_class = 0;
		connect(button, SIGNAL(triggered(bool)), this, SLOT(run()));
	}
	~CustomButton()
	{
		delete button;
		button = 0;
		slot_class = 0;
	}

public slots: 
	void setButtonText(const QString & text)
	{
		if(button && button->isVisible()) button->setText(text);
	}

	bool run();
};

class CustomButtonToolBar;

class CustomButtonSelectWidget : public QWidget
{
	Q_OBJECT

	public:
		CustomButtonSelectWidget(V3DPluginCallback2 &callback, QWidget * parent, CustomButtonToolBar * _toolBar);

		~CustomButtonSelectWidget(){}

		int isIn(QCheckBox * checkbox, QList<QCheckBox *> & checkboxList);

		QAction * getButton(QCheckBox* checkbox);

		public slots:
			void setToolBarButton(bool state);

	public:
		CustomButtonToolBar * toolBar;

		QTabWidget * tabWidget;
		QWidget * pageTriView;
		QWidget * page3dView;
		QWidget * pagePlugin;

		QHBoxLayout * mainLayout;
		QGridLayout * pageTriViewLayout;
		QVBoxLayout * pagePluginLayout;

		QTreeWidget * triViewTreeWidget;
		QList<QLineEdit *> triViewEditorList;
		QList<QCheckBox *> triViewCheckboxList;
		QList<CustomButton *> triViewCustomButtonList;

		QTreeWidget * view3dTreeWidget;
		QList<QLineEdit *> view3dEditorList;
		QList<QCheckBox *> view3dCheckboxList;
		QList<CustomButton *> view3dCustomButtonList;

		QTreeWidget * pluginTreeWidget;
		QList<QLineEdit *> pluginEditorList;
		QList<QCheckBox *> pluginCheckboxList;
		QList<CustomButton *> pluginCustomButtonList;

		QIcon pluginIcon;
		QIcon interfaceIcon;
		QIcon menuIcon;
		QIcon funcIcon;
};

class CustomButtonToolBar : public QToolBar
{
	Q_OBJECT

	public:
		CustomButtonToolBar(V3DPluginCallback2 &callback, QWidget * parent);
		~CustomButtonToolBar();

		void loadSetting();
		void saveSetting();

		public slots:
			void openSelectWidget();

	protected:
		void closeEvent(QCloseEvent *event);
	public:
		QIcon icon;
		CustomButtonSelectWidget * selectWidget;
};

#endif

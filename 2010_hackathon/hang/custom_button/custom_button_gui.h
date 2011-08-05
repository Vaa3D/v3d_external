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
		QString plugin_path;

		int buttonIndex;

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

class CustomButtonSetting
{
	public:
		QToolBar * toolBar;
		QString toolBarTitle;
		Qt::ToolBarArea position;
		QStringList preLoadPluginPathList;
		QStringList preLoadPluginLabelList;

		QList<CustomButton*> activeTriViewButtonList;
		QList<CustomButton*> activeView3dButtonList;
		QList<CustomButton*> activePluginButtonList;
	public:
		CustomButtonSetting(QString title = "Custom Toolbar")
		{
			toolBarTitle = title;
			toolBar = new QToolBar(toolBarTitle);
			position = Qt::LeftToolBarArea;
		}
		CustomButtonSetting(QToolBar* _toolBar)
		{
			toolBar = _toolBar;
			toolBarTitle = toolBar->windowTitle();
			position = Qt::LeftToolBarArea;
		}
		~CustomButtonSetting()
		{
		}
};

bool loadToolBarSettings();
bool saveToolBarSettings();
QList<CustomButtonSetting*>& getToolBarSettingList();
void setToolBarSettingList(QList<CustomButtonSetting*> & _settingList);

class CustomButtonSelectWidget : public QWidget
{
	Q_OBJECT

	public:
		CustomButtonSelectWidget(V3DPluginCallback2 &callback, QWidget * parent, CustomButtonSetting * _cbs);

		~CustomButtonSelectWidget();

		CustomButton * getButton(QCheckBox* checkbox);
		CustomButton * getButton(QAction* action);

		public slots:
			void setToolBarButton(bool state);
			void saveToolBarState();
		void openMe();
	protected:
		void closeEvent(QCloseEvent *event);

	public:
		CustomButtonSetting* cbs;
		QToolBar * toolBar;

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
		QList<QPluginLoader *> pluginLoaderList;

		QIcon pluginIcon;
		QIcon interfaceIcon;
		QIcon menuIcon;
		QIcon funcIcon;

		QIcon toolButtonIcon;
};

#endif

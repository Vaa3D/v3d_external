#ifndef __CUSTOM_BUTTON_H__
#define __CUSTOM_BUTTON_H__

#include <QtGui>
#include <v3d_interface.h>

class EmptyClass{};

typedef void (EmptyClass::*VoidFunc)();
typedef void (TriviewControl::*TriviewFunc)();
typedef void (View3DControl::*View3DFunc)();
typedef void (V3DPluginCallback2::*Callback2Func)(v3dhandle);
typedef void (V3DPluginInterface2_1::*PluginMenuFunc)(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

void getAllFiles(QString dirname, QStringList & fileList);
void getObjectList(QStringList & fileList,QList<QObject*> &objectList);
QString v3d_getInterfaceName(QObject *plugin);
QStringList v3d_getInterfaceMenuList(QObject *plugin);
QStringList v3d_getInterfaceFuncList(QObject *plugin);

QStringList getTriViewButtonStringList();
QList<VoidFunc> getTriViewButtonFuncList();

QStringList getView3dButtonStringList();
QList<VoidFunc> getView3dButtonFuncList();

class CustomToolButton : public QObject
{
	Q_OBJECT
	public:
		QAction * button;

		void * slot_class;
		VoidFunc slot_func;
		int bt;

		QString menu_name;
		V3DPluginCallback2 * callback;
		QWidget * parent;
		QString plugin_path;
		QString buttonName;

	public:
		CustomToolButton(QIcon * icon, const QString &text, QObject* parent)
		{
			if(!icon) button = new QAction(text, parent);
			else button = new QAction(*icon, text, parent);
			slot_class = 0;
			connect(button, SIGNAL(triggered(bool)), this, SLOT(run()));
		}
		~CustomToolButton()
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

class CustomToolbarSetting
{
	public:
		QToolBar * toolBar;
		QString toolBarTitle;
		Qt::ToolBarArea position;

		QStringList preLoadTriViewButtonNameList;
		QStringList preLoadTriViewButtonAliasList;

		QStringList preLoadView3dButtonNameList;
		QStringList preLoadView3dButtonAliasList;

		QStringList preLoadPluginPathList;
		QStringList preLoadPluginLabelList;

		QList<CustomToolButton*> activeTriViewButtonList;
		QList<CustomToolButton*> activeView3dButtonList;
		QList<CustomToolButton*> activePluginButtonList;
	public:
		CustomToolbarSetting(QString title)
		{
			toolBar = 0; //new QToolBar(toolBarTitle);
			toolBarTitle = title;
			position = Qt::TopToolBarArea;
		}
		CustomToolbarSetting(QToolBar* _toolBar)
		{
			toolBar = _toolBar;
			toolBarTitle = toolBar->windowTitle();
			position = Qt::TopToolBarArea;
		}
		~CustomToolbarSetting()
		{
		}
};

bool loadToolBarSettings();
bool saveToolBarSettings();
QList<CustomToolbarSetting*>& getToolBarSettingList();
void setToolBarSettingList(QList<CustomToolbarSetting*> & _settingList);

class CustomToolbarSelectWidget : public QWidget
{
	Q_OBJECT

	public:
		CustomToolbarSelectWidget(CustomToolbarSetting * _cts, V3DPluginCallback2 *callback, QWidget * parent);

		~CustomToolbarSelectWidget();

		CustomToolButton * getButton(QCheckBox* checkbox);
		CustomToolButton * getButton(QAction* action);

	public slots:
		void setToolBarButton(bool state);
		void saveToolBarState();
		void openMe();
	protected:
		void closeEvent(QCloseEvent *event);

	public:
		CustomToolbarSetting* cts;
		QToolBar * toolBar;

		QTabWidget * tabWidget;
		QWidget * pageTriView;
		QWidget * pageView3d;
		QWidget * pagePlugin;

		QHBoxLayout * mainLayout;
		QVBoxLayout * pageTriViewLayout;
		QVBoxLayout * pageView3dLayout;
		QVBoxLayout * pagePluginLayout;

		QTreeWidget * triViewTreeWidget;
		QList<QLineEdit *> triViewEditorList;
		QList<QCheckBox *> triViewCheckboxList;
		QList<CustomToolButton *> triViewCustomToolButtonList;

		QTreeWidget * view3dTreeWidget;
		QList<QLineEdit *> view3dEditorList;
		QList<QCheckBox *> view3dCheckboxList;
		QList<CustomToolButton *> view3dCustomToolButtonList;

		QTreeWidget * pluginTreeWidget;
		QList<QLineEdit *> pluginEditorList;
		QList<QCheckBox *> pluginCheckboxList;
		QList<CustomToolButton *> pluginCustomToolButtonList;
		QList<QPluginLoader *> pluginLoaderList;

		QIcon pluginIcon;
		QIcon interfaceIcon;
		QIcon menuIcon;
		QIcon funcIcon;

		QIcon toolButtonIcon;
};

class CustomToolbar : public QToolBar
{
public:
	CustomToolbar(QString title , V3DPluginCallback2 * callback, QWidget * parent);
	CustomToolbar(CustomToolbarSetting * _cts , V3DPluginCallback2 * callback, QWidget * parent);
	~CustomToolbar();

	bool showToMainWindow();
public:
	CustomToolbarSetting* cts;
	CustomToolbarSelectWidget* selectWidget;
};

#endif

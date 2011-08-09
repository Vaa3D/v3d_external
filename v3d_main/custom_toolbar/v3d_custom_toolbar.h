#ifndef __CUSTOM_BUTTON_H__
#define __CUSTOM_BUTTON_H__

//#define  __v3d_custom_toolbar_plugin_

#include <QtGui>
#include <map>

#ifdef __v3d_custom_toolbar_plugin_
	#include <v3d_interface.h>
#else
	#include "../v3d/mainwindow.h"
	#include "../basic_c_fun/v3d_interface.h"
#endif

class EmptyClass{};

typedef void (EmptyClass::*VoidFunc)();
#ifndef __v3d_custom_toolbar_plugin_
typedef void (MainWindow::*MainWindowFunc)();
#endif
typedef void (TriviewControl::*TriviewFunc)();
typedef void (View3DControl::*View3DFunc)();
typedef void (V3DPluginCallback2::*Callback2Func)(v3dhandle);
typedef void (V3DPluginInterface2_1::*PluginMenuFunc)(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

void getAllFiles(QString dirname, QStringList & fileList);
void getObjectList(QStringList & fileList,QList<QObject*> &objectList);

bool setPluginRootPath(QString);
bool setPluginRootPathAutomaticly();
bool setToolbarSettingFilePath(QString);
bool setToolbarSettingFilePathAutomaticly();

QList<std::pair<QString, VoidFunc> > getMainWindowMenuStringAndFuncList();

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
		int bt;  			// 0 mainwindow menu, 1 triview button, 2 view3d button, 3 plugin menu

		V3DPluginCallback2 * callback;
		QWidget * parent;   // parent should be v3d MainWindow type, when invoke QMainWindow menu action
		QString plugin_path;

		QString menu_name;  // the original menu name of plugin menu (eg. lobeseg) and full mainwindow menu (eg. File::Open)
		QString buttonName; // the original button name of triview and view3d button

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
		QString toolBarTitle;                        // preloaded toolbar title
		Qt::ToolBarArea position;                    // preloaded toolbar position

		QStringList preLoadMainWindowMenuNameList;
		QStringList preLoadMainWindowMenuAliasList;

		QStringList preLoadTriViewButtonNameList;
		QStringList preLoadTriViewButtonAliasList;

		QStringList preLoadView3dButtonNameList;
		QStringList preLoadView3dButtonAliasList;

		QStringList preLoadPluginMenuPathList;      // Formatted as : PluginPath::MenuName
		QStringList preLoadPluginMenuAliasList;

		QList<CustomToolButton*> activeMainWindowMenuList;
		QList<CustomToolButton*> activeTriViewButtonList;
		QList<CustomToolButton*> activeView3dButtonList;
		QList<CustomToolButton*> activePluginButtonList;

	public:
		CustomToolbarSetting(QString title)
		{
			toolBar = 0; 
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

		CustomToolButton * getButtonFromCheckbox(QCheckBox* checkbox);
		CustomToolButton * getButtonFromAction(QAction* action);

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
		QWidget * pageMainWindow;
		QWidget * pageTriView;
		QWidget * pageView3d;
		QWidget * pagePlugin;

		QHBoxLayout * mainLayout;
		QVBoxLayout * pageMainWindowLayout;
		QVBoxLayout * pageTriViewLayout;
		QVBoxLayout * pageView3dLayout;
		QVBoxLayout * pagePluginLayout;

		QTreeWidget * mainWindowTreeWidget;
		QList<QLineEdit *> mainWindowEditorList;
		QList<QCheckBox *> mainWindowCheckboxList;
		QList<CustomToolButton *> mainWindowCustomToolButtonList;

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

	bool showToMainWindow(QMainWindow* _mw = 0);
public:
	CustomToolbarSetting* cts;
	CustomToolbarSelectWidget* selectWidget;
};

#endif

#include <iostream>
#include <fstream>
#include "v3d_custom_toolbar.h"

using namespace std;

static QString pluginRootPath = QObject::tr("/Users/xiaoh10/Applications/v3d/plugins");
static QString settingFilePath = QObject::tr("/Users/xiaoh10/.v3d_toolbox_layout");
static QList<CustomToolbarSetting*> settingList;

static Qt::ToolBarArea getToolBarArea(QToolBar* toolBar);

bool setPluginRootPath(QString plugin_path){pluginRootPath = plugin_path;}
bool setPluginRootPathAutomaticly()
{
	QDir testPluginsDir = QDir(qApp->applicationDirPath());
#if defined(Q_OS_WIN)
	if (testPluginsDir.dirName().toLower() == "debug" || testPluginsDir.dirName().toLower() == "release")
		testPluginsDir.cdUp();
#elif defined(Q_OS_MAC)
	// In a Mac app bundle, plugins directory could be either
	//  a - below the actual executable i.e. v3d.app/Contents/MacOS/plugins/
	//  b - parallel to v3d.app i.e. foo/v3d.app and foo/plugins/
	if (testPluginsDir.dirName() == "MacOS") {
		QDir testUpperPluginsDir = testPluginsDir;
		testUpperPluginsDir.cdUp();
		testUpperPluginsDir.cdUp();
		testUpperPluginsDir.cdUp(); // like foo/plugins next to foo/v3d.app
		if (testUpperPluginsDir.cd("plugins")) testPluginsDir = testUpperPluginsDir;
	}
#endif
	pluginRootPath=testPluginsDir.absolutePath();
	qDebug()<<"plugin path : "<<testPluginsDir.absolutePath();
}
bool setToolbarSettingFilePath(QString file_path){settingFilePath = file_path;}
bool setToolbarSettingFilePathAutomaticly()
{
	settingFilePath = qApp->applicationDirPath() + QObject::tr("/.v3d_toolbar");
}

void getAllFiles(QString dirname, QStringList & fileList)
{
	QDir dir(dirname);
	QStringList dirlist = dir.entryList(QDir::Dirs);
	if(dirlist.size() == 2) 
	{
		QStringList files = dir.entryList(QDir::Files);
		QStringList::iterator fit = files.begin();
		while(fit != files.end())
		{
			fileList.append(dir.absoluteFilePath(*fit));
			fit++;
		}
		return;
	}

	for(QStringList::iterator it = dirlist.begin(); it != dirlist.end(); it++)
	{
		//cout<<(*it).toStdString().c_str()<<endl;
		if(((*it) == ".") || ((*it) == "..")) continue;
		getAllFiles(dir.absoluteFilePath(*it), fileList);
	}
}

void getObjectList(QStringList & fileList,QList<QObject*> &objectList)
{
	if(fileList.empty()){objectList.empty(); return;}

	QStringList::iterator fit = fileList.begin();
	while(fit != fileList.end())
	{
		QPluginLoader loader(*fit);
		QObject * plugin = loader.instance();
		objectList.push_back(plugin);
		fit++;
	}
}

static QString v3d_getInterfaceName(QObject *plugin)
{
	QString name;

	// Derived class must appear first, to be selected
	V3DSingleImageInterface2_1 *iFilter2_1 = qobject_cast<V3DSingleImageInterface2_1 *>(plugin);
	if (iFilter2_1 )  return (name = "V3DSingleImageInterface/2.1");

	// Base class must appear later, so derived class has a chance
	V3DSingleImageInterface *iFilter = qobject_cast<V3DSingleImageInterface *>(plugin);
	if (iFilter )  return (name = "V3DSingleImageInterface/1.0");

	V3DPluginInterface2_1 *iface2_1 = qobject_cast<V3DPluginInterface2_1 *>(plugin);
	if (iface2_1 )  return (name = "V3DPluginInterface/2.1");

	V3DPluginInterface2 *iface2 = qobject_cast<V3DPluginInterface2 *>(plugin);
	if (iface2 )  return (name = "V3DPluginInterface/2.0");

	V3DPluginInterface *iface = qobject_cast<V3DPluginInterface *>(plugin);
	if (iface )  return (name = "V3DPluginInterface/1.1");

	return name;
}

static QStringList v3d_getInterfaceMenuList(QObject *plugin)
{
	QStringList qslist;

	V3DSingleImageInterface2_1 *iFilter2_1 = qobject_cast<V3DSingleImageInterface2_1 *>(plugin);
	if (iFilter2_1 )  return (qslist = iFilter2_1->menulist());

	V3DSingleImageInterface *iFilter = qobject_cast<V3DSingleImageInterface *>(plugin);
	if (iFilter )  return (qslist = iFilter->menulist());

	V3DPluginInterface *iface = qobject_cast<V3DPluginInterface *>(plugin);
	if (iface )  return (qslist = iface->menulist());

	V3DPluginInterface2 *iface2 = qobject_cast<V3DPluginInterface2 *>(plugin);
	if (iface2 )  return (qslist = iface2->menulist());

	V3DPluginInterface2_1 *iface2_1 = qobject_cast<V3DPluginInterface2_1 *>(plugin);
	if (iface2_1 )  return (qslist = iface2_1->menulist());

	return qslist;
}

static QStringList v3d_getInterfaceFuncList(QObject *plugin)
{
	QStringList qslist;

	V3DPluginInterface *iface = qobject_cast<V3DPluginInterface *>(plugin);
	if (iface )  return (qslist = iface->funclist());

	V3DPluginInterface2 *iface2 = qobject_cast<V3DPluginInterface2 *>(plugin);
	if (iface2 )  return (qslist = iface2->funclist());

	V3DPluginInterface2_1 *iface2_1 = qobject_cast<V3DPluginInterface2_1 *>(plugin);
	if (iface2_1 )  return (qslist = iface2_1->funclist());

	return qslist;
}


CustomToolbarSelectWidget::CustomToolbarSelectWidget(CustomToolbarSetting* _cts, V3DPluginCallback2 *callback, QWidget * parent): QWidget(parent)
{
	if(!_cts) return;

	cts = _cts;

	toolBar = cts->toolBar;

	connect(toolBar, SIGNAL(visibilityChanged(bool)), this, SLOT(saveToolBarState()));

	toolButtonIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirHomeIcon));
	toolBar->addAction(toolButtonIcon, tr("Add custom button"),this, SLOT(openMe()));
	toolBar->addSeparator();

	pluginIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirHomeIcon));
	interfaceIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirOpenIcon),
			QIcon::Normal, QIcon::On);
	interfaceIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirClosedIcon),
			QIcon::Normal, QIcon::Off);
	menuIcon.addPixmap(style()->standardPixmap(QStyle::SP_FileIcon));
	funcIcon.addPixmap(style()->standardPixmap(QStyle::SP_MessageBoxInformation));

	tabWidget = new QTabWidget();
	// page tri view
	pageTriView = new QWidget();

	triViewTreeWidget = new QTreeWidget();
	triViewTreeWidget->setColumnCount(2);
	triViewTreeWidget->setSortingEnabled(true);

	QStringList triViewHeaderStringList = QStringList() <<tr("Button Name")<<tr("Alias");
	QTreeWidgetItem * triViewHeaderItem = new QTreeWidgetItem(triViewHeaderStringList);
	triViewTreeWidget->setHeaderItem(triViewHeaderItem);

	QStringList triViewButtonStringList = getTriViewButtonStringList();
	QList<VoidFunc> triViewButtonFuncList = getTriViewButtonFuncList();
	int ii = 0;
	foreach(QString buttonName, triViewButtonStringList)
	{
		QTreeWidgetItem * triViewItem = new QTreeWidgetItem(triViewTreeWidget);

		QCheckBox * checkbox = new QCheckBox(buttonName);
		checkbox->setChecked(false);
		triViewTreeWidget->setItemWidget(triViewItem, 0, checkbox);
		triViewCheckboxList.push_back(checkbox);

		QLineEdit * editor = new QLineEdit(buttonName);
		triViewTreeWidget->setItemWidget(triViewItem, 1, editor);
		editor->setDisabled(true);
		triViewEditorList.push_back(editor);

		CustomToolButton * qb = new CustomToolButton(0, editor->text(), toolBar);
		qb->button->setVisible(false);
		qb->slot_func = triViewButtonFuncList.at(ii);
		qb->bt = 0;
		qb->buttonName = buttonName;
		qb->callback = callback;
		qb->parent = parent;
		triViewCustomToolButtonList.push_back(qb);

		connect(checkbox, SIGNAL(clicked(bool)), editor, SLOT(setEnabled(bool)));
		connect(checkbox, SIGNAL(clicked(bool)), this, SLOT(setToolBarButton(bool)));
		connect(editor, SIGNAL(textChanged(const QString &)), qb, SLOT(setButtonText(const QString &)));
		if(cts->preLoadTriViewButtonNameList.contains(buttonName))
		{
			int i = cts->preLoadTriViewButtonNameList.indexOf(buttonName);
			editor->setText(cts->preLoadTriViewButtonAliasList.at(i));
			editor->setEnabled(true);
			checkbox->setChecked(Qt::Checked);

			qb->button->setVisible(true);
			toolBar->addAction(qb->button);
			cts->activeTriViewButtonList.push_back(qb);
		}
		ii++;
	}

	pageTriViewLayout = new QVBoxLayout();
	pageTriViewLayout->addWidget(triViewTreeWidget);
	pageTriView->setLayout(pageTriViewLayout);

	// page view 3d
	pageView3d = new QWidget();

	view3dTreeWidget = new QTreeWidget();
	view3dTreeWidget->setColumnCount(2);
	view3dTreeWidget->setSortingEnabled(true);

	QStringList view3dHeaderStringList = QStringList() <<tr("Button Name")<<tr("Alias");
	QTreeWidgetItem * view3dHeaderItem = new QTreeWidgetItem(view3dHeaderStringList);
	view3dTreeWidget->setHeaderItem(view3dHeaderItem);

	QStringList view3dButtonStringList = getView3dButtonStringList();
	QList<VoidFunc> view3dButtonFuncList = getView3dButtonFuncList();
	int jj = 0;
	foreach(QString buttonName, view3dButtonStringList)
	{
		QTreeWidgetItem * view3dItem = new QTreeWidgetItem(view3dTreeWidget);

		QCheckBox * checkbox = new QCheckBox(buttonName);
		checkbox->setChecked(false);
		view3dTreeWidget->setItemWidget(view3dItem, 0, checkbox);
		view3dCheckboxList.push_back(checkbox);

		QLineEdit * editor = new QLineEdit(buttonName);
		view3dTreeWidget->setItemWidget(view3dItem, 1, editor);
		editor->setDisabled(true);
		view3dEditorList.push_back(editor);

		CustomToolButton * qb = new CustomToolButton(0, editor->text(), toolBar);
		qb->button->setVisible(false);
		qb->slot_func = view3dButtonFuncList.at(jj);
		qb->bt = 1;
		qb->buttonName = buttonName;
		qb->callback = callback;
		qb->parent = parent;
		view3dCustomToolButtonList.push_back(qb);

		connect(checkbox, SIGNAL(clicked(bool)), editor, SLOT(setEnabled(bool)));
		connect(checkbox, SIGNAL(clicked(bool)), this, SLOT(setToolBarButton(bool)));
		connect(editor, SIGNAL(textChanged(const QString &)), qb, SLOT(setButtonText(const QString &)));
		if(cts->preLoadView3dButtonNameList.contains(buttonName))
		{
			int i = cts->preLoadView3dButtonNameList.indexOf(buttonName);
			editor->setText(cts->preLoadView3dButtonAliasList.at(i));
			editor->setEnabled(true);
			checkbox->setChecked(Qt::Checked);

			qb->button->setVisible(true);
			toolBar->addAction(qb->button);
			cts->activeView3dButtonList.push_back(qb);
		}
		jj++;
	}

	pageView3dLayout = new QVBoxLayout();
	pageView3dLayout->addWidget(view3dTreeWidget);
	pageView3d->setLayout(pageView3dLayout);


	// page plugin
	pagePlugin = new QWidget();

	pluginTreeWidget = new QTreeWidget();
	pluginTreeWidget->setColumnCount(2);
	pluginTreeWidget->setSortingEnabled(true);

	QStringList pluginHeaderStringList = QStringList() <<tr("Plugin Name")<<tr("Set Button Name");
	QTreeWidgetItem * pluginHeaderItem = new QTreeWidgetItem(pluginHeaderStringList);
	pluginTreeWidget->setHeaderItem(pluginHeaderItem);

	pagePluginLayout = new QVBoxLayout();

	QStringList fileList; getAllFiles(pluginRootPath, fileList);

	foreach(QString file, fileList)
	{
		QPluginLoader* loader = new QPluginLoader(file);
		pluginLoaderList.push_back(loader);

		QObject * plugin = loader->instance();

		if(plugin)
		{
			QTreeWidgetItem *pluginItem = new QTreeWidgetItem(pluginTreeWidget);
			QString pluginPath = file;
			pluginItem->setText(0, pluginPath.replace(0,pluginRootPath.size(),tr("")));
			pluginItem->setIcon(0, pluginIcon);

			QStringList menulist = v3d_getInterfaceMenuList(plugin);
			foreach(QString menu_name, menulist)
			{
				QTreeWidgetItem * menuItem = new QTreeWidgetItem(pluginItem);

				QCheckBox * checkbox = new QCheckBox(menu_name);
				checkbox->setChecked(false);
				pluginTreeWidget->setItemWidget(menuItem, 0, checkbox);
				pluginCheckboxList.push_back(checkbox);

				QLineEdit * editor = new QLineEdit(menu_name);
				pluginTreeWidget->setItemWidget(menuItem, 1, editor);
				editor->setDisabled(true);
				pluginEditorList.push_back(editor);

				CustomToolButton * qb = new CustomToolButton(0, editor->text(), toolBar);
				qb->button->setVisible(false);
				qb->slot_class = plugin;
				qb->bt = 2;
				qb->menu_name = menu_name;
				qb->callback = callback;
				qb->parent = parent;
				qb->plugin_path = file;

				pluginCustomToolButtonList.push_back(qb);

				connect(checkbox, SIGNAL(clicked(bool)), editor, SLOT(setEnabled(bool)));
				connect(checkbox, SIGNAL(clicked(bool)), this, SLOT(setToolBarButton(bool)));
				connect(editor, SIGNAL(textChanged(const QString &)), qb, SLOT(setButtonText(const QString &)));
				if(cts->preLoadPluginPathList.contains(file + "::" + menu_name))
				{
					int i = cts->preLoadPluginPathList.indexOf(file + "::" + menu_name);
					editor->setText(cts->preLoadPluginAliasList.at(i));
					editor->setEnabled(true);
					checkbox->setChecked(Qt::Checked);
					pluginItem->setExpanded(true);

					qb->button->setVisible(true);
					toolBar->addAction(qb->button);
					cts->activePluginButtonList.push_back(qb);
				}
			}
		}
	}

	pluginTreeWidget->resizeColumnToContents(0);
	pagePluginLayout->addWidget(pluginTreeWidget);

	pagePlugin->setLayout(pagePluginLayout);

	tabWidget->addTab(pageTriView, tr("Tri View"));
	tabWidget->addTab(pageView3d, tr("3D View"));
	tabWidget->addTab(pagePlugin, tr("Plugin"));
	tabWidget->setCurrentIndex(2);

	mainLayout = new QHBoxLayout();
	mainLayout->addWidget(tabWidget);

	setLayout(mainLayout);
	setMaximumSize(1000,800);
	setGeometry(400,400,1000, 800);
}

CustomToolbarSelectWidget::~CustomToolbarSelectWidget()
{
	if(!pluginLoaderList.empty())
	{
		foreach(QPluginLoader * loader, pluginLoaderList)
		{
			loader->unload();
		}
	}

	delete cts;
}

CustomToolButton * CustomToolbarSelectWidget::getButtonFromCheckbox(QCheckBox* checkbox)
{
	int i = -1;
	if((i = triViewCheckboxList.indexOf(checkbox))!= -1)
	{
		return triViewCustomToolButtonList.at(i);
	}
	else if((i = view3dCheckboxList.indexOf(checkbox))!= -1)
	{
		return view3dCustomToolButtonList.at(i);
	}
	else if((i = pluginCheckboxList.indexOf(checkbox))!= -1)
	{
		return pluginCustomToolButtonList.at(i);
	}
	return 0;
}

CustomToolButton * CustomToolbarSelectWidget::getButtonFromAction(QAction* action)
{
	int i = -1;
	return 0;
}

void CustomToolbarSelectWidget::setToolBarButton(bool state)
{
	QCheckBox * checkbox = dynamic_cast<QCheckBox*>(sender());
	CustomToolButton* cb = getButtonFromCheckbox(checkbox);
	if(cb==0) QMessageBox::information(0,"","0");
	if(state && !cb->button->isVisible())
	{
		cb->button->setVisible(true);
		toolBar->addAction(cb->button);
		if(cb->bt == 0)cts->activeTriViewButtonList.push_back(cb);
		if(cb->bt == 1)cts->activeView3dButtonList.push_back(cb);
		if(cb->bt == 2)cts->activePluginButtonList.push_back(cb);
	}
	else if(!state && cb->button->isVisible()) 
	{
		cb->button->setVisible(false);
		int i = -1;
		if((i = cts->activePluginButtonList.indexOf(cb)) != -1)
			cts->activePluginButtonList.removeAt(i);
		else if((i = cts->activeTriViewButtonList.indexOf(cb)) != -1)
			cts->activeTriViewButtonList.removeAt(i);
		else if((i = cts->activeView3dButtonList.indexOf(cb)) != -1)
			cts->activeView3dButtonList.removeAt(i);
	}
	saveToolBarState();
}

void CustomToolbarSelectWidget::openMe()
{
	if(isVisible()) 
		setHidden(true);
	else
		show();
}

void CustomToolbarSelectWidget::closeEvent(QCloseEvent *event)
{
	if(toolBar && !toolBar->isVisible()) toolBar->show();
}
void CustomToolbarSelectWidget::saveToolBarState()
{
	saveToolBarSettings();
}

bool CustomToolButton::run()
{
	if(bt == 0) // Triview
	{
		v3dhandleList winlist = callback->getImageWindowList();
		if(winlist.empty()) {QMessageBox::information(0,"","No image stack!"); return true;}
		v3dhandle handle = callback->currentImageWindow();
		if(handle)
		{
			//slot_class =(void*) callback;//(void*)(callback->getTriviewControl(handle));
			(callback->*(Callback2Func)slot_func)(handle);
			//callback->open3DWindow(handle);
		}
		return true;
	}
	else if(bt == 1) // View3d
	{
		v3dhandleList winlist = callback->getImageWindowList();
		if(winlist.empty()) {QMessageBox::information(0,"","No image stack!"); return true;}
		v3dhandle handle = callback->currentImageWindow();
		if(handle)
		{
			callback->open3DWindow(handle);
			View3DControl * view3d = callback->getView3DControl(handle);
			(view3d->*(View3DFunc)slot_func)();
		}
		return true;
	}
	else if(bt == 2) // Plugin do menu
	{
		QObject * plugin = (QObject*) slot_class;
		V3DSingleImageInterface2_1 *iFilter2_1 = qobject_cast<V3DSingleImageInterface2_1 *>(plugin);
		if (iFilter2_1 )
		{
		}

		V3DSingleImageInterface *iFilter = qobject_cast<V3DSingleImageInterface *>(plugin);
		if (iFilter )
		{
		}

		V3DPluginInterface2_1 *iface2_1 = qobject_cast<V3DPluginInterface2_1 *>(plugin);
		if (iface2_1 )
		{
			iface2_1->domenu(menu_name, *callback, parent);
		}

		V3DPluginInterface2 *iface2 = qobject_cast<V3DPluginInterface2 *>(plugin);
		if (iface2 )
		{
			iface2->domenu(menu_name, *callback,parent);
		}

		V3DPluginInterface *iface = qobject_cast<V3DPluginInterface *>(plugin);
		if (iface )
		{
			iface->domenu(menu_name, *callback, parent);
		}

		return true;
	}
}

bool saveToolBarSettings() 
{
	ofstream ofs(settingFilePath.toStdString().c_str());
	if(ofs.fail())
	{
		ofs.close();
		return false;
	}

	foreach(CustomToolbarSetting* cts, settingList)
	{
		if(cts->toolBar->isVisible())
		{
			ofs<<"ToolBar"<<endl;
			ofs<<"\t"<<cts->toolBar->windowTitle().toStdString()<<endl;
			ofs<<"\t"<<(int)(getToolBarArea(cts->toolBar))<<endl;
			cout<<"triview button list size : "<<cts->activeTriViewButtonList.size()<<endl;
			foreach(CustomToolButton* cb, cts->activeTriViewButtonList)
			{
				cout<<"triview"<<endl;
				ofs<<"TriViewButton"<<endl;
				ofs<<"\t"<<cb->buttonName.toStdString()<<endl;
				ofs<<"\t"<<cb->button->text().toStdString()<<endl;
			}
			cout<<"view3d button list size : "<<cts->activeView3dButtonList.size()<<endl;
			foreach(CustomToolButton* cb, cts->activeView3dButtonList)
			{
				cout<<"view3d"<<endl;
				ofs<<"View3dButton"<<endl;
				ofs<<"\t"<<cb->buttonName.toStdString()<<endl;
				ofs<<"\t"<<cb->button->text().toStdString()<<endl;
			}
			foreach(CustomToolButton* cb, cts->activePluginButtonList)
			{
				ofs<<"PluginButton"<<endl;
				ofs<<"\t"<<cb->plugin_path.toStdString()<<"::"<<cb->menu_name.toStdString()<<endl;
				ofs<<"\t"<<cb->button->text().toStdString()<<endl;
			}
		}
	}
	ofs.close();
	return true;
}

bool loadToolBarSettings()
{
	ifstream ifs(settingFilePath.toStdString().c_str());
	if(ifs.fail())
	{
		ifs.close();
		return false;
	}
	CustomToolbarSetting * cts = 0;
	while(ifs.good())
	{
		string str;
		ifs>>str;
		if(str == "ToolBar")
		{
			char title[1000]; ifs.ignore(1000, '\t'); ifs.getline(title, 1000);
			int position; ifs.ignore(1000,'\t');ifs >> position;
			cout<<"title = \""<<title<<"\""<<endl;
			cout<<"position = "<<position<<endl;

			cts = new CustomToolbarSetting(QString(title));
			cts->position = (Qt::ToolBarArea)position;
			settingList.push_back(cts);
		}
		else if(str == "TriViewButton")
		{
			char name[1000]; ifs.ignore(1000,'\t'); ifs.getline(name, 1000);
			char alias[1000]; ifs.ignore(1000,'\t'); ifs.getline(alias,1000);
			if(cts)
			{
				cts->preLoadTriViewButtonNameList.push_back(QString(name).trimmed());
				cts->preLoadTriViewButtonAliasList.push_back(QString(alias).trimmed());
			}
		}
		else if(str == "View3dButton")
		{
			char name[1000]; ifs.ignore(1000,'\t'); ifs.getline(name, 1000);
			char alias[1000]; ifs.ignore(1000,'\t'); ifs.getline(alias,1000);
			if(cts)
			{
				cts->preLoadView3dButtonNameList.push_back(QString(name).trimmed());
				cts->preLoadView3dButtonAliasList.push_back(QString(alias).trimmed());
			}
		}
		else if(str == "PluginButton")
		{
			char path[1000]; ifs.ignore(1000,'\t'); ifs.getline(path, 1000);
			char label[1000]; ifs.ignore(1000,'\t'); ifs.getline(label,1000);
			cout<<"path = \""<<path<<"\""<<endl;
			cout<<"label = \""<<label<<"\""<<endl;
			if(cts)
			{
				cts->preLoadPluginPathList.push_back(QString(path).trimmed());
				cts->preLoadPluginAliasList.push_back(QString(label).trimmed());
			}
		}
	}
	ifs.close();
	return true;
}
QList<CustomToolbarSetting*>& getToolBarSettingList()
{
	return settingList;
}
void setToolBarSettingList(QList<CustomToolbarSetting*> & _settingList)
{
	settingList = _settingList;
}

QStringList getTriViewButtonStringList()
{
	return QStringList()<<QObject::tr("See in 3D");
}

QList<VoidFunc> getTriViewButtonFuncList()
{
	return QList<VoidFunc>() << (VoidFunc)(&V3DPluginCallback2::open3DWindow);
}

QStringList getView3dButtonStringList()
{
	return QStringList()<<QObject::tr("Sync Tri-view Objs");
}

// refer to 3drenderer/v3dr_control_signal.cpp
QList<VoidFunc> getView3dButtonFuncList()
{
	return QList<VoidFunc>() << (VoidFunc)(&View3DControl::updateWithTriView);
}

CustomToolbar::CustomToolbar(QString title, V3DPluginCallback2 * callback, QWidget * parent) : QToolBar(parent)
{
	setWindowTitle(title);
	cts = new CustomToolbarSetting(this);
	selectWidget = new CustomToolbarSelectWidget(cts, callback, parent);
}

CustomToolbar::CustomToolbar(CustomToolbarSetting* _cts, V3DPluginCallback2 * callback, QWidget * parent) : QToolBar(parent)
{
	cts = _cts;
	setWindowTitle(cts->toolBarTitle);
	cts->toolBar = this;
	selectWidget = new CustomToolbarSelectWidget(cts, callback, parent);
}

CustomToolbar::~CustomToolbar()
{
	if(selectWidget)
	{
		delete selectWidget;
		selectWidget = 0;
	}
}

bool CustomToolbar::showToMainWindow(QMainWindow * _mw)
{
	if(_mw)
	{
		_mw->addToolBar(cts->position, this);
		return true;
	}

	if(this->parent() == 0)
	{
		QWidget * w = QApplication::activeWindow();
		QMainWindow * mw = qobject_cast<QMainWindow*>(w);
		if(mw)
		{
			mw->addToolBar(cts->position, this);
			return true;
		}
		else return false;
	}
	return false;
}

Qt::ToolBarArea getToolBarArea(QToolBar* toolBar)
{
	QWidget * w = QApplication::activeWindow();
	QMainWindow * mw = qobject_cast<QMainWindow*>(w);
	if(mw && toolBar->parent() == mw)
		return mw->toolBarArea(toolBar);
	else return Qt::TopToolBarArea;
}

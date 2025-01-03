#include <iostream>
#include <fstream>
#include "v3d_custom_toolbar.h"

using namespace std;

static QString pluginRootPath = QObject::tr("/Users/xiaoh10/Applications/v3d/plugins");
static QString settingFilePath = QObject::tr("/Users/xiaoh10/.v3d_toolbox_layout");
static QList<CustomToolbarSetting*> settingList;

static Qt::ToolBarArea getToolBarArea(QToolBar* toolBar);

bool setPluginRootPath(QString plugin_path){pluginRootPath = plugin_path; return true;}
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

	return true;
}
bool setToolbarSettingFilePath(QString file_path){settingFilePath = file_path; return true;}
bool setToolbarSettingFilePathAutomaticly()
{
	//settingFilePath = qApp->applicationDirPath() + QObject::tr("/.v3d_toolbar");
	settingFilePath = QDir::homePath() + QObject::tr("/.v3d_toolbar");

	return true;
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
#ifdef __v3d_custom_toolbar_plugin__
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
#endif

#ifdef __hierarchical_file_menu__
QTreeWidgetItem * CustomToolbarSelectWidget::createTreeWidgetItem(QString menuName, QMap<QString, QTreeWidgetItem*> & treeWidgetItemOfMenuName, QTreeWidget * treeWidget)
{
	if(treeWidgetItemOfMenuName.find(menuName) != treeWidgetItemOfMenuName.end())
	{
		qDebug()<<"duplicated menu "<<menuName;
		return treeWidgetItemOfMenuName[menuName];
	}
	int prev_pos = 0, next_pos = -1;
	QTreeWidgetItem * newItem = 0;
	QTreeWidgetItem * parentItem = 0;
	while((next_pos = menuName.indexOf("::", prev_pos)) != -1)
	{
		QString par_menu_name = menuName.mid(0, next_pos);
		if(treeWidgetItemOfMenuName.find(par_menu_name) == treeWidgetItemOfMenuName.end())
		{
			if(parentItem) newItem = new QTreeWidgetItem(parentItem);
			else newItem = new QTreeWidgetItem(treeWidget);
			if(par_menu_name.lastIndexOf("::") != -1)
				newItem->setText(0, par_menu_name.right(par_menu_name.size() - par_menu_name.lastIndexOf("::") - 2));
			else newItem->setText(0, par_menu_name);
			newItem->setIcon(0, fileMenuIcon);
			treeWidgetItemOfMenuName[par_menu_name] = newItem;
			parentItem = newItem;
		}
		else parentItem = treeWidgetItemOfMenuName[par_menu_name];
		prev_pos = next_pos + 2;
	}
	if(parentItem) newItem = new QTreeWidgetItem(parentItem);
	else newItem = new QTreeWidgetItem(treeWidget);
	treeWidgetItemOfMenuName[menuName] = newItem;
	return newItem;
}
#endif

CustomToolbarSelectWidget::CustomToolbarSelectWidget(CustomToolbarSetting* _cts, V3DPluginCallback2 *callback, QWidget * parent): QWidget(0/*parent*/)
{
	if(!_cts) return;

	cts = _cts;

	toolBar = cts->toolBar;

	connect(toolBar, SIGNAL(visibilityChanged(bool)), this, SLOT(saveToolBarState()));

	toolButtonIcon.addFile(":/button_add.png");
	toolBar->addAction(toolButtonIcon, tr("Add custom button"),this, SLOT(openMe()));
	toolBar->addSeparator();

	fileMenuIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirHomeIcon));
	pluginIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirHomeIcon));
	interfaceIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirOpenIcon),
			QIcon::Normal, QIcon::On);
	interfaceIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirClosedIcon),
			QIcon::Normal, QIcon::Off);
	menuIcon.addPixmap(style()->standardPixmap(QStyle::SP_FileIcon));
	funcIcon.addPixmap(style()->standardPixmap(QStyle::SP_MessageBoxInformation));

	tabWidget = new QTabWidget();
	// page main window
	pageMainWindow = new QWidget();

	mainWindowTreeWidget = new QTreeWidget();
	mainWindowTreeWidget->setColumnCount(2);
	mainWindowTreeWidget->setSortingEnabled(false);

	QStringList mainWindowHeaderStringList = QStringList() <<tr("Menu Name")<<tr("Alias");
	QTreeWidgetItem * mainWindowHeaderItem = new QTreeWidgetItem(mainWindowHeaderStringList);
	mainWindowTreeWidget->setHeaderItem(mainWindowHeaderItem);
	mainWindowTreeWidget->setColumnWidth(0, 400);

#ifdef __hierarchical_file_menu__
	QMap<QString, QTreeWidgetItem*> treeWidgetItemOfMenuName;
#endif
	QList<std::pair<QString, VoidFunc> > mainWindowMenuStringAndFuncList = getMainWindowButtonStringAndFuncList();
	for(int i = 0; i < mainWindowMenuStringAndFuncList.size(); i++)
	{
		QString menuName = mainWindowMenuStringAndFuncList.at(i).first;
		QString menuAlias = menuName.right(menuName.size() - menuName.lastIndexOf(tr("::")) - 2);
		VoidFunc menuFunc = mainWindowMenuStringAndFuncList.at(i).second;

#ifdef __hierarchical_file_menu__
		QTreeWidgetItem * mainWindowItem = createTreeWidgetItem(menuName, treeWidgetItemOfMenuName, mainWindowTreeWidget);
		QCheckBox * checkbox = new QCheckBox(menuAlias);
#else
		QTreeWidgetItem * mainWindowItem = new QTreeWidgetItem(mainWindowTreeWidget);
		QCheckBox * checkbox = new QCheckBox(menuName);
#endif

		checkbox->setChecked(false);
		mainWindowTreeWidget->setItemWidget(mainWindowItem, 0, checkbox);
		mainWindowCheckboxList.push_back(checkbox);

		QLineEdit * editor = new QLineEdit(menuAlias);
		mainWindowTreeWidget->setItemWidget(mainWindowItem, 1, editor);
		editor->setDisabled(true);
		mainWindowEditorList.push_back(editor);

		CustomToolButton * qb = new CustomToolButton(0, editor->text(), toolBar);
		qb->button->setVisible(false);
		qb->slot_func = menuFunc;
		qb->bt = 0;
		qb->buttonName = menuName;
		qb->callback = callback;
		qb->parent = parent;
		mainWindowCustomToolButtonList.push_back(qb);

		connect(checkbox, SIGNAL(clicked(bool)), editor, SLOT(setEnabled(bool)));
		connect(checkbox, SIGNAL(clicked(bool)), this, SLOT(setToolBarButton(bool)));
		connect(editor, SIGNAL(textChanged(const QString &)), qb, SLOT(setButtonText(const QString &)));
		if(cts->preLoadedNameList.contains(menuName))
		{
			int i = cts->preLoadedNameList.indexOf(menuName);
			editor->setText(cts->preLoadedAliasList.at(i));
			editor->setEnabled(true);
			checkbox->setChecked(Qt::Checked);
#ifdef __hierarchical_file_menu__
			{
				mainWindowItem->setExpanded(true);
				QString par_menu_name = menuName;
				int n = -1;
				while((n = par_menu_name.lastIndexOf("::"))!=-1){
					par_menu_name = par_menu_name.left(n);
					treeWidgetItemOfMenuName[par_menu_name]->setExpanded(true);
				}
			}
#endif
		}
	}

	pageMainWindowLayout = new QVBoxLayout();
	pageMainWindowLayout->addWidget(mainWindowTreeWidget);
	pageMainWindow->setLayout(pageMainWindowLayout);

	// page tri view
	pageTriView = new QWidget();

	triViewTreeWidget = new QTreeWidget();
	triViewTreeWidget->setColumnCount(2);
	triViewTreeWidget->setSortingEnabled(true);

	QStringList triViewHeaderStringList = QStringList() <<tr("Button Name")<<tr("Alias");
	QTreeWidgetItem * triViewHeaderItem = new QTreeWidgetItem(triViewHeaderStringList);
	triViewTreeWidget->setHeaderItem(triViewHeaderItem);
	triViewTreeWidget->setColumnWidth(0, 400);

	QList<std::pair<QString, VoidFunc> > triViewButtonStringAndFuncList = getTriViewButtonStringAndFuncList();
	for(int i = 0; i < triViewButtonStringAndFuncList.size(); i++)
	{
		QString buttonName =triViewButtonStringAndFuncList.at(i).first;
		VoidFunc buttonFunc = triViewButtonStringAndFuncList.at(i).second;

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
		qb->slot_func = buttonFunc;
		qb->bt = 1;
		qb->buttonName = buttonName;
		qb->callback = callback;
		qb->parent = parent;
		triViewCustomToolButtonList.push_back(qb);

		connect(checkbox, SIGNAL(clicked(bool)), editor, SLOT(setEnabled(bool)));
		connect(checkbox, SIGNAL(clicked(bool)), this, SLOT(setToolBarButton(bool)));
		connect(editor, SIGNAL(textChanged(const QString &)), qb, SLOT(setButtonText(const QString &)));
		if(cts->preLoadedNameList.contains(buttonName))
		{
			int i = cts->preLoadedNameList.indexOf(buttonName);
			editor->setText(cts->preLoadedAliasList.at(i));
			editor->setEnabled(true);
			checkbox->setChecked(Qt::Checked);
		}
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
	view3dTreeWidget->setColumnWidth(0, 400);

	QList<std::pair<QString, VoidFunc> > view3dButtonStringAndFuncList = getView3dButtonStringAndFuncList();
	for(int i = 0; i < view3dButtonStringAndFuncList.size(); i++)
	{
		QString buttonName =view3dButtonStringAndFuncList.at(i).first;
		VoidFunc buttonFunc = view3dButtonStringAndFuncList.at(i).second;

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
		qb->slot_func = buttonFunc;
		qb->bt = 2;
		qb->buttonName = buttonName;
		qb->callback = callback;
		qb->parent = parent;
		view3dCustomToolButtonList.push_back(qb);

		connect(checkbox, SIGNAL(clicked(bool)), editor, SLOT(setEnabled(bool)));
		connect(checkbox, SIGNAL(clicked(bool)), this, SLOT(setToolBarButton(bool)));
		connect(editor, SIGNAL(textChanged(const QString &)), qb, SLOT(setButtonText(const QString &)));
		if(cts->preLoadedNameList.contains(buttonName))
		{
			int i = cts->preLoadedNameList.indexOf(buttonName);
			editor->setText(cts->preLoadedAliasList.at(i));
			editor->setEnabled(true);
			checkbox->setChecked(Qt::Checked);
		}
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
	//pluginTreeWidget->setColumnWidth(0, 400);

	pagePluginLayout = new QVBoxLayout();

	QStringList fileList; getAllFiles(pluginRootPath, fileList);

	foreach(QString file, fileList)
	{
		QPluginLoader* loader = new QPluginLoader(file);
		if(!loader) continue;

		//pluginLoaderList.push_back(loader);

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
				qb->plugin_path = file;
				qb->bt = 3;
				qb->callback = callback;
				qb->parent = parent;
				qb->buttonName = file + tr("::") + menu_name;

				pluginCustomToolButtonList.push_back(qb);

				connect(checkbox, SIGNAL(clicked(bool)), editor, SLOT(setEnabled(bool)));
				connect(checkbox, SIGNAL(clicked(bool)), this, SLOT(setToolBarButton(bool)));
				connect(editor, SIGNAL(textChanged(const QString &)), qb, SLOT(setButtonText(const QString &)));
				if(cts->preLoadedNameList.contains(file + "::" + menu_name))
				{
					int i = cts->preLoadedNameList.indexOf(file + "::" + menu_name);
					editor->setText(cts->preLoadedAliasList.at(i));
					editor->setEnabled(true);
					checkbox->setChecked(Qt::Checked);
					pluginItem->setExpanded(true);
				}
			}
		}
		//while(loader->isLoaded())
			loader->unload();
		delete loader;
	}

	pluginTreeWidget->resizeColumnToContents(0);
	pagePluginLayout->addWidget(pluginTreeWidget);

	pagePlugin->setLayout(pagePluginLayout);

#ifndef __v3d_custom_toolbar_plugin__
	tabWidget->addTab(pageMainWindow, tr("Menu"));
#endif
	tabWidget->addTab(pageTriView, tr("Tri View"));
	tabWidget->addTab(pageView3d, tr("3D View"));
	tabWidget->addTab(pagePlugin, tr("Plugin"));
	tabWidget->setCurrentIndex(3);

	mainLayout = new QHBoxLayout();
	mainLayout->addWidget(tabWidget);

	setLayout(mainLayout);
	setMaximumSize(1000,800);
	setGeometry(400,400,1000, 800);

	this->setInitialToolBarButton();
}

CustomToolbarSelectWidget::~CustomToolbarSelectWidget()
{
	/*
	if(!pluginLoaderList.empty())
	{
		foreach(QPluginLoader * loader, pluginLoaderList)
		{
			loader->unload();
		}
	}
	*/

	delete cts;
}

CustomToolButton * CustomToolbarSelectWidget::getButtonFromButtonName(QString buttonName)
{
	foreach(CustomToolButton* cb, mainWindowCustomToolButtonList)
	{
		if(cb->buttonName == buttonName) return cb;
	}
	foreach(CustomToolButton* cb, triViewCustomToolButtonList)
	{
		if(cb->buttonName == buttonName) return cb;
	}
	foreach(CustomToolButton* cb, view3dCustomToolButtonList)
	{
		if(cb->buttonName == buttonName) return cb;
	}
	foreach(CustomToolButton* cb, pluginCustomToolButtonList)
	{
		if(cb->buttonName == buttonName) return cb;
	}
	return 0;
}

CustomToolButton * CustomToolbarSelectWidget::getButtonFromCheckbox(QCheckBox* checkbox)
{
	int i = -1;
	if((i = mainWindowCheckboxList.indexOf(checkbox))!= -1)
	{
		return mainWindowCustomToolButtonList.at(i);
	}
	else if((i = triViewCheckboxList.indexOf(checkbox))!= -1)
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

void CustomToolbarSelectWidget::setInitialToolBarButton()
{
	if(cts->preLoadedNameList.empty()) return;
	int i = 0;
	foreach(QString buttonName, cts->preLoadedNameList)
	{
		QString buttonAlias = cts->preLoadedAliasList.at(i);

		CustomToolButton * cb = getButtonFromButtonName(buttonName);
		if(cb)
		{
			cb->button->setVisible(true);
			cb->button->setText(buttonAlias);

			toolBar->addAction(cb->button);
			cts->allActiveButtonList.push_back(cb);
		}
		else
		{
			qDebug()<<"unable to find button"<<buttonName;
		}
		i++;
	}
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
		cts->allActiveButtonList.push_back(cb);
	}
	else if(!state && cb->button->isVisible())
	{
		cb->button->setVisible(false);
		cts->allActiveButtonList.removeOne(cb);
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
	saveToolBarSettings();
}
void CustomToolbarSelectWidget::saveToolBarState()
{
	saveToolBarSettings();
}

bool CustomToolButton::run()
{
	try
	{
		if(bt == 0) // Main Window
		{
#ifndef __v3d_custom_toolbar_plugin__
			MainWindow * mw = qobject_cast<MainWindow*>(parent);
			if(mw)
			{
				if(buttonName.indexOf(tr("Window::")) != -1 ) (mw->workspace->*(WorkspaceFunc)slot_func)();
				else if(buttonName.indexOf(tr("Plug-In::")) != -1) (mw->pluginLoader->*(V3dPluginLoaderFunc)slot_func)();
				else (mw->*(MainWindowFunc)slot_func)();
			}
#endif
			return true;
		}
		if(bt == 1) // Triview
		{
			v3dhandleList winlist = callback->getImageWindowList();
			if(winlist.empty()) {QMessageBox::information(0,"","No image stack!"); return true;}
			v3dhandle handle = callback->currentImageWindow();
			if(handle)
			{
				(callback->*(Callback2Func)slot_func)(handle);
				//callback->open3DWindow(handle);
			}
			return true;
		}
		else if(bt == 2) // View3d
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
		else if(bt == 3) // Plugin do menu
		{
			QPluginLoader* loader = new QPluginLoader(plugin_path);
			cout<<"plugin_file = "<<plugin_path.toStdString()<<endl;
			if(!loader)
			{
				QMessageBox::information(0,"","Unable to load this plugin!");
				return true;
			}

			loader->unload();
			QObject * plugin = loader->instance();
			QString menu_name = buttonName.right(buttonName.size() - buttonName.lastIndexOf(tr("::")) - 2);
			V3DSingleImageInterface2_1 *iFilter2_1 = qobject_cast<V3DSingleImageInterface2_1 *>(plugin);
			if (iFilter2_1 )
			{
				QMessageBox::information(0,"","This is V3DSingleImageInterface2_1 plugin , only V3DPluginInterface2 and V3DPluginInterface2_1 is supported!");
				return false;
			}
			V3DSingleImageInterface *iFilter = qobject_cast<V3DSingleImageInterface *>(plugin);
			if (iFilter )
			{
				QMessageBox::information(0,"","This is V3DSingleImageInterface plugin , only V3DPluginInterface2 and V3DPluginInterface2_1 is supported!");
				return false;
			}
			V3DPluginInterface2_1 *iface2_1 = qobject_cast<V3DPluginInterface2_1 *>(plugin);
			if (iface2_1 )
			{
				iface2_1->domenu(menu_name, *callback, 0/*parent*/); // 20120705 Hang, set parent to 0
			}
			V3DPluginInterface2 *iface2 = qobject_cast<V3DPluginInterface2 *>(plugin);
			if (iface2 )
			{
				iface2->domenu(menu_name, *callback, 0/*parent*/); // 20120705 Hang, set parent to 0
			}
			V3DPluginInterface *iface = qobject_cast<V3DPluginInterface *>(plugin);
			if (iface )
			{
				iface->domenu(menu_name, *callback, 0/*parent*/); // 20120705 Hang, set parent to 0
			}
			//while(loader->isLoaded())
			loader->unload();
			delete loader;

			return true;
		}
	}
	catch(...)
	{
		QMessageBox::information(0,"","Unable to run this action, please check the corresponding code!");
		return false;
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
			foreach(CustomToolButton* cb, cts->allActiveButtonList)
			{
				if (cb->bt == 0) ofs<<"MainWindowButton"<<endl;
				else if(cb->bt == 1) ofs<<"TriViewButton"<<endl;
				else if(cb->bt == 2) ofs<<"View3dButton"<<endl;
				else if(cb->bt == 3) ofs<<"PluginButton"<<endl;
				ofs<<"\t"<<cb->buttonName.toStdString()<<endl;
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

			cts = new CustomToolbarSetting(QString(title));
			cts->position = (Qt::ToolBarArea)position;
			settingList.push_back(cts);
		}
		else if(str == "MainWindowButton")
		{
			char name[1000]; ifs.ignore(1000,'\t'); ifs.getline(name, 1000);
			char alias[1000]; ifs.ignore(1000,'\t'); ifs.getline(alias,1000);
			if(cts)
			{
				cts->preLoadedNameList.push_back(QString(name).trimmed());
				cts->preLoadedAliasList.push_back(QString(alias).trimmed());
			}
		}
		else if(str == "TriViewButton")
		{
			char name[1000]; ifs.ignore(1000,'\t'); ifs.getline(name, 1000);
			char alias[1000]; ifs.ignore(1000,'\t'); ifs.getline(alias,1000);
			if(cts)
			{
				cts->preLoadedNameList.push_back(QString(name).trimmed());
				cts->preLoadedAliasList.push_back(QString(alias).trimmed());
			}
		}
		else if(str == "View3dButton")
		{
			char name[1000]; ifs.ignore(1000,'\t'); ifs.getline(name, 1000);
			char alias[1000]; ifs.ignore(1000,'\t'); ifs.getline(alias,1000);
			if(cts)
			{
				cts->preLoadedNameList.push_back(QString(name).trimmed());
				cts->preLoadedAliasList.push_back(QString(alias).trimmed());
			}
		}
		else if(str == "PluginButton")
		{
			char name[1000]; ifs.ignore(1000,'\t'); ifs.getline(name, 1000);
			char alias[1000]; ifs.ignore(1000,'\t'); ifs.getline(alias,1000);
			if(cts)
			{
				cts->preLoadedNameList.push_back(QString(name).trimmed());
				cts->preLoadedAliasList.push_back(QString(alias).trimmed());
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

#define StringAndFunc pair<QString, VoidFunc>
#define SAF pair<QString, VoidFunc>

QList<pair<QString, VoidFunc> > getMainWindowButtonStringAndFuncList()
{
#ifdef __v3d_custom_toolbar_plugin__
	return QList<StringAndFunc>();
#else
	return QList<StringAndFunc>()
		<<SAF (QObject::tr("File::Open image/stack in a new window ..."), \
				(VoidFunc)(&MainWindow::open))
		<<SAF (QObject::tr("File::Open web image/stack ..."), \
				(VoidFunc)(&MainWindow::openWebUrl))
		<<SAF (QObject::tr("File::Save or Save as"), \
				(VoidFunc)(&MainWindow::saveAs))
		<<SAF (QObject::tr("File::Adjust preferences"), \
				(VoidFunc)(&MainWindow::func_procSettings))
		<<SAF (QObject::tr("File::Import::Import general image series to an image stack..."), \
				(VoidFunc)(&MainWindow::import_GeneralImageFile))
		<<SAF (QObject::tr("File::Import::Import Leica 2D tiff series to an image stack..."), \
				(VoidFunc)(&MainWindow::import_Leica))
		<<SAF (QObject::tr("File::Import::Build an atlas linker file for [registered] images under a folder"), \
				(VoidFunc)(&MainWindow::func_procIO_import_atlas_imgfolder))
		<<SAF (QObject::tr("File::Export::Export 3D [cell] segmentation results to VANO annotation files"), \
				(VoidFunc)(&MainWindow::func_procIO_export_to_vano_format))
		<<SAF (QObject::tr("File::Export::Export landmarks to pointcloud (.apo) file"), \
				(VoidFunc)(&MainWindow::func_procIO_export_landmark_to_pointcloud))
		<<SAF (QObject::tr("File::Export::Export traced neuron/fibrous-structure path-info to graph (.swc) file"), \
				(VoidFunc)(&MainWindow::func_procIO_export_tracedneuron_to_swc))
		<<SAF (QObject::tr("Image/Data::image type::convert indexed/mask image to RGB image"), \
				(VoidFunc)(&MainWindow::func_procGeneral_indexedimg2rgb))
		<<SAF (QObject::tr("Image/Data::image type::linear rescale to [0~255] and convert to 8bit image"), \
				(VoidFunc)(&MainWindow::func_procGeneral_scaleandconvert28bit))
		<<SAF (QObject::tr("Image/Data::image type::saturate top/bottom 1% voxels and linear-rescale to [0~255]/8bit"), \
				(VoidFunc)(&MainWindow::func_procGeneral_scaleandconvert28bit_1percent))
		<<SAF (QObject::tr("Image/Data::image type::convert 16bit image to 8bit via bit-shift"), \
				(VoidFunc)(&MainWindow::func_procGeneral_16bit_to_8bit))
		<<SAF (QObject::tr("Image/Data::image type::convert 32bit image to 8bit via bit-shift"), \
				(VoidFunc)(&MainWindow::func_procGeneral_32bit_to_8bit))
		<<SAF (QObject::tr("Image/Data::geometry::rotate principal axis"), \
				(VoidFunc)(&MainWindow::func_procGeneral_rotate_paxis))
		<<SAF (QObject::tr("Image/Data::geometry::rotate arbitrary angle"), \
				(VoidFunc)(&MainWindow::func_procGeneral_rotate_angle))
		<<SAF (QObject::tr("Image/Data::geometry::flip image"), \
				(VoidFunc)(&MainWindow::func_procGeneral_flip))
		<<SAF (QObject::tr("Image/Data::geometry::crop image (minMax Bounding Box)"),
				(VoidFunc)(&MainWindow::func_procGeneral_crop_image_minMaxBox))
		<<SAF (QObject::tr("Image/Data::geometry::crop image (ROI-based)"), \
				(VoidFunc)(&MainWindow::func_procGeneral_crop_bbox_roi))
		<<SAF (QObject::tr("Image/Data::geometry::image resampling"), \
				(VoidFunc)(&MainWindow::func_procGeneral_resample_image))
		<<SAF (QObject::tr("Image/Data::intensity::mask ROI or non-ROI"), \
				(VoidFunc)(&MainWindow::func_procGeneral_mask_roi))
		<<SAF (QObject::tr("Image/Data::intensity::fill value to non-ROI region for all XY planes"), \
				(VoidFunc)(&MainWindow::func_procGeneral_mask_nonroi_xy))
		<<SAF (QObject::tr("Image/Data::intensity::mask channel"), \
				(VoidFunc)(&MainWindow::func_procGeneral_mask_channel))
		<<SAF (QObject::tr("Image/Data::intensity::clear ROI"), \
				(VoidFunc)(&MainWindow::func_procGeneral_clear_roi))
		<<SAF (QObject::tr("Image/Data::intensity::max projection"), \
				(VoidFunc)(&MainWindow::func_procGeneral_projection_max))
		<<SAF (QObject::tr("Image/Data::intensity::histogram equalization"), \
				(VoidFunc)(&MainWindow::func_procGeneral_histogram_equalization))
		<<SAF (QObject::tr("Image/Data::intensity::rescaling"), \
				(VoidFunc)(&MainWindow::func_procGeneral_intensity_rescale))
		<<SAF (QObject::tr("Image/Data::intensity::thresholding"), \
				(VoidFunc)(&MainWindow::func_procGeneral_intensity_threshold))
		<<SAF (QObject::tr("Image/Data::intensity::binarization"), \
				(VoidFunc)(&MainWindow::func_procGeneral_intensity_binarize))
		<<SAF (QObject::tr("Image/Data::intensity::invert color"), \
				(VoidFunc)(&MainWindow::func_procGeneral_color_invert))
		<<SAF (QObject::tr("Image/Data::intensity::update the displayed min/max value(s)"), \
				(VoidFunc)(&MainWindow::func_procGeneral_intensity_updateminmax))
		<<SAF (QObject::tr("Image/Data::color channel::split color channels"), \
				(VoidFunc)(&MainWindow::func_procGeneral_split_channels))
		<<SAF (QObject::tr("Image/Data::color channel::extract a color channel"), \
				(VoidFunc)(&MainWindow::func_procGeneral_extract_a_channel))
		<<SAF (QObject::tr("Image/Data::color channel::image blending"), \
				(VoidFunc)(&MainWindow::func_procGeneral_blend_image))
		<<SAF (QObject::tr("Image/Data::color channel::invert color"), \
				(VoidFunc)(&MainWindow::func_procGeneral_color_invert))
		<<SAF (QObject::tr("Image/Data::landmark::landmark manager"), \
				(VoidFunc)(&MainWindow::func_procLandmarkManager))
		<<SAF (QObject::tr("Image/Data::landmark::clear all landmarks"), \
				(VoidFunc)(&MainWindow::func_procGeneral_clear_all_landmark))
		<<SAF (QObject::tr("Image/Data::landmark::clear graph edges/connection map"), \
				(VoidFunc)(&MainWindow::func_procGeneral_clear_connectmap))
		<<SAF (QObject::tr("Image/Data::landmark::rescale landmark only"), \
				(VoidFunc)(&MainWindow::func_procGeneral_rescale_landmarks_only))
		<<SAF (QObject::tr("Image/Data::landmark::turn on/off displaying landmark labels"), \
				(VoidFunc)(&MainWindow::func_procGeneral_toggle_landmark_label))
		<<SAF (QObject::tr("Visualize::3D viewer for entire image"), \
				(VoidFunc)(&MainWindow::func_proc3DViewer))
		<<SAF (QObject::tr("Visualize::3D viewer for Region of Interest (ROI)"), \
				(VoidFunc)(&MainWindow::func_proc3DLocalRoiViewer))
		<<SAF (QObject::tr("Advanced::3D tracing (V3D-Neuron tracing v2.0)::trace from one landmark to all others"), \
				(VoidFunc)(&MainWindow::func_procTracing_one2others))
		<<SAF (QObject::tr("Advanced::3D tracing (V3D-Neuron tracing v2.0)::trace a path between two landmarks"), \
				(VoidFunc)(&MainWindow::func_procTracing_trace_a_curve))
		<<SAF (QObject::tr("Advanced::3D tracing (V3D-Neuron tracing v2.0)::undo the last tracing step"), \
				(VoidFunc)(&MainWindow::func_procTracing_undo_laststep))
		<<SAF (QObject::tr("Advanced::3D tracing (V3D-Neuron tracing v2.0)::redo the last tracing step"), \
				(VoidFunc)(&MainWindow::func_procTracing_redo_laststep))
		<<SAF (QObject::tr("Advanced::3D tracing (V3D-Neuron tracing v2.0)::clear the traced neuron"), \
				(VoidFunc)(&MainWindow::func_procTracing_clear))
		<<SAF (QObject::tr("Advanced::3D tracing (V3D-Neuron tracing v2.0)::update 3D view(s) of traced neuron"), \
				(VoidFunc)(&MainWindow::func_procTracing_update3Dview))
		<<SAF (QObject::tr("Advanced::3D tracing (V3D-Neuron tracing v2.0)::save the traced neuron to a file"), \
				(VoidFunc)(&MainWindow::func_procTracing_save))
		<<SAF (QObject::tr("Advanced::3D image atlas::3D image atlas viewer"), \
				(VoidFunc)(&MainWindow::func_procAtlasViewer))
		<<SAF (QObject::tr("Advanced::3D image atlas::Build an atlas linker file for [registered] images under a folder"), \
				(VoidFunc)(&MainWindow::func_procIO_import_atlas_imgfolder))
		<<SAF (QObject::tr("Plug-In::Plug-in manager"), \
				(VoidFunc)(&V3d_PluginLoader::aboutPlugins))
		<<SAF (QObject::tr("Plug-In::Re-scan all plugins"), \
				(VoidFunc)(&V3d_PluginLoader::rescanPlugins))
		<<SAF (QObject::tr("Window::Close"), \
				(VoidFunc)(&QMdiArea::closeActiveSubWindow))
		<<SAF (QObject::tr("Window::Close All"), \
				(VoidFunc)(&QMdiArea::closeAllSubWindows))
		<<SAF (QObject::tr("Window::Tile"), \
				(VoidFunc)(&QMdiArea::tileSubWindows))
		<<SAF (QObject::tr("Window::Cascade"), \
				(VoidFunc)(&QMdiArea::cascadeSubWindows))
//		<<SAF (QObject::tr("Window::Arrange icons"), \
//				(VoidFunc)(&QMdiArea::arrangeIcons))
		<<SAF (QObject::tr("Window::Next"), \
				(VoidFunc)(&QMdiArea::activateNextSubWindow))
		<<SAF (QObject::tr("Window::Previous"), \
				(VoidFunc)(&QMdiArea::activatePreviousSubWindow))
		//<<SAF (QObject::tr("Work-Mode::V3D Default"), \
				(VoidFunc)(&MainWindow::func_procModeDefault))
		//<<SAF (QObject::tr("Work-Mode::Neuron Annotator"), \
				(VoidFunc)(&MainWindow::func_procModeNeuronAnnotator))
		;
#endif
}

QList<pair<QString, VoidFunc> > getTriViewButtonStringAndFuncList()
{
	return QList<SAF >()
		<<SAF (QObject::tr("See in 3D"), \
				(VoidFunc)(&V3DPluginCallback2::open3DWindow))
		;
}

QList<pair<QString, VoidFunc> > getView3dButtonStringAndFuncList()
{
	// refer to 3drenderer/v3dr_control_signal.cpp
	return QList<SAF >()
		<<SAF (QObject::tr("Sync Tri-view Objs"), \
				(VoidFunc)(&View3DControl::updateWithTriView))
		;
}

// parent won't be used as the parent QWidget
CustomToolbar::CustomToolbar(QString title, V3DPluginCallback2 * callback, QWidget * parent) : QToolBar(0/*parent*/)
{
	setWindowTitle(title);
	cts = new CustomToolbarSetting(this);
	selectWidget = new CustomToolbarSelectWidget(cts, callback, parent);
}

CustomToolbar::CustomToolbar(CustomToolbarSetting* _cts, V3DPluginCallback2 * callback, QWidget * parent) : QToolBar(0/*parent*/)
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

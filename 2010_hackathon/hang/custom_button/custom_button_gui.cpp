#include <iostream>
#include "custom_button_gui.h"

using namespace std;

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

QString v3d_getInterfaceName(QObject *plugin)
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

QStringList v3d_getInterfaceMenuList(QObject *plugin)
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

QStringList v3d_getInterfaceFuncList(QObject *plugin)
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


CustomButtonSelectWidget::CustomButtonSelectWidget(V3DPluginCallback2 &callback, QWidget * parent, QToolBar * _toolBar)
{
	toolBar = _toolBar;

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
	pageTriView = new QWidget();
	page3dView = new QWidget();
	pagePlugin = new QWidget();

	pageTriViewLayout = new QGridLayout();

	pageTriView->setLayout(pageTriViewLayout);

	QString pluginRoot = tr("/Users/xiaoh10/Applications/v3d/plugins");
	QStringList fileList;
	//QList<QObject*> objectList;

	getAllFiles(pluginRoot, fileList);
	//getObjectList(fileList, objectList);

	pluginTreeWidget = new QTreeWidget();
	pluginTreeWidget->setColumnCount(2);
	//pluginTreeWidget->header()->hide();

	QStringList headerStringList = QStringList() <<tr("Plugin Name")<<tr("Set Button Name");
	QTreeWidgetItem * headerItem = new QTreeWidgetItem(headerStringList);
	pluginTreeWidget->setHeaderItem(headerItem);

	pagePluginLayout = new QVBoxLayout();

	int plugin_num = fileList.size();
	for(int i = 0; i < plugin_num; i++)
	{
		QString file = fileList.at(i);
		QPluginLoader* loader = new QPluginLoader(file);
		QObject * plugin = loader->instance();

		if(plugin)
		{
			QTreeWidgetItem *pluginItem = new QTreeWidgetItem(pluginTreeWidget);
			pluginItem->setText(0, file.replace(0,pluginRoot.size(),tr("")));
			pluginItem->setIcon(0, pluginIcon);

			QStringList menulist = v3d_getInterfaceMenuList(plugin);
			foreach(QString menu, menulist)
			{
				QTreeWidgetItem * menuItem = new QTreeWidgetItem(pluginItem);

				QCheckBox * checkbox = new QCheckBox(menu);
				pluginTreeWidget->setItemWidget(menuItem, 0, checkbox);
				pluginCheckboxList.push_back(checkbox);

				QLineEdit * editor = new QLineEdit(menu);
				pluginTreeWidget->setItemWidget(menuItem, 1, editor);
				editor->setDisabled(true);
				pluginEditorList.push_back(editor);

				CustomButton * qb = new CustomButton(0, editor->text(), toolBar);
				qb->button->setVisible(false);
				qb->slot_class = plugin;
				qb->bt = 2;
				qb->menu_name = menu;
				qb->callback = &callback;
				qb->parent = parent;

				pluginCustomButtonList.push_back(qb);

				connect(checkbox, SIGNAL(clicked(bool)), editor, SLOT(setEnabled(bool)));
				connect(checkbox, SIGNAL(clicked(bool)), this, SLOT(setToolBarButton(bool)));
				connect(editor, SIGNAL(textChanged(const QString &)), qb, SLOT(setButtonText(const QString &)));
			}
		}
		//loader->unload();
	}

	pluginTreeWidget->resizeColumnToContents(0);
	pagePluginLayout->addWidget(pluginTreeWidget);

	pagePlugin->setLayout(pagePluginLayout);

	tabWidget->addTab(pageTriView, tr("Tri View"));
	tabWidget->addTab(page3dView, tr("3D View"));
	tabWidget->addTab(pagePlugin, tr("Plugin"));

	mainLayout = new QHBoxLayout();
	mainLayout->addWidget(tabWidget);

	setLayout(mainLayout);
}

int CustomButtonSelectWidget::isIn(QCheckBox * checkbox, QList<QCheckBox *> & checkboxList)
	{
		int i = 0;
		foreach(QCheckBox* cb, checkboxList)
		{
			if(checkbox == cb) return i;
			i++;
		}
		return -1;
	}

QAction * CustomButtonSelectWidget::getButton(QCheckBox* checkbox)
{
	int i = -1;
	if((i = isIn(checkbox, triViewCheckboxList))!= -1)
	{
	}
	else if((i = isIn(checkbox,view3dCheckboxList))!= -1)
	{
	}
	else if((i = isIn(checkbox,pluginCheckboxList))!= -1)
	{
		return pluginCustomButtonList.at(i)->button;
	}
	return 0;
}

void CustomButtonSelectWidget::setToolBarButton(bool state)
{
	QCheckBox * checkbox = dynamic_cast<QCheckBox*>(sender());
	QAction * button = getButton(checkbox);
	if(state && !button->isVisible())
	{
		toolBar->addAction(button);
	}
	button->setVisible(state);
}

void CustomButtonSelectWidget::loadSetting(){}

void CustomButtonSelectWidget::saveSetting(){}

void CustomButtonSelectWidget::openMe()
{
	if(isVisible()) 
		setHidden(true);
	else
		show();
}
bool CustomButton::run()
{
	cout<<"go to run"<<endl;
	if(slot_class==0) return false;
	if(bt == 0) // Triview
	{
		(((TriviewControl*)slot_class)->*(TriviewFunc)slot_func)();
		return true;
	}
	else if(bt == 1) // View3d
	{
		(((View3DControl*)slot_class)->*(View3DFunc)slot_func)();
		return true;
	}
	else if(bt == 2) // Plugin do menu
	{
		QObject * plugin = slot_class;
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

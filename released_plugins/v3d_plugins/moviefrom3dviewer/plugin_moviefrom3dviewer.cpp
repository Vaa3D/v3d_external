//plugin_moviefrom3dviewer.cpp
//by Lei Qu
//2010-11-08

#include "plugin_moviefrom3dviewer.h"
#include "v3d_message.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(moviefrom3dviewer, MovieFrom3DviewerPlugin);

void MovieFrom3Dviewer(V3DPluginCallback2 & v3d, QWidget * parent);

//plugin funcs
const QString title = "Movie From 3Dviewer";
lookPanel* lookPanel::panel = 0;
QStringList MovieFrom3DviewerPlugin::menulist() const
{
    return QStringList()
		<< tr("movie from 3Dviewer");
}

void MovieFrom3DviewerPlugin::domenu(const QString & menu_name, V3DPluginCallback2 & v3d, QWidget * parent)
{
    if (menu_name == tr("movie from 3Dviewer"))
    {
    	MovieFrom3Dviewer(v3d,parent);
    }
}


void MovieFrom3Dviewer(V3DPluginCallback2 & v3d,QWidget * parent)
{
	v3dhandle curwin = v3d.currentImageWindow();
	if(!curwin)
	{
		v3d_msg("You don't have any image open in the main window.");
		return;
	}
	v3d.open3DWindow(curwin);
	
	if(lookPanel::panel)
	{
		lookPanel::panel->show();
		return;
	}
	
	lookPanel* p=new lookPanel(v3d, parent);
	if(p)	p->show();
}


lookPanel::lookPanel(V3DPluginCallback2 &_v3d, QWidget *parent) : QDialog(parent),v3d(_v3d)
{
	panel=this;
	l_frameind=0;

	m_pLineEdit_fps=new QLineEdit(QObject::tr("4"));
	m_pLineEdit_filepath=new QLineEdit();
	QPushButton *ok     = new QPushButton(QObject::tr("Strat"));
	QPushButton *cancel = new QPushButton(QObject::tr("stop"));
	QFormLayout *formLayout = new QFormLayout;
	formLayout->addRow(QObject::tr("choose fps(ms): "), m_pLineEdit_fps);
	formLayout->addRow(QObject::tr("choose output dir: "), m_pLineEdit_filepath);
	formLayout->addRow(ok, cancel);

	//QDialog d(parent);
	setLayout(formLayout);
	setWindowTitle(QString("make movie"));

	connect(ok,     SIGNAL(clicked()), this, SLOT(_slot_start()));
	connect(cancel, SIGNAL(clicked()), this, SLOT(_slot_end()));
}
lookPanel::~lookPanel()
{
	panel=0;
}
void lookPanel::_slot_start()
{
	long interval=1.0/m_pLineEdit_fps->text().toLong()*1000;

	timer=new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(_slot_timerupdate()));
	timer->start(interval);
}
void lookPanel::_slot_end()
{
	timer->stop();
}
void lookPanel::_slot_timerupdate()
{
	QString outputdir=m_pLineEdit_filepath->text();
//	QString outputdir="/Users/qul/work/v3d_2.0/plugin_demo/moviefrom3dviewer/";

	v3dhandle curwin=v3d.currentImageWindow();
	QString BMPfilename=outputdir+QString("/aaa_%1").arg(l_frameind);
	v3d.screenShot3DWindow(curwin, BMPfilename);

	printf("%d\n",l_frameind);
	l_frameind++;
}

#ifndef __SWC_EDITOR_GUI_H__
#define __SWC_EDITOR_GUI_H__

#include <QtGui>
#include <v3d_interface.h>
#include "v3d_monitor.h"


class SWCEditorWidget : public QWidget
{
	Q_OBJECT

enum ModeType {DISABLE_MODE,ADD_EDGE_MODE, REMOVE_EDGE_MODE, ADD_VERTEX_MODE, REMOVE_VERTEX_MODE};

public:
	SWCEditorWidget(V3DPluginCallback2 &callback, QWidget * parent) : QWidget(parent)
	{
		this->callback = &callback;
		curwin = callback.currentImageWindow();
		mode = DISABLE_MODE;

		v3dhandleList win_list = callback.getImageWindowList();
		QStringList items;
		for(int i = 0; i < win_list.size(); i++) items << callback.getImageName(win_list[i]);

		add_edge_mode_button = new QRadioButton(tr("add edge"));
		remove_edge_mode_button = new QRadioButton(tr("remove edge"));
		add_vertex_mode_button = new QRadioButton(tr("add vertex"));
		remove_vertex_mode_button = new QRadioButton(tr("remove vertex"));
		disable_mode_button = new QRadioButton(tr("disable"));
		grid_layout = new QGridLayout();
		grid_layout->addWidget(add_edge_mode_button, 0, 0);
		grid_layout->addWidget(remove_edge_mode_button, 0, 1);
		grid_layout->addWidget(add_vertex_mode_button, 1, 0);
		grid_layout->addWidget(remove_vertex_mode_button, 1, 1);
		grid_layout->addWidget(disable_mode_button, 2, 0);
		set_mode_box = new QGroupBox(tr("set mode :"));
		set_mode_box->setLayout(grid_layout);

		clear_swc_button = new QPushButton(tr("clear swc"));

		vbox = new QVBoxLayout();
		vbox->addWidget(set_mode_box,1);
		vbox->addWidget(clear_swc_button);
		disable_mode_button->setChecked(true);
		this->setLayout(vbox);

		connect(this, SIGNAL(add_edge_mode_button), this, SLOT(onEditorModeChanged()));
		connect(this, SIGNAL(remove_edge_mode_button), this, SLOT(onEditorModeChanged()));
		connect(this, SIGNAL(add_vertex_mode_button), this, SLOT(onEditorModeChanged()));
		connect(this, SIGNAL(remove_vertex_mode_button), this, SLOT(onEditorModeChanged()));
		connect(this, SIGNAL(disable_eidtor_mode_button), this, SLOT(onEditorModeChanged()));

		connect(this, SIGNAL(clear_swc_button), this, SLOT(onClearSWC()));

		v3d_monitor = new V3dMonitor(&callback, curwin);
		connect(v3d_monitor, SIGNAL(mark_changed(LocationSimple)), this, SLOT(onMouseClicked(LocationSimple)));
		connect(v3d_monitor, SIGNAL(win_closed()), this, SLOT(onParentWindowClosed()));
		v3d_monitor->start();
	}

	~SWCEditorWidget()
	{
		delete v3d_monitor; 
	}

public slots:
	void onParentWindowClosed()
	{
		//if(v3d_monitor && v3d_monitor->isRunning()) v3d_monitor->terminate();
		v3d_msg("Image widget is closed !");
		this->close();
	}

	void onMouseClicked(LocationSimple loc)
	{
		v3d_msg(tr("(%1, %2, %3) is clicked! current mode : %4").arg(loc.x).arg(loc.y).arg(loc.z).arg(mode));
	}

	void onEditorModeChanged()
	{
		QRadioButton* button = (QRadioButton * )sender();
		if(button == add_edge_mode_button) mode = ADD_EDGE_MODE;
		else if(button == remove_edge_mode_button) mode = REMOVE_EDGE_MODE;
		else if(button == add_vertex_mode_button) mode = ADD_VERTEX_MODE;
		else if(button == remove_vertex_mode_button) mode = REMOVE_VERTEX_MODE; 
		else mode = DISABLE_MODE;
	}

	void onClearSWC()
	{
		NeuronTree nt;// = callback->getSWC(curwin);
		callback->setSWC(curwin, nt);
	}

public:
	QRadioButton * add_edge_mode_button;
	QRadioButton * remove_edge_mode_button;
	QRadioButton * add_vertex_mode_button;
	QRadioButton * remove_vertex_mode_button;
	QRadioButton * disable_mode_button;
	QGridLayout * grid_layout;
	QGroupBox * set_mode_box;

	QPushButton * clear_swc_button;

	QVBoxLayout * vbox;

	V3DPluginCallback2 * callback;
	v3dhandle curwin;
	V3dMonitor * v3d_monitor;
	ModeType mode;
};


class CreateNetworkWidget : public QWidget
{
	Q_OBJECT

public:
	CreateNetworkWidget(V3DPluginCallback2 &callback, QWidget * parent)
	{
		this->callback = &callback;
		curwin = 0;

		v3dhandleList win_list = callback.getImageWindowList();
		QStringList items;
		for(int i = 0; i < win_list.size(); i++) items << callback.getImageName(win_list[i]);

		nrows_label = new QLabel(tr("rows :"));
		nrows_spin = new QSpinBox();
		nrows_spin->setValue(5);

		ncols_label = new QLabel(tr("cols :"));
		ncols_spin = new QSpinBox();
		ncols_spin->setValue(3);

		nlayers_label = new QLabel(tr("layers :"));
		nlayers_spin = new QSpinBox();
		nlayers_spin->setValue(1);

		clear_swc_button = new QPushButton(tr("clear swc"));
		create_network_button = new QPushButton(tr("create network"));

		grid_layout = new QGridLayout();
		grid_layout->addWidget(nrows_label, 0, 0);
		grid_layout->addWidget(nrows_spin, 0, 1);
		grid_layout->addWidget(ncols_label, 1, 0);
		grid_layout->addWidget(ncols_spin, 1, 1);
		grid_layout->addWidget(nlayers_label, 2, 0);
		grid_layout->addWidget(nlayers_spin, 2, 1);
		grid_layout->addWidget(create_network_button, 3, 0, 1, 2);
		grid_layout->addWidget(clear_swc_button, 4, 0, 1, 2);
		group_box = new QGroupBox();
		group_box->setLayout(grid_layout);

		vbox = new QVBoxLayout();
		vbox->addWidget(group_box);

		this->setLayout(vbox);

		connect(nrows_spin, SIGNAL(valueChanged()), this, SLOT(update()));
		connect(ncols_spin, SIGNAL(valueChanged()), this, SLOT(update()));
		connect(nlayers_spin, SIGNAL(valueChanged()), this, SLOT(update()));

		connect(create_network_button, SIGNAL(clicked()), this, SLOT(onCreateNetwork()));
		connect(clear_swc_button, SIGNAL(clicked()), this, SLOT(onClearSWC()));

	}

	~CreateNetworkWidget(){}

public slots:
	void update()
	{
		nrows =  nrows_spin->text().toInt();
		ncols =  ncols_spin->text().toInt();
		nlayers =  nlayers_spin->text().toInt();

	}

	void onCreateNetwork()
	{
	}

	void onClearSWC()
	{
	}

public:
	int nrows;
	int ncols;
	int nlayers;
	QLabel * nrows_label;
	QSpinBox * nrows_spin;

	QLabel * ncols_label;
	QSpinBox * ncols_spin;

	QLabel * nlayers_label;
	QSpinBox * nlayers_spin;

	QPushButton * clear_swc_button;
	QPushButton * create_network_button;

	QGridLayout * grid_layout;
	QGroupBox * group_box;

	QVBoxLayout * vbox;

	V3DPluginCallback2 * callback;
	v3dhandle curwin;
};

#endif

#ifndef __TANGENT_PLANE_GUI_H__
#define __TANGENT_PLANE_GUI_H__

#include <QtGui>
#include <v3d_interface.h>

class TangentPlaneWidget : public QWidget
{
	Q_OBJECT

public:
	TangentPlaneWidget(V3DPluginCallback2 &callback, QWidget * parent) : QWidget(parent)
	{
		this->callback = &callback;
		curwin = callback.currentImageWindow();

		v3dhandleList win_list = callback.getImageWindowList();
		QStringList items;
		for(int i = 0; i < win_list.size(); i++) items << callback.getImageName(win_list[i]);
		landmarks = callback.getLandmark(curwin);

		factor_label = new QLabel(tr("radius factor (1 ~ 100)"));
		thresh_label = new QLabel(tr("image threshold (0 ~ 255)"));
		thick_label = new QLabel(tr("plane thickness (0 ~ 100)"));
		forward_label = new QLabel(tr("forward (2 ~ %1)").arg(landmarks.size()));
		backward_label = new QLabel(tr("backward (1 ~ %1)").arg(landmarks.size()-1));
		estimated_label = new QLabel(tr("estimated radius"));
		centroid_method_label = new QLabel(tr("centroid method"));

		view3d_checker = new QCheckBox("open view3d");

		factor_scroller = new QScrollBar(Qt::Horizontal);
		factor_scroller->setMaximum(100);
		factor_scroller->setValue(1);
		thresh_scroller = new QScrollBar(Qt::Horizontal);
		thresh_scroller->setMaximum(255);
		thresh_scroller->setMinimum(-1);
		thresh_scroller->setValue(-1);
		thick_scroller = new QScrollBar(Qt::Horizontal);
		thick_scroller->setMaximum(100);
		thick_scroller->setValue(0);
		forward_scroller = new QScrollBar(Qt::Horizontal);
		forward_scroller->setMaximum(landmarks.size());
		forward_scroller->setMinimum(2);
		forward_scroller->setValue(2);
		backward_scroller = new QScrollBar(Qt::Horizontal);
		backward_scroller->setMaximum(landmarks.size()-1);
		backward_scroller->setMinimum(1);
		backward_scroller->setValue(1);

		factor_spin = new QSpinBox();
		factor_spin->setMaximum(100);
		factor_spin->setValue(1);
		thresh_spin = new QSpinBox();
		thresh_spin->setMaximum(255);
		thresh_spin->setMinimum(-1);
		thresh_spin->setValue(-1);
		thick_spin = new QSpinBox();
		thick_spin->setMaximum(100);
		thick_spin->setValue(0);
		forward_spin = new QSpinBox();
		forward_spin->setMaximum(landmarks.size());
		forward_spin->setMinimum(2);
		forward_spin->setValue(2);
		backward_spin = new QSpinBox();
		backward_spin->setMaximum(landmarks.size()-1);
		backward_spin->setMinimum(1);
		backward_spin->setValue(1);

		centroid_method_combo = new QComboBox();
		centroid_method_combo->addItem("maximum intensity density");
		centroid_method_combo->addItem("center of mass");
		centroid_method_combo->addItem("local distance transformation");

		gridLayout = new QGridLayout();

		gridLayout->addWidget(factor_label,0,0);
		gridLayout->addWidget(factor_scroller,0,1,1,5);
		gridLayout->addWidget(factor_spin,0,6);
		gridLayout->addWidget(thresh_label,1,0);
		gridLayout->addWidget(thresh_scroller,1,1,1,5);
		gridLayout->addWidget(thresh_spin,1,6);
		gridLayout->addWidget(thick_label,2,0);
		gridLayout->addWidget(thick_scroller,2,1,1,5);
		gridLayout->addWidget(thick_spin,2,6);
		gridLayout->addWidget(forward_label,3,0);
		gridLayout->addWidget(forward_scroller,3,1,1,5);
		gridLayout->addWidget(forward_spin,3,6);
		gridLayout->addWidget(backward_label,4,0);
		gridLayout->addWidget(backward_scroller,4,1,1,5);
		gridLayout->addWidget(backward_spin,4,6);
		gridLayout->addWidget(centroid_method_label,5,0);
		gridLayout->addWidget(centroid_method_combo,5,1,1,5);
		gridLayout->addWidget(view3d_checker,5,6);
		gridLayout->addWidget(estimated_label,6,0,1,7);

		connect(factor_scroller, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(thresh_scroller, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(thick_scroller, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(forward_scroller, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(backward_scroller, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(view3d_checker, SIGNAL(stateChanged(int)), this, SLOT(update()));

		connect(factor_scroller,SIGNAL(valueChanged(int)), factor_spin,SLOT(setValue(int)));
		connect(factor_spin,SIGNAL(valueChanged(int)), factor_scroller,SLOT(setValue(int)));

		connect(thresh_scroller,SIGNAL(valueChanged(int)), thresh_spin,SLOT(setValue(int)));
		connect(thresh_spin,SIGNAL(valueChanged(int)), thresh_scroller,SLOT(setValue(int)));

		connect(thick_scroller,SIGNAL(valueChanged(int)), thick_spin,SLOT(setValue(int)));
		connect(thick_spin,SIGNAL(valueChanged(int)), thick_scroller,SLOT(setValue(int)));

		connect(forward_scroller,SIGNAL(valueChanged(int)), forward_spin,SLOT(setValue(int)));
		connect(forward_spin,SIGNAL(valueChanged(int)), forward_scroller,SLOT(setValue(int)));

		connect(backward_scroller,SIGNAL(valueChanged(int)), backward_spin,SLOT(setValue(int)));
		connect(backward_spin,SIGNAL(valueChanged(int)), backward_scroller,SLOT(setValue(int)));

		connect(centroid_method_combo, SIGNAL(valueChanged(int)), this, SLOT(update()));

		this->setLayout(gridLayout);
		this->setWindowTitle("Tagent Plane");

		direction = 0;
		tangent_win = callback.newImageWindow();
	}

	~TangentPlaneWidget(){}

public slots:
	void update();
	
public:
	double radius_factor;
	double threshold;
	double plane_thick;
	int forward_id;
	int backward_id;
	bool direction;
	LandmarkList landmarks;

	QLabel * factor_label;
	QLabel * thresh_label;
	QLabel * thick_label;
	QLabel * forward_label;
	QLabel * backward_label;
	QLabel * estimated_label;
	QLabel * centroid_method_label;

	QScrollBar * factor_scroller;
	QScrollBar * thresh_scroller;
	QScrollBar * thick_scroller;
	QScrollBar * forward_scroller;
	QScrollBar * backward_scroller;

	QSpinBox * factor_spin;
	QSpinBox * thresh_spin;
	QSpinBox * thick_spin;
	QSpinBox * forward_spin;
	QSpinBox * backward_spin;
	QComboBox * centroid_method_combo;
	
	QCheckBox * view3d_checker;

	QGridLayout * gridLayout;


	V3DPluginCallback2 * callback;
	v3dhandle curwin;
	v3dhandle tangent_win;
};

class TrackingWithoutBranchWidget : public QWidget
{
	Q_OBJECT

public:
	TrackingWithoutBranchWidget(V3DPluginCallback2 &callback, QWidget * parent) : QWidget(parent)
	{
		this->callback = &callback;
		curwin = callback.currentImageWindow();

		v3dhandleList win_list = callback.getImageWindowList();
		QStringList items;
		for(int i = 0; i < win_list.size(); i++) items << callback.getImageName(win_list[i]);
		landmarks = callback.getLandmark(curwin);

		factor_label = new QLabel(tr("radius factor (1 ~ 100)"));
		thresh_label = new QLabel(tr("image threshold (0 ~ 255)"));
		thick_label = new QLabel(tr("plane thickness (0 ~ 100)"));
		marker1_label = new QLabel(tr("marker1 (1 ~ %1)").arg(landmarks.size()));
		marker2_label = new QLabel(tr("marker2 (1 ~ %1)").arg(landmarks.size()));
		centroid_method_label = new QLabel(tr("centroid method"));
		direction_label = new QLabel(tr("trace direction"));

		view3d_checker = new QCheckBox("open view3d");

		factor_scroller = new QScrollBar(Qt::Horizontal);
		factor_scroller->setMaximum(100);
		factor_scroller->setValue(1);
		thresh_scroller = new QScrollBar(Qt::Horizontal);
		thresh_scroller->setMaximum(255);
		thresh_scroller->setMinimum(-1);
		thresh_scroller->setValue(-1);
		thick_scroller = new QScrollBar(Qt::Horizontal);
		thick_scroller->setMaximum(100);
		thick_scroller->setValue(0);
		marker1_scroller = new QScrollBar(Qt::Horizontal);
		marker1_scroller->setMaximum(landmarks.size());
		marker1_scroller->setMinimum(1);
		marker1_scroller->setValue(1);
		marker2_scroller = new QScrollBar(Qt::Horizontal);
		marker2_scroller->setMaximum(landmarks.size());
		marker2_scroller->setMinimum(1);
		marker2_scroller->setValue(2);

		factor_spin = new QSpinBox();
		factor_spin->setMaximum(100);
		factor_spin->setValue(1);
		thresh_spin = new QSpinBox();
		thresh_spin->setMaximum(255);
		thresh_spin->setMinimum(-1);
		thresh_spin->setValue(-1);
		thick_spin = new QSpinBox();
		thick_spin->setMaximum(100);
		thick_spin->setValue(0);
		marker1_spin = new QSpinBox();
		marker1_spin->setMaximum(landmarks.size());
		marker1_spin->setMinimum(1);
		marker1_spin->setValue(1);
		marker2_spin = new QSpinBox();
		marker2_spin->setMaximum(landmarks.size());
		marker2_spin->setMinimum(1);
		marker2_spin->setValue(1);

		centroid_method_combo = new QComboBox();
		centroid_method_combo->addItem("maximum intensity density");
		centroid_method_combo->addItem("center of mass");
		centroid_method_combo->addItem("local distance transformation");

		direction_combo = new QComboBox();
		direction_combo->addItem("Forward Tracking");
		direction_combo->addItem("Backward Tracking");

		gridLayout = new QGridLayout();

		gridLayout->addWidget(factor_label,0,0);
		gridLayout->addWidget(factor_scroller,0,1,1,5);
		gridLayout->addWidget(factor_spin,0,6);
		gridLayout->addWidget(thresh_label,1,0);
		gridLayout->addWidget(thresh_scroller,1,1,1,5);
		gridLayout->addWidget(thresh_spin,1,6);
		gridLayout->addWidget(thick_label,2,0);
		gridLayout->addWidget(thick_scroller,2,1,1,5);
		gridLayout->addWidget(thick_spin,2,6);
		gridLayout->addWidget(marker1_label,3,0);
		gridLayout->addWidget(marker1_scroller,3,1,1,5);
		gridLayout->addWidget(marker1_spin,3,6);
		gridLayout->addWidget(marker2_label,4,0);
		gridLayout->addWidget(marker2_scroller,4,1,1,5);
		gridLayout->addWidget(marker2_spin,4,6);
		gridLayout->addWidget(centroid_method_label,5,0);
		gridLayout->addWidget(centroid_method_combo,5,1,1,2);
		gridLayout->addWidget(direction_label,5,4);
		gridLayout->addWidget(direction_combo,5,5,1,2);
		gridLayout->addWidget(view3d_checker,6,0);

		connect(factor_scroller, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(thresh_scroller, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(thick_scroller, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(view3d_checker, SIGNAL(stateChanged(int)), this, SLOT(update()));

		connect(factor_scroller,SIGNAL(valueChanged(int)), factor_spin,SLOT(setValue(int)));
		connect(factor_spin,SIGNAL(valueChanged(int)), factor_scroller,SLOT(setValue(int)));

		connect(thresh_scroller,SIGNAL(valueChanged(int)), thresh_spin,SLOT(setValue(int)));
		connect(thresh_spin,SIGNAL(valueChanged(int)), thresh_scroller,SLOT(setValue(int)));

		connect(thick_scroller,SIGNAL(valueChanged(int)), thick_spin,SLOT(setValue(int)));
		connect(thick_spin,SIGNAL(valueChanged(int)), thick_scroller,SLOT(setValue(int)));

		connect(marker1_scroller,SIGNAL(valueChanged(int)), marker1_spin,SLOT(setValue(int)));
		connect(marker1_spin,SIGNAL(valueChanged(int)), marker1_scroller,SLOT(setValue(int)));

		connect(marker2_scroller,SIGNAL(valueChanged(int)), marker2_spin,SLOT(setValue(int)));
		connect(marker2_spin,SIGNAL(valueChanged(int)), marker2_scroller,SLOT(setValue(int)));

		connect(centroid_method_combo, SIGNAL(valueChanged(int)), this, SLOT(update()));

		connect(direction_combo, SIGNAL(valueChanged(int)), this, SLOT(update()));

		this->setLayout(gridLayout);
		this->setWindowTitle("Tracking without branch");

		direction = 0;
	}

	~TrackingWithoutBranchWidget(){}

public slots:
	void update();
	
public:
	double radius_factor;
	double threshold;
	double plane_thick;
	int marker1_id;
	int marker2_id;
	bool direction;
	LandmarkList landmarks;
	int centroid_method_id;

	QLabel * factor_label;
	QLabel * thresh_label;
	QLabel * thick_label;
	QLabel * marker1_label;
	QLabel * marker2_label;
	QLabel * centroid_method_label;
	QLabel * direction_label;

	QScrollBar * factor_scroller;
	QScrollBar * thresh_scroller;
	QScrollBar * thick_scroller;
	QScrollBar * marker1_scroller;
	QScrollBar * marker2_scroller;

	QSpinBox * factor_spin;
	QSpinBox * thresh_spin;
	QSpinBox * thick_spin;
	QSpinBox * marker1_spin;
	QSpinBox * marker2_spin;
	QComboBox * centroid_method_combo;
	QComboBox * direction_combo;
	
	QCheckBox * view3d_checker;


	QGridLayout * gridLayout;


	V3DPluginCallback2 * callback;
	v3dhandle curwin;
};
#endif

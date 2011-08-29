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
		df_checker = new QCheckBox("df");
		out_thresh_type_label = new QLabel(tr("out thresh"));
		out_thresh_type_combo = new QComboBox();
		out_thresh_type_combo->addItem("manually");
		out_thresh_type_combo->addItem("average");
		out_thresh_type_combo->addItem("otsu");

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

		out_thresh_spin = new QSpinBox();
		out_thresh_spin->setMaximum(255);
		out_thresh_spin->setMinimum(0);
		out_thresh_spin->setValue(0);

		refresh_button = new QPushButton("refresh");

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
		gridLayout->addWidget(out_thresh_type_label,6,0);
		gridLayout->addWidget(out_thresh_type_combo,6,1);
		gridLayout->addWidget(out_thresh_spin,6,2);
		gridLayout->addWidget(df_checker,6,3);
		gridLayout->addWidget(refresh_button,6,4);
		gridLayout->addWidget(estimated_label,7,0,1,7);

		connect(factor_scroller, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(thresh_scroller, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(thick_scroller, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(forward_scroller, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(backward_scroller, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(view3d_checker, SIGNAL(stateChanged(int)), this, SLOT(update()));
		connect(df_checker, SIGNAL(stateChanged(int)), this, SLOT(update()));
		connect(out_thresh_type_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));
		connect(out_thresh_spin, SIGNAL(valueChanged(int)), this, SLOT(update()));

		connect(factor_scroller,SIGNAL(valueChanged(int)), factor_spin,SLOT(setValue(int)));
		//connect(factor_spin,SIGNAL(valueChanged(int)), factor_scroller,SLOT(setValue(int)));

		connect(thresh_scroller,SIGNAL(valueChanged(int)), thresh_spin,SLOT(setValue(int)));
		//connect(thresh_spin,SIGNAL(valueChanged(int)), thresh_scroller,SLOT(setValue(int)));

		connect(thick_scroller,SIGNAL(valueChanged(int)), thick_spin,SLOT(setValue(int)));
		//connect(thick_spin,SIGNAL(valueChanged(int)), thick_scroller,SLOT(setValue(int)));

		connect(forward_scroller,SIGNAL(valueChanged(int)), forward_spin,SLOT(setValue(int)));
		//connect(forward_spin,SIGNAL(valueChanged(int)), forward_scroller,SLOT(setValue(int)));

		connect(backward_scroller,SIGNAL(valueChanged(int)), backward_spin,SLOT(setValue(int)));
		//connect(backward_spin,SIGNAL(valueChanged(int)), backward_scroller,SLOT(setValue(int)));

		connect(centroid_method_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));
		connect(refresh_button, SIGNAL(clicked()), this, SLOT(update()));

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
	int out_thresh_type;
	double out_thresh;
	LandmarkList landmarks;
	bool is_df;

	QLabel * factor_label;
	QLabel * thresh_label;
	QLabel * thick_label;
	QLabel * forward_label;
	QLabel * backward_label;
	QLabel * estimated_label;
	QLabel * centroid_method_label;
	QLabel * out_thresh_type_label;

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
	QCheckBox * df_checker;
	QComboBox * out_thresh_type_combo;
	QSpinBox * out_thresh_spin;

	QPushButton * refresh_button;

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
		tangent_radius_label = new QLabel(tr("tangent radius"));
		thick_label = new QLabel(tr("plane thickness (0 ~ 100)"));
		marker1_label = new QLabel(tr("marker1 (1 ~ %1)").arg(landmarks.size()));
		marker2_label = new QLabel(tr("marker2 (1 ~ %1)").arg(landmarks.size()));
		centroid_method_label = new QLabel(tr("centroid method"));
		direction_label = new QLabel(tr("trace direction"));

		view3d_checker = new QCheckBox("open view3d");
		display_temp_points_checker = new QCheckBox("temp points");
		refresh_button = new QPushButton("refresh");

		factor_scroller = new QScrollBar(Qt::Horizontal);
		factor_scroller->setMaximum(100);
		factor_scroller->setValue(1);
		thresh_scroller = new QScrollBar(Qt::Horizontal);
		thresh_scroller->setMaximum(255);
		thresh_scroller->setMinimum(-1);
		thresh_scroller->setValue(-1);
		thresh_type_combo = new QComboBox();
		thresh_type_combo->addItem("manually global threshold");
		thresh_type_combo->addItem("local average threshold");
		thresh_type_combo->addItem("local otsu threshold");
		tangent_radius_scroller = new QScrollBar(Qt::Horizontal);
		tangent_radius_scroller->setRange(1,100);
		tangent_radius_scroller->setValue(5);
		tangent_radius_type_combo = new QComboBox();
		tangent_radius_type_combo->addItem("manually global radius");
		tangent_radius_type_combo->addItem("automatically local radius");
		thick_scroller = new QScrollBar(Qt::Horizontal);
		thick_scroller->setMaximum(100);
		thick_scroller->setValue(0);
		marker1_combo = new QComboBox();
		for(int i = 0; i < landmarks.size(); i++) marker1_combo->addItem(tr("%1").arg(i+1));
		marker2_combo = new QComboBox();
		for(int i = 0; i < landmarks.size(); i++) marker2_combo->addItem(tr("%1").arg(i+1));

		factor_spin = new QSpinBox();
		factor_spin->setMaximum(100);
		factor_spin->setValue(1);
		thick_spin = new QSpinBox();
		thick_spin->setMaximum(100);
		thick_spin->setValue(0);

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
		gridLayout->addWidget(thresh_type_combo,1,6);
		gridLayout->addWidget(tangent_radius_label,2,0);
		gridLayout->addWidget(tangent_radius_scroller,2,1,1,5);
		gridLayout->addWidget(tangent_radius_type_combo,2,6);
		gridLayout->addWidget(thick_label,3,0);
		gridLayout->addWidget(thick_scroller,3,1,1,5);
		gridLayout->addWidget(thick_spin,3,6);
		gridLayout->addWidget(marker1_label,4,0);
		gridLayout->addWidget(marker1_combo,4,1,1,2);
		gridLayout->addWidget(marker2_label,4,4);
		gridLayout->addWidget(marker2_combo,4,5,1,2);
		gridLayout->addWidget(centroid_method_label,5,0);
		gridLayout->addWidget(centroid_method_combo,5,1,1,2);
		gridLayout->addWidget(direction_label,5,4);
		gridLayout->addWidget(direction_combo,5,5,1,2);
		gridLayout->addWidget(view3d_checker,6,0);
		gridLayout->addWidget(display_temp_points_checker,6,1);
		gridLayout->addWidget(refresh_button,6,2);

		connect(factor_scroller, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(thresh_scroller, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(thresh_type_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));
		connect(tangent_radius_scroller, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(tangent_radius_type_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));
		connect(thick_scroller, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(view3d_checker, SIGNAL(stateChanged(int)), this, SLOT(update()));
		connect(display_temp_points_checker, SIGNAL(stateChanged(int)), this, SLOT(update()));
		connect(refresh_button, SIGNAL(clicked()), this, SLOT(update()));

		connect(factor_scroller,SIGNAL(valueChanged(int)), factor_spin,SLOT(setValue(int)));

		connect(thick_scroller,SIGNAL(valueChanged(int)), thick_spin,SLOT(setValue(int)));

		connect(centroid_method_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));
		//connect(marker1_combo, SIGNAL(valueChanged(int)), this, SLOT(update()));
		//connect(marker2_combo, SIGNAL(valueChanged(int)), this, SLOT(update()));

		connect(direction_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));

		this->setLayout(gridLayout);
		this->setWindowTitle("Tracking without branch");

		direction = 0;
	}

	~TrackingWithoutBranchWidget(){}

public slots:
	void update();
	
public:
	double radius_factor;
	double global_tangent_radius;
	double threshold;
	double plane_thick;
	int marker1_id;
	int marker2_id;
	bool direction;
	bool is_display_temp_points;
	int centroid_method_id;
	int out_thresh_type;
	LandmarkList landmarks;
	int thresh_method;

	QLabel * factor_label;
	QLabel * thresh_label;
	QLabel * tangent_radius_label;
	QLabel * thick_label;
	QLabel * marker1_label;
	QLabel * marker2_label;
	QLabel * centroid_method_label;
	QLabel * direction_label;

	QScrollBar * factor_scroller;
	QScrollBar * thresh_scroller;
	QScrollBar * tangent_radius_scroller;
	QScrollBar * thick_scroller;
	QComboBox * marker1_combo;
	QComboBox * marker2_combo;

	QSpinBox * factor_spin;
	//QSpinBox * thresh_spin;
	QSpinBox * thick_spin;
	QComboBox * centroid_method_combo;
	QComboBox * direction_combo;
	QComboBox * thresh_type_combo;
	QComboBox * tangent_radius_type_combo;
	
	QCheckBox * view3d_checker;
	QCheckBox * display_temp_points_checker;
	QPushButton * refresh_button;


	QGridLayout * gridLayout;


	V3DPluginCallback2 * callback;
	v3dhandle curwin;
};
#endif

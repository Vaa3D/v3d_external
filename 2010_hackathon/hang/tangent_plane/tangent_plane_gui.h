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
		gridLayout->addWidget(view3d_checker,5,0);
		gridLayout->addWidget(estimated_label,6,0,1,6);

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
	
	QCheckBox * view3d_checker;

	QGridLayout * gridLayout;


	V3DPluginCallback2 * callback;
	v3dhandle curwin;
	v3dhandle tangent_win;
};

#endif

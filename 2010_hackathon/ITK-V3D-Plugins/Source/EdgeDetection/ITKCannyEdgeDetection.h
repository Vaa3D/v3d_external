/* ITKCannyEdgeDetection.h
 * 2010-06-02: create this program by Yang Yu
 */

#ifndef __ITKCANNYEDGEDETECTION_H__
#define __ITKCANNYEDGEDETECTION_H__

//   Canny edge detection.
//

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>

class ITKCannyEdgeDetectionPlugin : public QObject, public V3DPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface);
	
public:
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent);
	
	QStringList funclist() const {return QStringList();}
	void dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, QWidget *parent) {}
	
};

class ITKCannyEdgeDetectionDialog : public QDialog
{
	Q_OBJECT
	
public:
	ITKCannyEdgeDetectionDialog(V3DPluginCallback &callback, QWidget *parent)
	{
		Image4DSimple* p4DImage = callback.getImage(callback.currentImageWindow());
		
		if (! p4DImage) return;
		
		printf("[ITKCannyEdgeDetectionDialog] Passing data to data1d\n");
		
		ok     = new QPushButton("OK");
		cancel = new QPushButton("Cancel");

                labelLowerThreshold = new QLabel();
                labelLowerThreshold->setObjectName(QString::fromUtf8("LowerThreshold"));
                labelLowerThreshold->setText(QApplication::translate("MainWindow", "Lower Threshold", 0, QApplication::UnicodeUTF8));
                sbLowerThreshold = new QSpinBox();
                sbLowerThreshold->setObjectName(QString::fromUtf8("sbLowerThreshold"));
                sbLowerThreshold->setGeometry(QRect(130, 60, 55, 27));
                sbLowerThreshold->setMaximum(255);
		
                labelUpperThreshold = new QLabel();
                labelUpperThreshold->setObjectName(QString::fromUtf8("UpperThreshold"));
                labelUpperThreshold->setText(QApplication::translate("MainWindow", "Upper Threshold", 0, QApplication::UnicodeUTF8));
                sbUpperThreshold = new QSpinBox();
                sbUpperThreshold->setObjectName(QString::fromUtf8("sbUpperThreshold"));
                sbUpperThreshold->setGeometry(QRect(130, 60, 55, 27));
                sbUpperThreshold->setMaximum(255);

		gridLayout = new QGridLayout();
		
		gridLayout->addWidget(cancel, 0,0);
                gridLayout->addWidget(ok, 0,1);
                gridLayout->addWidget(labelLowerThreshold, 1,0);
                gridLayout->addWidget(sbLowerThreshold, 1,1);
                gridLayout->addWidget(labelUpperThreshold, 2,0);
                gridLayout->addWidget(sbUpperThreshold, 2,1);
		setLayout(gridLayout);
		setWindowTitle(QString("Canny Edge Detection"));
		
		connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
	}
	
	~ITKCannyEdgeDetectionDialog(){}
	
public slots:
	
	
public:
	QGridLayout *gridLayout;
	
	QPushButton* ok;
	QPushButton* cancel;
        QLabel* labelLowerThreshold;
        QLabel* labelUpperThreshold;
        QSpinBox* sbLowerThreshold;
        QSpinBox* sbUpperThreshold;
};


#endif


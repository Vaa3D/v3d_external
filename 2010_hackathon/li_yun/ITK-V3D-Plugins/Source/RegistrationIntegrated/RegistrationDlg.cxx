#include <QtGui>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "RegistrationDlg.h"
#include "Registration.h"

RegistrationDlg::RegistrationDlg()
{
	QGroupBox *RegistrationGroup = new QGroupBox(tr("Registration Configure"));
	TransformLabel=new QLabel(tr("Transform:"));
	TransformMethod=new QComboBox;
	TransformMethod->addItem(tr("VersorRigid3DTransform"));
	TransformMethod->addItem(tr("TranslationTransform"));
	TransformMethod->addItem(tr("AffineTransform"));
	TransformMethod->addItem(tr("BSplineDeformableTransform"));

	InterpolateLabel=new QLabel(tr("Interpolate:"));
	InterpolateMethod=new QComboBox;
        InterpolateMethod->addItem(tr("LinearInterpolate"));
        InterpolateMethod->addItem(tr("NearestNeighborInterpolate"));
        InterpolateMethod->addItem(tr("BSplineInterpolate"));

	MetricLabel=new QLabel(tr("Metric:"));
	MetricMethod=new QComboBox;
        MetricMethod->addItem(tr("MeanSquaresMetric"));
        MetricMethod->addItem(tr("GradientDifferenceMetric"));
        //MetricMethod->addItem(tr("NormalizedMutualInformationHistogramMetric"));
        MetricMethod->addItem(tr("MatchCardinalityMetric"));
        MetricMethod->addItem(tr("MattesMutualInformationMetric"));
        MetricMethod->addItem(tr("MutualInformationMetric"));

	OptimizerLabel=new QLabel(tr("Optimizer:"));
	OptimizerMethod=new QComboBox;
        if(TransformMethod->currentText() == "VersorRigid3DTransform")
            OptimizerMethod->addItem(tr("VersorRigid3DTransformOptimizer"));
        else
        {
            OptimizerMethod->addItem(tr("RegularStepGradientDescentOptimizer"));
            OptimizerMethod->addItem(tr("GradientDescentOptimizer"));
            OptimizerMethod->addItem(tr("VersorRigid3DTransformOptimizer"));
            OptimizerMethod->addItem(tr("AmoebaOptimizer"));
            OptimizerMethod->addItem(tr("OnePlusOneEvolutionaryOptimizer"));
        }


        QGridLayout *grouplayout=new QGridLayout;
        grouplayout->setVerticalSpacing(12);
        grouplayout->addWidget(TransformLabel,1,0);
        grouplayout->addWidget(TransformMethod,1,1);
        grouplayout->addWidget(InterpolateLabel,2,0);
        grouplayout->addWidget(InterpolateMethod,2,1);
        grouplayout->addWidget(MetricLabel,3,0);
        grouplayout->addWidget(MetricMethod,3,1);
        grouplayout->addWidget(OptimizerLabel,4,0);
        grouplayout->addWidget(OptimizerMethod,4,1);
	RegistrationGroup->setLayout(grouplayout);

        RegSelectLabel=new QLabel(tr("Configure Result:"));
        RegSelectList=new QListWidget;
        QVBoxLayout *selectlayout=new QVBoxLayout;
        selectlayout->addWidget(RegSelectLabel);
        selectlayout->addWidget(RegSelectList);

	RegTypeLabel=new QLabel(tr("RegistrationType:"));
	RegType=new QComboBox;
        RegType->addItem(tr("ImageRegistrationMethod"));
        RegType->addItem(tr("MultiResolutionImageRegistrationMethod"));
	StartBtn=new QPushButton(tr("Start"));
        QGridLayout *reglayout=new QGridLayout;
	reglayout->addWidget(RegTypeLabel,0,0);
	reglayout->addWidget(RegType,0,1);
	reglayout->addWidget(StartBtn,1,1);


        QVBoxLayout *leftlayout = new QVBoxLayout;
        leftlayout->addWidget(RegistrationGroup);
        leftlayout->addLayout(selectlayout);
        leftlayout->addLayout(reglayout);

	QGroupBox *FinalParameterGroup = new QGroupBox(tr("Final Parameters:"));
	FinalParameterLabel=new QLabel(tr("Result:"));
	FinalParameterEdit=new QTextEdit;
	QVBoxLayout *resultlayout=new QVBoxLayout;
	resultlayout->addWidget(FinalParameterLabel);
        resultlayout->addWidget(FinalParameterEdit);
        FinalParameterGroup->setLayout(resultlayout);

	SubtractBtn=new QPushButton(tr("Subtract"));
	ExitBtn=new QPushButton(tr("Exit"));
	QGridLayout* buttonlayout=new QGridLayout;
	buttonlayout->addWidget(SubtractBtn,0,1);
        buttonlayout->addWidget(ExitBtn,1,1);

        QVBoxLayout *rightlayout=new QVBoxLayout;
        rightlayout->addWidget(FinalParameterGroup);
        rightlayout->addLayout(buttonlayout);

        QHBoxLayout *layout=new QHBoxLayout;
        layout->addLayout(leftlayout);
        layout->addLayout(rightlayout);
        layout->setSpacing(20);
	this->setLayout(layout);    

        connect(StartBtn,SIGNAL(clicked()),this,SLOT(Start()));
	connect(SubtractBtn,SIGNAL(clicked()),this,SLOT(Subtract()));
        connect(ExitBtn,SIGNAL(clicked()),this,SLOT(close()));
        connect(TransformMethod,SIGNAL(currentIndexChanged(int)),this,SLOT(updateOptim(int)));

        connect(TransformMethod,SIGNAL(currentIndexChanged(int)),this,SLOT(updateConfig()));
        connect(InterpolateMethod,SIGNAL(currentIndexChanged(int)),this,SLOT(updateConfig()));
        connect(MetricMethod,SIGNAL(currentIndexChanged(int)),this,SLOT(updateConfig()));
        connect(OptimizerMethod,SIGNAL(currentIndexChanged(int)),this,SLOT(updateConfig()));

        transform = getTransformType();
        interpolator =  getInterpolateType() + "ImageFunction";
        QString mtc = getMetricType();
        metric = mtc.mid(0,mtc.size()-6) + "ImageToImageMetric";
        optimizer = getOptimizerType();
        reg_str = RegType->currentText();

        RegSelectList->clear();
        QString str = "You have selected:";
        RegSelectList->addItem(str);
        RegSelectList->addItem(transform);
        RegSelectList->addItem(interpolator);
        RegSelectList->addItem(metric);
        RegSelectList->addItem(optimizer);
        RegSelectList->addItem(reg_str);
}
void RegistrationDlg::updateOptim(int i)
{
    std::cout<<"i: "<<i<<std::endl;
    if(i == 0)
    {
        OptimizerMethod->clear();
        OptimizerMethod->addItem(tr("VersorRigid3DTransformOptimizer"));
    }
    else
    {
        OptimizerMethod->clear();
        OptimizerMethod->addItem(tr("RegularStepGradientDescentOptimizer"));
        OptimizerMethod->addItem(tr("GradientDescentOptimizer"));
        OptimizerMethod->addItem(tr("VersorRigid3DTransformOptimizer"));
        OptimizerMethod->addItem(tr("AmoebaOptimizer"));
        OptimizerMethod->addItem(tr("OnePlusOneEvolutionaryOptimizer"));
    }
}
void RegistrationDlg::updateConfig()
{
    transform = getTransformType();
    interpolator =  getInterpolateType() + "ImageFunction";
    QString mtc = getMetricType();
    metric = mtc.mid(0,mtc.size()-6) + "ImageToImageMetric";
    optimizer = getOptimizerType();
    reg_str = RegType->currentText();

    RegSelectList->clear();
    QString str = "You have selected:";
    RegSelectList->addItem(str);
    RegSelectList->addItem(transform);
    RegSelectList->addItem(interpolator);
    RegSelectList->addItem(metric);
    RegSelectList->addItem(optimizer);
    RegSelectList->addItem(reg_str);
}
void RegistrationDlg::SetCallback( V3DPluginCallback &callback)
{
	this->callback=&callback;
}

QString RegistrationDlg::getTransformType()
{
	return TransformMethod->currentText();	 
}  
QString RegistrationDlg::getInterpolateType()
{
	return InterpolateMethod->currentText();	 
}
QString RegistrationDlg::getMetricType()
{
	return MetricMethod->currentText();	 
}
QString RegistrationDlg::getOptimizerType()
{
	return OptimizerMethod->currentText();
}
void RegistrationDlg::Start()
{
     QString transform = getTransformType();
     QString interpolate = getInterpolateType();
     QString metric = getMetricType();
     QString optimizer = getOptimizerType();
     QString reg_str = RegType->currentText();	   
     
     PluginSpecialized<unsigned char> runner(this->callback,transform,interpolate,metric,optimizer,reg_str);

     runner.Execute(0);
}
void RegistrationDlg::Subtract()
{
     PluginSpecialized<unsigned char> runner(this->callback);
     runner.ImageSubtract();
     
}
void RegistrationDlg::Exit()
{}

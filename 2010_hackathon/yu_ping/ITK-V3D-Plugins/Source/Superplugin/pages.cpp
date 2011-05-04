
#include <QtGui>
#include <stdio.h>

#include "pages.h"
#include "Superplugin.h"


QString picName=":/images/app.png";

AutoPipePage::AutoPipePage(QWidget *parent):QWidget(parent)
{
    QGroupBox *pipelineGroup = new QGroupBox(tr("Example of Segmentation Pipeline "));

    pipelineName=new QLabel(tr("Segmentation Pipeline:"));
    pipelineExp=new QComboBox;
    pipelineExp->addItem(tr("Example Pipeline1"));
    pipelineExp->addItem(tr("Example Pipeline2"));
    pipelineExp->addItem(tr("Example Pipeline3"));
    pipelineExp->addItem(tr("Example Pipeline4"));
    pipelineExp->addItem(tr("Example Pipeline5"));

    QHBoxLayout *serverLayout = new QHBoxLayout;
    serverLayout->addWidget(pipelineName);
    serverLayout->addWidget(pipelineExp);

    StartPipe=new QPushButton(tr("Start the Pipeline"));

    PaintFilter=new QListWidget;
    PaintFilter->setViewMode(QListView::IconMode);
    PaintFilter->setIconSize(QSize(100, 100));
    PaintFilter->setMovement(QListView::Static);
    PaintFilter->setMaximumHeight(250);
    //PaintFilter->setSpacing(12);
    //PaintFilter->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //PaintFilter->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    //PaintFilter->setFlow(QListView::LeftToRight);
    //PaintFilter->setLayoutDirection(Qt::LeftToRight);
    QVBoxLayout *pipelineLayout=new QVBoxLayout;
    pipelineLayout->addWidget(PaintFilter);
    pipelineLayout->addSpacing(12);
    pipelineLayout->addLayout(serverLayout);
    pipelineLayout->addSpacing(12);
    pipelineLayout->addWidget(StartPipe);
    pipelineLayout->addSpacing(12);
    pipelineGroup->setLayout(pipelineLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(pipelineGroup);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
    this->CreateIcon();
    connect(StartPipe,SIGNAL(clicked()),this,SLOT(CallPipeline()));
    connect(pipelineExp,SIGNAL(currentIndexChanged(int)),this,SLOT(CreateIcon()));
}
void AutoPipePage::CallPipeline()
{
	QString end=".so";
	QString mid="/";
	PluginSpecialized<unsigned char> runner(callback);
    for(int i=0;i<PaintFilter->count();i++)
    {
	QString name="ITK/Superplugin/Plugin2Call/";
        QString plugin_name=PaintFilter->item(i)->text();
	//PluginSpecialized<unsigned char> runner(callback);
        name+=plugin_name;
	name+=mid;
	name+=plugin_name;
	name+=end;
	runner.AddPluginName(name);
	//runner.SetPluginName(name);
	//runner.Execute(menu_name,0);
    }
	runner.SetUsePipeline(true);
	runner.Execute(menu_name,0);	
}
void AutoPipePage::CreateIcon()
{
    PaintFilter->clear();
    int i=pipelineExp->currentIndex();
    if(i==0){QListWidgetItem * item=new QListWidgetItem;
        item->setIcon(QIcon(picName));
        item->setText(tr("Cast2Float"));
        item->setTextAlignment(Qt::AlignLeft);
        PaintFilter->addItem(item);
        QListWidgetItem * item1=new QListWidgetItem;
        item1->setIcon(QIcon(picName));
        item1->setText(tr("CurvatureFlow"));
        item1->setTextAlignment(Qt::AlignLeft);
        PaintFilter->addItem(item1);
        QListWidgetItem * item2=new QListWidgetItem;
        item2->setIcon(QIcon(picName));
        item2->setText(tr("ITKConfidenceConnected"));
        item2->setTextAlignment(Qt::AlignLeft);
        PaintFilter->addItem(item2);
        }
    else if(i==1){
        QListWidgetItem * item=new QListWidgetItem;
                item->setIcon(QIcon(picName));
                item->setText(tr("Cast2Float"));
                item->setTextAlignment(Qt::AlignLeft);
                PaintFilter->addItem(item);
                QListWidgetItem * item1=new QListWidgetItem;
                item1->setIcon(QIcon(picName));
                item1->setText(tr("CurvatureFlow"));
                item1->setTextAlignment(Qt::AlignLeft);
                PaintFilter->addItem(item1);
                QListWidgetItem * item2=new QListWidgetItem;
                item2->setIcon(QIcon(picName));
                item2->setText(tr("ITKConnectedThreshold"));
                item2->setTextAlignment(Qt::AlignLeft);
                PaintFilter->addItem(item2);
    }
    else if(i==2){
        QListWidgetItem * item=new QListWidgetItem;
                item->setIcon(QIcon(picName));
                item->setText(tr("Cast2Float"));
                item->setTextAlignment(Qt::AlignLeft);
                PaintFilter->addItem(item);
                QListWidgetItem * item1=new QListWidgetItem;
                item1->setIcon(QIcon(picName));
                item1->setText(tr("CurvatureFlow"));
                item1->setTextAlignment(Qt::AlignLeft);
                PaintFilter->addItem(item1);
                QListWidgetItem * item2=new QListWidgetItem;
                item2->setIcon(QIcon(picName));
                item2->setText(tr("ITKIsolatedConnected"));
                item2->setTextAlignment(Qt::AlignLeft);
                PaintFilter->addItem(item2);

    }
    else {
        QListWidgetItem * item=new QListWidgetItem;
                item->setIcon(QIcon(picName));
                item->setText(tr("Filter"));
                item->setTextAlignment(Qt::AlignLeft);
                PaintFilter->addItem(item);
                QListWidgetItem * item1=new QListWidgetItem;
                item1->setIcon(QIcon(picName));
                item1->setText(tr("Filter"));
                item1->setTextAlignment(Qt::AlignLeft);
                PaintFilter->addItem(item1);
    }
}
void AutoPipePage::SetCallback( V3DPluginCallback &callback)
{
	this->callback=&callback;
}

//=============================================================
UserPipePage::UserPipePage(QWidget *parent):QWidget(parent)
{
    QGroupBox *UserPipe=new QGroupBox(tr("User's Own Segmentation Pipeline"));
    PaintFilter=new QListWidget;
    PaintFilter->setViewMode(QListView::IconMode);
    PaintFilter->setIconSize(QSize(90, 90));
    PaintFilter->setMovement(QListView::Static);
    PaintFilter->setMaximumHeight(250);

    StartPipe=new QPushButton("Start PipeLine");
    SourceLabel=new QLabel(tr("Source Filter"));
    UsedLabel=new QLabel(tr("Used Filter"));

    AddItem=new QPushButton(tr("Add Filter"));
    ClearItem=new QPushButton(tr("Clear All"));

    SourceFilter=new QListWidget;
    SourceFilter->setMaximumWidth(170);
    QListWidgetItem *Filter1=new QListWidgetItem(SourceFilter);
    Filter1->setText(tr("BinaryThreshold"));
    QListWidgetItem *Filter2=new QListWidgetItem(SourceFilter);
    Filter2->setText(tr("Cast"));
    QListWidgetItem *Filter3=new QListWidgetItem(SourceFilter);
    Filter3->setText(tr("DiscreteGaussianFilter"));
    QListWidgetItem *Filter4=new QListWidgetItem(SourceFilter);
    Filter4->setText(tr("GradientMagnitudeRecursiveGaussian"));
    QListWidgetItem *Filter5=new QListWidgetItem(SourceFilter);
    Filter5->setText(tr("Kmeans"));
    QListWidgetItem *Filter6=new QListWidgetItem(SourceFilter);
    Filter6->setText(tr("MeanFilter"));
    QListWidgetItem *Filter7=new QListWidgetItem(SourceFilter);
    Filter7->setText(tr("MedianFilter"));

    QListWidgetItem *Filter8=new QListWidgetItem(SourceFilter);
    Filter8->setText(tr("RescaleIntensity"));
    QListWidgetItem *Filter9=new QListWidgetItem(SourceFilter);
    Filter9->setText(tr("Sigmoid"));


    UsedFilter=new QListWidget;
    UsedFilter->setMaximumWidth(170);

    QHBoxLayout *listLayout=new QHBoxLayout;
    listLayout->addWidget(SourceFilter);
    listLayout->addWidget(UsedFilter);
    QHBoxLayout *labelLayout=new QHBoxLayout;
    labelLayout->addWidget(SourceLabel);
    labelLayout->addWidget(UsedLabel);
    QHBoxLayout *buttonLayout=new QHBoxLayout;
    buttonLayout->addWidget(AddItem);
    buttonLayout->addWidget(ClearItem);

    QVBoxLayout *pipeLayout=new QVBoxLayout;
    pipeLayout->addWidget(PaintFilter);
    pipeLayout->addLayout(labelLayout);
    pipeLayout->addLayout(listLayout);
    pipeLayout->addLayout(buttonLayout);
    pipeLayout->addWidget(StartPipe);
    UserPipe->setLayout(pipeLayout);

    QVBoxLayout *mainLayout=new QVBoxLayout;
    mainLayout->addWidget(UserPipe);
    mainLayout->addStretch(1);
    setLayout(mainLayout);

    connect(AddItem,SIGNAL(clicked()),this,SLOT(ADDItem()));
    connect(ClearItem,SIGNAL(clicked()),UsedFilter,SLOT(clear()));
    connect(this,SIGNAL(countChanged()),this,SLOT(CreatIcon()));
    connect(ClearItem,SIGNAL(clicked()),PaintFilter,SLOT(clear()));
    connect(StartPipe,SIGNAL(clicked()),this,SLOT(CallPipeline()));
}
void UserPipePage::CallPipeline()
{
	QString end=".so";
	QString mid="/";
    for(int i=0;i<UsedFilter->count();i++)
    {
	QString name="ITK/Superplugin/Plugin2Call/";
        QString plugin_name=UsedFilter->item(i)->text();
	PluginSpecialized<unsigned char> runner(callback);
        name+=plugin_name;
	name+=mid;
	name+=plugin_name;
	name+=end;
	runner.SetPluginName(name);
	runner.Execute(menu_name,0);
	
    }	
}
void UserPipePage::ADDItem()
{
    UsedFilter->addItem(new QListWidgetItem(*SourceFilter->currentItem()));
    emit this->countChanged();
}
void UserPipePage::CreatIcon()
{
    PaintFilter->clear();
    for(int i=0;i<UsedFilter->count();i++)
    {
        QListWidgetItem * item=new QListWidgetItem;
        item->setIcon(QIcon(picName));
        item->setText(UsedFilter->item(i)->text());
        item->setTextAlignment(Qt::AlignLeft);
        PaintFilter->addItem(item);
    }
}
void UserPipePage::SetCallback( V3DPluginCallback &callback)
{
	this->callback=&callback;
}

//===============================================================
UserFilterPage::UserFilterPage(QWidget *parent):QWidget(parent)
{
    QGroupBox *UserFilter=new QGroupBox(tr("User's Own Filter"));
    FilterName=new QLabel(tr("Filter Name:"));
    FilterItem=new QComboBox;
    FilterItem->addItem(tr("BinaryThreshold"));
    FilterItem->addItem(tr("Cast2Float"));
    FilterItem->addItem(tr("DiscreteGaussianFilter"));
    FilterItem->addItem(tr("Erode"));
    FilterItem->addItem(tr("GradientMagnitudeRecursiveGaussian"));
    FilterItem->addItem(tr("Kmeans"));
    FilterItem->addItem(tr("MeanFilter"));
    FilterItem->addItem(tr("MedianFilter"));
    FilterItem->addItem(tr("RescaleIntensity"));
    FilterItem->addItem(tr("Sigmoid"));
    FilterItem->addItem(tr("ITKCannySegmentation"));
    FilterItem->addItem(tr("And"));
    FilterItem->addItem(tr("ITKThresholdSegmentation"));    				

    StartButton=new QPushButton(tr("Start Call Other Filter"));
    QHBoxLayout *FilterLayout=new QHBoxLayout;
    FilterLayout->addWidget(FilterName);
    FilterLayout->addWidget(FilterItem);

    QVBoxLayout *UserLayout=new QVBoxLayout;
    UserLayout->addLayout(FilterLayout);
    UserLayout->addSpacing(20);
    UserLayout->addWidget(StartButton);
    UserFilter->setLayout(UserLayout);

    QVBoxLayout *mainLayout=new QVBoxLayout;
    mainLayout->addWidget(UserFilter);
    mainLayout->addStretch(1);
    setLayout(mainLayout);

    connect(StartButton,SIGNAL(clicked()),this,SLOT(CallFilter()));

}
void UserFilterPage::CallFilter()
{
	QString name="ITK/Superplugin/Plugin2Call/";
	QString end=".so";
	QString mid="/";
	QString plugin_name=FilterItem->currentText();
	if(plugin_name=="ITKCannySegmentation"||plugin_name=="And")
	{
	PluginSpecializedForDual<unsigned char> runner(callback);
        name+=plugin_name;
	name+=mid;
	name+=plugin_name;
	name+=end;
	runner.SetPluginName(name);
	runner.Execute(menu_name,0);
	}
	else
	{ 
	PluginSpecialized<unsigned char> runner(callback);	
        name+=plugin_name;
	name+=mid;
	name+=plugin_name;
	name+=end;
	runner.SetPluginName(name);
	runner.Execute(menu_name,0);
	}
}

void UserFilterPage::SetCallback( V3DPluginCallback &callback)
{
	this->callback=&callback;
}

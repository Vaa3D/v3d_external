/**
 * wavelets.cpp
 *
 * Written by
 *
 * Ihor Smal
 * Nicolas Chenouard
 * Fabrice de Chaumont
 *
 * Paper reference: ISBI and JC ref
 *
 * This code is under GPL License
 *
 *
 */
#include <QtGui>

#include <math.h>
#include <stdlib.h>
#include <list>

#include "wavelets.h"
#include "scaleinfo.h"

Q_EXPORT_PLUGIN2(newwindow, WaveletPlugin);

const QString title = "Wavelets";

QStringList WaveletPlugin::menulist() const
{
    return QStringList()
    << tr("Wavelets");

}

/**
 * From Interface V3DPluginInterface. Called when user click wavelets in plugin menu.
 */
void WaveletPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
	initGUI( callback, parent );
}

WaveletPlugin::WaveletPlugin()
{
	scaleInfoList = new std::list<ScaleInfo*>();
}

void WaveletPlugin::addScaleButtonPressed()
{
	printf("WAVELETS : add scale 2 !\n");

	ScaleInfo *scaleInfo = new ScaleInfo( qBox );  // qbox
	scaleInfoList->push_back( scaleInfo );

	refreshScaleInterface();
}

void WaveletPlugin::removeScaleButtonPressed()
{
	printf("WAVELETS : remove scale !\n");
	if ( !scaleInfoList->empty() )
	{
		ListType::iterator lend = scaleInfoList->end();
		ScaleInfo *scaleInfo = (*lend);
		delete scaleInfo;
		scaleInfoList->pop_back( );
	}

	refreshScaleInterface();
}

void WaveletPlugin::refreshScaleInterface()
{
	printf("WAVELETS : refresh Scale interface\n");

	removeScaleButton->setDisabled( scaleInfoList->empty() );


	// remove everything in the layout.

	/*
	QLayoutItem *child;
	while ((child = ui->gbCoeffs->layout()->takeAt(0)) != 0) {
		ui->gbCoeffs->layout()->removeItem(child);

		delete child->widget();
		delete child;
	}
*/
	/*
	QLayoutItem *child;
	while ((child = ui->gbCoeffs->layout()->takeAt(0)) != 0)
	{
		formLayoutGroupBox
	}*/
	QLayoutItem *child;
	while ((child = formLayoutGroupBox->takeAt(0)) != 0) {
		formLayoutGroupBox->removeItem(child);

		}

	// write back all the scale interface

	ListType::iterator litr = scaleInfoList->begin();
	ListType::iterator lend = scaleInfoList->end();

	while( litr != lend )
	{
		printf("adding scale interface..");
		formLayoutGroupBox->addRow( (*litr)->groupBox );
		++litr;
	}

}

void WaveletPlugin::cancel()
{
	printf("WAVELETS : cancel !\n");

}

void WaveletPlugin::sliderChange(int value )
{
	printf("WAVELETS : slide change ! (value is : %i)\n" , value );

	if ( !sourceImage )
	{
		printf("WAVELETS : no source image.. \n");
		return;
	}

	if ( !sourceImage->getRawData() )
		{
			printf("WAVELETS : no data in source image.. \n");
			return;
		}

	unsigned char* imageRaw = sourceImage->getRawData();

	if ( !imageRaw )
	{
		printf("WAVELETS : no image.. \n");
		return;
	}

	printf("WAVELETS : enter1");

	for ( int i =0 ; i< 10000 ; i++ )
		{
		imageRaw[i] = value;
		}

	printf("WAVELETS : enter2");

	myCallback->updateImageWindow(sourceWindow);

	// This would crash ( Bug reported on NTRC V3D Bug tracker )
	//myCallback->setImage(sourceWindow, sourceImage );

}

void WaveletPlugin::initGUI( V3DPluginCallback &callback, QWidget *parent)
{
	// Building the main interface.

	myCallback = &callback;

	myDialog = new QDialog(parent);

	sourceWindow = callback.currentImageWindow();
	Image4DSimple* p4DImage = callback.getImage(sourceWindow);
	if (!p4DImage)
	{
		QMessageBox::information(0, "Cloning", QObject::tr("No image is open."));
		return;
	}

	sourceImage = p4DImage;

	QPushButton* ok     = new QPushButton("OK");

	QPushButton* addScaleButton     = new QPushButton("Add");
	removeScaleButton     = new QPushButton("Remove");

	QPushButton* cancel = new QPushButton("Cancel");
	formLayout = new QFormLayout;

	formLayout->addRow( removeScaleButton , addScaleButton  );

	QSlider *slider = new QSlider(Qt::Horizontal);
	     slider->setFocusPolicy(Qt::StrongFocus);
	     slider->setTickPosition(QSlider::TicksBothSides);
	     slider->setTickInterval(10);
	     slider->setSingleStep(1);
	     slider->setMaximum(255);

	 	formLayout->addRow( slider );

	QProgressBar *progressBar = new QProgressBar(  );
	progressBar->setRange(0,100);
	progressBar->setValue( 50 );

// 	formLayout->addRow( processButton );
 	formLayout->addRow( progressBar );

	//QLabel* label2 = new QLabel("Test");
 	qBox= new QGroupBox( myDialog );
	formLayoutGroupBox = new QFormLayout();
	qBox->setLayout( formLayoutGroupBox );
	//formLayoutGroupBox->addRow( label2 );

 	formLayout->addRow(qBox);

	formLayout->addRow(ok, cancel);
	myDialog->setLayout(formLayout);
	myDialog->setWindowTitle(QString("Wavelets"));

	myDialog->connect(ok,     SIGNAL(clicked()), myDialog, SLOT(accept()));
	myDialog->connect(slider, SIGNAL(valueChanged(int)), this, SLOT(sliderChange(int)));
	myDialog->connect(cancel, SIGNAL(clicked()), this, SLOT(cancel()));
	myDialog->connect(addScaleButton, SIGNAL(clicked()), this, SLOT(addScaleButtonPressed()));
	myDialog->connect(removeScaleButton, SIGNAL(clicked()), this, SLOT(removeScaleButtonPressed()));

	refreshScaleInterface();

	myDialog->exec();

}

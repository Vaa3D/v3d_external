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
#include <v3d_basicdatatype.h>
#include "../basic_c_fun/basic_landmark.h"

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
	printf("WAVELETS : add scale !\n");

	ScaleInfo *scaleInfo = new ScaleInfo( scaleInfoList->size() , qBox );
	scaleInfoList->push_back( scaleInfo );
	printf("%p" , scaleInfo );
	refreshScaleInterface();
}

void WaveletPlugin::removeScaleButtonPressed()
{
	printf("WAVELETS : remove scale !\n");
	if ( scaleInfoList->size() > 1 )
	{
		ScaleInfo *scaleInfo = scaleInfoList->back(); //(*lend);
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

	QLayoutItem *child;
	while ((child = formLayoutGroupBox->takeAt(0)) != 0) {
		formLayoutGroupBox->removeItem(child);
	}

//	qBox->adjustSize();
	myDialog->adjustSize();

	// write back all the scale interface

	ListType::iterator litr = scaleInfoList->begin();
	ListType::iterator lend = scaleInfoList->end();

	while( litr != lend )
	{
		printf("adding scale interface..");
		formLayoutGroupBox->addRow( (*litr)->groupBox );
		litr++;
	}



	myDialog->adjustSize();

}

/**
 * Cancel Action.
 */
void WaveletPlugin::cancel()
{
	printf("WAVELETS : cancel !\n");
	printf("WAVELETS : Restoring original image...\n");
	restoreOriginalImage();
	printf("WAVELETS : Restore done.\n");
	myDialog->close();
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

/**
 * Copy original image
 */
void WaveletPlugin::copyOriginalImage()
{
	Image4DSimple *sourceImage = myCallback->getImage( myCallback->currentImageWindow() );

	originalImageCopy = new Image4DSimple();
	unsigned char *bufferSource = sourceImage->getRawData();
	unsigned char *bufferCopy = new unsigned char[sourceImage->getTotalBytes()];
	memcpy( bufferCopy , bufferSource , sourceImage->getTotalBytes() );

	originalImageCopy->setData( bufferCopy , sourceImage->getXDim() , sourceImage->getYDim(),
			sourceImage->getZDim(), sourceImage->getCDim() , sourceImage->getDatatype()
			);

//	sourceImage = originalImageCopy;

//	myCallback->updateImageWindow( myCallback->currentImageWindow() );
/*
	originalImageCopy = new Image4DSimple();
	unsigned char *bufferSource = sourceImage->getRawData();
	unsigned char *bufferCopy = new unsigned char[sourceImage->getTotalBytes()];
	memcpy( bufferCopy , bufferSource , sourceImage->getTotalBytes() );

	printf( "size image : %d" , sourceImage->getTotalBytes() );

	originalImageCopy->setData( bufferCopy , sourceImage->getXDim() , sourceImage->getYDim(),
			sourceImage->getZDim(), sourceImage->getCDim() , sourceImage->getDatatype()
			);

	v3dhandle newWindow = myCallback->newImageWindow();
	myCallback->setImage( newWindow , originalImageCopy);
	myCallback->updateImageWindow(newWindow);
*/
}

/**
 * Restore original image.
 */
void WaveletPlugin::restoreOriginalImage()
{

	unsigned char *bufferSource = originalImageCopy->getRawData();
	unsigned char *bufferCopy = new unsigned char[originalImageCopy->getTotalBytes()];

	memcpy( bufferCopy , bufferSource , originalImageCopy->getTotalBytes() );

	printf("%d ", originalImageCopy->getTotalBytes() );

	sourceImage->setNewRawDataPointer( bufferCopy );

	/*
	sourceImage->setData(
			bufferCopy ,
			originalImageCopy->getXDim() ,
			originalImageCopy->getYDim(),
			originalImageCopy->getZDim(),
			originalImageCopy->getCDim() ,
			originalImageCopy->getDatatype()
			);
*/

	myCallback->updateImageWindow(sourceWindow);

}


/**
 *	Init the GUI of the plugin.
 */
void WaveletPlugin::initGUI( V3DPluginCallback &callback, QWidget *parent )
{
	sourceWindow = callback.currentImageWindow();
	Image4DSimple* p4DImage = callback.getImage(sourceWindow);
	if (!p4DImage)
	{
		QMessageBox::information(0, "Cloning", QObject::tr("No image is open."));
		return;
	}

	sourceImage = p4DImage;

	myCallback = &callback;

	copyOriginalImage(); // for cancel purpose.

	// Building the main interface.



	myDialog = new QDialog(parent);

	QPushButton* ok     = new QPushButton("OK");

	QPushButton* addScaleButton     = new QPushButton("Add");
	removeScaleButton     = new QPushButton("Remove");

	QPushButton* cancel = new QPushButton("Cancel");
	formLayout = new QFormLayout( myDialog );

	formLayout->addRow( removeScaleButton , addScaleButton  );

	QSlider *slider = new QSlider(Qt::Horizontal);
	     slider->setFocusPolicy(Qt::StrongFocus);
	     slider->setTickPosition(QSlider::TicksBothSides);
	     slider->setTickInterval(10);
	     slider->setSingleStep(1);
	     slider->setMaximum(255);

	 	formLayout->addRow( slider );

	progressBar = new QProgressBar(  );
	progressBar->setRange(0,100);
	progressBar->setValue( 100 );

// 	formLayout->addRow( processButton );
 	formLayout->addRow( progressBar );

	//QLabel* label2 = new QLabel("Test");
 	qBox= new QGroupBox( myDialog );
	formLayoutGroupBox = new QFormLayout( qBox );
	//qBox->setLayout( formLayoutGroupBox );
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

	for (int i = 0 ; i < 4 ; i++ ) // add some scales.
	{
		addScaleButtonPressed();
	}

	QPushButton* dev1Button = new QPushButton("Dev #1");
	QPushButton* dev2Button = new QPushButton("Dev #2");
	QPushButton* dev3Button = new QPushButton("Dev #3");
	QPushButton* dev4Button = new QPushButton("Dev #4");

	QPushButton* denoiseButton = new QPushButton("Denoise");
	QPushButton* detectSpotsButton = new QPushButton("Detect spots");

	formLayout->addRow( denoiseButton, detectSpotsButton );

	formLayout->addRow(dev1Button, dev2Button);
	formLayout->addRow(dev3Button, dev4Button);

	myDialog->connect(dev1Button, SIGNAL(clicked()), this, SLOT(dev1ButtonPressed()));
	myDialog->connect(dev2Button, SIGNAL(clicked()), this, SLOT(dev2ButtonPressed()));
	myDialog->connect(dev3Button, SIGNAL(clicked()), this, SLOT(dev3ButtonPressed()));
	myDialog->connect(dev4Button, SIGNAL(clicked()), this, SLOT(dev4ButtonPressed()));

	myDialog->connect(denoiseButton, SIGNAL(clicked()), this, SLOT(denoiseButtonPressed()));
	myDialog->connect(detectSpotsButton, SIGNAL(clicked()), this, SLOT(detectSpotsButtonPressed()));





	myDialog->exec();



}

/**
 * Denoise
 */
void WaveletPlugin::denoiseButtonPressed()
{
	printf("WAVELET : denoise pressed\n");

	// TODO: denoise code here

	printf("WAVELET : denoise finished\n");
}

/**
 * Detect spots
 */
void WaveletPlugin::detectSpotsButtonPressed()
{
	printf("WAVELET : detection spots pressed\n");
	printf("test\n");

	// building landmark test ( crashy )

	LandmarkList list;

	LocationSimple *ls = new LocationSimple( 10 , 10 , 10 );

	list.push_back( *ls );

	myCallback->setLandmark( sourceWindow , list );

		// TODO: detection code here

	printf("WAVELET : detection spots finished\n");
}

// This is a dev button. Use it for test purpose !
// User : nobody
// Should do : nothing
void WaveletPlugin::dev1ButtonPressed()
{
	printf("dev 1 pressed\n");
	// use myCallback if you need the one provide by
	// WaveletPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)	//
	printf("dev 1 finished\n");
}

// This is a dev button. Use it for test purpose !
// User : nobody
// Should do : nothing
void WaveletPlugin::dev2ButtonPressed()
{
	printf("dev 2 pressed\n");
	// use myCallback if you need the one provide by
	// WaveletPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)	//

	printf("dev 2 finished\n");
}

// This is a dev button. Use it for test purpose !
// User : nobody
// Should do : nothing
void WaveletPlugin::dev3ButtonPressed()
{
	printf("dev 3 pressed\n");
	// use myCallback if you need the one provide by
	// WaveletPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)	//
	printf("dev 3 finished\n");
}

// This is a dev button. Use it for test purpose !
// User : nobody
// Should do : nothing
void WaveletPlugin::dev4ButtonPressed()
{
	printf("dev 4 pressed\n");
	// use myCallback if you need the one provide by
	// WaveletPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)	//
	printf("dev 4 finished\n");
}













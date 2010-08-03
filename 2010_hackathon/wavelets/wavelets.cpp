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
#include "waveletConfigException.h"
#include <iostream>


Q_EXPORT_PLUGIN2(newwindow, WaveletPlugin);

const QString title = "Wavelets";

QStringList WaveletPlugin::menulist() const
{
    return QStringList()<< tr("Wavelets")<< tr("Cloning")<<tr("FFT")<<tr("Wavelet Transform");


}

/**
 * From Interface V3DPluginInterface. Called when user click wavelets in plugin menu.
 */
void WaveletPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
	if (menu_name == tr("Wavelets"))
	{
		initGUI( callback, parent );
	}
	if (menu_name == tr("Cloning"))
    {
    	Cloning(callback, parent);
    }
    if (menu_name == tr("FFT"))
    {
#if USING_FFT
    	FFT(callback, parent);
#endif
    }
    if (menu_name == tr("Wavelet Transform"))
    {
    	WaveletTransform(callback, parent);
    }
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
//	Image4DSimple *sourceImage = myCallback->getImage( myCallback->currentImageWindow() );
//	printf("p sourceimage %p \n", myCallback->getImage( myCallback->currentImageWindow() ) );

	originalImageCopy = new Image4DSimple();
	//printf("POINTER NULL? %b\n", originalImageCopy->getRawData()==NULL);
	unsigned char *bufferSource = sourceImage->getRawData();
//	printf("\n\n Bytes= %dl,  size %dl\n\n", sourceImage->getTotalBytes()*sizeof(unsigned char), sourceImage->sz0*sourceImage->sz1*sourceImage->sz2*sourceImage->sz3);
	unsigned char *bufferCopy = new unsigned char[sourceImage->getTotalBytes()];
	memcpy( bufferCopy , bufferSource , sourceImage->getTotalBytes() );
	
	//printf("buffer Copy %p", bufferCopy);
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
*/
//	v3dhandle newWindow = myCallback->newImageWindow();
//	myCallback->setImage( newWindow , originalImageCopy);
//	myCallback->updateImageWindow(newWindow);

}

/**
 * Restore original image.
 */
void WaveletPlugin::restoreOriginalImage()
{
//	unsigned char *bufferSource = originalImageCopy->getRawData();
//	unsigned char *bufferCopy = new unsigned char[originalImageCopy->getTotalBytes()];

	sourceImage->setXDim( originalImageCopy->getXDim() );
	sourceImage->setYDim( originalImageCopy->getYDim() );
	sourceImage->setZDim( originalImageCopy->getZDim() );
	sourceImage->setCDim( originalImageCopy->getCDim() );
	sourceImage->setDatatype( originalImageCopy->getDatatype() );

	memcpy( sourceImage->getRawData() , originalImageCopy->getRawData() , originalImageCopy->getTotalBytes() );

	printf("%d source" , sourceImage->getTotalBytes() );
	printf("%d original" , originalImageCopy->getTotalBytes() );

	myCallback->updateImageWindow(sourceWindow);
	//myCallback->setImage(sourceWindow, sourceImage);

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
	
	
	v3dhandle sourceWindow = myCallback->currentImageWindow();
	Image4DSimple* p4DImage = myCallback->getImage(sourceWindow);
	
	printf("\n POINTERS %p   |    %p\n\n", p4DImage, &sourceWindow);
	
	
	// 
// 	//use myCallback if you need the one provide by
// 	// WaveletPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)	//
// 	
// 	
// 	unsigned char* imageRaw = sourceImage->getRawData();
// 
// 	if ( !imageRaw )
// 	{
// 		printf("WAVELETS : no image.. \n");
// 		return;
// 	}
// 
// 	printf("WAVELETS : enter1");
// 
// 	for ( int i =0 ; i< 10000 ; i++ )
// 		{
// 		imageRaw[i] = 255;
// 		}
// 
// 	printf("WAVELETS : enter2");
// 
// 	myCallback->updateImageWindow(sourceWindow);
	
	printf("dev 4 finished\n");
}


#if USING_FFT
void WaveletPlugin::FFT(V3DPluginCallback &callback, QWidget *parent)
{
	v3dhandle oldwin = callback.currentImageWindow();
	Image4DSimple* p4DImage = callback.getImage(oldwin);
	if (!p4DImage)
	{
		QMessageBox::information(0, "Cloning", QObject::tr("No image is open."));
		return;
	}
	double* data1dD = channelToDoubleArray(p4DImage, 1);

	//get dims
    V3DLONG szx = p4DImage->getXDim();
    V3DLONG szy = p4DImage->getYDim();
    V3DLONG szz = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();
    V3DLONG N = szx * szy * szz;
  
	// FFTW
	//careful, fftw is using row major indexing = z dimension is varying first, while v3d is using column major indexing = x dimension is varying first
	//the last dimension is cut in half (x dimension for v3d convention)
	V3DLONG nxOut = ((szx / 2 + 1));
	V3DLONG nyOut = szy;
	V3DLONG nzOut = szz;
	V3DLONG nOut = 	nxOut * nyOut * nzOut;
	
	//create output array
	fftw_complex *out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nOut);
	
	//prepare execution plan
	fftw_plan p;
	time_t seconds0 = time (NULL);
	p = fftw_plan_dft_r2c_3d(szz, szy, szx, data1dD, out, FFTW_MEASURE);
	//p = fftw_plan_dft_r2c_2d(szy, szx, data1dD, out, FFTW_ESTIMATE);

 	time_t seconds1 = time (NULL);
	printf ("prepare %ld\n", seconds1-seconds0);

	//execute plan
	fftw_execute(p);
 	time_t seconds2 = time (NULL);
	printf ("execute %ld\n", seconds2-seconds1);

	
	double max = 0;
	double min = 0;	
	for (int i = 0; i < nOut; i++)
	{
 		double a0 = ((double*)out[i])[0];
 		double a1 = ((double*)out[i])[1];
 		data1dD[i] = log(sqrt(a0 * a0 + a1 * a1));
 		max = (max < data1dD[i]) ? data1dD[i] : max;	
 		min = (min > data1dD[i]) ? data1dD[i] : min;	
	}
	
	rescaleForDisplay(data1dD, data1dD, nOut, p4DImage->datatype);
	
	//free memory
	fftw_destroy_plan(p);
	fftw_free(out);
	
	// output image 
	Image4DSimple outImage;
	unsigned char* dataOut1d = doubleArrayToCharArray(data1dD, nOut, p4DImage->datatype);
    outImage.setData(dataOut1d, nxOut, p4DImage->sz1, p4DImage->sz2, 1, p4DImage->datatype);
    v3dhandle newwin = callback.newImageWindow();
	callback.setImage(newwin, &outImage);
	callback.setImageName(newwin,"fft test");
    callback.updateImageWindow(newwin);
	free(data1dD);
}
#endif

void WaveletPlugin::Cloning(V3DPluginCallback &callback, QWidget *parent)
{
	v3dhandle oldwin = callback.currentImageWindow();
	Image4DSimple* p4DImage = callback.getImage(oldwin);
	if (!p4DImage)
	{
		QMessageBox::information(0, "Cloning", QObject::tr("No image is open."));
		return;
	}
	double* data1dD = channelToDoubleArray(p4DImage, 1);

	//get dims
    V3DLONG szx = p4DImage->getXDim();
    V3DLONG szy = p4DImage->getYDim();
    V3DLONG szz = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();
    V3DLONG N = szx * szy * szz;


	Image4DSimple outImage;
	unsigned char* dataOut1d = doubleArrayToCharArray(data1dD, N, p4DImage->datatype);	
    outImage.setData(dataOut1d, p4DImage->sz0, p4DImage->sz1, p4DImage->sz2, 1, p4DImage->datatype);
    v3dhandle newwin = callback.newImageWindow();
	callback.setImage(newwin, &outImage);
	callback.setImageName(newwin,"cloning test");
    callback.updateImageWindow(newwin);
}

void WaveletPlugin::WaveletTransform(V3DPluginCallback &callback, QWidget *parent)
{
	v3dhandle oldwin = callback.currentImageWindow();
	Image4DSimple* p4DImage = callback.getImage(oldwin);
	if (!p4DImage)
	{
		QMessageBox::information(0, "WaveletTransform", QObject::tr("No image is open."));
		return;
	}
	double* data1dD = channelToDoubleArray(p4DImage, 1);

	//get dims
    V3DLONG szx = p4DImage->getXDim();
    V3DLONG szy = p4DImage->getYDim();
    V3DLONG szz = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();
    V3DLONG N = szx * szy * szz;

	int numScales = 2;
 	//compute wavelet scales
 	double** resTab = NULL;
	try { 	
			time_t seconds0 = time (NULL);
 			if (szz>1)
 				resTab = b3WaveletScalesOptimized(data1dD, szx, szy, szz, numScales);
 			else
 				resTab = b3WaveletScales2D(data1dD, szx, szy, numScales);
 			time_t seconds1 = time (NULL);
 			printf("Computation time : %d" , (seconds1-seconds0) );
 			}
 	catch(WaveletConfigException e)
 	{
 		printf("\nEXCEPTION\n %s \n" , e.what() );
 		return;
 	}

 	
 	//compute waveletCoefficients
 	double* lowPassResidual = new double[N];
 	b3WaveletCoefficientsInplace(resTab, data1dD, lowPassResidual, numScales, N);
	
	delete(data1dD);
		
	//reconstruct image from coefficients
	double* rec = new double[N];
	b3WaveletReconstruction(resTab, lowPassResidual, rec, numScales, N);
	

	
	//display reconstructed image
	rescaleForDisplay(rec, rec, N, p4DImage->datatype);
	unsigned char* dataOut1d = doubleArrayToCharArray(rec, N, p4DImage->datatype);
	Image4DSimple outImage;
	outImage.setData(dataOut1d, p4DImage->sz0, p4DImage->sz1, p4DImage->sz2, 1, p4DImage->datatype);
	v3dhandle newwin = callback.newImageWindow();
	callback.setImage(newwin, &outImage);
	callback.setImageName(newwin, "reconstruction");
	callback.updateImageWindow(newwin);
	delete(rec);

	
	//output wavelet coefficients
	for (int j = 0; j<numScales; j++)
	{
		//rescale
		double* out = resTab[j];
		rescaleForDisplay(out, out, N, p4DImage->datatype);
		unsigned char* dataOut1d = doubleArrayToCharArray(out, N, p4DImage->datatype);
    	Image4DSimple outImage;
    	outImage.setData(dataOut1d, p4DImage->sz0, p4DImage->sz1, p4DImage->sz2, 1, p4DImage->datatype);
    	v3dhandle newwin = callback.newImageWindow();
		callback.setImage(newwin, &outImage);
		char buffer [50];
		sprintf(buffer, "wavelet scale %d", j+1);
		callback.setImageName(newwin, buffer);
    	callback.updateImageWindow(newwin);
	}
	for (int j = 0; j<numScales; j++)
 	{
 		free(resTab[j]);
 	}
	delete(resTab);

	//output low pass image
	rescaleForDisplay(lowPassResidual, lowPassResidual, N, p4DImage->datatype);
	unsigned char* dataOut1dL = doubleArrayToCharArray(lowPassResidual, N, p4DImage->datatype);
	Image4DSimple outImageL;
	outImageL.setData(dataOut1dL, p4DImage->sz0, p4DImage->sz1, p4DImage->sz2, 1, p4DImage->datatype);
	v3dhandle newwinL = callback.newImageWindow();
	callback.setImage(newwinL, &outImageL);
	callback.setImageName(newwinL, "low pass residual");
	callback.updateImageWindow(newwinL);
	
	delete(lowPassResidual);	
}













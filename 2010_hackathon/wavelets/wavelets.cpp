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
 */
#include <QtGui>

#include <math.h>
#include <stdlib.h>
#include <list>

#include "wavelets.h"
#include "scaleinfo.h"


//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
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
	compute( callback, parent );
}

WaveletPlugin::WaveletPlugin()
{
	scaleInfoList = new std::list<ScaleInfo*>();
}

void WaveletPlugin::processButtonPressed()
{
	printf("process !\n");

//	ScaleInfo *scaleInfo = new ScaleInfo();
//	scaleInfoList->push_back( scaleInfo );
//	printf("scale ok !\n");

	refreshScaleInterface();
}

void WaveletPlugin::refreshScaleInterface()
{
	printf("refresh Scale interface");
/*
	QLabel* scaleLabel = new QLabel("Scale x");
	QCheckBox* enableCheckBox = new QCheckBox("Enabled");
	QLineEdit* thresholdLineEdit = new QLineEdit("10");
*/
	//formLayoutGroupBox




	ScaleInfo *scaleInfo = new ScaleInfo( qBox );  // qbox
	formLayoutGroupBox->addRow( scaleInfo->groupBox );


/*
 	QGroupBox *qBox2= new QGroupBox( qBox );

	QGridLayout *gridLayout = new QGridLayout( qBox2 );
	gridLayout->addWidget( scaleLabel , 0 , 0 );
	gridLayout->addWidget( enableCheckBox , 0 , 1 );
	gridLayout->addWidget( thresholdLineEdit , 0 , 2 );
	formLayoutGroupBox->addRow(qBox2);
*/



}

void WaveletPlugin::cancel()
{
	printf("cancel !\n");

}

void WaveletPlugin::sliderChange(int value )
{
	printf("slide change ! (value is : %i)\n" , value );

	if ( !sourceImage )
	{
		printf("no source image.. \n");
		return;
	}

	if ( !sourceImage->getRawData() )
		{
			printf("no data in source image.. \n");
			return;
		}

	unsigned char* imageRaw = sourceImage->getRawData();

	if ( !imageRaw )
	{
		printf("no image.. \n");
		return;
	}

	printf("enter1");

	for ( int i =0 ; i< 10000 ; i++ )
		{
		imageRaw[i] = value;
		}

	printf("enter2");

	/*
 	unsigned char* dataOut1d = new unsigned char[sourceImage->getTotalBytes()];
	Image4DSimple outImage;
	outImage.setData(dataOut1d, sourceImage->sz0, sourceImage->sz1, sourceImage->sz2, 1, sourceImage->datatype);
	//v3dhandle newwin = callback.newImageWindow();
	myCallback->setImage(sourceWindow, &outImage);
	myCallback->setImageName(sourceWindow,"cloning test");
*/

	myCallback->updateImageWindow(sourceWindow);

	// This would crash ( Bug reported on NTRC V3D Bug tracker )
	//myCallback->setImage(sourceWindow, sourceImage );

}

void WaveletPlugin::compute( V3DPluginCallback &callback, QWidget *parent)
{
	// Build the interface.

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



	//myCallback->setImage(sourceWindow, &sourceImage);


	    //convert first channel to double values
	/*// working
	unsigned char* dataOut1d = new unsigned char[p4DImage->getTotalBytes()];

	Image4DSimple outImage;
	outImage.setData(dataOut1d, p4DImage->sz0, p4DImage->sz1, p4DImage->sz2, 1, p4DImage->datatype);
	//v3dhandle newwin = callback.newImageWindow();
		callback.setImage(sourceWindow, &outImage);
		callback.setImageName(sourceWindow,"cloning test");
	    callback.updateImageWindow(sourceWindow);
*/
//	callback.setImage(sourceWindow, p4DImage);


//	callback.setImageName(sourceWindow,"cloning test");

//	return;


//	    unsigned char* data1d = p4DImage->getRawData();//get raw data


//	sourceImage = callback.getImage(0);





/*
 	 v3dhandleList win_list = callback.getImageWindowList();
	if (win_list.size()<1)
	{
		QMessageBox::information(0, title, QObject::tr("Need at least 1 images."));
		return;
	}
*/
//	QStringList items;
//	for (int i=0; i<win_list.size(); i++) items << callback.getImageName(win_list[i]);

//	QDialog d(parent);
//	QComboBox* combo1 = new QComboBox(); combo1->addItems(items);
//	QComboBox* combo2 = new QComboBox(); combo2->addItems(items);


	QPushButton* ok     = new QPushButton("OK");

	QPushButton* processButton     = new QPushButton("Process");

	QPushButton* cancel = new QPushButton("Cancel");
	formLayout = new QFormLayout;

	QLabel* label = new QLabel("Test");

	formLayout->addRow( label );

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

 	formLayout->addRow( processButton );

	// 	faire l'image a cet endroit
	//Image4DSimple* image1 = callback.getImage(win_list[i1]);

//	formLayout->addRow(QObject::tr("image1: "), combo1);
//	formLayout->addRow(QObject::tr("image2: "), combo2);


 	formLayout->addRow( progressBar );

	QLabel* label2 = new QLabel("Test");
 	qBox= new QGroupBox( myDialog );
	formLayoutGroupBox = new QFormLayout();
	qBox->setLayout( formLayoutGroupBox );
	formLayoutGroupBox->addRow( label2 );

 	formLayout->addRow(qBox);

	formLayout->addRow(ok, cancel);
	myDialog->setLayout(formLayout);
	myDialog->setWindowTitle(QString("Wavelets"));

	myDialog->connect(ok,     SIGNAL(clicked()), myDialog, SLOT(accept()));
	myDialog->connect(slider, SIGNAL(valueChanged(int)), this, SLOT(sliderChange(int)));
	myDialog->connect(cancel, SIGNAL(clicked()), this, SLOT(cancel()));
	myDialog->connect(processButton, SIGNAL(clicked()), this, SLOT(processButtonPressed()));


	myDialog->exec();




//	dialog->connect(cancel, SIGNAL(clicked()), &dialog, SLOT(reject()));
//	if (dialog->exec()!=QDialog::Accepted)
//		return;




	/*
	int i1 = combo1->currentIndex();
	int i2 = combo2->currentIndex();

	Image4DSimple* image1 = callback.getImage(win_list[i1]);
	Image4DSimple* image2 = callback.getImage(win_list[i2]);


	if (!image1 || !image2)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return;
	}
	if (image1->getDatatype()!=V3D_UINT8 || image2->getDatatype()!=V3D_UINT8)
	{
		QMessageBox::information(0, title, QObject::tr("This demo program only supports 8-bit data. Your current image data type is not supported."));
		return;
	}


	V3DLONG N1 = image1->getTotalBytes();
	unsigned char* newdata1d = new unsigned char[N1];

	Image4DSimple tmp;
	tmp.setData(newdata1d, image1->sz0,image1->sz1,image1->sz2,image1->sz3, image1->datatype);

	Image4DProxy<Image4DSimple> p1(image1);
	Image4DProxy<Image4DSimple> p2(image2);
	Image4DProxy<Image4DSimple> p(&tmp);

	Image4DProxy_foreach(p, x,y,z,c)
	{
		float f = 0;
		float f1 = 0;
		float f2 = 0;
		if (p1.is_inner(x,y,z,c)) f1 = (*p1.at_uint8(x,y,z,c))/255.f;
		if (p2.is_inner(x,y,z,c)) f2 = (*p2.at_uint8(x,y,z,c))/255.f;

		switch (op)
		{
		case '+': f = f1 + f2; break;
		case '-': f = f1 - f2; break;
		case '*': f = f1 * f2; break;
		}
		f = fabs(f);
		if (f>1) f = 1;

		*p.at_uint8(x,y,z,c) = (unsigned char)(f*255);
	}


	v3dhandle newwin = callback.newImageWindow();
	callback.setImage(newwin, &tmp);
	callback.setImageName(newwin, "new_image_arithmetic");
    callback.updateImageWindow(newwin);
    */



}

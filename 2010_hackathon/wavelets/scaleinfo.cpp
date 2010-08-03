/**
 * scaleinfo.cpp
 *
 * Written by
 *
 * Ihor Smal
 * Nicolas Chenouard
 * Fabrice de Chaumont
 *
 * Paper reference: ISBI and JC Ref
 *
 * This code is under GPL License
 *
 */

#include <QtGui>
#include <string>
#include <math.h>
#include <stdlib.h>
#include <list>

#include "scaleinfo.h"

ScaleInfo::~ScaleInfo()
{
	printf("SCALE INFO : destructing scaleInfo.\n");

	QLayoutItem *child;
	while ((child = gridLayout->takeAt(0)) != 0) {
		gridLayout->removeItem(child);
	}

	delete gridLayout ;
	delete enableCheckBox ;
	delete thresholdSlider;
	delete groupBox ;

}

ScaleInfo::ScaleInfo( int scaleNumber , QGroupBox *parent ) // TODO: Generalized it (might have som QContainer ?)
{
	printf( "%i \n", scaleNumber );
	groupBox = new QGroupBox( parent );
	gridLayout = new QGridLayout( groupBox );

	enable= true;

	char buff[50];
	sprintf(buff, "Scale %i", scaleNumber);

	groupBox->setTitle( buff );

	QLabel* thresholdLabel = new QLabel( "Threshold:" );
	enableCheckBox = new QCheckBox("Enabled" );
	enableCheckBox->setChecked( true );
//	thresholdLineEdit = new QLineEdit("10");

	QSlider *thresholdSlider = new QSlider(Qt::Horizontal);
	thresholdSlider->setFocusPolicy(Qt::StrongFocus);
	thresholdSlider->setTickPosition(QSlider::TicksBothSides);
	thresholdSlider->setTickInterval(1);
	thresholdSlider->setSingleStep(1);
	thresholdSlider->setMaximum(20);


	gridLayout->addWidget( enableCheckBox , 0 , 0 );
	gridLayout->addWidget( thresholdLabel , 0 , 1 );
	gridLayout->addWidget( thresholdSlider , 0 , 2 );

	//dialog->
	this->connect(enableCheckBox, SIGNAL(clicked()), this, SLOT(enableButtonPressed()));
	this->connect(thresholdSlider, SIGNAL(valueChanged(int)), this, SLOT(sliderChange(int)));
}

void ScaleInfo::enableButtonPressed()
{
	printf("enable button pressed\n");
//	thresholdSlider->setEnabled( enableCheckBox->isChecked() );
	enable = enableCheckBox->isChecked();
}

void ScaleInfo::sliderChange(int value )
{
	thresholdValue = value;
	printf( "%d" , thresholdValue );
}












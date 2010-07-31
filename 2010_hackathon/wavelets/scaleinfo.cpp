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

#include <math.h>
#include <stdlib.h>
#include <list>

#include "scaleinfo.h"

ScaleInfo::~ScaleInfo()
{
	printf("SCALE INFO : destructing scaleInfo.\n");
}

ScaleInfo::ScaleInfo( QGroupBox *parent ) // TODO: Generalized it (might have som QContainer ?)
{

	groupBox = new QGroupBox( parent );
	gridLayout = new QGridLayout( groupBox );

	QLabel* scaleLabel = new QLabel("Scale x");
	QCheckBox* enableCheckBox = new QCheckBox("Enabled");
	QLineEdit* thresholdLineEdit = new QLineEdit("10");

	gridLayout->addWidget( scaleLabel , 0 , 0 );
	gridLayout->addWidget( enableCheckBox , 0 , 1 );
	gridLayout->addWidget( thresholdLineEdit , 0 , 2 );

}

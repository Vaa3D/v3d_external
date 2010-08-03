/**
 * scaleinfo.h
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

#ifndef ScaleInfo_H
#define ScaleInfo_H

#include <QtGui>

#include <math.h>
#include <stdlib.h>
#include <list>

class ScaleInfo : public QObject
{
	 Q_OBJECT

public:

// QGroupBox *parent
	ScaleInfo( int scalNumber , QGroupBox *parent );
	virtual ~ScaleInfo( );

	// data
	double enabled;     // is this scale enabled ?
	double threshold;   // threshold in wavelet domain.

	// Qt Interface
	QGroupBox *groupBox ; //= new QGroupBox( qBox );
	QGridLayout *gridLayout ; //= new QGridLayout( qBox2 );
	QCheckBox* enableCheckBox ;
	//QLineEdit* thresholdLineEdit;
	QSlider *thresholdSlider;
//	QDialog *myDialog;
	int thresholdValue;
	bool enable;

public slots:
	void enableButtonPressed();
	void sliderChange(int value );
};


#endif

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

class ScaleInfo
{
//	 Q_OBJECT

public:

// QGroupBox *parent
	ScaleInfo( QGroupBox *parent );

	// data
	double enabled;     // is this scale enabled ?
	double threshold;   // threshold in wavelet domain.

	// Qt Interface
	QGroupBox *groupBox ; //= new QGroupBox( qBox );
	QGridLayout *gridLayout ; //= new QGridLayout( qBox2 );

};


#endif

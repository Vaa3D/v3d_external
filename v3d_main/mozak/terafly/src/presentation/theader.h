//a header file for Qt5 / Qt4 migration

#ifndef __T_HEADER_PHC__
#define __T_HEADER_PHC__

#ifdef USE_Qt5 //added by PHC 2015May

#include <QtWidgets>
#include <QtConcurrent>
//#include </Users/pengh/Qt5.4.1//5.4/clang_64/lib/QtConcurrent.framework/Versions/5/Headers/QtConcurrent>


#else

#include <QtGui>
#include <QWidget>

#endif


#endif


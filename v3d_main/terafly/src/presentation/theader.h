// This is a header for migrating from Qt4 to Qt5, under VS2015 environment.

#ifndef __T_HEADER_PHC__
#define __T_HEADER_PHC__

#include <version_control.h>
#if defined(USE_Qt5) 

#include <QtWidgets>
#include <QtConcurrent/QtConcurrent> //by PHC 2020/01/31

#else

#include <QtGui>
#include <QWidget>

#endif


#endif


// This is a header for migrating from Qt4 to Qt5, under VS2015 environment.

#ifndef __T_HEADER_PHC__
#define __T_HEADER_PHC__

#include <version_control.h>
#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393) 

#include <QtWidgets>
#include <QtConcurrent>

#else

#include <QtGui>
#include <QWidget>

#endif


#endif


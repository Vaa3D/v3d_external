/* istitch.h
 * 2010-02-08: create this program by Yang Yu
 */


#ifndef __ISTITCH_H__
#define __ISTITCH_H__

#define COMPILE_TO_COMMANDLINE 1

//
#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>

//----------------------------------------------------------------------
#ifdef COMPILE_TO_COMMANDLINE
#else

class IStitchPlugin : public QObject, public V3DPluginInterface2
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2);
	
public:
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);
	
	QStringList funclist() const {return QStringList();}
	bool dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
				V3DPluginCallback2 & v3d, QWidget * parent) {return true;}
	float getPluginVersion() const {return 1.01f;} // version info 
};

#endif
//----------------------------------------------------------------------

#endif




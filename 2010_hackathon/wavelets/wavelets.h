/**
 * wavelets.h
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
#ifndef EXTRAFILTERSPLUGIN_H
#define EXTRAFILTERSPLUGIN_H


#include <v3d_interface.h>
#include <QtGui>
#include <QtCore>
#include <list>
#include "scaleinfo.h"

class WaveletPlugin : public QObject, public V3DPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface);

public:


    WaveletPlugin();
    void refreshScaleInterface();

    // interace.
    QDialog *myDialog;
    QFormLayout *formLayout;
    QFormLayout *formLayoutGroupBox;
    QGroupBox *qBox;

    // wavelet input interface

    typedef std::list<ScaleInfo*> ListType ;
    ListType *scaleInfoList ;
    QPushButton* removeScaleButton;

    // source image
    v3dhandle sourceWindow;
    Image4DSimple* sourceImage;
    V3DPluginCallback *myCallback;

    // Plugin stuff

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent);

	QStringList funclist() const {return QStringList();}
	void dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, QWidget *parent) {}

	void initGUI( V3DPluginCallback &callback, QWidget *parent);

public slots:
	void cancel();
	void sliderChange( int value );
	void addScaleButtonPressed();
	void removeScaleButtonPressed();

};

#endif

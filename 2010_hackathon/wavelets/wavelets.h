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
#include <v3d_basicdatatype.h>
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
    QProgressBar *progressBar;

    // wavelet input interface

    typedef std::list<LocationSimple*> LocationSimpleListType ;
    typedef std::list<ScaleInfo*> ListType ;
    ListType *scaleInfoList ;
    QPushButton* removeScaleButton;

    // source image
    v3dhandle sourceWindow;
    Image4DSimple* sourceImage;
    V3DPluginCallback *myCallback;

    // Copy of original Image;

    Image4DSimple *originalImageCopy;
    void copyOriginalImage();
    void restoreOriginalImage();

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
	void dev1ButtonPressed();
	void dev2ButtonPressed();
	void dev3ButtonPressed();
	void dev4ButtonPressed();
	void detectSpotsButtonPressed();
	void denoiseButtonPressed();
};

#endif

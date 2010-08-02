/* gaussianspotdetector.h
 * 2010-08-31: created by Ihor Smal and Nicolas Chenouard
 */

#ifndef __HELLOWORLD_H__
#define __HELLOWORLD_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>
#include <v3d_basicdatatype.h>
#include <fftw3.h>


class GaussianSpotDetector : public QObject, public V3DPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface);

public:
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent);

	QStringList funclist() const {return QStringList();}
	void dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, QWidget *parent) {}

	void Cloning(V3DPluginCallback &callback, QWidget *parent);
	void GaussianFilter(V3DPluginCallback &callback, QWidget *parent);

	double* channelToDoubleArray(Image4DSimple* inputImage, int channel);
	unsigned char* doubleArrayToCharArray(double* dataD, int numVoxels, ImagePixelType dataType);
};

#endif




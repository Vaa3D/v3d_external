/**
*  100811 RZC: change to handle any type image using  Image4DProxy's value_at/put_at
**/
#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <vector>

#include <QtGui>

#include "partial_align_plugin.h"
#include "partial_align_gui.h"

#include "myfeature.h"
using namespace std;

int calc_feature(V3DPluginCallback2 &callback, QWidget *parent, MyFeatureType type);

template <class T> Vol3DSimple<T> * create_vol3dsimple(T* &inimg1d, V3DLONG sz[3]);

Q_EXPORT_PLUGIN2(partial_align, PartialAlignPlugin);

const QString title = "Partial Alignment Plugin";

QStringList PartialAlignPlugin::menulist() const
{
	return QStringList() 
		<< tr("calc average feature")
		<< tr("calc standard variance feature")
		<< tr("calc invariant methods feature")
		<< tr("calc SIFT feature")
		<< tr("about");
}

void PartialAlignPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("calc average feature"))
	{
		calc_feature(callback,parent,AVERAGE_FEATURE);
	}
	else if(menu_name == tr("calc standard variance feature"))
	{
		calc_feature(callback,parent,STD_VAR_FEATURE);
	}
	else if(menu_name == tr("calc invariant methods feature"))
	{
		calc_feature(callback,parent,INVARIANT_MOMENT_FEATURE);
	}
	else if(menu_name == tr("calc SIFT feature"))
	{
		calc_feature(callback,parent,SIFT_FEATURE);
	}
	else
	{
		v3d_msg("Partial Alignment Plugin Demo version 1.0"
				"\ndeveloped by Hang Xiao 2011. (Janelia Farm Research Campus, HHMI)");
	}

}

int calc_feature(V3DPluginCallback2 &callback, QWidget *parent, MyFeatureType type)
{
	v3dhandleList win_list = callback.getImageWindowList();

	if(win_list.size()<1)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return -1;
	}

	PartialAlignDialog dialog(callback, parent);
	dialog.combo_subject->setDisabled(true);
	dialog.channel_sub->setDisabled(true);

	if (dialog.exec()!=QDialog::Accepted) return -1;

	dialog.update();
	int i1 = dialog.combo_subject->currentIndex();
	int i2 = dialog.combo_target->currentIndex();

	V3DLONG sub_c = dialog.sub_c - 1;
	V3DLONG tar_c = dialog.tar_c - 1;

	Image4DSimple *image = callback.getImage(win_list[i2]);
	
	if(image->getCDim() < dialog.tar_c) {QMessageBox::information(0, title, QObject::tr("The channel isn't existed.")); return -1;}

	V3DLONG sz[3];
	sz[0] = image->getXDim();
	sz[1] = image->getYDim();
	sz[2] = image->getZDim();

	unsigned char *data1d = image->getRawDataAtChannel(tar_c);
	Vol3DSimple<unsigned char> * img3d = create_vol3dsimple(data1d,sz);

	LandmarkList landmarks = callback.getLandmark(win_list[i2]);
	if(landmarks.empty()) {v3d_msg("No mark loaded"); return -1;}

	MyFeature myfeature;
	myfeature.setFeatures(landmarks, img3d, type);
	myfeature.printFeatures();

	delete img3d;
}

template <class T> Vol3DSimple<T>* create_vol3dsimple(T* &inimg1d, V3DLONG sz[3])
{
	Vol3DSimple<T> * vol3d = new Vol3DSimple<T>(sz[0], sz[1], sz[2]);
	T*** data3d = vol3d->getData3dHandle();
	T*** inimg3d = 0; new3dpointer(inimg3d, sz[0], sz[1], sz[2], inimg1d);

	int i,j,k;
	for(k = 0; k < sz[2]; k++)
	{
		for(j = 0; j < sz[1]; j++)
		{
			for(i = 0; i < sz[0]; i++)
			{
				data3d[k][j][i] = inimg3d[k][j][i];
			}
		}
	}
	if(inimg3d) delete3dpointer(inimg3d, sz[0],sz[1],sz[2]);

	return vol3d;
}

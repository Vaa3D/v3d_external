/*****************************************************************************************************
*
* V3D's plug-in interface of QObject
*
* Copyright: Hanchuan Peng (Howard Hughes Medical Institute, Janelia Farm Research Campus).
* The License Information and User Agreement should be seen at http://penglab.janelia.org/proj/v3d .
* 
* Last edit: 2009-Aug-21
*
********************************************************************************************************
*/

#ifndef _V3D_INTERFACE_H_
#define _V3D_INTERFACE_H_

#include <QtCore>

#include "basic_4dimage.h"
//#include "basic_surf_objs.h"
#include "basic_landmark.h"


struct V3DPluginArgItem
{
	QString type;	void * p;
};
typedef QList<V3DPluginArgItem> V3DPluginArgList;

typedef QList<QPolygon>        ROIList;
typedef QList<LocationSimple>  LandmarkList;
//typedef QList<ImageMarker>     MarkerList;
//typedef QList<LabelSurf>       LabelSurfList;
//typedef QList<NeuronSWC>       SWCList;
//typedef QList<CellAPO>         APOList;

typedef void* v3dhandle;
typedef QList<v3dhandle>       v3dhandleList;

class V3DPluginCallback
{
public:
	virtual ~V3DPluginCallback() {}
	virtual bool callPluginFunc(const QString & plugin_name, const QString & func_name,
			const V3DPluginArgList & input, V3DPluginArgList & output) = 0;

	virtual v3dhandleList getImageWindowList() const = 0;
	virtual v3dhandle currentImageWindow() = 0;
	virtual v3dhandle newImageWindow(QString name="new_image") = 0;
	virtual void updateImageWindow(v3dhandle image_window) = 0;

	virtual QString getImageName(v3dhandle image_window) const = 0;
	virtual void setImageName(v3dhandle image_window, QString name) = 0;

	virtual Image4DSimple * getImage(v3dhandle image_window) = 0;
	virtual bool setImage(v3dhandle image_window, Image4DSimple * image) = 0;

	virtual LandmarkList  getLandmark(v3dhandle image_window) = 0;
	virtual bool setLandmark(v3dhandle image_window, LandmarkList & landmark_list) = 0;

	virtual ROIList getROI(v3dhandle image_window) = 0;
	virtual bool setROI(v3dhandle image_window, ROIList & roi_list) = 0;

};

//this is the major V3D plugin interface, and will be enhanced continuously
class V3DPluginInterface
{
public:
	virtual ~V3DPluginInterface() {}

	virtual QStringList menulist() const = 0;
	virtual void domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent) = 0;

	virtual QStringList funclist() const = 0;
	virtual void dofunc(const QString & func_name,
			const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent) = 0;
};


// obsolete interface for manipulating only one image at a time. 
class V3DSingleImageInterface
{
public:
    virtual ~V3DSingleImageInterface() {}

    virtual QStringList menulist() const = 0;
    virtual void processImage(const QString & menu_name, Image4DSimple * image, QWidget * parent) = 0;
};


QT_BEGIN_NAMESPACE

	Q_DECLARE_INTERFACE(V3DPluginInterface, "com.janelia.v3d.V3DPluginInterface/1.0");

	Q_DECLARE_INTERFACE(V3DSingleImageInterface, "com.janelia.v3d.V3DSingleImageInterface/1.0");

QT_END_NAMESPACE

#endif /* _V3D_INTERFACE_H_ */

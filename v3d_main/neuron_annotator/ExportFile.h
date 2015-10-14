#ifndef EXPORTFILE_H
#define EXPORTFILE_H

#include "utility/ImageLoader.h"
#include "data_model/NaVolumeData.h"
#include "data_model/NeuronSelectionModel.h"
#include "data_model/DataColorModel.h"
#include "geometry/CameraModel.h"
#include <QString>
#include <QThread>
#include <QMutex>
#include <QFileInfo>
#include <QList>

#include <iostream>

#include "../v3d/v3d_core.h"
#include "../basic_c_fun/color_xyz.h"

template <class Tinput, class Tmask, class Tref, class Toutput>
Toutput* getCurrentStack(Tinput *input1d, Tmask *mask1d, Tref *ref1d, V3DLONG *szStack, QList<bool> maskStatusList, QList<bool> overlayStatusList, int datatype);

// export XYZC(T) tif file
class ExportFile : public QThread
{
    Q_OBJECT
    
public:
    ExportFile(
    		QString fileName,
    		const NaVolumeData& volumeData,
    		const NeuronSelectionModel& selectionModel,
            const DataColorModel& colorModel,
            const CameraModel& cameraModel,
            bool is2D=false);
    ~ExportFile();
	
signals:
    void exportFailed(QString fileName, QString message);
    void exportFinished(QString fileName);

public slots:

protected:
    virtual void run();
	
public:
    volatile bool stopped; //
    const NaVolumeData& volumeData;
    const NeuronSelectionModel& selectionModel;
    const DataColorModel& colorModel;
    const CameraModel& cameraModel;
    QString filename;
	
private:
    QMutex mutex;
    ImageLoader imageLoader;
    bool is2D;
    My4DImage* padAndRotateImage(My4DImage* image, const Rotation3D& rotation, bool flipY);
};


#endif // EXPORTFILE_H

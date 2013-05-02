/*
 * CutPlanner.h
 *
 *  Created on: Mar 26, 2013
 *      Author: Christopher M. Bruns
 */

#ifndef CUTPLANNER_H_
#define CUTPLANNER_H_

#include <QDialog>
#include "ui_micro_ct.h"
#include "../gui/Na3DWidget.h"
#include "../geometry/Vector3D.h"
#include "../geometry/Rotation3D.h"
#include "../geometry/CameraModel.h"

class CutPlanner : public QDialog
{
    Q_OBJECT

public:
    CutPlanner(CameraModel& camera,
            Na3DWidget& widget3d,
            QWidget* parent = NULL);
    virtual ~CutPlanner();
    double getMicrometersPerVoxel() const {return micrometersPerVoxel;}

signals:
    void clipPlaneRequested();
    void cutGuideRequested(bool doShow);
    void rotationAdjusted(Rotation3D rotation); // To maintain axis alignment

public slots:
    void onRotationChanged(Rotation3D rotation);

private:
    void initSingleCut(SingleCut* widget, QString name, QString axis);

    Ui::CutPlannerDialog ui;
    CameraModel& camera; // for access to focus, rotation, and restricting rotation
    Na3DWidget& widget3d; // for access to Renderer::applyCutPlaneInImageFrame()
    double micrometersPerVoxel;
};

#endif /* CUTPLANNER_H_ */

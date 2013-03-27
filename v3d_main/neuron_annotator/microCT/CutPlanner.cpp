/*
 * CutPlanner.cpp
 *
 *  Created on: Mar 26, 2013
 *      Author: Christopher M. Bruns
 */

#include "CutPlanner.h"

CutPlanner::CutPlanner(CameraModel& camera, Na3DWidget& widget3d, QWidget* parent)
    : QDialog(parent)
    , camera(camera)
    , widget3d(widget3d)
    , micrometersPerVoxel(0.64)
{
    ui.setupUi(this);
    initSingleCut(ui.topCutWidget, "Top", "+Z");
    initSingleCut(ui.frontCutWidget, "Front/A", "-Y");
    initSingleCut(ui.rightCutWidget, "Right/B", "+X");
    initSingleCut(ui.rearCutWidget, "Rear/C", "+Y");
    initSingleCut(ui.leftCutWidget, "Left/D", "-X");
}

CutPlanner::~CutPlanner()
{}

void CutPlanner::initSingleCut(SingleCut* widget, QString name, QString axis) {
    widget->setCamera(camera);
    widget->setName(name);
    widget->setAxis(axis);
    connect(widget, SIGNAL(clipPlaneRequested()),
            this, SIGNAL(clipPlaneRequested()));
    connect(widget, SIGNAL(cutGuideRequested(bool)),
            this, SIGNAL(cutGuideRequested(bool)));
    connect(widget, SIGNAL(rotationAdjusted(Rotation3D)),
            this, SIGNAL(rotationAdjusted(Rotation3D)));

}

/* slot */
void CutPlanner::onRotationChanged(Rotation3D rotation)
{
    // TODO - optionally restrict rotation
}


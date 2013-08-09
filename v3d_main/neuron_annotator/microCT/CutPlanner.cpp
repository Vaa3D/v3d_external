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
    , micrometersPerVoxel(1.00)
{
    ui.setupUi(this);
    setBasePlaneWidget(ui.basePlaneWidget, "Base", "+Z");
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
    widget->setMicrometersPerVoxel(micrometersPerVoxel);
    connect(widget, SIGNAL(clipPlaneRequested()),
           this, SIGNAL(clipPlaneRequested()));
    connect(widget, SIGNAL(cutGuideRequested(bool)),
            this, SIGNAL(cutGuideRequested(bool)));
    connect(widget, SIGNAL(rotationAdjusted(Rotation3D)),
            this, SIGNAL(rotationAdjusted(Rotation3D)));
}

void CutPlanner::setBasePlaneWidget(SingleCut* widget, QString name, QString axis) {
    widget->setCamera(camera);
    widget->setName(name);
    widget->setAxis(axis);
    widget->setMicrometersPerVoxel(micrometersPerVoxel);
    // Remove elements that are only relevant for cut planes, not show planes
    widget->ui.cutButton->hide();
    widget->ui.cutDistanceLineEdit->hide();
    widget->ui.label->hide();
    // connect(widget, SIGNAL(clipPlaneRequested()),
    //        this, SIGNAL(clipPlaneRequested()));
    connect(widget, SIGNAL(cutGuideRequested(bool)),
            this, SIGNAL(cutGuideRequested(bool)));
    connect(widget, SIGNAL(rotationAdjusted(Rotation3D)),
            this, SIGNAL(rotationAdjusted(Rotation3D)));
    // TODO show plane requested
}

void CutPlanner::setMicrometersPerVoxel(double val) {
    ui.basePlaneWidget->setMicrometersPerVoxel(val);
    ui.topCutWidget->setMicrometersPerVoxel(val);
    ui.frontCutWidget->setMicrometersPerVoxel(val);
    ui.rightCutWidget->setMicrometersPerVoxel(val);
    ui.rearCutWidget->setMicrometersPerVoxel(val);
    ui.leftCutWidget->setMicrometersPerVoxel(val);
}

/* slot */
void CutPlanner::onRotationChanged(Rotation3D rotation)
{
    // TODO - optionally restrict rotation
}

/* slot */
void CutPlanner::on_micrometersBox_valueChanged(double val) {
    qDebug("valueChanged");
    setMicrometersPerVoxel(val);
}


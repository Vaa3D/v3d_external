/*
 * SingleCut.cpp
 *
 *  Created on: Mar 26, 2013
 *      Author: Christopher M. Bruns
 */

#include "SingleCut.h"
#include "CutPlanner.h"
#include <iostream>

using namespace std;

SingleCut::SingleCut(QWidget* parent)
    : QWidget(parent)
    , camera(NULL)
    , hasCut(false)
    , micrometersPerVoxel(0.64)
    , upVector(1,0,0)
    , isKeepPlane(false)
{
    ui.setupUi(this);
    CutPlanner* planner = dynamic_cast<CutPlanner*>(parent);
    if (planner != NULL) {
        micrometersPerVoxel = planner->getMicrometersPerVoxel();
    }
}

SingleCut::~SingleCut()
{}

void SingleCut::init()
{
    hasCut = false;
    ui.cutButton->setEnabled(false);
    ui.edgeButton->setDefault(true);
}

void SingleCut::savePlanFileLine(QTextStream& out) {
    out << ui.axisBox->currentText()
        << "\t"
        << ui.nameField->text()
        << "\t"
        << ui.cutDistanceLineEdit->text()
        << "\t"
        << QChar(0x00b5) << "m" // micrometers
        << "\n";
}

void SingleCut::setMicrometersPerVoxel(double mpv) {
    if (micrometersPerVoxel == mpv)
        return;
    micrometersPerVoxel = mpv;
    updateCutDistance();
}

bool SingleCut::setUpDirectionFromLabel(const QString& text) {
    // TODO - WHY is X-axis appearance opposite to the other axes?
    // (judging by axis tool)
    Vector3D newDirection = upVector;
    if (text == "+X")
        newDirection = Vector3D(-1, 0, 0);
    else if (text == "-X")
        newDirection = Vector3D(1, 0, 0);
    else if (text == "+Y")
        newDirection = Vector3D(0, 1, 0);
    else if (text == "-Y")
        newDirection = Vector3D(0, -1, 0);
    else if (text == "+Z")
        newDirection = Vector3D(0, 0, 1);
    else if (text == "-Z")
        newDirection = Vector3D(0, 0, -1);
    else
        return false;
    if (upVector == newDirection)
        return false;
    setUpDirection(newDirection);
    return true;
}

/* slot */
void SingleCut::on_axisBox_activated(const QString& text)
{
    if (! setUpDirectionFromLabel(text))
        return;
    alignViewToUpVector();
    // Light up Edge button for next step
    // (unless we are already at the cut stage)
    if (! ui.cutButton->isDefault())
        ui.edgeButton->setDefault(true);
    emit cutGuideRequested(true);
    emit currentWidgetRequested(widgetIndex);
}

/* slot */
void SingleCut::on_cutButton_clicked()
{
    // cout << "Cut" << endl;
    if (camera == NULL)
        return;
    cutPoint = camera->focus();
    cutNormal = camera->rotation().transpose() * Vector3D(0, 1, 0);
    hasCut = true;
    ui.cutButton->setDefault(false);
    updateCutDistance();
    emit currentWidgetRequested(widgetIndex);
    emit cutGuideRequested(true);
}

/* slot */
void SingleCut::on_edgeButton_clicked()
{
    cout << "Edge" << endl;
    if (camera == NULL)
        return;
    edgePoint = camera->focus();
    edgeNormal = camera->rotation().transpose() * Vector3D(0, 1, 0);
    ui.cutButton->setEnabled(true);
    ui.edgeButton->setDefault(false);
    ui.cutButton->setDefault(true);
    updateCutDistance();
    emit currentWidgetRequested(widgetIndex);
    emit cutGuideRequested(true); // Why is this sometimes necessary?
}

void SingleCut::setAxis(const QString& axis)
{
    int index = ui.axisBox->findText(axis);
    if (index == -1)
        return; // not found
    ui.axisBox->setCurrentIndex(index);
    setUpDirectionFromLabel(axis);
}

void SingleCut::setName(const QString& name)
{
    ui.nameField->setText(name);
}

void SingleCut::setUpDirection(Vector3D up) {
    if (upVector == up)
        return;
    upVector = up;
}

void SingleCut::alignViewToUpVector() {
    const Rotation3D oldRot = camera->rotation();
    UnitVector3D oldUp = -oldRot[1];
    double dot = upVector.dot(oldUp);
    if ((dot - 1.0) > -1e-3) {
        cout << "close enough" << endl;
        return; // close enough
    }
    // Change rotation to place up axis vertical
    double angle = acos(dot);
    UnitVector3D axis = UnitVector3D(upVector.cross(oldUp));
    // axis can be bogus if newUp == -oldUp
    if (dot < -0.9999) {
        // 180 degree rotation might have poorly computed axis
        // Just choose any orthogonal axis
        // First choose a non-coaligned second vector from two possibilities
        Vector3D v1 = Vector3D(1,0,0);
        double d1 = std::abs(upVector.dot(v1));
        Vector3D v2 = Vector3D(0,1,0);
        double d2 = std::abs(upVector.dot(v2));
        if (d1 < d2) // First vector is less coaligned
            axis = UnitVector3D(upVector.cross(v1));
        else
            axis = UnitVector3D(upVector.cross(v2));
    }
    Rotation3D r;
    r.setRotationFromAngleAboutUnitVector(angle, axis);
    r = oldRot * r;
    // camera->setRotation(r);
    emit rotationAdjusted(r);
}

void SingleCut::updateCutDistance()
{
    if (isKeepPlane) {
        // Less bookkeeping needed when its just the Base plane...
        // qDebug() << "emit keep plane" << __FILE__ << __LINE__;
        emit keepPlaneRequested();
        return;
    }
    if (! hasCut) {
        ui.cutDistanceLineEdit->setText("?");
        return;
    }
    double dot = edgeNormal.dot(cutNormal);
    if (dot > 1.1)
        qDebug() << "Plane equation error!" << __FILE__ << __LINE__;
    else if (dot < 0.5)
        qDebug() << "Cut not parallel to edge" << __FILE__ << __LINE__;
    else { // Compute distance between planes (at start point)
        double dist1 = cutNormal.dot(edgePoint);
        double dist2 = cutNormal.dot(cutPoint);
        double dd = dist1 - dist2; // Distance in XY voxel units
        dd *= micrometersPerVoxel; // hard code microCT voxel size for now.
        // qDebug() << "Distance to previous plane =" << dd << "micrometers";
        ui.cutDistanceLineEdit->setText(QString("%1").arg(dd, 0, 'f', 0));
        emit clipPlaneRequested();
    }
}


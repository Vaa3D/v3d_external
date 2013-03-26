/*
 * SingleCut.cpp
 *
 *  Created on: Mar 26, 2013
 *      Author: Christopher M. Bruns
 */

#include "SingleCut.h"
#include <iostream>

using namespace std;

SingleCut::SingleCut(QWidget* parent)
    : QWidget(parent)
    , camera(NULL)
    , hasCut(false)
    , voxelsPerMicrometer(0.64)
{
    ui.setupUi(this);
}

SingleCut::~SingleCut()
{}

void SingleCut::init()
{
    hasCut = false;
    ui.cutButton->setEnabled(false);
    ui.edgeButton->setDefault(true);
}

/* slot */
void SingleCut::on_axisBox_activated(const QString& text)
{
    if (text == "Free")
        return; // no restriction
    else if (text == "+X")
        setUpVector(Vector3D(1, 0, 0));
    else if (text == "-X")
        setUpVector(Vector3D(-1, 0, 0));
    else if (text == "+Y")
        setUpVector(Vector3D(0, 1, 0));
    else if (text == "-Y")
        setUpVector(Vector3D(0, -1, 0));
    else if (text == "+Z")
        setUpVector(Vector3D(0, 0, 1));
    else if (text == "-Z")
        setUpVector(Vector3D(0, 0, -1));
    // Light up Edge button for next step
    // (unless we are already at the cut stage)
    if (! ui.cutButton->isDefault())
        ui.edgeButton->setDefault(true);
    emit cutGuideRequested(true);
}

/* slot */
void SingleCut::on_cutButton_clicked()
{
    cout << "Cut" << endl;
    if (camera == NULL)
        return;
    cutPoint = camera->focus();
    cutNormal = camera->rotation().transpose() * Vector3D(0, 1, 0);
    hasCut = true;
    ui.cutButton->setDefault(false);
    updateCutDistance();
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
}

void SingleCut::setAxis(const QString& axis)
{
    int index = ui.axisBox->findText(axis);
    if (index == -1)
        return; // not found
    ui.axisBox->setCurrentIndex(index);
}

void SingleCut::setName(const QString& name)
{
    ui.nameField->setText(name);
}

void SingleCut::setUpVector(Vector3D up) {
    const Rotation3D oldRot = camera->rotation();
    UnitVector3D oldUp = -oldRot[1];
    double dot = up.dot(oldUp);
    if ((dot - 1.0) > -1e-3) {
        cout << "close enough" << endl;
        return; // close enough
    }
    // Change rotation to place up axis vertical
    double angle = acos(dot);
    UnitVector3D axis = UnitVector3D(up.cross(oldUp));
    Rotation3D r;
    r.setRotationFromAngleAboutUnitVector(angle, axis);
    r = oldRot * r;
    // camera->setRotation(r);
    emit rotationAdjusted(r);
}

void SingleCut::updateCutDistance()
{
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
        dd *= voxelsPerMicrometer; // hard code microCT voxel size for now.
        // qDebug() << "Distance to previous plane =" << dd << "micrometers";
        ui.cutDistanceLineEdit->setText(QString("%1").arg(dd, 0, 'f', 0));
        emit clipPlaneRequested();
    }
}


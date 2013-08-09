/*
 * SingleCut.h
 *
 *  Created on: Mar 26, 2013
 *      Author: Christopher M. Bruns
 */

#ifndef SINGLECUT_H_
#define SINGLECUT_H_

#include <QWidget>
#include <QString>
#include "ui_single_cut.h"
#include "../gui/Na3DWidget.h"
#include "../geometry/CameraModel.h"

class SingleCut : public QWidget
{
    Q_OBJECT

public:
    SingleCut(QWidget* parent = NULL);
    virtual ~SingleCut();
    void init();
    void setCamera(CameraModel& cameraParam) {camera = &cameraParam;}
    void setName(const QString& name);
    void setAxis(const QString& axis);
    void setMicrometersPerVoxel(double mpv);

signals:
    void cutGuideRequested(bool doShow);
    void clipPlaneRequested();
    void rotationAdjusted(Rotation3D rotation); // To maintain axis alignment
    void orientRequested();

public slots:
    void on_axisBox_activated(const QString& text);
    void on_cutButton_clicked();
    void on_edgeButton_clicked();
    void on_orientButton_clicked() {alignViewToUpVector();}

private:
    void updateCutDistance();
    void alignViewToUpVector();
    void setUpDirection(Vector3D);
    bool setUpDirectionFromLabel(const QString& text);

    Ui::SingleCut ui;
    CameraModel* camera; // for access to focus, rotation, and restricting rotation
    Vector3D edgePoint;
    Vector3D edgeNormal;
    Vector3D cutPoint;
    Vector3D cutNormal;
    Vector3D upVector;
    bool hasCut;
    double micrometersPerVoxel;
};

#endif /* SINGLECUT_H_ */

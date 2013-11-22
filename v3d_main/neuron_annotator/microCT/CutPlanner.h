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
    void keepPlaneRequested();
    void cutGuideRequested(bool doShow);
    void rotationAdjusted(Rotation3D rotation); // To maintain axis alignment
    void compartmentNamingRequested();

public slots:
    void onRotationChanged(Rotation3D rotation);
    void on_micrometersBox_valueChanged(double val);
    void on_labelBrainCompartmentsButton_clicked() {emit compartmentNamingRequested();}
    void on_savePlanButton_clicked();
    void setCurrentWidget(int index) {
        if (indexWidgets.find(index) == indexWidgets.end())
            return;
        SingleCut* widget = indexWidgets.find(index)->second;
        // qDebug() << "set current widget" << __FILE__ << __LINE__;
        if (widget == currentWidget)
            return;
        if (currentWidget != NULL) {
            currentWidget->setStyleSheet("");
        }
        currentWidget = widget;
        currentWidget->setStyleSheet("background-color:#ffffee;");
    }

private:
    QString getPlanFileName();
    void initSingleCut(SingleCut* widget, QString name, QString axis);
    void savePlanFile(QTextStream& out);
    void setBasePlaneWidget(SingleCut* widget, QString name, QString axis);
    void setMicrometersPerVoxel(double val);

    Ui::CutPlannerDialog ui;
    CameraModel& camera; // for access to focus, rotation, and restricting rotation
    Na3DWidget& widget3d; // for access to Renderer::applyCutPlaneInImageFrame()
    double micrometersPerVoxel;
    SingleCut* currentWidget;
    int nextWidgetIndex;
    std::map<int, SingleCut*> indexWidgets;
};

#endif /* CUTPLANNER_H_ */

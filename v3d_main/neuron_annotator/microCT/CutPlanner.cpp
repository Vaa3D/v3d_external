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
    , currentWidget(NULL)
    , nextWidgetIndex(0)
{
    ui.setupUi(this);

    setBasePlaneWidget(ui.basePlaneWidget, "Base", "-Z");
    initSingleCut(ui.topCutWidget, "Top", "+Z");
    initSingleCut(ui.frontCutWidget, "Front/A", "-Y");
    initSingleCut(ui.rightCutWidget, "Right/B", "+X");
    initSingleCut(ui.rearCutWidget, "Rear/C", "+Y");
    initSingleCut(ui.leftCutWidget, "Left/D", "-X");

    setCurrentWidget(ui.basePlaneWidget->getWidgetIndex());
}

CutPlanner::~CutPlanner()
{}

void CutPlanner::initSingleCut(SingleCut* widget, QString name, QString axis) {
    widget->setCamera(camera);
    widget->setName(name);
    widget->setAxis(axis);
    widget->setMicrometersPerVoxel(micrometersPerVoxel);
    widget->setWidgetIndex(nextWidgetIndex);
    indexWidgets[nextWidgetIndex] = widget;
    nextWidgetIndex++;
    connect(widget, SIGNAL(clipPlaneRequested()),
           this, SIGNAL(clipPlaneRequested()));
    connect(widget, SIGNAL(cutGuideRequested(bool)),
            this, SIGNAL(cutGuideRequested(bool)));
    connect(widget, SIGNAL(rotationAdjusted(Rotation3D)),
            this, SIGNAL(rotationAdjusted(Rotation3D)));
    connect(widget, SIGNAL(currentWidgetRequested(int)),
        this, SLOT(setCurrentWidget(int)));
}

void CutPlanner::setBasePlaneWidget(SingleCut* widget, QString name, QString axis) {
    widget->setCamera(camera);
    widget->setName(name);
    widget->setAxis(axis);
    widget->setMicrometersPerVoxel(micrometersPerVoxel);
    widget->setKeepPlane(true);
    widget->setWidgetIndex(nextWidgetIndex);
    indexWidgets[nextWidgetIndex] = widget;
    nextWidgetIndex++;
    // Remove elements that are only relevant for cut planes, not show planes
    widget->ui.edgeButton->hide();
    widget->ui.cutDistanceLineEdit->hide();
    widget->ui.label->hide();
    widget->ui.cutButton->setEnabled(true);
    // connect(widget, SIGNAL(clipPlaneRequested()),
    //        this, SIGNAL(clipPlaneRequested()));
    connect(widget, SIGNAL(cutGuideRequested(bool)),
            this, SIGNAL(cutGuideRequested(bool)));
    connect(widget, SIGNAL(rotationAdjusted(Rotation3D)),
            this, SIGNAL(rotationAdjusted(Rotation3D)));
    connect(widget, SIGNAL(keepPlaneRequested()),
           this, SIGNAL(keepPlaneRequested()));
    connect(widget, SIGNAL(currentWidgetRequested(int)),
        this, SLOT(setCurrentWidget(int)));
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
    // qDebug("valueChanged");
    setMicrometersPerVoxel(val);
    emit cutGuideRequested(true);
}

QString CutPlanner::getPlanFileName() {
    static QString planFolder = "";
    QString planFileName = QFileDialog::getSaveFileName(this,
        "Save Trim Plan File",
        planFolder,
        "*.txt");
    if (planFileName.isEmpty())
        return planFileName;
    if (QFileInfo(planFileName).suffix().isEmpty())
        planFileName.append(".txt");
    return planFileName;
}

/* slot */
void CutPlanner::on_savePlanButton_clicked() {
    QString planFileName = getPlanFileName();
    if (planFileName.isEmpty())
        return;
    QFile file(planFileName);
    if (! file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(
            this,
            "Error writing plan file",
            "Could not create plan file");
        return;
    }
    QTextStream out(&file);
    savePlanFile(out);
    file.close();
    QMessageBox::information(
        this,
        "Plan file saved",
        "Finished saving file "+planFileName);
}

void CutPlanner::savePlanFile(QTextStream& out) {
    out << "MicroCT Trim plan\n";
    out << QDateTime::currentDateTime().toLocalTime().toString() << "\n";
    out << "Axis"
        << "\t"
        << "Name"
        << "\t"
        << "Amount"
        << "\t"
        << "Units"
        << "\n";
    ui.topCutWidget->savePlanFileLine(out);
    ui.frontCutWidget->savePlanFileLine(out);
    ui.rightCutWidget->savePlanFileLine(out);
    ui.rearCutWidget->savePlanFileLine(out);
    ui.leftCutWidget->savePlanFileLine(out);
}


#ifndef NAMAINWINDOW_H
#define NAMAINWINDOW_H

#include <QObject>
#include <QMenu>
#include <QAction>
#include <QMainWindow>
#include "ui_NaMainWindow.h"
#include "AnnotationSession.h"
#include "GalleryButton.h"

class NutateThread;
class NeuronSelector;

class NaMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    NaMainWindow();
    void setV3DDefaultModeCheck(bool checkState);
    void setNeuronAnnotatorModeCheck(bool checkState);
    void handleCoordinatedCloseEvent(QCloseEvent *event);
    bool loadAnnotationSessionFromDirectory(QDir imageInputDirectory);
    bool closeAnnotationSession();
    bool loadMy4DImage(const My4DImage * img, const My4DImage * neuronMaskImg = NULL);

public slots:
    void on_actionV3DDefault_triggered();
    void on_actionNeuronAnnotator_triggered();
    void on_actionQuit_triggered();
    void on_actionOpen_triggered();
    void on_actionLoad_Tiff_triggered();
    void on_actionCell_Counter_3D_2ch_lsm_triggered();

    void setZRange(int minZ, int maxZ); // update number of z slices
    void unifyCameras(bool bDoUnify); // keep all zoom, focus, rotate the same
    void setNutate(bool bDoNutate);
    void nutate(const Rotation3D&);

    void updateAnnotationModels();

protected slots:
    void updateThumbnailGamma(double gamma);
    void on3DViewerRotationChanged(const Rotation3D& rot)
    {
        Vector3D angles = rot.convertBodyFixedXYZRotationToThreeAngles();
        int rotX = Na3DWidget::radToDeg(angles.x());
        int rotY = Na3DWidget::radToDeg(angles.y());
        int rotZ = Na3DWidget::radToDeg(angles.z());
        int oldRotX = ui.rotXWidget->spinBox->value();
        int oldRotY = ui.rotYWidget->spinBox->value();
        int oldRotZ = ui.rotZWidget->spinBox->value();
        if (Na3DWidget::eulerAnglesAreEquivalent(rotX, rotY, rotZ, oldRotX, oldRotY, oldRotZ))
            return;
        // Block signals from individual rot widgets until we update them all
        ui.rotXWidget->blockSignals(true);
        ui.rotYWidget->blockSignals(true);
        ui.rotZWidget->blockSignals(true);

        ui.rotXWidget->setAngle(rotX);
        ui.rotYWidget->setAngle(rotY);
        ui.rotZWidget->setAngle(rotZ);

        ui.rotXWidget->blockSignals(false);
        ui.rotYWidget->blockSignals(false);
        ui.rotZWidget->blockSignals(false);
    }
    void update3DViewerXYZBodyRotation()
    {
        int rotX = ui.rotXWidget->spinBox->value();
        int rotY = ui.rotYWidget->spinBox->value();
        int rotZ = ui.rotZWidget->spinBox->value();
        // qDebug() << rotX << ", " << rotY << ", " << rotZ;
        ui.v3dr_glwidget->setXYZBodyRotationInt(rotX, rotY, rotZ);
    }

protected:
    void closeEvent(QCloseEvent *event);

private:
    AnnotationSession* annotationSession;
    Ui::NaMainWindow ui;
    void createMaskGallery();
    QList<GalleryButton*>
    mipGalleryButtonList;
    My4DImage currentStackImage;
    CameraModel sharedCameraModel; // optional camera sharing
    NutateThread * nutateThread;
	
	NeuronSelector* neuronSelector;
};

class NutateThread : public QThread
{
    Q_OBJECT

public:
    NutateThread(qreal cyclesPerSecond, QObject * parent = NULL)
        : QThread(parent)
        , speed(cyclesPerSecond)
        , interval(0.200) // update every 200 milliseconds
        , currentAngle(0.0)
    {
        deltaAngle = 2.0 * 3.14159 * cyclesPerSecond * interval;
    }

    void run()
    {
        while(true) {
            if (paused) {
                sleep(0.5);
                continue;
            }
            // qDebug() << "nutation angle = " << currentAngle;
            rot = deltaNutation(currentAngle, deltaAngle);
            emit nutate(rot);
            currentAngle += deltaAngle;
            while (currentAngle > 2.0 * 3.14159) currentAngle -= 2.0 * 3.14159;
            msleep( (1000.0 * deltaAngle) / (2.0 * 3.14159 * speed) );
        }
    }
    void pause() {paused = true;}
    void unpause() {paused = false;}

signals:
    void nutate(const Rotation3D&);

protected:
    bool paused;
    qreal speed;
    qreal currentAngle;
    qreal interval;
    qreal deltaAngle;
    Rotation3D rot;
};

#endif // NAMAINWINDOW_H

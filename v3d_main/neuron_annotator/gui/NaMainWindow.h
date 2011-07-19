#ifndef NAMAINWINDOW_H
#define NAMAINWINDOW_H

#include <QObject>
#include <QMenu>
#include <QAction>
#include <QMainWindow>
#include "ui_NaMainWindow.h"
#include "../AnnotationSession.h"
#include "GalleryButton.h"
#include "../FragmentSelectionModel.h"
#include "../ExportFile.h"

class NeuronSelector;

class NutateThread : public QThread
{
    Q_OBJECT
public:
    NutateThread(qreal cyclesPerSecond, QObject * parent = NULL);
    void run();
    void pause();
    void unpause();

signals:
    void nutate(const Rotation3D&);

protected:
    bool paused;
    qreal speed;
    qreal interval;
    qreal currentAngle;
    qreal deltaAngle;
    Rotation3D rot;
};

class NaMainWindow : public QMainWindow
{
    Q_OBJECT

    enum ViewerIndex {
        // indices correspond to children of ui.viewerStackedWidget
        VIEWER_MIP = 0,
        VIEWER_ZSTACK = 1,
        VIEWER_3D = 2
    };

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
    void on_action3D_Volume_triggered();
    void on_action2D_MIP_triggered();
    void on_actionScreenShot_triggered();

    void setZRange(int minZ, int maxZ); // update number of z slices
    void unifyCameras(bool bDoUnify); // keep all zoom, focus, rotate the same
    void setNutate(bool bDoNutate);
    void nutate(const Rotation3D&);
    void resetView();
    void resetColors();
    void updateViewers();
    void synchronizeGalleryButtonsToAnnotationSession(QString updateString);

protected slots:
    void updateThumbnailGamma(qreal gamma);
    void on3DViewerRotationChanged(const Rotation3D& rot);
    void update3DViewerXYZBodyRotation();
    void onViewerChanged(int viewerIndex);
    void set3DProgress(int);
    void complete3DProgress();
    void set3DProgressMessage(QString);

protected:
    void closeEvent(QCloseEvent *event);

private:
    AnnotationSession* annotationSession;
    Ui::NaMainWindow ui;
    void createNeuronGallery();
    void createOverlayGallery();
    QList<GalleryButton*> overlayGalleryButtonList;
    QList<GalleryButton*> neuronGalleryButtonList;
    CameraModel sharedCameraModel; // optional camera sharing
    NutateThread * nutateThread;
    NeuronSelector* neuronSelector;
    FragmentSelectionModel fragmentSelectionModel;
    QLabel * statusProgressMessage;
    QProgressBar * statusProgressBar;
};

#endif // NAMAINWINDOW_H

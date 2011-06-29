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
    qreal currentAngle;
    qreal interval;
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
    void on_actionLoad_Tiff_triggered();
    void on_actionCell_Counter_3D_2ch_lsm_triggered();

    void setZRange(int minZ, int maxZ); // update number of z slices
    void unifyCameras(bool bDoUnify); // keep all zoom, focus, rotate the same
    void setNutate(bool bDoNutate);
    void nutate(const Rotation3D&);
    void resetView();
    void resetColors();
    void updateViewers();

    void updateAnnotationModels();

protected slots:
    void updateThumbnailGamma(qreal gamma);
    void on3DViewerRotationChanged(const Rotation3D& rot);
    void update3DViewerXYZBodyRotation();
    void onViewerChanged(int viewerIndex);

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
    FragmentSelectionModel fragmentSelectionModel;
};

#endif // NAMAINWINDOW_H

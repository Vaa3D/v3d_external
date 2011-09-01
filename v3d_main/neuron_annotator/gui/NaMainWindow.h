#ifndef NAMAINWINDOW_H
#define NAMAINWINDOW_H

#include <QObject>
#include <QMenu>
#include <QAction>
#include <QMainWindow>
#include "ui_NaMainWindow.h"
#include "../AnnotationSession.h"
#include "GalleryButton.h"
#include "../data_model/NeuronSelectionModel.h"
#include "../ExportFile.h"
#include "../data_model/NeuronSelectionModel.h"

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

class OpenFileAction : public QAction
{
    Q_OBJECT

public:
    OpenFileAction(QObject * p_parent = NULL) : QAction(p_parent)
    {
        connect(this, SIGNAL(triggered()),
                this, SLOT(onTriggered()));
    }
    void setFileName(QString fileName) {
        setText(fileName);
        m_fileName = fileName;
    }

signals:
    void openFileRequested(QString fileName);

protected slots:
    void onTriggered() {emit openFileRequested(m_fileName);}

protected:
    QString m_fileName;
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
    AnnotationSession* getAnnotationSession() const;

public slots:
    void openMulticolorImageStack(QString dirName);
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
    void on3DViewerRotationChanged(const Rotation3D& rot);
    void update3DViewerXYZBodyRotation();
    void onViewerChanged(int viewerIndex);
    void set3DProgress(int);
    void complete3DProgress();
    void set3DProgressMessage(QString);
    void processUpdatedVolumeData(); // respond to newly loaded image data
    void updateGalleries();

protected:
    void closeEvent(QCloseEvent *event);
    // Recent files list
    void addDirToRecentFilesList(QDir);
    void updateRecentFileActions();

private:
    AnnotationSession* annotationSession;
    Ui::NaMainWindow ui;
    void updateNeuronGallery();
    void updateOverlayGallery();
    QList<GalleryButton*> overlayGalleryButtonList;
    QList<GalleryButton*> neuronGalleryButtonList;
    CameraModel sharedCameraModel; // optional camera sharing
    NutateThread *nutateThread;
    NeuronSelector* neuronSelector;
    QLabel * statusProgressMessage;
    QProgressBar * statusProgressBar;
    enum { MaxRecentFiles = 10 }; // clever trick to get constant in header and also use it
    OpenFileAction *recentFileActions[MaxRecentFiles];
    QString suggestedExportFilenameFromCurrentState(const NeuronSelectionModel::Reader&);
};

#endif // NAMAINWINDOW_H

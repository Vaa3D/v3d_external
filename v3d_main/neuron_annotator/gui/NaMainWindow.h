#ifndef NAMAINWINDOW_H
#define NAMAINWINDOW_H

#include <QObject>
#include <QMenu>
#include <QAction>
#include <QMainWindow>
#include <QFileSystemWatcher>

#include "ui_NaMainWindow.h"
#include "../DataFlowModel.h"
#include "GalleryButton.h"
#include "../data_model/NeuronSelectionModel.h"
#include "../ExportFile.h"
#include "../data_model/NeuronSelectionModel.h"
#include "NeuronContextMenu.h"
#include "DynamicRangeTool.h"
#include "../NeuronSelector.h"

class NutateThread : public QThread
{
    Q_OBJECT
public:
    NutateThread(qreal cyclesPerSecond, QObject * parent = NULL);
    void run();
    void pause();
    void unpause();
    bool isPaused() {return paused;}

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
        VIEWER_3D = 2,
        VIEWER_WAIT_LOADING_SCREEN
    };

    enum ViewMode {
        VIEW_SINGLE_STACK,
        VIEW_NEURON_SEPARATION
    };

public:
    NaMainWindow(QWidget * parent = 0, Qt::WindowFlags flags = 0);
    void setV3DDefaultModeCheck(bool checkState);
    void setNeuronAnnotatorModeCheck(bool checkState);
    void handleCoordinatedCloseEvent(QCloseEvent *event);
    virtual void keyPressEvent(QKeyEvent *e);
    bool loadSeparationDirectoryV1Pbd(QDir imageInputDirectory);
    bool loadSeparationDirectoryV2Mpeg(QDir imageInputDirectory);
    bool tearDownOldDataFlowModel();
    bool createNewDataFlowModel();
    DataFlowModel* getDataFlowModel() const;
    QString getStackPathWithDialog();
    QString getDataDirectoryPathWithDialog();

signals:
    void channelVisibilityChanged(int, bool);
    void nutatingChanged(bool);
    void defaultVaa3dFileLoadRequested(QString fileName);
    void crosshairVisibilityChanged(bool);
    void singleStackLoadRequested(QString fileName);
    void benchmarkTimerResetRequested();
    void benchmarkTimerPrintRequested(QString);
    void initializeColorModelRequested();
    void initializeSelectionModelRequested();
    void subsampleLabelPbdFileNamed(QString);
    void stagedLoadRequested();

public slots:
    void resetVolumeCutRange();
    void exitFullScreen();
    void setFullScreen(bool);
    void setViewMode(ViewMode mode);
    void loadSingleStack(QString fileName);
    void loadSingleStack(QString fileName, bool useVaa3dClassic);
    void onDataLoadStarted();
    void onDataLoadFinished();
    bool openMulticolorImageStack(QString dirName);
    void on_actionV3DDefault_triggered();
    void on_actionNeuronAnnotator_triggered();
    void on_actionQuit_triggered();
    void on_actionOpen_triggered();
    void on_actionOpen_Single_Movie_Stack_triggered();
    void on_action3D_Volume_triggered();
    void on_action2D_MIP_triggered();
    void on_actionScreenShot_triggered();
    void on_actionLoad_movie_as_texture_triggered();
    bool on_actionLoad_fast_separation_result_triggered();
    void on_actionPreferences_triggered();
    void setZRange(int minZ, int maxZ); // update number of z slices
    void unifyCameras(bool bDoUnify); // keep all zoom, focus, rotate the same
    void setNutate(bool bDoNutate);
    void nutate(const Rotation3D&);
    void resetView();
    void updateViewers();
    void synchronizeGalleryButtonsToAnnotationSession(QString updateString);
    void setChannelZeroVisibility(bool);
    void setChannelOneVisibility(bool);
    void setChannelTwoVisibility(bool);
    void setChannelThreeVisibility(bool); // reference channel
    // help comminicate reference channel changes between DataColorModel and NeuronSelectionModel
    void onColorModelChanged();
    void onSelectionModelVisibilityChanged();
    void onHdrChannelChanged(NaZStackWidget::Color);
    void supportQuadStereo(bool b);
    void showDynamicRangeTool();
    void setTitle(QString title);
    void setCrosshairVisibility(bool);

protected slots:
    void resetBenchmarkTimer();
    void printBenchmarkTimer(QString message);
    void on3DViewerRotationChanged(const Rotation3D& rot);
    void update3DViewerXYZBodyRotation();
    void onSlabThicknessChanged(int t);
    void onViewerChanged(int viewerIndex);
    void set3DProgress(int);
    void complete3DProgress();
    void set3DProgressMessage(QString);
    void processUpdatedVolumeData(); // respond to newly loaded image data
    void updateGalleries();
    void initializeGalleries();
    void setProgressValue(int);
    void setProgressMessage(QString);
    void completeProgress();
    void abortProgress(QString);
    void applyCustomCut();
    void toggleCustomCutMode();
    void onExportFinished(QString fileName);
    void onExportFailed(QString fileName, QString message);


protected:
    void setDataFlowModel(DataFlowModel* dataFlowModelParam);
    void initializeContextMenus();
    void initializeStereo3DOptions();
    void connectContextMenus(const NeuronSelectionModel& neuronSelectionModel);
    void closeEvent(QCloseEvent *event);
    // Recent files list
    void addDirToRecentFilesList(QDir);
    void addFileNameToRecentFilesList(QString fileName);
    void updateRecentFileActions();
    void connectCustomCut();
    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);
    void retrieveCrosshairVisibilitySetting();
    void moveEvent ( QMoveEvent * event );

private:
    QString suggestedExportFilenameFromCurrentState(const NeuronSelectionModel::Reader&);
    void initializeNeuronGallery();
    void initializeOverlayGallery();
    void updateNeuronGallery();
    void updateOverlayGallery();
    static const int maxRecentFiles = 10;

    DataFlowModel* dataFlowModel;
    Ui::NaMainWindow ui;
    CameraModel sharedCameraModel; // optional camera sharing
    NutateThread *nutateThread;
    NeuronSelector neuronSelector;
    QLabel * statusProgressMessage;
    QProgressBar * statusProgressBar;

    QUndoStack* undoStack;

    // Actions for viewer context menus
    QAction* showAllNeuronsInEmptySpaceAction;
    QAction* hideAllAction;
    QAction* selectNoneAction;
    //
    NeuronContextMenu* neuronContextMenu;
    QMenu* viewerContextMenu;
    ViewerIndex recentViewer;
    DynamicRangeTool* dynamicRangeTool;

    bool isInCustomCutMode;
    bool bShowCrosshair;
    ViewMode viewMode;

    QList<GalleryButton*> overlayGalleryButtonList;
    QList<GalleryButton*> neuronGalleryButtonList;

    QVector<OpenFileAction*> recentFileActions;
    QTime mainWindowStopWatch;
};

#endif // NAMAINWINDOW_H

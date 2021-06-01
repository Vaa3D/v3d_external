/****************************************************************************
** Meta object code from reading C++ file 'NaMainWindow.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../neuron_annotator/gui/NaMainWindow.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NaMainWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_NutateThread[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      14,   13,   13,   13, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_NutateThread[] = {
    "NutateThread\0\0nutate(Rotation3D)\0"
};

void NutateThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        NutateThread *_t = static_cast<NutateThread *>(_o);
        switch (_id) {
        case 0: _t->nutate((*reinterpret_cast< const Rotation3D(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData NutateThread::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject NutateThread::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_NutateThread,
      qt_meta_data_NutateThread, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &NutateThread::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *NutateThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *NutateThread::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_NutateThread))
        return static_cast<void*>(const_cast< NutateThread*>(this));
    return QThread::qt_metacast(_clname);
}

int NutateThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void NutateThread::nutate(const Rotation3D & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_OpenFileAction[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      25,   16,   15,   15, 0x05,

 // slots: signature, parameters, type, tag, flags
      52,   15,   15,   15, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_OpenFileAction[] = {
    "OpenFileAction\0\0fileName\0"
    "openFileRequested(QString)\0onTriggered()\0"
};

void OpenFileAction::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        OpenFileAction *_t = static_cast<OpenFileAction *>(_o);
        switch (_id) {
        case 0: _t->openFileRequested((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->onTriggered(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData OpenFileAction::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject OpenFileAction::staticMetaObject = {
    { &QAction::staticMetaObject, qt_meta_stringdata_OpenFileAction,
      qt_meta_data_OpenFileAction, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &OpenFileAction::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *OpenFileAction::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *OpenFileAction::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_OpenFileAction))
        return static_cast<void*>(const_cast< OpenFileAction*>(this));
    return QAction::qt_metacast(_clname);
}

int OpenFileAction::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAction::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void OpenFileAction::openFileRequested(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_NaMainWindow[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      89,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      12,       // signalCount

 // signals: signature, parameters, type, tag, flags
      16,   14,   13,   13, 0x05,
      51,   13,   13,   13, 0x05,
      82,   73,   13,   13, 0x05,
     125,  121,   13,   13, 0x05,
     160,   13,   13,   13, 0x05,
     193,  121,   13,   13, 0x05,
     224,   13,   13,   13, 0x05,
     255,   13,   13,   13, 0x05,
     293,   13,   13,   13, 0x05,
     325,   13,   13,   13, 0x05,
     361,   13,   13,   13, 0x05,
     394,   13,   13,   13, 0x05,

 // slots: signature, parameters, type, tag, flags
     416,   13,   13,   13, 0x0a,
     438,   13,   13,   13, 0x0a,
     455,   13,   13,   13, 0x0a,
     480,  475,   13,   13, 0x0a,
     502,   13,   13,   13, 0x0a,
     539,  121,   13,   13, 0x0a,
     561,   73,   13,   13, 0x0a,
     593,  586,   13,   13, 0x0a,
     638,  618,   13,   13, 0x0a,
     665,   13,   13,   13, 0x0a,
     685,   13,   13,   13, 0x0a,
     711,  121,  706,   13, 0x0a,
     750,  742,  706,   13, 0x0a,
     789,  784,   13,   13, 0x0a,
     812,   13,   13,   13, 0x0a,
     842,   13,   13,   13, 0x0a,
     886,   13,   13,   13, 0x0a,
     940,   13,   13,   13, 0x0a,
     977,   13,   13,   13, 0x0a,
    1010,   13,   13,   13, 0x0a,
    1042,   13,   13,   13, 0x0a,
    1082,   13,   13,   13, 0x0a,
    1128,   13,   13,   13, 0x0a,
    1165,   13,   13,   13, 0x0a,
    1191,   13,   13,   13, 0x0a,
    1217,   13,   13,   13, 0x0a,
    1257,   13,   13,   13, 0x0a,
    1302,   13,   13,   13, 0x0a,
    1334,   13,   13,   13, 0x0a,
    1365,   13,   13,   13, 0x0a,
    1393,   13,   13,   13, 0x0a,
    1425,   13,   13,   13, 0x0a,
    1468,   13,   13,   13, 0x0a,
    1501,   13,   13,   13, 0x0a,
    1540,   13,   13,   13, 0x0a,
    1582, 1578,   13,   13, 0x0a,
    1640, 1630,   13,   13, 0x0a,
    1668, 1659,   13,   13, 0x0a,
    1697, 1687,   13,   13, 0x0a,
    1713,   13,   13,   13, 0x0a,
    1737,   13,   13,   13, 0x0a,
    1756,   13,   13,   13, 0x0a,
    1768,   13,   13,   13, 0x0a,
    1797, 1784,   13,   13, 0x0a,
    1851,   13,   13,   13, 0x0a,
    1882,   13,   13,   13, 0x0a,
    1912,   13,   13,   13, 0x0a,
    1942,   13,   13,   13, 0x0a,
    1974,   13,   13,   13, 0x0a,
    1996,   13,   13,   13, 0x0a,
    2032,   13,   13,   13, 0x0a,
    2077, 2075,   13,   13, 0x0a,
    2101,   13,   13,   13, 0x0a,
    2130, 2124,   13,   13, 0x0a,
    2148,   13,   13,   13, 0x0a,
    2177,   13,   13,   13, 0x09,
    2207, 2199,   13,   13, 0x09,
    2240, 2236,   13,   13, 0x09,
    2278,   13,   13,   13, 0x09,
    2312, 2310,   13,   13, 0x09,
    2352, 2340,   13,   13, 0x09,
    2373,   13,   13,   13, 0x09,
    2392,   13,   13,   13, 0x09,
    2413,   13,   13,   13, 0x09,
    2443,   13,   13,   13, 0x09,
    2470,   13,   13,   13, 0x09,
    2488,   13,   13,   13, 0x09,
    2510,   13,   13,   13, 0x09,
    2532,   13,   13,   13, 0x09,
    2560,   13,   13,   13, 0x09,
    2579,   13,   13,   13, 0x09,
    2602,   13,   13,   13, 0x09,
    2619,   13,   13,   13, 0x09,
    2651, 2642,   13,   13, 0x09,
    2674,   13,   13,   13, 0x09,
    2696,   73,   13,   13, 0x09,
    2739, 2722,   13,   13, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_NaMainWindow[] = {
    "NaMainWindow\0\0,\0channelVisibilityChanged(int,bool)\0"
    "nutatingChanged(bool)\0fileName\0"
    "defaultVaa3dFileLoadRequested(QString)\0"
    "url\0defaultVaa3dUrlLoadRequested(QUrl)\0"
    "crosshairVisibilityChanged(bool)\0"
    "singleStackLoadRequested(QUrl)\0"
    "benchmarkTimerResetRequested()\0"
    "benchmarkTimerPrintRequested(QString)\0"
    "initializeColorModelRequested()\0"
    "initializeSelectionModelRequested()\0"
    "subsampleLabelPbdFileNamed(QUrl)\0"
    "stagedLoadRequested()\0resetVolumeCutRange()\0"
    "exitFullScreen()\0setFullScreen(bool)\0"
    "mode\0setViewMode(ViewMode)\0"
    "labelNeuronsAsFlyBrainCompartments()\0"
    "loadSingleStack(QUrl)\0loadSingleStack(QString)\0"
    "entity\0loadSingleStack(Entity*)\0"
    "url,useVaa3dClassic\0loadSingleStack(QUrl,bool)\0"
    "onDataLoadStarted()\0onDataLoadFinished()\0"
    "bool\0openMulticolorImageStack(QUrl)\0"
    "dirName\0openMulticolorImageStack(QString)\0"
    "name\0openFileOrUrl(QString)\0"
    "on_action1280x720_triggered()\0"
    "on_actionAdd_landmark_at_cursor_triggered()\0"
    "on_actionAppend_key_frame_at_current_view_triggered()\0"
    "on_actionClear_landmarks_triggered()\0"
    "on_actionClear_movie_triggered()\0"
    "on_actionV3DDefault_triggered()\0"
    "on_actionMeasure_Frame_Rate_triggered()\0"
    "on_actionOpen_microCT_Cut_Planner_triggered()\0"
    "on_actionNeuronAnnotator_triggered()\0"
    "on_actionQuit_triggered()\0"
    "on_actionOpen_triggered()\0"
    "on_actionOpen_Octree_Volume_triggered()\0"
    "on_actionOpen_Single_Movie_Stack_triggered()\0"
    "on_actionPlay_movie_triggered()\0"
    "on_action3D_Volume_triggered()\0"
    "on_action2D_MIP_triggered()\0"
    "on_actionScreenShot_triggered()\0"
    "on_actionLoad_movie_as_texture_triggered()\0"
    "on_actionPreferences_triggered()\0"
    "on_actionSave_movie_frames_triggered()\0"
    "on_actionX_Rotation_Movie_triggered()\0"
    "val\0on_zThicknessDoubleSpinBox_valueChanged(double)\0"
    "minZ,maxZ\0setZRange(int,int)\0bDoUnify\0"
    "unifyCameras(bool)\0bDoNutate\0"
    "setNutate(bool)\0setRotation(Rotation3D)\0"
    "nutate(Rotation3D)\0resetView()\0"
    "updateViewers()\0updateString\0"
    "synchronizeGalleryButtonsToAnnotationSession(QString)\0"
    "setChannelZeroVisibility(bool)\0"
    "setChannelOneVisibility(bool)\0"
    "setChannelTwoVisibility(bool)\0"
    "setChannelThreeVisibility(bool)\0"
    "onColorModelChanged()\0"
    "onSelectionModelVisibilityChanged()\0"
    "onHdrChannelChanged(NaZStackWidget::Color)\0"
    "b\0supportQuadStereo(bool)\0"
    "showDynamicRangeTool()\0title\0"
    "setTitle(QString)\0setCrosshairVisibility(bool)\0"
    "resetBenchmarkTimer()\0message\0"
    "printBenchmarkTimer(QString)\0rot\0"
    "on3DViewerRotationChanged(Rotation3D)\0"
    "update3DViewerXYZBodyRotation()\0t\0"
    "onSlabThicknessChanged(int)\0viewerIndex\0"
    "onViewerChanged(int)\0set3DProgress(int)\0"
    "complete3DProgress()\0set3DProgressMessage(QString)\0"
    "processUpdatedVolumeData()\0updateGalleries()\0"
    "initializeGalleries()\0setProgressValue(int)\0"
    "setProgressMessage(QString)\0"
    "completeProgress()\0abortProgress(QString)\0"
    "applyCustomCut()\0applyCustomKeepPlane()\0"
    "doCustom\0setCustomCutMode(bool)\0"
    "toggleCustomCutMode()\0onExportFinished(QString)\0"
    "fileName,message\0onExportFailed(QString,QString)\0"
};

void NaMainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        NaMainWindow *_t = static_cast<NaMainWindow *>(_o);
        switch (_id) {
        case 0: _t->channelVisibilityChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 1: _t->nutatingChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->defaultVaa3dFileLoadRequested((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 3: _t->defaultVaa3dUrlLoadRequested((*reinterpret_cast< QUrl(*)>(_a[1]))); break;
        case 4: _t->crosshairVisibilityChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->singleStackLoadRequested((*reinterpret_cast< QUrl(*)>(_a[1]))); break;
        case 6: _t->benchmarkTimerResetRequested(); break;
        case 7: _t->benchmarkTimerPrintRequested((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 8: _t->initializeColorModelRequested(); break;
        case 9: _t->initializeSelectionModelRequested(); break;
        case 10: _t->subsampleLabelPbdFileNamed((*reinterpret_cast< QUrl(*)>(_a[1]))); break;
        case 11: _t->stagedLoadRequested(); break;
        case 12: _t->resetVolumeCutRange(); break;
        case 13: _t->exitFullScreen(); break;
        case 14: _t->setFullScreen((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 15: _t->setViewMode((*reinterpret_cast< ViewMode(*)>(_a[1]))); break;
        case 16: _t->labelNeuronsAsFlyBrainCompartments(); break;
        case 17: _t->loadSingleStack((*reinterpret_cast< QUrl(*)>(_a[1]))); break;
        case 18: _t->loadSingleStack((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 19: _t->loadSingleStack((*reinterpret_cast< Entity*(*)>(_a[1]))); break;
        case 20: _t->loadSingleStack((*reinterpret_cast< QUrl(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 21: _t->onDataLoadStarted(); break;
        case 22: _t->onDataLoadFinished(); break;
        case 23: { bool _r = _t->openMulticolorImageStack((*reinterpret_cast< QUrl(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 24: { bool _r = _t->openMulticolorImageStack((*reinterpret_cast< QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 25: _t->openFileOrUrl((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 26: _t->on_action1280x720_triggered(); break;
        case 27: _t->on_actionAdd_landmark_at_cursor_triggered(); break;
        case 28: _t->on_actionAppend_key_frame_at_current_view_triggered(); break;
        case 29: _t->on_actionClear_landmarks_triggered(); break;
        case 30: _t->on_actionClear_movie_triggered(); break;
        case 31: _t->on_actionV3DDefault_triggered(); break;
        case 32: _t->on_actionMeasure_Frame_Rate_triggered(); break;
        case 33: _t->on_actionOpen_microCT_Cut_Planner_triggered(); break;
        case 34: _t->on_actionNeuronAnnotator_triggered(); break;
        case 35: _t->on_actionQuit_triggered(); break;
        case 36: _t->on_actionOpen_triggered(); break;
        case 37: _t->on_actionOpen_Octree_Volume_triggered(); break;
        case 38: _t->on_actionOpen_Single_Movie_Stack_triggered(); break;
        case 39: _t->on_actionPlay_movie_triggered(); break;
        case 40: _t->on_action3D_Volume_triggered(); break;
        case 41: _t->on_action2D_MIP_triggered(); break;
        case 42: _t->on_actionScreenShot_triggered(); break;
        case 43: _t->on_actionLoad_movie_as_texture_triggered(); break;
        case 44: _t->on_actionPreferences_triggered(); break;
        case 45: _t->on_actionSave_movie_frames_triggered(); break;
        case 46: _t->on_actionX_Rotation_Movie_triggered(); break;
        case 47: _t->on_zThicknessDoubleSpinBox_valueChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 48: _t->setZRange((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 49: _t->unifyCameras((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 50: _t->setNutate((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 51: _t->setRotation((*reinterpret_cast< Rotation3D(*)>(_a[1]))); break;
        case 52: _t->nutate((*reinterpret_cast< const Rotation3D(*)>(_a[1]))); break;
        case 53: _t->resetView(); break;
        case 54: _t->updateViewers(); break;
        case 55: _t->synchronizeGalleryButtonsToAnnotationSession((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 56: _t->setChannelZeroVisibility((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 57: _t->setChannelOneVisibility((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 58: _t->setChannelTwoVisibility((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 59: _t->setChannelThreeVisibility((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 60: _t->onColorModelChanged(); break;
        case 61: _t->onSelectionModelVisibilityChanged(); break;
        case 62: _t->onHdrChannelChanged((*reinterpret_cast< NaZStackWidget::Color(*)>(_a[1]))); break;
        case 63: _t->supportQuadStereo((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 64: _t->showDynamicRangeTool(); break;
        case 65: _t->setTitle((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 66: _t->setCrosshairVisibility((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 67: _t->resetBenchmarkTimer(); break;
        case 68: _t->printBenchmarkTimer((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 69: _t->on3DViewerRotationChanged((*reinterpret_cast< const Rotation3D(*)>(_a[1]))); break;
        case 70: _t->update3DViewerXYZBodyRotation(); break;
        case 71: _t->onSlabThicknessChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 72: _t->onViewerChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 73: _t->set3DProgress((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 74: _t->complete3DProgress(); break;
        case 75: _t->set3DProgressMessage((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 76: _t->processUpdatedVolumeData(); break;
        case 77: _t->updateGalleries(); break;
        case 78: _t->initializeGalleries(); break;
        case 79: _t->setProgressValue((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 80: _t->setProgressMessage((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 81: _t->completeProgress(); break;
        case 82: _t->abortProgress((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 83: _t->applyCustomCut(); break;
        case 84: _t->applyCustomKeepPlane(); break;
        case 85: _t->setCustomCutMode((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 86: _t->toggleCustomCutMode(); break;
        case 87: _t->onExportFinished((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 88: _t->onExportFailed((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData NaMainWindow::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject NaMainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_NaMainWindow,
      qt_meta_data_NaMainWindow, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &NaMainWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *NaMainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *NaMainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_NaMainWindow))
        return static_cast<void*>(const_cast< NaMainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int NaMainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 89)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 89;
    }
    return _id;
}

// SIGNAL 0
void NaMainWindow::channelVisibilityChanged(int _t1, bool _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void NaMainWindow::nutatingChanged(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void NaMainWindow::defaultVaa3dFileLoadRequested(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void NaMainWindow::defaultVaa3dUrlLoadRequested(QUrl _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void NaMainWindow::crosshairVisibilityChanged(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void NaMainWindow::singleStackLoadRequested(QUrl _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void NaMainWindow::benchmarkTimerResetRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 6, 0);
}

// SIGNAL 7
void NaMainWindow::benchmarkTimerPrintRequested(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void NaMainWindow::initializeColorModelRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 8, 0);
}

// SIGNAL 9
void NaMainWindow::initializeSelectionModelRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 9, 0);
}

// SIGNAL 10
void NaMainWindow::subsampleLabelPbdFileNamed(QUrl _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}

// SIGNAL 11
void NaMainWindow::stagedLoadRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 11, 0);
}
QT_END_MOC_NAMESPACE

/****************************************************************************
** Meta object code from reading C++ file 'v3dr_mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../3drenderer/v3dr_mainwindow.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'v3dr_mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_V3dR_MainWindow[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      24,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      17,   16,   16,   16, 0x0a,
      29,   16,   16,   16, 0x0a,
      51,   16,   16,   16, 0x0a,
      73,   16,   16,   16, 0x0a,
      95,   16,   16,   16, 0x0a,
     114,   16,   16,   16, 0x0a,
     136,   16,   16,   16, 0x0a,
     157,   16,   16,   16, 0x0a,
     179,   16,   16,   16, 0x0a,
     199,   16,   16,   16, 0x0a,
     218,   16,   16,   16, 0x0a,
     293,  244,  236,   16, 0x0a,
     324,  244,   16,   16, 0x0a,
     354,   16,   16,   16, 0x0a,
     366,   16,   16,   16, 0x0a,
     378,   16,   16,   16, 0x0a,
     391,   16,   16,   16, 0x0a,
     415,  405,  236,   16, 0x0a,
     448,   16,   16,   16, 0x0a,
     484,  472,  468,   16, 0x0a,
     527,   16,   16,   16, 0x0a,
     553,   16,   16,   16, 0x0a,
     577,   16,   16,   16, 0x0a,
     598,   16,   16,   16, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_V3dR_MainWindow[] = {
    "V3dR_MainWindow\0\0postClose()\0"
    "setXCutLockIcon(bool)\0setYCutLockIcon(bool)\0"
    "setZCutLockIcon(bool)\0initControlValue()\0"
    "initVolumeTimeRange()\0initVolumeCutRange()\0"
    "initSurfaceCutRange()\0onlySurfaceObjTab()\0"
    "doMenuOfSurfFile()\0doMenuOfAnimate()\0"
    "QString\0loop_scpript,rotation_frames,rotation_timepoints\0"
    "previewMovie(QString&,int,int)\0"
    "doSaveMovie(QString&,int,int)\0saveMovie()\0"
    "animateOn()\0animateOff()\0animateStep()\0"
    "qtitle,ok\0getAnimateRotType(QString,bool*)\0"
    "setAnimateRotType()\0int\0qtitle,ok,v\0"
    "getAnimateRotTimePoints(QString,bool*,int)\0"
    "setAnimateRotTimePoints()\0"
    "setAnimateRotSpeedSec()\0raise_and_activate()\0"
    "hideDisplayControls()\0"
};

void V3dR_MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        V3dR_MainWindow *_t = static_cast<V3dR_MainWindow *>(_o);
        switch (_id) {
        case 0: _t->postClose(); break;
        case 1: _t->setXCutLockIcon((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->setYCutLockIcon((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->setZCutLockIcon((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->initControlValue(); break;
        case 5: _t->initVolumeTimeRange(); break;
        case 6: _t->initVolumeCutRange(); break;
        case 7: _t->initSurfaceCutRange(); break;
        case 8: _t->onlySurfaceObjTab(); break;
        case 9: _t->doMenuOfSurfFile(); break;
        case 10: _t->doMenuOfAnimate(); break;
        case 11: { QString _r = _t->previewMovie((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 12: _t->doSaveMovie((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 13: _t->saveMovie(); break;
        case 14: _t->animateOn(); break;
        case 15: _t->animateOff(); break;
        case 16: _t->animateStep(); break;
        case 17: { QString _r = _t->getAnimateRotType((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< bool*(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 18: _t->setAnimateRotType(); break;
        case 19: { int _r = _t->getAnimateRotTimePoints((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< bool*(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 20: _t->setAnimateRotTimePoints(); break;
        case 21: _t->setAnimateRotSpeedSec(); break;
        case 22: _t->raise_and_activate(); break;
        case 23: _t->hideDisplayControls(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData V3dR_MainWindow::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject V3dR_MainWindow::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_V3dR_MainWindow,
      qt_meta_data_V3dR_MainWindow, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &V3dR_MainWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *V3dR_MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *V3dR_MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_V3dR_MainWindow))
        return static_cast<void*>(const_cast< V3dR_MainWindow*>(this));
    return QWidget::qt_metacast(_clname);
}

int V3dR_MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 24)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 24;
    }
    return _id;
}
QT_END_MOC_NAMESPACE

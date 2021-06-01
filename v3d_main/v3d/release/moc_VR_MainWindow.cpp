/****************************************************************************
** Meta object code from reading C++ file 'VR_MainWindow.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../vrrenderer/VR_MainWindow.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'VR_MainWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_VR_MainWindow[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x05,
      36,   14,   14,   14, 0x05,

 // slots: signature, parameters, type, tag, flags
      59,   51,   14,   14, 0x0a,
      79,   14,   14,   14, 0x2a,
      95,   14,   14,   14, 0x0a,
     113,   14,   14,   14, 0x0a,
     127,   14,   14,   14, 0x08,
     141,   14,   14,   14, 0x08,
     155,   14,   14,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_VR_MainWindow[] = {
    "VR_MainWindow\0\0VRSocketDisconnect()\0"
    "sendPoolHead()\0zoomPOS\0RunVRMainloop(XYZ*)\0"
    "RunVRMainloop()\0SendHMDPosition()\0"
    "onReadySend()\0onReadyRead()\0onConnected()\0"
    "onDisconnected()\0"
};

void VR_MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        VR_MainWindow *_t = static_cast<VR_MainWindow *>(_o);
        switch (_id) {
        case 0: _t->VRSocketDisconnect(); break;
        case 1: _t->sendPoolHead(); break;
        case 2: _t->RunVRMainloop((*reinterpret_cast< XYZ*(*)>(_a[1]))); break;
        case 3: _t->RunVRMainloop(); break;
        case 4: _t->SendHMDPosition(); break;
        case 5: _t->onReadySend(); break;
        case 6: _t->onReadyRead(); break;
        case 7: _t->onConnected(); break;
        case 8: _t->onDisconnected(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData VR_MainWindow::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject VR_MainWindow::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_VR_MainWindow,
      qt_meta_data_VR_MainWindow, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &VR_MainWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *VR_MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *VR_MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_VR_MainWindow))
        return static_cast<void*>(const_cast< VR_MainWindow*>(this));
    return QWidget::qt_metacast(_clname);
}

int VR_MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void VR_MainWindow::VRSocketDisconnect()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void VR_MainWindow::sendPoolHead()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}
QT_END_MOC_NAMESPACE

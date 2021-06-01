/****************************************************************************
** Meta object code from reading C++ file 'Na2DViewer.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../neuron_annotator/gui/Na2DViewer.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Na2DViewer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Na2DViewer[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      22,   12,   11,   11, 0x05,

 // slots: signature, parameters, type, tag, flags
      59,   57,   11,   11, 0x0a,
      79,   11,   11,   11, 0x0a,
      98,   92,   11,   11, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_Na2DViewer[] = {
    "Na2DViewer\0\0dx,dy,pos\0"
    "mouseLeftDragEvent(int,int,QPoint)\0b\0"
    "showCrosshair(bool)\0invalidate()\0dx,dy\0"
    "translateImage(int,int)\0"
};

void Na2DViewer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        Na2DViewer *_t = static_cast<Na2DViewer *>(_o);
        switch (_id) {
        case 0: _t->mouseLeftDragEvent((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< QPoint(*)>(_a[3]))); break;
        case 1: _t->showCrosshair((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->invalidate(); break;
        case 3: _t->translateImage((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData Na2DViewer::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Na2DViewer::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_Na2DViewer,
      qt_meta_data_Na2DViewer, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Na2DViewer::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Na2DViewer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Na2DViewer::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Na2DViewer))
        return static_cast<void*>(const_cast< Na2DViewer*>(this));
    if (!strcmp(_clname, "NaViewer"))
        return static_cast< NaViewer*>(const_cast< Na2DViewer*>(this));
    return QWidget::qt_metacast(_clname);
}

int Na2DViewer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void Na2DViewer::mouseLeftDragEvent(int _t1, int _t2, QPoint _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE

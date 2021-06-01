/****************************************************************************
** Meta object code from reading C++ file 'QGLRefSys.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../terafly/src/presentation/QGLRefSys.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'QGLRefSys.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_terafly__QGLRefSys[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: signature, parameters, type, tag, flags
      26,   20,   19,   19, 0x05,
      48,   20,   19,   19, 0x05,
      70,   20,   19,   19, 0x05,
      92,   19,   19,   19, 0x05,
     112,  108,   19,   19, 0x05,
     139,   19,   19,   19, 0x05,

 // slots: signature, parameters, type, tag, flags
     147,   20,   19,   19, 0x0a,
     165,   20,   19,   19, 0x0a,
     183,   20,   19,   19, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_terafly__QGLRefSys[] = {
    "terafly::QGLRefSys\0\0angle\0"
    "xRotationChanged(int)\0yRotationChanged(int)\0"
    "zRotationChanged(int)\0mouseReleased()\0"
    "str\0neuronInfoChanged(QString)\0reset()\0"
    "setXRotation(int)\0setYRotation(int)\0"
    "setZRotation(int)\0"
};

void terafly::QGLRefSys::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QGLRefSys *_t = static_cast<QGLRefSys *>(_o);
        switch (_id) {
        case 0: _t->xRotationChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->yRotationChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->zRotationChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->mouseReleased(); break;
        case 4: _t->neuronInfoChanged((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 5: _t->reset(); break;
        case 6: _t->setXRotation((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->setYRotation((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->setZRotation((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData terafly::QGLRefSys::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject terafly::QGLRefSys::staticMetaObject = {
    { &QGLWidget::staticMetaObject, qt_meta_stringdata_terafly__QGLRefSys,
      qt_meta_data_terafly__QGLRefSys, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &terafly::QGLRefSys::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *terafly::QGLRefSys::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *terafly::QGLRefSys::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_terafly__QGLRefSys))
        return static_cast<void*>(const_cast< QGLRefSys*>(this));
    return QGLWidget::qt_metacast(_clname);
}

int terafly::QGLRefSys::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGLWidget::qt_metacall(_c, _id, _a);
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
void terafly::QGLRefSys::xRotationChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void terafly::QGLRefSys::yRotationChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void terafly::QGLRefSys::zRotationChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void terafly::QGLRefSys::mouseReleased()
{
    QMetaObject::activate(this, &staticMetaObject, 3, 0);
}

// SIGNAL 4
void terafly::QGLRefSys::neuronInfoChanged(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void terafly::QGLRefSys::reset()
{
    QMetaObject::activate(this, &staticMetaObject, 5, 0);
}
QT_END_MOC_NAMESPACE

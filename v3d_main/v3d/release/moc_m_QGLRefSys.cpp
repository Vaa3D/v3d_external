/****************************************************************************
** Meta object code from reading C++ file 'm_QGLRefSys.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../mozak/m_terafly/src/presentation/m_QGLRefSys.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'm_QGLRefSys.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_teramanager__QGLRefSys[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: signature, parameters, type, tag, flags
      30,   24,   23,   23, 0x05,
      52,   24,   23,   23, 0x05,
      74,   24,   23,   23, 0x05,
      96,   23,   23,   23, 0x05,

 // slots: signature, parameters, type, tag, flags
     112,   24,   23,   23, 0x0a,
     130,   24,   23,   23, 0x0a,
     148,   24,   23,   23, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_teramanager__QGLRefSys[] = {
    "teramanager::QGLRefSys\0\0angle\0"
    "xRotationChanged(int)\0yRotationChanged(int)\0"
    "zRotationChanged(int)\0mouseReleased()\0"
    "setXRotation(int)\0setYRotation(int)\0"
    "setZRotation(int)\0"
};

void teramanager::QGLRefSys::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QGLRefSys *_t = static_cast<QGLRefSys *>(_o);
        switch (_id) {
        case 0: _t->xRotationChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->yRotationChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->zRotationChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->mouseReleased(); break;
        case 4: _t->setXRotation((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->setYRotation((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->setZRotation((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData teramanager::QGLRefSys::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject teramanager::QGLRefSys::staticMetaObject = {
    { &QGLWidget::staticMetaObject, qt_meta_stringdata_teramanager__QGLRefSys,
      qt_meta_data_teramanager__QGLRefSys, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &teramanager::QGLRefSys::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *teramanager::QGLRefSys::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *teramanager::QGLRefSys::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_teramanager__QGLRefSys))
        return static_cast<void*>(const_cast< QGLRefSys*>(this));
    return QGLWidget::qt_metacast(_clname);
}

int teramanager::QGLRefSys::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGLWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void teramanager::QGLRefSys::xRotationChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void teramanager::QGLRefSys::yRotationChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void teramanager::QGLRefSys::zRotationChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void teramanager::QGLRefSys::mouseReleased()
{
    QMetaObject::activate(this, &staticMetaObject, 3, 0);
}
QT_END_MOC_NAMESPACE

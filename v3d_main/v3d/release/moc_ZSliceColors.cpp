/****************************************************************************
** Meta object code from reading C++ file 'ZSliceColors.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../neuron_annotator/data_model/ZSliceColors.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ZSliceColors.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ZSliceColors[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      14,   13,   13,   13, 0x0a,
      25,   23,   13,   13, 0x0a,
      46,   40,   13,   13, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_ZSliceColors[] = {
    "ZSliceColors\0\0update()\0z\0setZIndex(int)\0"
    "focus\0onCameraFocusChanged(Vector3D)\0"
};

void ZSliceColors::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ZSliceColors *_t = static_cast<ZSliceColors *>(_o);
        switch (_id) {
        case 0: _t->update(); break;
        case 1: _t->setZIndex((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->onCameraFocusChanged((*reinterpret_cast< const Vector3D(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData ZSliceColors::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ZSliceColors::staticMetaObject = {
    { &NaLockableData::staticMetaObject, qt_meta_stringdata_ZSliceColors,
      qt_meta_data_ZSliceColors, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ZSliceColors::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ZSliceColors::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ZSliceColors::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ZSliceColors))
        return static_cast<void*>(const_cast< ZSliceColors*>(this));
    return NaLockableData::qt_metacast(_clname);
}

int ZSliceColors::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = NaLockableData::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}
QT_END_MOC_NAMESPACE

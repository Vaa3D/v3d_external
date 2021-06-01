/****************************************************************************
** Meta object code from reading C++ file 'landmark_property_dialog.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../landmark_property_dialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'landmark_property_dialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_LandmarkPropertyDialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      24,   23,   23,   23, 0x0a,
      45,   43,   23,   23, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_LandmarkPropertyDialog[] = {
    "LandmarkPropertyDialog\0\0compute_rgn_stat()\0"
    "c\0compute_rgn_stat(int)\0"
};

void LandmarkPropertyDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        LandmarkPropertyDialog *_t = static_cast<LandmarkPropertyDialog *>(_o);
        switch (_id) {
        case 0: _t->compute_rgn_stat(); break;
        case 1: _t->compute_rgn_stat((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData LandmarkPropertyDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject LandmarkPropertyDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_LandmarkPropertyDialog,
      qt_meta_data_LandmarkPropertyDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &LandmarkPropertyDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *LandmarkPropertyDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *LandmarkPropertyDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_LandmarkPropertyDialog))
        return static_cast<void*>(const_cast< LandmarkPropertyDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int LandmarkPropertyDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE

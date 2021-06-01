/****************************************************************************
** Meta object code from reading C++ file 'dialog_keypoint_features.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../dialog_keypoint_features.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'dialog_keypoint_features.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_KeypointFeaturesDialog[] = {

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
      48,   23,   43,   23, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_KeypointFeaturesDialog[] = {
    "KeypointFeaturesDialog\0\0compute_pointfea()\0"
    "bool\0compute_similarmap()\0"
};

void KeypointFeaturesDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        KeypointFeaturesDialog *_t = static_cast<KeypointFeaturesDialog *>(_o);
        switch (_id) {
        case 0: _t->compute_pointfea(); break;
        case 1: { bool _r = _t->compute_similarmap();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        default: ;
        }
    }
}

const QMetaObjectExtraData KeypointFeaturesDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject KeypointFeaturesDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_KeypointFeaturesDialog,
      qt_meta_data_KeypointFeaturesDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &KeypointFeaturesDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *KeypointFeaturesDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *KeypointFeaturesDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_KeypointFeaturesDialog))
        return static_cast<void*>(const_cast< KeypointFeaturesDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int KeypointFeaturesDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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

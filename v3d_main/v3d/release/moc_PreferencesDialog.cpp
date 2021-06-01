/****************************************************************************
** Meta object code from reading C++ file 'PreferencesDialog.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../neuron_annotator/gui/PreferencesDialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PreferencesDialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_PreferencesDialog[] = {

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
      19,   18,   18,   18, 0x0a,
      37,   18,   18,   18, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_PreferencesDialog[] = {
    "PreferencesDialog\0\0savePreferences()\0"
    "loadPreferences()\0"
};

void PreferencesDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        PreferencesDialog *_t = static_cast<PreferencesDialog *>(_o);
        switch (_id) {
        case 0: _t->savePreferences(); break;
        case 1: _t->loadPreferences(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData PreferencesDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject PreferencesDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_PreferencesDialog,
      qt_meta_data_PreferencesDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &PreferencesDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *PreferencesDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *PreferencesDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_PreferencesDialog))
        return static_cast<void*>(const_cast< PreferencesDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int PreferencesDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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

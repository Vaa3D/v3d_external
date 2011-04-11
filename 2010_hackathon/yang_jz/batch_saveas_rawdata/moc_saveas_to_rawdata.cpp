/****************************************************************************
** Meta object code from reading C++ file 'saveas_to_rawdata.h'
**
** Created: Sun Apr 10 11:12:24 2011
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "saveas_to_rawdata.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'saveas_to_rawdata.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SAVEAS_TO_RAWDATAlugin[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

       0        // eod
};

static const char qt_meta_stringdata_SAVEAS_TO_RAWDATAlugin[] = {
    "SAVEAS_TO_RAWDATAlugin\0"
};

const QMetaObject SAVEAS_TO_RAWDATAlugin::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_SAVEAS_TO_RAWDATAlugin,
      qt_meta_data_SAVEAS_TO_RAWDATAlugin, 0 }
};

const QMetaObject *SAVEAS_TO_RAWDATAlugin::metaObject() const
{
    return &staticMetaObject;
}

void *SAVEAS_TO_RAWDATAlugin::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SAVEAS_TO_RAWDATAlugin))
        return static_cast<void*>(const_cast< SAVEAS_TO_RAWDATAlugin*>(this));
    if (!strcmp(_clname, "V3DPluginInterface"))
        return static_cast< V3DPluginInterface*>(const_cast< SAVEAS_TO_RAWDATAlugin*>(this));
    if (!strcmp(_clname, "com.janelia.v3d.V3DPluginInterface/1.1"))
        return static_cast< V3DPluginInterface*>(const_cast< SAVEAS_TO_RAWDATAlugin*>(this));
    return QObject::qt_metacast(_clname);
}

int SAVEAS_TO_RAWDATAlugin::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_SetsizeDialog[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // slots: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_SetsizeDialog[] = {
    "SetsizeDialog\0\0update()\0"
};

const QMetaObject SetsizeDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_SetsizeDialog,
      qt_meta_data_SetsizeDialog, 0 }
};

const QMetaObject *SetsizeDialog::metaObject() const
{
    return &staticMetaObject;
}

void *SetsizeDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SetsizeDialog))
        return static_cast<void*>(const_cast< SetsizeDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int SetsizeDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: update(); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE

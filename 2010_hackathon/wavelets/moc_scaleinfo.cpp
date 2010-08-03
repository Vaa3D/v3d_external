/****************************************************************************
** Meta object code from reading C++ file 'scaleinfo.h'
**
** Created: Wed Aug 4 00:09:29 2010
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "scaleinfo.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'scaleinfo.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ScaleInfo[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // slots: signature, parameters, type, tag, flags
      11,   10,   10,   10, 0x0a,
      39,   33,   10,   10, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_ScaleInfo[] = {
    "ScaleInfo\0\0enableButtonPressed()\0value\0"
    "sliderChange(int)\0"
};

const QMetaObject ScaleInfo::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_ScaleInfo,
      qt_meta_data_ScaleInfo, 0 }
};

const QMetaObject *ScaleInfo::metaObject() const
{
    return &staticMetaObject;
}

void *ScaleInfo::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ScaleInfo))
        return static_cast<void*>(const_cast< ScaleInfo*>(this));
    return QObject::qt_metacast(_clname);
}

int ScaleInfo::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: enableButtonPressed(); break;
        case 1: sliderChange((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE

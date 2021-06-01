/****************************************************************************
** Meta object code from reading C++ file 'vano_linker_loader_dialog.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../vano_linker_loader_dialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'vano_linker_loader_dialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_VANO_LinkerLoadDialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      23,   22,   22,   22, 0x0a,
      40,   22,   22,   22, 0x0a,
      58,   22,   22,   22, 0x0a,
      77,   75,   22,   22, 0x0a,
     101,   75,   22,   22, 0x0a,
     126,   75,   22,   22, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_VANO_LinkerLoadDialog[] = {
    "VANO_LinkerLoadDialog\0\0select_rawfile()\0"
    "select_maskfile()\0select_apofile()\0s\0"
    "change_rawfile(QString)\0"
    "change_maskfile(QString)\0"
    "change_apofile(QString)\0"
};

void VANO_LinkerLoadDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        VANO_LinkerLoadDialog *_t = static_cast<VANO_LinkerLoadDialog *>(_o);
        switch (_id) {
        case 0: _t->select_rawfile(); break;
        case 1: _t->select_maskfile(); break;
        case 2: _t->select_apofile(); break;
        case 3: _t->change_rawfile((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->change_maskfile((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->change_apofile((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData VANO_LinkerLoadDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject VANO_LinkerLoadDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_VANO_LinkerLoadDialog,
      qt_meta_data_VANO_LinkerLoadDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &VANO_LinkerLoadDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *VANO_LinkerLoadDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *VANO_LinkerLoadDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_VANO_LinkerLoadDialog))
        return static_cast<void*>(const_cast< VANO_LinkerLoadDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int VANO_LinkerLoadDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}
QT_END_MOC_NAMESPACE

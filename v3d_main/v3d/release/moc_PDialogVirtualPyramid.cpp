/****************************************************************************
** Meta object code from reading C++ file 'PDialogVirtualPyramid.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../terafly/src/presentation/PDialogVirtualPyramid.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PDialogVirtualPyramid.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_terafly__PDialogVirtualPyramid[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      32,   31,   31,   31, 0x0a,
      52,   31,   31,   31, 0x0a,
      76,   31,   31,   31, 0x0a,
     110,   31,   31,   31, 0x0a,
     140,   31,   31,   31, 0x0a,
     171,  169,   31,   31, 0x0a,
     204,   31,   31,   31, 0x0a,
     239,  169,   31,   31, 0x0a,
     274,   31,   31,   31, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_terafly__PDialogVirtualPyramid[] = {
    "terafly::PDialogVirtualPyramid\0\0"
    "ok_button_clicked()\0browse_button_clicked()\0"
    "subsampling_radiobutton_changed()\0"
    "storage_radiobutton_changed()\0"
    "lowres_radiobutton_changed()\0v\0"
    "subsampling_spinbox_changed(int)\0"
    "subsamplings_line_changed(QString)\0"
    "block_format_combobox_changed(int)\0"
    "update_space_required()\0"
};

void terafly::PDialogVirtualPyramid::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        PDialogVirtualPyramid *_t = static_cast<PDialogVirtualPyramid *>(_o);
        switch (_id) {
        case 0: _t->ok_button_clicked(); break;
        case 1: _t->browse_button_clicked(); break;
        case 2: _t->subsampling_radiobutton_changed(); break;
        case 3: _t->storage_radiobutton_changed(); break;
        case 4: _t->lowres_radiobutton_changed(); break;
        case 5: _t->subsampling_spinbox_changed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->subsamplings_line_changed((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 7: _t->block_format_combobox_changed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->update_space_required(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData terafly::PDialogVirtualPyramid::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject terafly::PDialogVirtualPyramid::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_terafly__PDialogVirtualPyramid,
      qt_meta_data_terafly__PDialogVirtualPyramid, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &terafly::PDialogVirtualPyramid::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *terafly::PDialogVirtualPyramid::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *terafly::PDialogVirtualPyramid::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_terafly__PDialogVirtualPyramid))
        return static_cast<void*>(const_cast< PDialogVirtualPyramid*>(this));
    return QDialog::qt_metacast(_clname);
}

int terafly::PDialogVirtualPyramid::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    return _id;
}
QT_END_MOC_NAMESPACE

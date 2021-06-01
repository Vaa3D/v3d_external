/****************************************************************************
** Meta object code from reading C++ file 'import_filelist_dialog.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../import_filelist_dialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'import_filelist_dialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_import_filelist_Dialog[] = {

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
      44,   42,   23,   23, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_import_filelist_Dialog[] = {
    "import_filelist_Dialog\0\0updateFilterStr()\0"
    "s\0updateFilterStr(QString)\0"
};

void import_filelist_Dialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        import_filelist_Dialog *_t = static_cast<import_filelist_Dialog *>(_o);
        switch (_id) {
        case 0: _t->updateFilterStr(); break;
        case 1: _t->updateFilterStr((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData import_filelist_Dialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject import_filelist_Dialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_import_filelist_Dialog,
      qt_meta_data_import_filelist_Dialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &import_filelist_Dialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *import_filelist_Dialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *import_filelist_Dialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_import_filelist_Dialog))
        return static_cast<void*>(const_cast< import_filelist_Dialog*>(this));
    if (!strcmp(_clname, "Ui_import_filelist_tool"))
        return static_cast< Ui_import_filelist_tool*>(const_cast< import_filelist_Dialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int import_filelist_Dialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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

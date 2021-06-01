/****************************************************************************
** Meta object code from reading C++ file 'QPrefixSuffixLineEdit.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../terafly/src/presentation/QPrefixSuffixLineEdit.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'QPrefixSuffixLineEdit.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_terafly__QPrefixSuffixLineEdit[] = {

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
      39,   32,   31,   31, 0x0a,
      65,   58,   31,   31, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_terafly__QPrefixSuffixLineEdit[] = {
    "terafly::QPrefixSuffixLineEdit\0\0prefix\0"
    "setPrefix(QString)\0suffix\0setSuffix(QString)\0"
};

void terafly::QPrefixSuffixLineEdit::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QPrefixSuffixLineEdit *_t = static_cast<QPrefixSuffixLineEdit *>(_o);
        switch (_id) {
        case 0: _t->setPrefix((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->setSuffix((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData terafly::QPrefixSuffixLineEdit::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject terafly::QPrefixSuffixLineEdit::staticMetaObject = {
    { &QLineEdit::staticMetaObject, qt_meta_stringdata_terafly__QPrefixSuffixLineEdit,
      qt_meta_data_terafly__QPrefixSuffixLineEdit, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &terafly::QPrefixSuffixLineEdit::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *terafly::QPrefixSuffixLineEdit::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *terafly::QPrefixSuffixLineEdit::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_terafly__QPrefixSuffixLineEdit))
        return static_cast<void*>(const_cast< QPrefixSuffixLineEdit*>(this));
    return QLineEdit::qt_metacast(_clname);
}

int terafly::QPrefixSuffixLineEdit::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QLineEdit::qt_metacall(_c, _id, _a);
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

/****************************************************************************
** Meta object code from reading C++ file 'CImport.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../terafly/src/control/CImport.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CImport.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_terafly__CImport[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      34,   18,   17,   17, 0x05,
      88,   85,   17,   17, 0x25,

       0        // eod
};

static const char qt_meta_stringdata_terafly__CImport[] = {
    "terafly::CImport\0\0ex,elapsed_time\0"
    "sendOperationOutcome(tf::RuntimeException*,qint64)\0"
    "ex\0sendOperationOutcome(tf::RuntimeException*)\0"
};

void terafly::CImport::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        CImport *_t = static_cast<CImport *>(_o);
        switch (_id) {
        case 0: _t->sendOperationOutcome((*reinterpret_cast< tf::RuntimeException*(*)>(_a[1])),(*reinterpret_cast< qint64(*)>(_a[2]))); break;
        case 1: _t->sendOperationOutcome((*reinterpret_cast< tf::RuntimeException*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData terafly::CImport::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject terafly::CImport::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_terafly__CImport,
      qt_meta_data_terafly__CImport, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &terafly::CImport::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *terafly::CImport::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *terafly::CImport::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_terafly__CImport))
        return static_cast<void*>(const_cast< CImport*>(this));
    return QThread::qt_metacast(_clname);
}

int terafly::CImport::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void terafly::CImport::sendOperationOutcome(tf::RuntimeException * _t1, qint64 _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE

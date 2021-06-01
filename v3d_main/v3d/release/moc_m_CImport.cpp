/****************************************************************************
** Meta object code from reading C++ file 'm_CImport.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../mozak/m_terafly/src/control/m_CImport.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'm_CImport.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_teramanager__CImport[] = {

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
      38,   22,   21,   21, 0x05,
      93,   90,   21,   21, 0x25,

       0        // eod
};

static const char qt_meta_stringdata_teramanager__CImport[] = {
    "teramanager::CImport\0\0ex,elapsed_time\0"
    "sendOperationOutcome(itm::RuntimeException*,qint64)\0"
    "ex\0sendOperationOutcome(itm::RuntimeException*)\0"
};

void teramanager::CImport::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        CImport *_t = static_cast<CImport *>(_o);
        switch (_id) {
        case 0: _t->sendOperationOutcome((*reinterpret_cast< itm::RuntimeException*(*)>(_a[1])),(*reinterpret_cast< qint64(*)>(_a[2]))); break;
        case 1: _t->sendOperationOutcome((*reinterpret_cast< itm::RuntimeException*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData teramanager::CImport::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject teramanager::CImport::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_teramanager__CImport,
      qt_meta_data_teramanager__CImport, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &teramanager::CImport::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *teramanager::CImport::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *teramanager::CImport::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_teramanager__CImport))
        return static_cast<void*>(const_cast< CImport*>(this));
    return QThread::qt_metacast(_clname);
}

int teramanager::CImport::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void teramanager::CImport::sendOperationOutcome(itm::RuntimeException * _t1, qint64 _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE

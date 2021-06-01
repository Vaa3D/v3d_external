/****************************************************************************
** Meta object code from reading C++ file 'm_CVolume.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../mozak/m_terafly/src/control/m_CVolume.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'm_CVolume.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_teramanager__CVolume[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: signature, parameters, type, tag, flags
      83,   22,   21,   21, 0x05,
     255,  199,   21,   21, 0x25,
     416,  367,   21,   21, 0x25,
     556,  520,   21,   21, 0x25,
     686,  653,   21,   21, 0x25,

       0        // eod
};

static const char qt_meta_stringdata_teramanager__CVolume[] = {
    "teramanager::CVolume\0\0"
    "data,data_s,data_c,dest,finished,ex,elapsed_time,op_dsc,step\0"
    "sendData(itm::uint8*,itm::integer_array,itm::integer_array,QWidget*,bo"
    "ol,itm::RuntimeException*,qint64,QString,int)\0"
    "data,data_s,data_c,dest,finished,ex,elapsed_time,op_dsc\0"
    "sendData(itm::uint8*,itm::integer_array,itm::integer_array,QWidget*,bo"
    "ol,itm::RuntimeException*,qint64,QString)\0"
    "data,data_s,data_c,dest,finished,ex,elapsed_time\0"
    "sendData(itm::uint8*,itm::integer_array,itm::integer_array,QWidget*,bo"
    "ol,itm::RuntimeException*,qint64)\0"
    "data,data_s,data_c,dest,finished,ex\0"
    "sendData(itm::uint8*,itm::integer_array,itm::integer_array,QWidget*,bo"
    "ol,itm::RuntimeException*)\0"
    "data,data_s,data_c,dest,finished\0"
    "sendData(itm::uint8*,itm::integer_array,itm::integer_array,QWidget*,bo"
    "ol)\0"
};

void teramanager::CVolume::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        CVolume *_t = static_cast<CVolume *>(_o);
        switch (_id) {
        case 0: _t->sendData((*reinterpret_cast< itm::uint8*(*)>(_a[1])),(*reinterpret_cast< itm::integer_array(*)>(_a[2])),(*reinterpret_cast< itm::integer_array(*)>(_a[3])),(*reinterpret_cast< QWidget*(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5])),(*reinterpret_cast< itm::RuntimeException*(*)>(_a[6])),(*reinterpret_cast< qint64(*)>(_a[7])),(*reinterpret_cast< QString(*)>(_a[8])),(*reinterpret_cast< int(*)>(_a[9]))); break;
        case 1: _t->sendData((*reinterpret_cast< itm::uint8*(*)>(_a[1])),(*reinterpret_cast< itm::integer_array(*)>(_a[2])),(*reinterpret_cast< itm::integer_array(*)>(_a[3])),(*reinterpret_cast< QWidget*(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5])),(*reinterpret_cast< itm::RuntimeException*(*)>(_a[6])),(*reinterpret_cast< qint64(*)>(_a[7])),(*reinterpret_cast< QString(*)>(_a[8]))); break;
        case 2: _t->sendData((*reinterpret_cast< itm::uint8*(*)>(_a[1])),(*reinterpret_cast< itm::integer_array(*)>(_a[2])),(*reinterpret_cast< itm::integer_array(*)>(_a[3])),(*reinterpret_cast< QWidget*(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5])),(*reinterpret_cast< itm::RuntimeException*(*)>(_a[6])),(*reinterpret_cast< qint64(*)>(_a[7]))); break;
        case 3: _t->sendData((*reinterpret_cast< itm::uint8*(*)>(_a[1])),(*reinterpret_cast< itm::integer_array(*)>(_a[2])),(*reinterpret_cast< itm::integer_array(*)>(_a[3])),(*reinterpret_cast< QWidget*(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5])),(*reinterpret_cast< itm::RuntimeException*(*)>(_a[6]))); break;
        case 4: _t->sendData((*reinterpret_cast< itm::uint8*(*)>(_a[1])),(*reinterpret_cast< itm::integer_array(*)>(_a[2])),(*reinterpret_cast< itm::integer_array(*)>(_a[3])),(*reinterpret_cast< QWidget*(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData teramanager::CVolume::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject teramanager::CVolume::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_teramanager__CVolume,
      qt_meta_data_teramanager__CVolume, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &teramanager::CVolume::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *teramanager::CVolume::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *teramanager::CVolume::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_teramanager__CVolume))
        return static_cast<void*>(const_cast< CVolume*>(this));
    return QThread::qt_metacast(_clname);
}

int teramanager::CVolume::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void teramanager::CVolume::sendData(itm::uint8 * _t1, itm::integer_array _t2, itm::integer_array _t3, QWidget * _t4, bool _t5, itm::RuntimeException * _t6, qint64 _t7, QString _t8, int _t9)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)), const_cast<void*>(reinterpret_cast<const void*>(&_t5)), const_cast<void*>(reinterpret_cast<const void*>(&_t6)), const_cast<void*>(reinterpret_cast<const void*>(&_t7)), const_cast<void*>(reinterpret_cast<const void*>(&_t8)), const_cast<void*>(reinterpret_cast<const void*>(&_t9)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE

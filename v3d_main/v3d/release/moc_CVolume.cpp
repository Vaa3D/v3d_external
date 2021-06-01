/****************************************************************************
** Meta object code from reading C++ file 'CVolume.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../terafly/src/control/CVolume.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CVolume.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_terafly__CVolume[] = {

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
      79,   18,   17,   17, 0x05,
     247,  191,   17,   17, 0x25,
     404,  355,   17,   17, 0x25,
     540,  504,   17,   17, 0x25,
     666,  633,   17,   17, 0x25,

       0        // eod
};

static const char qt_meta_stringdata_terafly__CVolume[] = {
    "terafly::CVolume\0\0"
    "data,data_s,data_c,dest,finished,ex,elapsed_time,op_dsc,step\0"
    "sendData(tf::uint8*,tf::integer_array,tf::integer_array,QWidget*,bool,"
    "tf::RuntimeException*,qint64,QString,int)\0"
    "data,data_s,data_c,dest,finished,ex,elapsed_time,op_dsc\0"
    "sendData(tf::uint8*,tf::integer_array,tf::integer_array,QWidget*,bool,"
    "tf::RuntimeException*,qint64,QString)\0"
    "data,data_s,data_c,dest,finished,ex,elapsed_time\0"
    "sendData(tf::uint8*,tf::integer_array,tf::integer_array,QWidget*,bool,"
    "tf::RuntimeException*,qint64)\0"
    "data,data_s,data_c,dest,finished,ex\0"
    "sendData(tf::uint8*,tf::integer_array,tf::integer_array,QWidget*,bool,"
    "tf::RuntimeException*)\0"
    "data,data_s,data_c,dest,finished\0"
    "sendData(tf::uint8*,tf::integer_array,tf::integer_array,QWidget*,bool)\0"
};

void terafly::CVolume::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        CVolume *_t = static_cast<CVolume *>(_o);
        switch (_id) {
        case 0: _t->sendData((*reinterpret_cast< tf::uint8*(*)>(_a[1])),(*reinterpret_cast< tf::integer_array(*)>(_a[2])),(*reinterpret_cast< tf::integer_array(*)>(_a[3])),(*reinterpret_cast< QWidget*(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5])),(*reinterpret_cast< tf::RuntimeException*(*)>(_a[6])),(*reinterpret_cast< qint64(*)>(_a[7])),(*reinterpret_cast< QString(*)>(_a[8])),(*reinterpret_cast< int(*)>(_a[9]))); break;
        case 1: _t->sendData((*reinterpret_cast< tf::uint8*(*)>(_a[1])),(*reinterpret_cast< tf::integer_array(*)>(_a[2])),(*reinterpret_cast< tf::integer_array(*)>(_a[3])),(*reinterpret_cast< QWidget*(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5])),(*reinterpret_cast< tf::RuntimeException*(*)>(_a[6])),(*reinterpret_cast< qint64(*)>(_a[7])),(*reinterpret_cast< QString(*)>(_a[8]))); break;
        case 2: _t->sendData((*reinterpret_cast< tf::uint8*(*)>(_a[1])),(*reinterpret_cast< tf::integer_array(*)>(_a[2])),(*reinterpret_cast< tf::integer_array(*)>(_a[3])),(*reinterpret_cast< QWidget*(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5])),(*reinterpret_cast< tf::RuntimeException*(*)>(_a[6])),(*reinterpret_cast< qint64(*)>(_a[7]))); break;
        case 3: _t->sendData((*reinterpret_cast< tf::uint8*(*)>(_a[1])),(*reinterpret_cast< tf::integer_array(*)>(_a[2])),(*reinterpret_cast< tf::integer_array(*)>(_a[3])),(*reinterpret_cast< QWidget*(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5])),(*reinterpret_cast< tf::RuntimeException*(*)>(_a[6]))); break;
        case 4: _t->sendData((*reinterpret_cast< tf::uint8*(*)>(_a[1])),(*reinterpret_cast< tf::integer_array(*)>(_a[2])),(*reinterpret_cast< tf::integer_array(*)>(_a[3])),(*reinterpret_cast< QWidget*(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData terafly::CVolume::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject terafly::CVolume::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_terafly__CVolume,
      qt_meta_data_terafly__CVolume, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &terafly::CVolume::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *terafly::CVolume::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *terafly::CVolume::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_terafly__CVolume))
        return static_cast<void*>(const_cast< CVolume*>(this));
    return QThread::qt_metacast(_clname);
}

int terafly::CVolume::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void terafly::CVolume::sendData(tf::uint8 * _t1, tf::integer_array _t2, tf::integer_array _t3, QWidget * _t4, bool _t5, tf::RuntimeException * _t6, qint64 _t7, QString _t8, int _t9)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)), const_cast<void*>(reinterpret_cast<const void*>(&_t5)), const_cast<void*>(reinterpret_cast<const void*>(&_t6)), const_cast<void*>(reinterpret_cast<const void*>(&_t7)), const_cast<void*>(reinterpret_cast<const void*>(&_t8)), const_cast<void*>(reinterpret_cast<const void*>(&_t9)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE

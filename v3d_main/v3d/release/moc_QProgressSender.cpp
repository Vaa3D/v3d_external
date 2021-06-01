/****************************************************************************
** Meta object code from reading C++ file 'QProgressSender.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../terafly/src/terarepo/src/common/QProgressSender.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'QProgressSender.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_terastitcher__QProgressSender[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      59,   31,   30,   30, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_terastitcher__QProgressSender[] = {
    "terastitcher::QProgressSender\0\0"
    "val,minutes,seconds,message\0"
    "sendProgressBarChanged(int,int,int,std::string)\0"
};

void terastitcher::QProgressSender::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QProgressSender *_t = static_cast<QProgressSender *>(_o);
        switch (_id) {
        case 0: _t->sendProgressBarChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< std::string(*)>(_a[4]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData terastitcher::QProgressSender::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject terastitcher::QProgressSender::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_terastitcher__QProgressSender,
      qt_meta_data_terastitcher__QProgressSender, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &terastitcher::QProgressSender::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *terastitcher::QProgressSender::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *terastitcher::QProgressSender::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_terastitcher__QProgressSender))
        return static_cast<void*>(const_cast< QProgressSender*>(this));
    return QWidget::qt_metacast(_clname);
}

int terastitcher::QProgressSender::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void terastitcher::QProgressSender::sendProgressBarChanged(int _t1, int _t2, int _t3, std::string _t4)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE

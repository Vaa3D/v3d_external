/****************************************************************************
** Meta object code from reading C++ file 'ConsoleObserverServiceImpl.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../webservice/impl/ConsoleObserverServiceImpl.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ConsoleObserverServiceImpl.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_obs__ConsoleObserverServiceImpl[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       9,       // signalCount

 // signals: signature, parameters, type, tag, flags
      40,   33,   32,   32, 0x05,
      65,   33,   32,   32, 0x05,
     116,   89,   32,   32, 0x05,
     171,  153,   32,   32, 0x05,
     214,  205,   32,   32, 0x05,
     236,  205,   32,   32, 0x05,
     264,  205,   32,   32, 0x05,
     301,  291,   32,   32, 0x05,
     325,   32,   32,   32, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_obs__ConsoleObserverServiceImpl[] = {
    "obs::ConsoleObserverServiceImpl\0\0"
    "rootId\0ontologySelected(qint64)\0"
    "ontologyChanged(qint64)\0"
    "category,uniqueId,clearAll\0"
    "entitySelected(QString,QString,bool)\0"
    "category,uniqueId\0entityDeselected(QString,QString)\0"
    "entityId\0entityChanged(qint64)\0"
    "entityViewRequested(qint64)\0"
    "annotationsChanged(qint64)\0sessionId\0"
    "sessionSelected(qint64)\0sessionDeselected()\0"
};

void obs::ConsoleObserverServiceImpl::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ConsoleObserverServiceImpl *_t = static_cast<ConsoleObserverServiceImpl *>(_o);
        switch (_id) {
        case 0: _t->ontologySelected((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        case 1: _t->ontologyChanged((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        case 2: _t->entitySelected((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 3: _t->entityDeselected((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 4: _t->entityChanged((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        case 5: _t->entityViewRequested((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        case 6: _t->annotationsChanged((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        case 7: _t->sessionSelected((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        case 8: _t->sessionDeselected(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData obs::ConsoleObserverServiceImpl::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject obs::ConsoleObserverServiceImpl::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_obs__ConsoleObserverServiceImpl,
      qt_meta_data_obs__ConsoleObserverServiceImpl, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &obs::ConsoleObserverServiceImpl::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *obs::ConsoleObserverServiceImpl::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *obs::ConsoleObserverServiceImpl::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_obs__ConsoleObserverServiceImpl))
        return static_cast<void*>(const_cast< ConsoleObserverServiceImpl*>(this));
    if (!strcmp(_clname, "ConsoleObserverService"))
        return static_cast< ConsoleObserverService*>(const_cast< ConsoleObserverServiceImpl*>(this));
    return QThread::qt_metacast(_clname);
}

int obs::ConsoleObserverServiceImpl::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void obs::ConsoleObserverServiceImpl::ontologySelected(qint64 _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void obs::ConsoleObserverServiceImpl::ontologyChanged(qint64 _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void obs::ConsoleObserverServiceImpl::entitySelected(const QString & _t1, const QString & _t2, bool _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void obs::ConsoleObserverServiceImpl::entityDeselected(const QString & _t1, const QString & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void obs::ConsoleObserverServiceImpl::entityChanged(qint64 _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void obs::ConsoleObserverServiceImpl::entityViewRequested(qint64 _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void obs::ConsoleObserverServiceImpl::annotationsChanged(qint64 _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void obs::ConsoleObserverServiceImpl::sessionSelected(qint64 _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void obs::ConsoleObserverServiceImpl::sessionDeselected()
{
    QMetaObject::activate(this, &staticMetaObject, 8, 0);
}
QT_END_MOC_NAMESPACE

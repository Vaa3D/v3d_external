/****************************************************************************
** Meta object code from reading C++ file 'NaLockableData.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../neuron_annotator/data_model/NaLockableData.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NaLockableData.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_NaLockableData[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: signature, parameters, type, tag, flags
      16,   15,   15,   15, 0x05,
      34,   30,   15,   15, 0x05,
      66,   15,   15,   15, 0x05,
      92,   30,   15,   15, 0x05,
     117,   15,   15,   15, 0x05,
     137,   15,   15,   15, 0x05,

 // slots: signature, parameters, type, tag, flags
     151,   15,   15,   15, 0x0a,
     160,   15,   15,   15, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_NaLockableData[] = {
    "NaLockableData\0\0dataChanged()\0msg\0"
    "progressMessageChanged(QString)\0"
    "progressValueChanged(int)\0"
    "progressAborted(QString)\0progressCompleted()\0"
    "invalidated()\0update()\0invalidate()\0"
};

void NaLockableData::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        NaLockableData *_t = static_cast<NaLockableData *>(_o);
        switch (_id) {
        case 0: _t->dataChanged(); break;
        case 1: _t->progressMessageChanged((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->progressValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->progressAborted((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 4: _t->progressCompleted(); break;
        case 5: _t->invalidated(); break;
        case 6: _t->update(); break;
        case 7: _t->invalidate(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData NaLockableData::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject NaLockableData::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_NaLockableData,
      qt_meta_data_NaLockableData, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &NaLockableData::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *NaLockableData::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *NaLockableData::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_NaLockableData))
        return static_cast<void*>(const_cast< NaLockableData*>(this));
    return QObject::qt_metacast(_clname);
}

int NaLockableData::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void NaLockableData::dataChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void NaLockableData::progressMessageChanged(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void NaLockableData::progressValueChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void NaLockableData::progressAborted(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void NaLockableData::progressCompleted()
{
    QMetaObject::activate(this, &staticMetaObject, 4, 0);
}

// SIGNAL 5
void NaLockableData::invalidated()
{
    QMetaObject::activate(this, &staticMetaObject, 5, 0);
}
QT_END_MOC_NAMESPACE

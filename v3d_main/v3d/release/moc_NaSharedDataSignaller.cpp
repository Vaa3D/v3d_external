/****************************************************************************
** Meta object code from reading C++ file 'NaSharedDataSignaller.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../neuron_annotator/data_model/NaSharedDataSignaller.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NaSharedDataSignaller.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_NaSharedDataSignaller[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       7,       // signalCount

 // signals: signature, parameters, type, tag, flags
      23,   22,   22,   22, 0x05,
      41,   37,   22,   22, 0x05,
      73,   22,   22,   22, 0x05,
      99,   37,   22,   22, 0x05,
     124,   22,   22,   22, 0x05,
     143,   22,   22,   22, 0x05,
     157,   22,   22,   22, 0x05,

 // slots: signature, parameters, type, tag, flags
     181,   22,   22,   22, 0x0a,
     190,   22,   22,   22, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_NaSharedDataSignaller[] = {
    "NaSharedDataSignaller\0\0dataChanged()\0"
    "msg\0progressMessageChanged(QString)\0"
    "progressValueChanged(int)\0"
    "progressAborted(QString)\0progressComplete()\0"
    "invalidated()\0actualDataRepresented()\0"
    "update()\0invalidate()\0"
};

void NaSharedDataSignaller::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        NaSharedDataSignaller *_t = static_cast<NaSharedDataSignaller *>(_o);
        switch (_id) {
        case 0: _t->dataChanged(); break;
        case 1: _t->progressMessageChanged((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->progressValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->progressAborted((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 4: _t->progressComplete(); break;
        case 5: _t->invalidated(); break;
        case 6: _t->actualDataRepresented(); break;
        case 7: _t->update(); break;
        case 8: _t->invalidate(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData NaSharedDataSignaller::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject NaSharedDataSignaller::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_NaSharedDataSignaller,
      qt_meta_data_NaSharedDataSignaller, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &NaSharedDataSignaller::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *NaSharedDataSignaller::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *NaSharedDataSignaller::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_NaSharedDataSignaller))
        return static_cast<void*>(const_cast< NaSharedDataSignaller*>(this));
    return QObject::qt_metacast(_clname);
}

int NaSharedDataSignaller::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
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
void NaSharedDataSignaller::dataChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void NaSharedDataSignaller::progressMessageChanged(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void NaSharedDataSignaller::progressValueChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void NaSharedDataSignaller::progressAborted(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void NaSharedDataSignaller::progressComplete()
{
    QMetaObject::activate(this, &staticMetaObject, 4, 0);
}

// SIGNAL 5
void NaSharedDataSignaller::invalidated()
{
    QMetaObject::activate(this, &staticMetaObject, 5, 0);
}

// SIGNAL 6
void NaSharedDataSignaller::actualDataRepresented()
{
    QMetaObject::activate(this, &staticMetaObject, 6, 0);
}
QT_END_MOC_NAMESPACE

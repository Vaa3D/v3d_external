/****************************************************************************
** Meta object code from reading C++ file 'MipMergedData.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../neuron_annotator/data_model/MipMergedData.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MipMergedData.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MipMergedData[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x0a,
      37,   24,   14,   14, 0x0a,
      70,   24,   14,   14, 0x0a,
     104,   14,   14,   14, 0x0a,
     129,   14,   14,   14, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_MipMergedData[] = {
    "MipMergedData\0\0update()\0index,status\0"
    "toggleNeuronVisibility(int,bool)\0"
    "toggleOverlayVisibility(int,bool)\0"
    "updateNeuronVisibility()\0colorizeImage()\0"
};

void MipMergedData::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MipMergedData *_t = static_cast<MipMergedData *>(_o);
        switch (_id) {
        case 0: _t->update(); break;
        case 1: _t->toggleNeuronVisibility((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 2: _t->toggleOverlayVisibility((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 3: _t->updateNeuronVisibility(); break;
        case 4: _t->colorizeImage(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData MipMergedData::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MipMergedData::staticMetaObject = {
    { &NaLockableData::staticMetaObject, qt_meta_stringdata_MipMergedData,
      qt_meta_data_MipMergedData, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MipMergedData::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MipMergedData::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MipMergedData::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MipMergedData))
        return static_cast<void*>(const_cast< MipMergedData*>(this));
    return NaLockableData::qt_metacast(_clname);
}

int MipMergedData::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = NaLockableData::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}
static const uint qt_meta_data_MipMergeLayer[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x05,

 // slots: signature, parameters, type, tag, flags
      29,   14,   14,   14, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_MipMergeLayer[] = {
    "MipMergeLayer\0\0dataChanged()\0update()\0"
};

void MipMergeLayer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MipMergeLayer *_t = static_cast<MipMergeLayer *>(_o);
        switch (_id) {
        case 0: _t->dataChanged(); break;
        case 1: _t->update(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData MipMergeLayer::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MipMergeLayer::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_MipMergeLayer,
      qt_meta_data_MipMergeLayer, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MipMergeLayer::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MipMergeLayer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MipMergeLayer::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MipMergeLayer))
        return static_cast<void*>(const_cast< MipMergeLayer*>(this));
    return QObject::qt_metacast(_clname);
}

int MipMergeLayer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
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
void MipMergeLayer::dataChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE

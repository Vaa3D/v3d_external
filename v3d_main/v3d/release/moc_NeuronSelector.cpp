/****************************************************************************
** Meta object code from reading C++ file 'NeuronSelector.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../neuron_annotator/NeuronSelector.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NeuronSelector.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_NeuronSelector[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: signature, parameters, type, tag, flags
      16,   15,   15,   15, 0x05,
      39,   15,   15,   15, 0x05,
      81,   15,   15,   15, 0x05,
     110,  104,   15,   15, 0x05,

 // slots: signature, parameters, type, tag, flags
     130,   15,   15,   15, 0x0a,
     152,   15,   15,   15, 0x0a,
     184,  178,   15,   15, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_NeuronSelector[] = {
    "NeuronSelector\0\0landmarksClearNeeded()\0"
    "landmarksUpdateNeeded(QList<ImageMarker>)\0"
    "selectionClearNeeded()\0index\0"
    "neuronSelected(int)\0onVolumeDataChanged()\0"
    "onSelectionModelChanged()\0x,y,z\0"
    "updateSelectedPosition(double,double,double)\0"
};

void NeuronSelector::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        NeuronSelector *_t = static_cast<NeuronSelector *>(_o);
        switch (_id) {
        case 0: _t->landmarksClearNeeded(); break;
        case 1: _t->landmarksUpdateNeeded((*reinterpret_cast< const QList<ImageMarker>(*)>(_a[1]))); break;
        case 2: _t->selectionClearNeeded(); break;
        case 3: _t->neuronSelected((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->onVolumeDataChanged(); break;
        case 5: _t->onSelectionModelChanged(); break;
        case 6: _t->updateSelectedPosition((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData NeuronSelector::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject NeuronSelector::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_NeuronSelector,
      qt_meta_data_NeuronSelector, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &NeuronSelector::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *NeuronSelector::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *NeuronSelector::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_NeuronSelector))
        return static_cast<void*>(const_cast< NeuronSelector*>(this));
    return QObject::qt_metacast(_clname);
}

int NeuronSelector::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void NeuronSelector::landmarksClearNeeded()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void NeuronSelector::landmarksUpdateNeeded(const QList<ImageMarker> _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void NeuronSelector::selectionClearNeeded()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}

// SIGNAL 3
void NeuronSelector::neuronSelected(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_END_MOC_NAMESPACE

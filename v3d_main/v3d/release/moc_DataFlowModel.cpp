/****************************************************************************
** Meta object code from reading C++ file 'DataFlowModel.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../neuron_annotator/DataFlowModel.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DataFlowModel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_DataFlowModel[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: signature, parameters, type, tag, flags
      21,   15,   14,   14, 0x05,
      71,   14,   14,   14, 0x05,
      88,   14,   14,   14, 0x05,
     107,   14,   14,   14, 0x05,
     138,   14,   14,   14, 0x05,
     182,  176,   14,   14, 0x05,

 // slots: signature, parameters, type, tag, flags
     204,   14,   14,   14, 0x0a,
     213,   14,   14,   14, 0x0a,
     231,   14,   14,   14, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_DataFlowModel[] = {
    "DataFlowModel\0\0index\0"
    "scrollBarFocus(NeuronSelectionModel::NeuronIndex)\0"
    "deselectNeuron()\0volumeDataNeeded()\0"
    "benchmarkTimerResetRequested()\0"
    "benchmarkTimerPrintRequested(QString)\0"
    "ratio\0zRatioChanged(double)\0cancel()\0"
    "debugColorModel()\0synchronizeColorModels()\0"
};

void DataFlowModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DataFlowModel *_t = static_cast<DataFlowModel *>(_o);
        switch (_id) {
        case 0: _t->scrollBarFocus((*reinterpret_cast< NeuronSelectionModel::NeuronIndex(*)>(_a[1]))); break;
        case 1: _t->deselectNeuron(); break;
        case 2: _t->volumeDataNeeded(); break;
        case 3: _t->benchmarkTimerResetRequested(); break;
        case 4: _t->benchmarkTimerPrintRequested((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 5: _t->zRatioChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 6: _t->cancel(); break;
        case 7: _t->debugColorModel(); break;
        case 8: _t->synchronizeColorModels(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData DataFlowModel::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject DataFlowModel::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_DataFlowModel,
      qt_meta_data_DataFlowModel, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DataFlowModel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DataFlowModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DataFlowModel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DataFlowModel))
        return static_cast<void*>(const_cast< DataFlowModel*>(this));
    return QObject::qt_metacast(_clname);
}

int DataFlowModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void DataFlowModel::scrollBarFocus(NeuronSelectionModel::NeuronIndex _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void DataFlowModel::deselectNeuron()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void DataFlowModel::volumeDataNeeded()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}

// SIGNAL 3
void DataFlowModel::benchmarkTimerResetRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 3, 0);
}

// SIGNAL 4
void DataFlowModel::benchmarkTimerPrintRequested(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void DataFlowModel::zRatioChanged(double _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_END_MOC_NAMESPACE

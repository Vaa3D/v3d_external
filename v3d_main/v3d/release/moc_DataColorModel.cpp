/****************************************************************************
** Meta object code from reading C++ file 'DataColorModel.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../neuron_annotator/data_model/DataColorModel.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DataColorModel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_DataColorModel[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      16,   15,   15,   15, 0x05,

 // slots: signature, parameters, type, tag, flags
      36,   15,   15,   15, 0x0a,
      54,   15,   49,   15, 0x0a,
      73,   15,   49,   15, 0x0a,
      92,   15,   15,   15, 0x0a,
     126,  114,   15,   15, 0x0a,
     175,  151,   15,   15, 0x0a,
     226,  212,   15,   15, 0x0a,
     274,  262,   15,   15, 0x0a,
     307,  301,   15,   15, 0x0a,
     329,  301,   15,   15, 0x0a,
     372,  354,   15,   15, 0x0a,
     403,   15,   15,   15, 0x0a,
     417,   15,   15,   15, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_DataColorModel[] = {
    "DataColorModel\0\0colorsInitialized()\0"
    "initialize()\0bool\0initializeRgba32()\0"
    "initializeRgba48()\0colorizeIncremental()\0"
    "index,color\0setChannelColor(int,int)\0"
    "index,minParam,maxParam\0"
    "setChannelDataRange(int,qreal,qreal)\0"
    "index,min,max\0setChannelHdrRange(int,qreal,qreal)\0"
    "index,gamma\0setChannelGamma(int,qreal)\0"
    "gamma\0setSharedGamma(qreal)\0"
    "setReferenceGamma(qreal)\0channel,isVisible\0"
    "setChannelVisibility(int,bool)\0"
    "resetColors()\0updateVolumeTextureMetadata()\0"
};

void DataColorModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DataColorModel *_t = static_cast<DataColorModel *>(_o);
        switch (_id) {
        case 0: _t->colorsInitialized(); break;
        case 1: _t->initialize(); break;
        case 2: { bool _r = _t->initializeRgba32();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 3: { bool _r = _t->initializeRgba48();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 4: _t->colorizeIncremental(); break;
        case 5: _t->setChannelColor((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 6: _t->setChannelDataRange((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< qreal(*)>(_a[2])),(*reinterpret_cast< qreal(*)>(_a[3]))); break;
        case 7: _t->setChannelHdrRange((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< qreal(*)>(_a[2])),(*reinterpret_cast< qreal(*)>(_a[3]))); break;
        case 8: _t->setChannelGamma((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< qreal(*)>(_a[2]))); break;
        case 9: _t->setSharedGamma((*reinterpret_cast< qreal(*)>(_a[1]))); break;
        case 10: _t->setReferenceGamma((*reinterpret_cast< qreal(*)>(_a[1]))); break;
        case 11: _t->setChannelVisibility((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 12: _t->resetColors(); break;
        case 13: _t->updateVolumeTextureMetadata(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData DataColorModel::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject DataColorModel::staticMetaObject = {
    { &NaSharedDataModel<PrivateDataColorModel>::staticMetaObject, qt_meta_stringdata_DataColorModel,
      qt_meta_data_DataColorModel, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DataColorModel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DataColorModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DataColorModel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DataColorModel))
        return static_cast<void*>(const_cast< DataColorModel*>(this));
    return NaSharedDataModel<PrivateDataColorModel>::qt_metacast(_clname);
}

int DataColorModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = NaSharedDataModel<PrivateDataColorModel>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    }
    return _id;
}

// SIGNAL 0
void DataColorModel::colorsInitialized()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE

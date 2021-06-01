/****************************************************************************
** Meta object code from reading C++ file 'DynamicRangeTool.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../neuron_annotator/gui/DynamicRangeTool.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DynamicRangeTool.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_DynamicRangeTool[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: signature, parameters, type, tag, flags
      24,   18,   17,   17, 0x05,
      56,   44,   17,   17, 0x05,
      99,   85,   17,   17, 0x05,
     151,  139,   17,   17, 0x05,

 // slots: signature, parameters, type, tag, flags
     195,  182,   17,   17, 0x0a,
     211,   17,   17,   17, 0x0a,
     229,  225,   17,   17, 0x0a,
     248,  244,   17,   17, 0x0a,
     269,  263,   17,   17, 0x0a,
     286,   17,   17,   17, 0x0a,
     300,   17,   17,   17, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_DynamicRangeTool[] = {
    "DynamicRangeTool\0\0index\0channelChanged(int)\0"
    "index,color\0channelColorChanged(int,int)\0"
    "index,min,max\0channelHdrRangeChanged(int,qreal,qreal)\0"
    "index,gamma\0channelGammaChanged(int,qreal)\0"
    "channelIndex\0setChannel(int)\0selectColor()\0"
    "min\0setHdrMin(int)\0max\0setHdrMax(int)\0"
    "gamma\0setGamma(double)\0resetColors()\0"
    "initializeColors()\0"
};

void DynamicRangeTool::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DynamicRangeTool *_t = static_cast<DynamicRangeTool *>(_o);
        switch (_id) {
        case 0: _t->channelChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->channelColorChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->channelHdrRangeChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< qreal(*)>(_a[2])),(*reinterpret_cast< qreal(*)>(_a[3]))); break;
        case 3: _t->channelGammaChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< qreal(*)>(_a[2]))); break;
        case 4: _t->setChannel((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->selectColor(); break;
        case 6: _t->setHdrMin((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->setHdrMax((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->setGamma((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 9: _t->resetColors(); break;
        case 10: _t->initializeColors(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData DynamicRangeTool::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject DynamicRangeTool::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_DynamicRangeTool,
      qt_meta_data_DynamicRangeTool, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DynamicRangeTool::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DynamicRangeTool::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DynamicRangeTool::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DynamicRangeTool))
        return static_cast<void*>(const_cast< DynamicRangeTool*>(this));
    return QDialog::qt_metacast(_clname);
}

int DynamicRangeTool::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void DynamicRangeTool::channelChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void DynamicRangeTool::channelColorChanged(int _t1, int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void DynamicRangeTool::channelHdrRangeChanged(int _t1, qreal _t2, qreal _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void DynamicRangeTool::channelGammaChanged(int _t1, qreal _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_END_MOC_NAMESPACE

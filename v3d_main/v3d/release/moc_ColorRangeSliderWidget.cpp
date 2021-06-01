/****************************************************************************
** Meta object code from reading C++ file 'ColorRangeSliderWidget.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../neuron_annotator/gui/ColorRangeSliderWidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ColorRangeSliderWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ColorRangeSliderWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      28,   24,   23,   23, 0x05,
      51,   47,   23,   23, 0x05,
      76,   70,   23,   23, 0x05,

 // slots: signature, parameters, type, tag, flags
      97,   24,   23,   23, 0x0a,
     112,   47,   23,   23, 0x0a,
     127,   70,   23,   23, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_ColorRangeSliderWidget[] = {
    "ColorRangeSliderWidget\0\0min\0"
    "hdrMinChanged(int)\0max\0hdrMaxChanged(int)\0"
    "gamma\0gammaChanged(double)\0setHdrMin(int)\0"
    "setHdrMax(int)\0setGamma(double)\0"
};

void ColorRangeSliderWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ColorRangeSliderWidget *_t = static_cast<ColorRangeSliderWidget *>(_o);
        switch (_id) {
        case 0: _t->hdrMinChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->hdrMaxChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->gammaChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 3: _t->setHdrMin((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->setHdrMax((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->setGamma((*reinterpret_cast< double(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData ColorRangeSliderWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ColorRangeSliderWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_ColorRangeSliderWidget,
      qt_meta_data_ColorRangeSliderWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ColorRangeSliderWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ColorRangeSliderWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ColorRangeSliderWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ColorRangeSliderWidget))
        return static_cast<void*>(const_cast< ColorRangeSliderWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int ColorRangeSliderWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void ColorRangeSliderWidget::hdrMinChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ColorRangeSliderWidget::hdrMaxChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void ColorRangeSliderWidget::gammaChanged(double _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_END_MOC_NAMESPACE

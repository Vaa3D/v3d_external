/****************************************************************************
** Meta object code from reading C++ file 'RendererNeuronAnnotator.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../neuron_annotator/gui/RendererNeuronAnnotator.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'RendererNeuronAnnotator.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_RendererNeuronAnnotator[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      15,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       7,       // signalCount

 // signals: signature, parameters, type, tag, flags
      25,   24,   24,   24, 0x05,
      51,   24,   24,   24, 0x05,
      70,   24,   24,   24, 0x05,
     102,   24,   24,   24, 0x05,
     127,   24,   24,   24, 0x05,
     154,   24,   24,   24, 0x05,
     182,   24,   24,   24, 0x05,

 // slots: signature, parameters, type, tag, flags
     208,   24,   24,   24, 0x0a,
     233,  231,   24,   24, 0x0a,
     262,   24,  257,   24, 0x0a,
     296,  284,   24,   24, 0x0a,
     318,   24,  257,   24, 0x0a,
     349,  339,   24,   24, 0x0a,
     372,  339,   24,   24, 0x0a,
     399,   24,   24,   24, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_RendererNeuronAnnotator[] = {
    "RendererNeuronAnnotator\0\0"
    "progressValueChanged(int)\0progressComplete()\0"
    "progressMessageChanged(QString)\0"
    "progressAborted(QString)\0"
    "alphaBlendingChanged(bool)\0"
    "showCornerAxesChanged(bool)\0"
    "slabThicknessChanged(int)\0"
    "setAlphaBlending(bool)\0b\0"
    "setShowCornerAxes(bool)\0bool\0"
    "setSlabThickness(int)\0cameraModel\0"
    "clipSlab(CameraModel)\0resetSlabThickness()\0"
    "textureId\0set3dTextureMode(uint)\0"
    "setColorMapTextureId(uint)\0clearImage()\0"
};

void RendererNeuronAnnotator::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        RendererNeuronAnnotator *_t = static_cast<RendererNeuronAnnotator *>(_o);
        switch (_id) {
        case 0: _t->progressValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->progressComplete(); break;
        case 2: _t->progressMessageChanged((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 3: _t->progressAborted((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 4: _t->alphaBlendingChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->showCornerAxesChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 6: _t->slabThicknessChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->setAlphaBlending((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 8: _t->setShowCornerAxes((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 9: { bool _r = _t->setSlabThickness((*reinterpret_cast< int(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 10: _t->clipSlab((*reinterpret_cast< const CameraModel(*)>(_a[1]))); break;
        case 11: { bool _r = _t->resetSlabThickness();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 12: _t->set3dTextureMode((*reinterpret_cast< uint(*)>(_a[1]))); break;
        case 13: _t->setColorMapTextureId((*reinterpret_cast< uint(*)>(_a[1]))); break;
        case 14: _t->clearImage(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData RendererNeuronAnnotator::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject RendererNeuronAnnotator::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_RendererNeuronAnnotator,
      qt_meta_data_RendererNeuronAnnotator, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &RendererNeuronAnnotator::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *RendererNeuronAnnotator::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *RendererNeuronAnnotator::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_RendererNeuronAnnotator))
        return static_cast<void*>(const_cast< RendererNeuronAnnotator*>(this));
    if (!strcmp(_clname, "Renderer_gl2"))
        return static_cast< Renderer_gl2*>(const_cast< RendererNeuronAnnotator*>(this));
    return QObject::qt_metacast(_clname);
}

int RendererNeuronAnnotator::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 15)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    }
    return _id;
}

// SIGNAL 0
void RendererNeuronAnnotator::progressValueChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void RendererNeuronAnnotator::progressComplete()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void RendererNeuronAnnotator::progressMessageChanged(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void RendererNeuronAnnotator::progressAborted(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void RendererNeuronAnnotator::alphaBlendingChanged(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void RendererNeuronAnnotator::showCornerAxesChanged(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void RendererNeuronAnnotator::slabThicknessChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}
QT_END_MOC_NAMESPACE

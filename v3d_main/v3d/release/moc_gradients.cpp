/****************************************************************************
** Meta object code from reading C++ file 'gradients.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../3drenderer/gradients.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'gradients.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ShadeWidget[] = {

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
      19,   13,   12,   12, 0x05,

 // slots: signature, parameters, type, tag, flags
      52,   48,   12,   12, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_ShadeWidget[] = {
    "ShadeWidget\0\0type,\0colorsChanged(int,QPolygonF)\0"
    "pts\0changeColors(QPolygonF)\0"
};

void ShadeWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ShadeWidget *_t = static_cast<ShadeWidget *>(_o);
        switch (_id) {
        case 0: _t->colorsChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const QPolygonF(*)>(_a[2]))); break;
        case 1: _t->changeColors((*reinterpret_cast< const QPolygonF(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData ShadeWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ShadeWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_ShadeWidget,
      qt_meta_data_ShadeWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ShadeWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ShadeWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ShadeWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ShadeWidget))
        return static_cast<void*>(const_cast< ShadeWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int ShadeWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
void ShadeWidget::colorsChanged(int _t1, const QPolygonF & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_GradientEditor[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      16,   15,   15,   15, 0x05,

 // slots: signature, parameters, type, tag, flags
      59,   53,   15,   15, 0x0a,
     103,   15,   88,   15, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_GradientEditor[] = {
    "GradientEditor\0\0gradientStopsChanged(QGradientStops)\0"
    "type,\0pointsUpdated(int,QPolygonF)\0"
    "QGradientStops\0updateAlphaStops()\0"
};

void GradientEditor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        GradientEditor *_t = static_cast<GradientEditor *>(_o);
        switch (_id) {
        case 0: _t->gradientStopsChanged((*reinterpret_cast< const QGradientStops(*)>(_a[1]))); break;
        case 1: _t->pointsUpdated((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const QPolygonF(*)>(_a[2]))); break;
        case 2: { QGradientStops _r = _t->updateAlphaStops();
            if (_a[0]) *reinterpret_cast< QGradientStops*>(_a[0]) = _r; }  break;
        default: ;
        }
    }
}

const QMetaObjectExtraData GradientEditor::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject GradientEditor::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_GradientEditor,
      qt_meta_data_GradientEditor, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &GradientEditor::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *GradientEditor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *GradientEditor::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_GradientEditor))
        return static_cast<void*>(const_cast< GradientEditor*>(this));
    return QWidget::qt_metacast(_clname);
}

int GradientEditor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void GradientEditor::gradientStopsChanged(const QGradientStops & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_GradientRenderer[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      24,   18,   17,   17, 0x0a,
      57,   17,   17,   17, 0x0a,
      72,   17,   17,   17, 0x0a,
      90,   17,   17,   17, 0x0a,
     109,   17,   17,   17, 0x0a,
     129,   17,   17,   17, 0x0a,
     149,   17,   17,   17, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_GradientRenderer[] = {
    "GradientRenderer\0\0stops\0"
    "setGradientStops(QGradientStops)\0"
    "setPadSpread()\0setRepeatSpread()\0"
    "setReflectSpread()\0setLinearGradient()\0"
    "setRadialGradient()\0setConicalGradient()\0"
};

void GradientRenderer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        GradientRenderer *_t = static_cast<GradientRenderer *>(_o);
        switch (_id) {
        case 0: _t->setGradientStops((*reinterpret_cast< const QGradientStops(*)>(_a[1]))); break;
        case 1: _t->setPadSpread(); break;
        case 2: _t->setRepeatSpread(); break;
        case 3: _t->setReflectSpread(); break;
        case 4: _t->setLinearGradient(); break;
        case 5: _t->setRadialGradient(); break;
        case 6: _t->setConicalGradient(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData GradientRenderer::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject GradientRenderer::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_GradientRenderer,
      qt_meta_data_GradientRenderer, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &GradientRenderer::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *GradientRenderer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *GradientRenderer::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_GradientRenderer))
        return static_cast<void*>(const_cast< GradientRenderer*>(this));
    return QWidget::qt_metacast(_clname);
}

int GradientRenderer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}
static const uint qt_meta_data_GradientWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      16,   15,   15,   15, 0x0a,
      30,   15,   15,   15, 0x0a,
      44,   15,   15,   15, 0x0a,
      58,   15,   15,   15, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_GradientWidget[] = {
    "GradientWidget\0\0setDefault1()\0"
    "setDefault2()\0setDefault3()\0setDefault4()\0"
};

void GradientWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        GradientWidget *_t = static_cast<GradientWidget *>(_o);
        switch (_id) {
        case 0: _t->setDefault1(); break;
        case 1: _t->setDefault2(); break;
        case 2: _t->setDefault3(); break;
        case 3: _t->setDefault4(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData GradientWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject GradientWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_GradientWidget,
      qt_meta_data_GradientWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &GradientWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *GradientWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *GradientWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_GradientWidget))
        return static_cast<void*>(const_cast< GradientWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int GradientWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}
QT_END_MOC_NAMESPACE

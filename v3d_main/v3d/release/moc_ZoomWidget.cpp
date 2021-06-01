/****************************************************************************
** Meta object code from reading C++ file 'ZoomWidget.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../neuron_annotator/gui/ZoomWidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ZoomWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ZoomWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      22,   12,   11,   11, 0x05,

 // slots: signature, parameters, type, tag, flags
      58,   46,   11,   11, 0x0a,
      78,   11,   11,   11, 0x0a,
      86,   11,   11,   11, 0x09,
     110,   11,   11,   11, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_ZoomWidget[] = {
    "ZoomWidget\0\0zoomValue\0zoomValueChanged(qreal)\0"
    "doubleValue\0setZoomValue(qreal)\0reset()\0"
    "onDialValueChanged(int)\0"
    "onSpinBoxValueChanged(qreal)\0"
};

void ZoomWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ZoomWidget *_t = static_cast<ZoomWidget *>(_o);
        switch (_id) {
        case 0: _t->zoomValueChanged((*reinterpret_cast< qreal(*)>(_a[1]))); break;
        case 1: _t->setZoomValue((*reinterpret_cast< qreal(*)>(_a[1]))); break;
        case 2: _t->reset(); break;
        case 3: _t->onDialValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->onSpinBoxValueChanged((*reinterpret_cast< qreal(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData ZoomWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ZoomWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_ZoomWidget,
      qt_meta_data_ZoomWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ZoomWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ZoomWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ZoomWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ZoomWidget))
        return static_cast<void*>(const_cast< ZoomWidget*>(this));
    if (!strcmp(_clname, "Ui::ZoomWidget"))
        return static_cast< Ui::ZoomWidget*>(const_cast< ZoomWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int ZoomWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void ZoomWidget::zoomValueChanged(qreal _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE

/****************************************************************************
** Meta object code from reading C++ file 'AngleWidget.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../neuron_annotator/gui/AngleWidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AngleWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_AngleWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       1,   29, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      28,   13,   12,   12, 0x05,

 // slots: signature, parameters, type, tag, flags
      52,   46,   12,   12, 0x0a,
      68,   12,   12,   12, 0x0a,

 // properties: name, type, flags
      84,   76, 0x0a095103,

       0        // eod
};

static const char qt_meta_stringdata_AngleWidget[] = {
    "AngleWidget\0\0angleInDegrees\0"
    "angleChanged(int)\0angle\0setAngle(qreal)\0"
    "reset()\0QString\0textLabel\0"
};

void AngleWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        AngleWidget *_t = static_cast<AngleWidget *>(_o);
        switch (_id) {
        case 0: _t->angleChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->setAngle((*reinterpret_cast< qreal(*)>(_a[1]))); break;
        case 2: _t->reset(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData AngleWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject AngleWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_AngleWidget,
      qt_meta_data_AngleWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &AngleWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *AngleWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *AngleWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_AngleWidget))
        return static_cast<void*>(const_cast< AngleWidget*>(this));
    if (!strcmp(_clname, "Ui::AngleWidget"))
        return static_cast< Ui::AngleWidget*>(const_cast< AngleWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int AngleWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = textLabel(); break;
        }
        _id -= 1;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: setTextLabel(*reinterpret_cast< QString*>(_v)); break;
        }
        _id -= 1;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 1;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void AngleWidget::angleChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE

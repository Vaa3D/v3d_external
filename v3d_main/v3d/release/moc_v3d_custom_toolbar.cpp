/****************************************************************************
** Meta object code from reading C++ file 'v3d_custom_toolbar.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../custom_toolbar/v3d_custom_toolbar.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'v3d_custom_toolbar.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_CustomToolButton[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      23,   18,   17,   17, 0x0a,
      51,   17,   46,   17, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_CustomToolButton[] = {
    "CustomToolButton\0\0text\0setButtonText(QString)\0"
    "bool\0run()\0"
};

void CustomToolButton::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        CustomToolButton *_t = static_cast<CustomToolButton *>(_o);
        switch (_id) {
        case 0: _t->setButtonText((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: { bool _r = _t->run();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        default: ;
        }
    }
}

const QMetaObjectExtraData CustomToolButton::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject CustomToolButton::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_CustomToolButton,
      qt_meta_data_CustomToolButton, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &CustomToolButton::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *CustomToolButton::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *CustomToolButton::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CustomToolButton))
        return static_cast<void*>(const_cast< CustomToolButton*>(this));
    return QObject::qt_metacast(_clname);
}

int CustomToolButton::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
static const uint qt_meta_data_CustomToolbarSelectWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      33,   27,   26,   26, 0x0a,
      56,   26,   26,   26, 0x0a,
      75,   26,   26,   26, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_CustomToolbarSelectWidget[] = {
    "CustomToolbarSelectWidget\0\0state\0"
    "setToolBarButton(bool)\0saveToolBarState()\0"
    "openMe()\0"
};

void CustomToolbarSelectWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        CustomToolbarSelectWidget *_t = static_cast<CustomToolbarSelectWidget *>(_o);
        switch (_id) {
        case 0: _t->setToolBarButton((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->saveToolBarState(); break;
        case 2: _t->openMe(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData CustomToolbarSelectWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject CustomToolbarSelectWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_CustomToolbarSelectWidget,
      qt_meta_data_CustomToolbarSelectWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &CustomToolbarSelectWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *CustomToolbarSelectWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *CustomToolbarSelectWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CustomToolbarSelectWidget))
        return static_cast<void*>(const_cast< CustomToolbarSelectWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int CustomToolbarSelectWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
QT_END_MOC_NAMESPACE

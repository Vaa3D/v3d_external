/****************************************************************************
** Meta object code from reading C++ file 'old_arthurwidgets.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../old_arthurwidgets.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'old_arthurwidgets.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ArthurFrame[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      13,   12,   12,   12, 0x05,

 // slots: signature, parameters, type, tag, flags
      48,   45,   12,   12, 0x0a,
      77,   69,   12,   12, 0x0a,
     105,   12,   12,   12, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_ArthurFrame[] = {
    "ArthurFrame\0\0descriptionEnabledChanged(bool)\0"
    "pi\0setPreferImage(bool)\0enabled\0"
    "setDescriptionEnabled(bool)\0showSource()\0"
};

void ArthurFrame::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ArthurFrame *_t = static_cast<ArthurFrame *>(_o);
        switch (_id) {
        case 0: _t->descriptionEnabledChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->setPreferImage((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->setDescriptionEnabled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->showSource(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData ArthurFrame::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ArthurFrame::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_ArthurFrame,
      qt_meta_data_ArthurFrame, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ArthurFrame::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ArthurFrame::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ArthurFrame::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ArthurFrame))
        return static_cast<void*>(const_cast< ArthurFrame*>(this));
    return QWidget::qt_metacast(_clname);
}

int ArthurFrame::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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

// SIGNAL 0
void ArthurFrame::descriptionEnabledChanged(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE

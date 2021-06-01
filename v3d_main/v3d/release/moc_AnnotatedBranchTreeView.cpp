/****************************************************************************
** Meta object code from reading C++ file 'AnnotatedBranchTreeView.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../neuron_annotator/gui/trees/AnnotatedBranchTreeView.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AnnotatedBranchTreeView.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_AnnotatedBranchTreeView[] = {

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
      36,   25,   24,   24, 0x05,

 // slots: signature, parameters, type, tag, flags
      72,   68,   24,   24, 0x08,
      96,   24,   24,   24, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_AnnotatedBranchTreeView[] = {
    "AnnotatedBranchTreeView\0\0annotation\0"
    "removeAnnotation(const Entity*)\0pnt\0"
    "showContextMenu(QPoint)\0removeAnnotation()\0"
};

void AnnotatedBranchTreeView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        AnnotatedBranchTreeView *_t = static_cast<AnnotatedBranchTreeView *>(_o);
        switch (_id) {
        case 0: _t->removeAnnotation((*reinterpret_cast< const Entity*(*)>(_a[1]))); break;
        case 1: _t->showContextMenu((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 2: _t->removeAnnotation(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData AnnotatedBranchTreeView::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject AnnotatedBranchTreeView::staticMetaObject = {
    { &EntityTreeView::staticMetaObject, qt_meta_stringdata_AnnotatedBranchTreeView,
      qt_meta_data_AnnotatedBranchTreeView, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &AnnotatedBranchTreeView::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *AnnotatedBranchTreeView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *AnnotatedBranchTreeView::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_AnnotatedBranchTreeView))
        return static_cast<void*>(const_cast< AnnotatedBranchTreeView*>(this));
    return EntityTreeView::qt_metacast(_clname);
}

int AnnotatedBranchTreeView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = EntityTreeView::qt_metacall(_c, _id, _a);
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
void AnnotatedBranchTreeView::removeAnnotation(const Entity * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE

/****************************************************************************
** Meta object code from reading C++ file 'PAnoToolBar.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../terafly/src/presentation/PAnoToolBar.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PAnoToolBar.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_terafly__PAnoToolBar[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      30,   22,   21,   21, 0x0a,
      62,   22,   21,   21, 0x0a,
      95,   22,   21,   21, 0x0a,
     127,   22,   21,   21, 0x0a,
     162,   22,   21,   21, 0x0a,
     195,   22,   21,   21, 0x0a,
     227,   21,   21,   21, 0x0a,
     247,   21,   21,   21, 0x0a,
     267,   21,   21,   21, 0x0a,
     285,   21,   21,   21, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_terafly__PAnoToolBar[] = {
    "terafly::PAnoToolBar\0\0checked\0"
    "buttonMarkerCreateChecked(bool)\0"
    "buttonMarkerCreate2Checked(bool)\0"
    "buttonMarkerDeleteChecked(bool)\0"
    "buttonMarkerRoiDeleteChecked(bool)\0"
    "buttonMarkerRoiViewChecked(bool)\0"
    "buttonFragmenTraceChecked(bool)\0"
    "buttonUndoClicked()\0buttonRedoClicked()\0"
    "saveAnnotations()\0autosaveAnnotations()\0"
};

void terafly::PAnoToolBar::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        PAnoToolBar *_t = static_cast<PAnoToolBar *>(_o);
        switch (_id) {
        case 0: _t->buttonMarkerCreateChecked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->buttonMarkerCreate2Checked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->buttonMarkerDeleteChecked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->buttonMarkerRoiDeleteChecked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->buttonMarkerRoiViewChecked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->buttonFragmenTraceChecked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 6: _t->buttonUndoClicked(); break;
        case 7: _t->buttonRedoClicked(); break;
        case 8: _t->saveAnnotations(); break;
        case 9: _t->autosaveAnnotations(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData terafly::PAnoToolBar::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject terafly::PAnoToolBar::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_terafly__PAnoToolBar,
      qt_meta_data_terafly__PAnoToolBar, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &terafly::PAnoToolBar::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *terafly::PAnoToolBar::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *terafly::PAnoToolBar::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_terafly__PAnoToolBar))
        return static_cast<void*>(const_cast< PAnoToolBar*>(this));
    return QWidget::qt_metacast(_clname);
}

int terafly::PAnoToolBar::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    return _id;
}
QT_END_MOC_NAMESPACE

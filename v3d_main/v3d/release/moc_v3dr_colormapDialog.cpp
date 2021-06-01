/****************************************************************************
** Meta object code from reading C++ file 'v3dr_colormapDialog.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../3drenderer/v3dr_colormapDialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'v3dr_colormapDialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_V3dr_colormapDialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      15,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      23,   21,   20,   20, 0x0a,
      42,   40,   20,   20, 0x0a,
      58,   20,   20,   20, 0x0a,
      72,   20,   20,   20, 0x0a,
      86,   20,   20,   20, 0x0a,
     100,   20,   20,   20, 0x0a,
     114,   20,   20,   20, 0x0a,
     128,   20,   20,   20, 0x0a,
     142,   20,   20,   20, 0x0a,
     156,   20,   20,   20, 0x0a,
     163,   20,   20,   20, 0x0a,
     177,   20,   20,   20, 0x0a,
     194,   20,   20,   20, 0x0a,
     201,   20,   20,   20, 0x0a,
     208,   20,   20,   20, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_V3dr_colormapDialog[] = {
    "V3dr_colormapDialog\0\0w\0linkTo(QWidget*)\0"
    "k\0setDefault(int)\0setDefault0()\0"
    "setDefault1()\0setDefault2()\0setDefault3()\0"
    "setDefault4()\0setDefault5()\0setDefault6()\0"
    "undo()\0updateStops()\0updateColormap()\0"
    "load()\0save()\0applyToImage()\0"
};

void V3dr_colormapDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        V3dr_colormapDialog *_t = static_cast<V3dr_colormapDialog *>(_o);
        switch (_id) {
        case 0: _t->linkTo((*reinterpret_cast< QWidget*(*)>(_a[1]))); break;
        case 1: _t->setDefault((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->setDefault0(); break;
        case 3: _t->setDefault1(); break;
        case 4: _t->setDefault2(); break;
        case 5: _t->setDefault3(); break;
        case 6: _t->setDefault4(); break;
        case 7: _t->setDefault5(); break;
        case 8: _t->setDefault6(); break;
        case 9: _t->undo(); break;
        case 10: _t->updateStops(); break;
        case 11: _t->updateColormap(); break;
        case 12: _t->load(); break;
        case 13: _t->save(); break;
        case 14: _t->applyToImage(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData V3dr_colormapDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject V3dr_colormapDialog::staticMetaObject = {
    { &SharedToolDialog::staticMetaObject, qt_meta_stringdata_V3dr_colormapDialog,
      qt_meta_data_V3dr_colormapDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &V3dr_colormapDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *V3dr_colormapDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *V3dr_colormapDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_V3dr_colormapDialog))
        return static_cast<void*>(const_cast< V3dr_colormapDialog*>(this));
    return SharedToolDialog::qt_metacast(_clname);
}

int V3dr_colormapDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = SharedToolDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 15)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    }
    return _id;
}
QT_END_MOC_NAMESPACE

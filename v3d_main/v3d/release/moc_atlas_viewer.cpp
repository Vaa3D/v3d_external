/****************************************************************************
** Meta object code from reading C++ file 'atlas_viewer.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../atlas_viewer.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'atlas_viewer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_V3D_atlas_viewerDialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      31,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      24,   23,   23,   23, 0x0a,
      33,   23,   23,   23, 0x0a,
      42,   23,   23,   23, 0x0a,
      52,   23,   23,   23, 0x0a,
      61,   59,   23,   23, 0x0a,
      90,   23,   23,   23, 0x0a,
     102,   23,   23,   23, 0x0a,
     116,   23,   23,   23, 0x0a,
     132,   23,   23,   23, 0x0a,
     145,   23,   23,   23, 0x0a,
     159,   23,   23,   23, 0x0a,
     177,  175,   23,   23, 0x0a,
     203,   23,   23,   23, 0x0a,
     227,  225,   23,   23, 0x0a,
     243,   23,   23,   23, 0x0a,
     254,   23,   23,   23, 0x0a,
     265,   23,   23,   23, 0x0a,
     285,  281,   23,   23, 0x0a,
     304,   23,   23,   23, 0x2a,
     320,   23,   23,   23, 0x0a,
     339,   23,   23,   23, 0x0a,
     364,  356,   23,   23, 0x0a,
     386,  356,   23,   23, 0x0a,
     442,  408,   23,   23, 0x0a,
     477,  356,   23,   23, 0x0a,
     503,   23,   23,   23, 0x0a,
     525,   23,   23,   23, 0x0a,
     542,   23,   23,   23, 0x0a,
     561,   23,   23,   23, 0x0a,
     586,   23,   23,   23, 0x0a,
     610,   23,   23,   23, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_V3D_atlas_viewerDialog[] = {
    "V3D_atlas_viewerDialog\0\0accept()\0"
    "reject()\0done(int)\0undo()\0w\0"
    "reCreateTables(XFormWidget*)\0selectAll()\0"
    "deselectAll()\0inverseSelect()\0"
    "onSelected()\0offSelected()\0colorSelected()\0"
    "c\0colorChannelSelected(int)\0"
    "maskImgStateChanged()\0t\0tabChanged(int)\0"
    "findNext()\0findPrev()\0doMenuOfColor()\0"
    "map\0selectedColor(int)\0selectedColor()\0"
    "mapHanchuanColor()\0mapRandomColor()\0"
    "row,col\0pickAtlasRow(int,int)\0"
    "pickLandmark(int,int)\0"
    "row,col,previous_row,previous_col\0"
    "highlightLandmark(int,int,int,int)\0"
    "pickColorChannel(int,int)\0"
    "seeLandmarkProperty()\0moveLandmarkUp()\0"
    "moveLandmarkDown()\0deleteSelectedLandmark()\0"
    "resetAllLandmarkNames()\0"
    "resetAllLandmarkComments()\0"
};

void V3D_atlas_viewerDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        V3D_atlas_viewerDialog *_t = static_cast<V3D_atlas_viewerDialog *>(_o);
        switch (_id) {
        case 0: _t->accept(); break;
        case 1: _t->reject(); break;
        case 2: _t->done((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->undo(); break;
        case 4: _t->reCreateTables((*reinterpret_cast< XFormWidget*(*)>(_a[1]))); break;
        case 5: _t->selectAll(); break;
        case 6: _t->deselectAll(); break;
        case 7: _t->inverseSelect(); break;
        case 8: _t->onSelected(); break;
        case 9: _t->offSelected(); break;
        case 10: _t->colorSelected(); break;
        case 11: _t->colorChannelSelected((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->maskImgStateChanged(); break;
        case 13: _t->tabChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 14: _t->findNext(); break;
        case 15: _t->findPrev(); break;
        case 16: _t->doMenuOfColor(); break;
        case 17: _t->selectedColor((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 18: _t->selectedColor(); break;
        case 19: _t->mapHanchuanColor(); break;
        case 20: _t->mapRandomColor(); break;
        case 21: _t->pickAtlasRow((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 22: _t->pickLandmark((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 23: _t->highlightLandmark((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4]))); break;
        case 24: _t->pickColorChannel((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 25: _t->seeLandmarkProperty(); break;
        case 26: _t->moveLandmarkUp(); break;
        case 27: _t->moveLandmarkDown(); break;
        case 28: _t->deleteSelectedLandmark(); break;
        case 29: _t->resetAllLandmarkNames(); break;
        case 30: _t->resetAllLandmarkComments(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData V3D_atlas_viewerDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject V3D_atlas_viewerDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_V3D_atlas_viewerDialog,
      qt_meta_data_V3D_atlas_viewerDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &V3D_atlas_viewerDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *V3D_atlas_viewerDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *V3D_atlas_viewerDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_V3D_atlas_viewerDialog))
        return static_cast<void*>(const_cast< V3D_atlas_viewerDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int V3D_atlas_viewerDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 31)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 31;
    }
    return _id;
}
QT_END_MOC_NAMESPACE

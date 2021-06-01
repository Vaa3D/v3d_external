/****************************************************************************
** Meta object code from reading C++ file 'v3dr_surfaceDialog.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../3drenderer/v3dr_surfaceDialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'v3dr_surfaceDialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_V3dr_surfaceDialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      41,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      22,   20,   19,   19, 0x0a,
      43,   20,   39,   19, 0x0a,
      60,   19,   19,   19, 0x0a,
      77,   19,   19,   19, 0x0a,
      84,   19,   19,   19, 0x0a,
      96,   19,   19,   19, 0x0a,
     110,   19,   19,   19, 0x0a,
     132,  126,   19,   19, 0x0a,
     152,   19,   19,   19, 0x0a,
     165,   19,   19,   19, 0x0a,
     179,   19,   19,   19, 0x0a,
     199,  195,   19,   19, 0x0a,
     218,   19,   19,   19, 0x2a,
     234,   19,   19,   19, 0x0a,
     252,   19,   19,   19, 0x0a,
     274,   19,   19,   19, 0x0a,
     293,   19,   19,   19, 0x0a,
     310,   19,   19,   19, 0x0a,
     334,  332,   19,   19, 0x0a,
     357,   19,   19,   19, 0x0a,
     392,   19,   19,   19, 0x0a,
     417,   19,   19,   19, 0x0a,
     450,  442,   19,   19, 0x0a,
     479,  442,   19,   19, 0x0a,
     507,  442,   19,   19, 0x0a,
     525,  442,   19,   19, 0x0a,
     552,  442,   19,   19, 0x0a,
     569,  442,   19,   19, 0x0a,
     586,  442,   19,   19, 0x0a,
     607,  442,   19,   19, 0x0a,
     627,   19,   19,   19, 0x0a,
     652,   19,   19,   19, 0x0a,
     676,   19,   19,   19, 0x0a,
     687,   19,   19,   19, 0x0a,
     698,   19,   19,   19, 0x0a,
     717,   19,   19,   19, 0x0a,
     737,   19,   19,   19, 0x0a,
     766,  758,   19,   19, 0x0a,
     803,   19,   19,   19, 0x0a,
     820,   19,   39,   19, 0x0a,
     840,  835,   19,   19, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_V3dr_surfaceDialog[] = {
    "V3dr_surfaceDialog\0\0w\0linkTo(QWidget*)\0"
    "int\0DecRef(QWidget*)\0onAttached(bool)\0"
    "undo()\0selectAll()\0deselectAll()\0"
    "selectInverse()\0state\0selectedOnOff(bool)\0"
    "selectedOn()\0selectedOff()\0doMenuOfColor()\0"
    "map\0selectedColor(int)\0selectedColor()\0"
    "mapSegmentColor()\0mapMultiNeuronColor()\0"
    "mapHanchuanColor()\0mapRandomColor()\0"
    "doMenuOfDisplayMode()\0v\0setSWCDisplayMode(int)\0"
    "setSWCDisplayUsingGlobalSettings()\0"
    "setSWCDisplayUsingLine()\0"
    "setSWCDisplayUsingTube()\0row,col\0"
    "pressedClickHandler(int,int)\0"
    "doubleClickHandler(int,int)\0"
    "pickSurf(int,int)\0pickNeuronSegment(int,int)\0"
    "pickSWC(int,int)\0pickAPO(int,int)\0"
    "pickAPO_Set(int,int)\0pickMarker(int,int)\0"
    "editObjNameAndComments()\0"
    "editNeuronSegmentType()\0findNext()\0"
    "findPrev()\0findAllHighlight()\0"
    "onMarkerLocalView()\0zoomMarkerLocation()\0"
    "markers\0updateMarkerList(QList<ImageMarker>)\0"
    "menuExecBuffer()\0getMarkerNum()\0item\0"
    "sortNeuronSegmentByType(QTableWidgetItem*)\0"
};

void V3dr_surfaceDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        V3dr_surfaceDialog *_t = static_cast<V3dr_surfaceDialog *>(_o);
        switch (_id) {
        case 0: _t->linkTo((*reinterpret_cast< QWidget*(*)>(_a[1]))); break;
        case 1: { int _r = _t->DecRef((*reinterpret_cast< QWidget*(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 2: _t->onAttached((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->undo(); break;
        case 4: _t->selectAll(); break;
        case 5: _t->deselectAll(); break;
        case 6: _t->selectInverse(); break;
        case 7: _t->selectedOnOff((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 8: _t->selectedOn(); break;
        case 9: _t->selectedOff(); break;
        case 10: _t->doMenuOfColor(); break;
        case 11: _t->selectedColor((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->selectedColor(); break;
        case 13: _t->mapSegmentColor(); break;
        case 14: _t->mapMultiNeuronColor(); break;
        case 15: _t->mapHanchuanColor(); break;
        case 16: _t->mapRandomColor(); break;
        case 17: _t->doMenuOfDisplayMode(); break;
        case 18: _t->setSWCDisplayMode((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 19: _t->setSWCDisplayUsingGlobalSettings(); break;
        case 20: _t->setSWCDisplayUsingLine(); break;
        case 21: _t->setSWCDisplayUsingTube(); break;
        case 22: _t->pressedClickHandler((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 23: _t->doubleClickHandler((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 24: _t->pickSurf((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 25: _t->pickNeuronSegment((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 26: _t->pickSWC((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 27: _t->pickAPO((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 28: _t->pickAPO_Set((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 29: _t->pickMarker((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 30: _t->editObjNameAndComments(); break;
        case 31: _t->editNeuronSegmentType(); break;
        case 32: _t->findNext(); break;
        case 33: _t->findPrev(); break;
        case 34: _t->findAllHighlight(); break;
        case 35: _t->onMarkerLocalView(); break;
        case 36: _t->zoomMarkerLocation(); break;
        case 37: _t->updateMarkerList((*reinterpret_cast< QList<ImageMarker>(*)>(_a[1]))); break;
        case 38: _t->menuExecBuffer(); break;
        case 39: { int _r = _t->getMarkerNum();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 40: _t->sortNeuronSegmentByType((*reinterpret_cast< QTableWidgetItem*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData V3dr_surfaceDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject V3dr_surfaceDialog::staticMetaObject = {
    { &SharedToolDialog::staticMetaObject, qt_meta_stringdata_V3dr_surfaceDialog,
      qt_meta_data_V3dr_surfaceDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &V3dr_surfaceDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *V3dr_surfaceDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *V3dr_surfaceDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_V3dr_surfaceDialog))
        return static_cast<void*>(const_cast< V3dr_surfaceDialog*>(this));
    return SharedToolDialog::qt_metacast(_clname);
}

int V3dr_surfaceDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = SharedToolDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 41)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 41;
    }
    return _id;
}
QT_END_MOC_NAMESPACE

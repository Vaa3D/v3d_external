/****************************************************************************
** Meta object code from reading C++ file 'Mozak3DView.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../mozak/Mozak3DView.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Mozak3DView.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_mozak__Mozak3DView[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      30,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      24,   20,   19,   19, 0x0a,
      44,   19,   19,   19, 0x0a,
      64,   19,   19,   19, 0x0a,
      84,   19,   19,   19, 0x0a,
     115,  107,   19,   19, 0x0a,
     146,  107,   19,   19, 0x0a,
     173,  107,   19,   19, 0x0a,
     199,  107,   19,   19, 0x0a,
     223,  107,   19,   19, 0x0a,
     251,  107,   19,   19, 0x0a,
     284,  107,   19,   19, 0x0a,
     318,  107,   19,   19, 0x0a,
     350,  107,   19,   19, 0x0a,
     384,  107,   19,   19, 0x0a,
     409,  107,   19,   19, 0x0a,
     444,  107,   19,   19, 0x0a,
     487,  480,   19,   19, 0x0a,
     515,   19,   19,   19, 0x0a,
     536,  528,   19,   19, 0x0a,
     552,  549,   19,   19, 0x0a,
     573,   19,   19,   19, 0x0a,
     601,  599,  590,   19, 0x0a,
     628,   19,   19,   19, 0x0a,
     647,   19,   19,   19, 0x0a,
     679,  669,   19,   19, 0x0a,
     761,  700,   19,   19, 0x0a,
     936,  880,   19,   19, 0x2a,
    1100, 1051,   19,   19, 0x2a,
    1243, 1207,   19,   19, 0x2a,
    1376, 1343,   19,   19, 0x2a,

       0        // eod
};

static const char qt_meta_stringdata_mozak__Mozak3DView[] = {
    "mozak::Mozak3DView\0\0con\0updateContrast(int)\0"
    "buttonUndoClicked()\0buttonRedoClicked()\0"
    "buttonOptionsClicked()\0checked\0"
    "invertImageButtonToggled(bool)\0"
    "connectButtonToggled(bool)\0"
    "extendButtonToggled(bool)\0"
    "joinButtonToggled(bool)\0"
    "polyLineButtonToggled(bool)\0"
    "polyLineAutoZButtonToggled(bool)\0"
    "retypeSegmentsButtonToggled(bool)\0"
    "splitSegmentButtonToggled(bool)\0"
    "deleteSegmentsButtonToggled(bool)\0"
    "zLockButtonClicked(bool)\0"
    "overviewMonitorButtonClicked(bool)\0"
    "highlightSubtreeButtonClicked(bool)\0"
    "ignore\0setZSurfaceLimitValues(int)\0"
    "updateGrid()\0spacing\0setGrid(int)\0zr\0"
    "updateZoomLabel(int)\0paintTimerCall()\0"
    "GLdouble\0i\0wriggleDegreeFunction(int)\0"
    "wriggleTimerCall()\0overviewSyncOneShot()\0"
    "colorMode\0updateColorMode(int)\0"
    "data,data_s,data_c,dest,finished,ex,elapsed_time,op_dsc,step\0"
    "receiveData(itm::uint8*,itm::integer_array,itm::integer_array,QWidget*"
    ",bool,itm::RuntimeException*,qint64,QString,int)\0"
    "data,data_s,data_c,dest,finished,ex,elapsed_time,op_dsc\0"
    "receiveData(itm::uint8*,itm::integer_array,itm::integer_array,QWidget*"
    ",bool,itm::RuntimeException*,qint64,QString)\0"
    "data,data_s,data_c,dest,finished,ex,elapsed_time\0"
    "receiveData(itm::uint8*,itm::integer_array,itm::integer_array,QWidget*"
    ",bool,itm::RuntimeException*,qint64)\0"
    "data,data_s,data_c,dest,finished,ex\0"
    "receiveData(itm::uint8*,itm::integer_array,itm::integer_array,QWidget*"
    ",bool,itm::RuntimeException*)\0"
    "data,data_s,data_c,dest,finished\0"
    "receiveData(itm::uint8*,itm::integer_array,itm::integer_array,QWidget*"
    ",bool)\0"
};

void mozak::Mozak3DView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        Mozak3DView *_t = static_cast<Mozak3DView *>(_o);
        switch (_id) {
        case 0: _t->updateContrast((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->buttonUndoClicked(); break;
        case 2: _t->buttonRedoClicked(); break;
        case 3: _t->buttonOptionsClicked(); break;
        case 4: _t->invertImageButtonToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->connectButtonToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 6: _t->extendButtonToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 7: _t->joinButtonToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 8: _t->polyLineButtonToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 9: _t->polyLineAutoZButtonToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 10: _t->retypeSegmentsButtonToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 11: _t->splitSegmentButtonToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 12: _t->deleteSegmentsButtonToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 13: _t->zLockButtonClicked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 14: _t->overviewMonitorButtonClicked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 15: _t->highlightSubtreeButtonClicked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 16: _t->setZSurfaceLimitValues((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 17: _t->updateGrid(); break;
        case 18: _t->setGrid((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 19: _t->updateZoomLabel((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 20: _t->paintTimerCall(); break;
        case 21: { GLdouble _r = _t->wriggleDegreeFunction((*reinterpret_cast< int(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< GLdouble*>(_a[0]) = _r; }  break;
        case 22: _t->wriggleTimerCall(); break;
        case 23: _t->overviewSyncOneShot(); break;
        case 24: _t->updateColorMode((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 25: _t->receiveData((*reinterpret_cast< itm::uint8*(*)>(_a[1])),(*reinterpret_cast< itm::integer_array(*)>(_a[2])),(*reinterpret_cast< itm::integer_array(*)>(_a[3])),(*reinterpret_cast< QWidget*(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5])),(*reinterpret_cast< itm::RuntimeException*(*)>(_a[6])),(*reinterpret_cast< qint64(*)>(_a[7])),(*reinterpret_cast< QString(*)>(_a[8])),(*reinterpret_cast< int(*)>(_a[9]))); break;
        case 26: _t->receiveData((*reinterpret_cast< itm::uint8*(*)>(_a[1])),(*reinterpret_cast< itm::integer_array(*)>(_a[2])),(*reinterpret_cast< itm::integer_array(*)>(_a[3])),(*reinterpret_cast< QWidget*(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5])),(*reinterpret_cast< itm::RuntimeException*(*)>(_a[6])),(*reinterpret_cast< qint64(*)>(_a[7])),(*reinterpret_cast< QString(*)>(_a[8]))); break;
        case 27: _t->receiveData((*reinterpret_cast< itm::uint8*(*)>(_a[1])),(*reinterpret_cast< itm::integer_array(*)>(_a[2])),(*reinterpret_cast< itm::integer_array(*)>(_a[3])),(*reinterpret_cast< QWidget*(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5])),(*reinterpret_cast< itm::RuntimeException*(*)>(_a[6])),(*reinterpret_cast< qint64(*)>(_a[7]))); break;
        case 28: _t->receiveData((*reinterpret_cast< itm::uint8*(*)>(_a[1])),(*reinterpret_cast< itm::integer_array(*)>(_a[2])),(*reinterpret_cast< itm::integer_array(*)>(_a[3])),(*reinterpret_cast< QWidget*(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5])),(*reinterpret_cast< itm::RuntimeException*(*)>(_a[6]))); break;
        case 29: _t->receiveData((*reinterpret_cast< itm::uint8*(*)>(_a[1])),(*reinterpret_cast< itm::integer_array(*)>(_a[2])),(*reinterpret_cast< itm::integer_array(*)>(_a[3])),(*reinterpret_cast< QWidget*(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData mozak::Mozak3DView::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject mozak::Mozak3DView::staticMetaObject = {
    { &teramanager::CViewer::staticMetaObject, qt_meta_stringdata_mozak__Mozak3DView,
      qt_meta_data_mozak__Mozak3DView, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &mozak::Mozak3DView::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *mozak::Mozak3DView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *mozak::Mozak3DView::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_mozak__Mozak3DView))
        return static_cast<void*>(const_cast< Mozak3DView*>(this));
    typedef teramanager::CViewer QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int mozak::Mozak3DView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef teramanager::CViewer QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 30)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 30;
    }
    return _id;
}
QT_END_MOC_NAMESPACE

/****************************************************************************
** Meta object code from reading C++ file 'CViewer.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../terafly/src/control/CViewer.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CViewer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_terafly__CViewer[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      27,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      79,   18,   17,   17, 0x0a,
     250,  194,   17,   17, 0x2a,
     410,  361,   17,   17, 0x2a,
     549,  513,   17,   17, 0x2a,
     678,  645,   17,   17, 0x2a,
     754,  752,   17,   17, 0x0a,
     777,  752,   17,   17, 0x0a,
     800,  752,   17,   17, 0x0a,
     823,  752,   17,   17, 0x0a,
     846,  752,   17,   17, 0x0a,
     869,  752,   17,   17, 0x0a,
     910,  892,   17,   17, 0x0a,
     940,  752,   17,   17, 0x2a,
     975,  965,   17,   17, 0x0a,
    1004,  752,   17,   17, 0x0a,
    1028,  752,   17,   17, 0x0a,
    1052,  752,   17,   17, 0x0a,
    1076,  752,   17,   17, 0x0a,
    1100,  752,   17,   17, 0x0a,
    1124,  752,   17,   17, 0x0a,
    1148,  752,   17,   17, 0x0a,
    1175,   17,   17,   17, 0x0a,
    1201, 1199,   17,   17, 0x0a,
    1214,   17,   17,   17, 0x0a,
    1232, 1226,   17,   17, 0x0a,
    1258,   17,   17,   17, 0x0a,
    1284,   17,   17,   17, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_terafly__CViewer[] = {
    "terafly::CViewer\0\0"
    "data,data_s,data_c,dest,finished,ex,elapsed_time,op_dsc,step\0"
    "receiveData(tf::uint8*,tf::integer_array,tf::integer_array,QWidget*,bo"
    "ol,tf::RuntimeException*,qint64,QString,int)\0"
    "data,data_s,data_c,dest,finished,ex,elapsed_time,op_dsc\0"
    "receiveData(tf::uint8*,tf::integer_array,tf::integer_array,QWidget*,bo"
    "ol,tf::RuntimeException*,qint64,QString)\0"
    "data,data_s,data_c,dest,finished,ex,elapsed_time\0"
    "receiveData(tf::uint8*,tf::integer_array,tf::integer_array,QWidget*,bo"
    "ol,tf::RuntimeException*,qint64)\0"
    "data,data_s,data_c,dest,finished,ex\0"
    "receiveData(tf::uint8*,tf::integer_array,tf::integer_array,QWidget*,bo"
    "ol,tf::RuntimeException*)\0"
    "data,data_s,data_c,dest,finished\0"
    "receiveData(tf::uint8*,tf::integer_array,tf::integer_array,QWidget*,bo"
    "ol)\0"
    "s\0Vaa3D_changeXCut0(int)\0"
    "Vaa3D_changeXCut1(int)\0Vaa3D_changeYCut0(int)\0"
    "Vaa3D_changeYCut1(int)\0Vaa3D_changeZCut0(int)\0"
    "Vaa3D_changeZCut1(int)\0s,editingFinished\0"
    "Vaa3D_changeTSlider(int,bool)\0"
    "Vaa3D_changeTSlider(int)\0direction\0"
    "ShiftToAnotherDirection(int)\0"
    "PMain_changeV0sbox(int)\0PMain_changeV1sbox(int)\0"
    "PMain_changeH0sbox(int)\0PMain_changeH1sbox(int)\0"
    "PMain_changeD0sbox(int)\0PMain_changeD1sbox(int)\0"
    "Vaa3D_rotationchanged(int)\0"
    "PMain_rotationchanged()\0z\0setZoom(int)\0"
    "translate()\0value\0zoomOutMethodChanged(int)\0"
    "inSituZoomOutTranslated()\0resetEvents()\0"
};

void terafly::CViewer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        CViewer *_t = static_cast<CViewer *>(_o);
        switch (_id) {
        case 0: _t->receiveData((*reinterpret_cast< tf::uint8*(*)>(_a[1])),(*reinterpret_cast< tf::integer_array(*)>(_a[2])),(*reinterpret_cast< tf::integer_array(*)>(_a[3])),(*reinterpret_cast< QWidget*(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5])),(*reinterpret_cast< tf::RuntimeException*(*)>(_a[6])),(*reinterpret_cast< qint64(*)>(_a[7])),(*reinterpret_cast< QString(*)>(_a[8])),(*reinterpret_cast< int(*)>(_a[9]))); break;
        case 1: _t->receiveData((*reinterpret_cast< tf::uint8*(*)>(_a[1])),(*reinterpret_cast< tf::integer_array(*)>(_a[2])),(*reinterpret_cast< tf::integer_array(*)>(_a[3])),(*reinterpret_cast< QWidget*(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5])),(*reinterpret_cast< tf::RuntimeException*(*)>(_a[6])),(*reinterpret_cast< qint64(*)>(_a[7])),(*reinterpret_cast< QString(*)>(_a[8]))); break;
        case 2: _t->receiveData((*reinterpret_cast< tf::uint8*(*)>(_a[1])),(*reinterpret_cast< tf::integer_array(*)>(_a[2])),(*reinterpret_cast< tf::integer_array(*)>(_a[3])),(*reinterpret_cast< QWidget*(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5])),(*reinterpret_cast< tf::RuntimeException*(*)>(_a[6])),(*reinterpret_cast< qint64(*)>(_a[7]))); break;
        case 3: _t->receiveData((*reinterpret_cast< tf::uint8*(*)>(_a[1])),(*reinterpret_cast< tf::integer_array(*)>(_a[2])),(*reinterpret_cast< tf::integer_array(*)>(_a[3])),(*reinterpret_cast< QWidget*(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5])),(*reinterpret_cast< tf::RuntimeException*(*)>(_a[6]))); break;
        case 4: _t->receiveData((*reinterpret_cast< tf::uint8*(*)>(_a[1])),(*reinterpret_cast< tf::integer_array(*)>(_a[2])),(*reinterpret_cast< tf::integer_array(*)>(_a[3])),(*reinterpret_cast< QWidget*(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5]))); break;
        case 5: _t->Vaa3D_changeXCut0((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->Vaa3D_changeXCut1((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->Vaa3D_changeYCut0((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->Vaa3D_changeYCut1((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->Vaa3D_changeZCut0((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->Vaa3D_changeZCut1((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: _t->Vaa3D_changeTSlider((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 12: _t->Vaa3D_changeTSlider((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: _t->ShiftToAnotherDirection((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 14: _t->PMain_changeV0sbox((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 15: _t->PMain_changeV1sbox((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 16: _t->PMain_changeH0sbox((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 17: _t->PMain_changeH1sbox((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 18: _t->PMain_changeD0sbox((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 19: _t->PMain_changeD1sbox((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 20: _t->Vaa3D_rotationchanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 21: _t->PMain_rotationchanged(); break;
        case 22: _t->setZoom((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 23: _t->translate(); break;
        case 24: _t->zoomOutMethodChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 25: _t->inSituZoomOutTranslated(); break;
        case 26: _t->resetEvents(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData terafly::CViewer::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject terafly::CViewer::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_terafly__CViewer,
      qt_meta_data_terafly__CViewer, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &terafly::CViewer::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *terafly::CViewer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *terafly::CViewer::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_terafly__CViewer))
        return static_cast<void*>(const_cast< CViewer*>(this));
    return QWidget::qt_metacast(_clname);
}

int terafly::CViewer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 27)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 27;
    }
    return _id;
}
QT_END_MOC_NAMESPACE

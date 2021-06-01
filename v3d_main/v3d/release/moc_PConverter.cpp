/****************************************************************************
** Meta object code from reading C++ file 'PConverter.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../terafly/src/presentation/PConverter.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PConverter.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_terafly__PConverter[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      21,   20,   20,   20, 0x0a,
      42,   20,   20,   20, 0x0a,
      62,   20,   20,   20, 0x0a,
      83,   20,   20,   20, 0x0a,
     105,   20,   20,   20, 0x0a,
     128,   20,   20,   20, 0x0a,
     150,   20,   20,   20, 0x0a,
     172,   20,   20,   20, 0x0a,
     216,  188,   20,   20, 0x0a,
     263,  260,   20,   20, 0x0a,
     300,   20,   20,   20, 0x0a,
     318,   20,   20,   20, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_terafly__PConverter[] = {
    "terafly::PConverter\0\0startButtonClicked()\0"
    "stopButtonClicked()\0inDirButtonClicked()\0"
    "inFileButtonClicked()\0outFileButtonClicked()\0"
    "outDirButtonClicked()\0volformatChanged(int)\0"
    "addResolution()\0val,minutes,seconds,message\0"
    "progressBarChanged(int,int,int,std::string)\0"
    "ex\0operationDone(tf::RuntimeException*)\0"
    "settingsChanged()\0updateContent()\0"
};

void terafly::PConverter::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        PConverter *_t = static_cast<PConverter *>(_o);
        switch (_id) {
        case 0: _t->startButtonClicked(); break;
        case 1: _t->stopButtonClicked(); break;
        case 2: _t->inDirButtonClicked(); break;
        case 3: _t->inFileButtonClicked(); break;
        case 4: _t->outFileButtonClicked(); break;
        case 5: _t->outDirButtonClicked(); break;
        case 6: _t->volformatChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->addResolution(); break;
        case 8: _t->progressBarChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< std::string(*)>(_a[4]))); break;
        case 9: _t->operationDone((*reinterpret_cast< tf::RuntimeException*(*)>(_a[1]))); break;
        case 10: _t->settingsChanged(); break;
        case 11: _t->updateContent(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData terafly::PConverter::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject terafly::PConverter::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_terafly__PConverter,
      qt_meta_data_terafly__PConverter, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &terafly::PConverter::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *terafly::PConverter::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *terafly::PConverter::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_terafly__PConverter))
        return static_cast<void*>(const_cast< PConverter*>(this));
    return QWidget::qt_metacast(_clname);
}

int terafly::PConverter::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    }
    return _id;
}
QT_END_MOC_NAMESPACE

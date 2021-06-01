/****************************************************************************
** Meta object code from reading C++ file 'm_PConverter.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../mozak/m_terafly/src/presentation/m_PConverter.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'm_PConverter.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_teramanager__PConverter[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      53,   25,   24,   24, 0x05,

 // slots: signature, parameters, type, tag, flags
     101,   24,   24,   24, 0x0a,
     122,   24,   24,   24, 0x0a,
     142,   24,   24,   24, 0x0a,
     163,   24,   24,   24, 0x0a,
     185,   24,   24,   24, 0x0a,
     208,   24,   24,   24, 0x0a,
     230,   24,   24,   24, 0x0a,
     252,   24,   24,   24, 0x0a,
     268,   25,   24,   24, 0x0a,
     315,  312,   24,   24, 0x0a,
     353,   24,   24,   24, 0x0a,
     371,   24,   24,   24, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_teramanager__PConverter[] = {
    "teramanager::PConverter\0\0"
    "val,minutes,seconds,message\0"
    "sendProgressBarChanged(int,int,int,const char*)\0"
    "startButtonClicked()\0stopButtonClicked()\0"
    "inDirButtonClicked()\0inFileButtonClicked()\0"
    "outFileButtonClicked()\0outDirButtonClicked()\0"
    "volformatChanged(int)\0addResolution()\0"
    "progressBarChanged(int,int,int,const char*)\0"
    "ex\0operationDone(itm::RuntimeException*)\0"
    "settingsChanged()\0updateContent()\0"
};

void teramanager::PConverter::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        PConverter *_t = static_cast<PConverter *>(_o);
        switch (_id) {
        case 0: _t->sendProgressBarChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< const char*(*)>(_a[4]))); break;
        case 1: _t->startButtonClicked(); break;
        case 2: _t->stopButtonClicked(); break;
        case 3: _t->inDirButtonClicked(); break;
        case 4: _t->inFileButtonClicked(); break;
        case 5: _t->outFileButtonClicked(); break;
        case 6: _t->outDirButtonClicked(); break;
        case 7: _t->volformatChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->addResolution(); break;
        case 9: _t->progressBarChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< const char*(*)>(_a[4]))); break;
        case 10: _t->operationDone((*reinterpret_cast< itm::RuntimeException*(*)>(_a[1]))); break;
        case 11: _t->settingsChanged(); break;
        case 12: _t->updateContent(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData teramanager::PConverter::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject teramanager::PConverter::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_teramanager__PConverter,
      qt_meta_data_teramanager__PConverter, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &teramanager::PConverter::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *teramanager::PConverter::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *teramanager::PConverter::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_teramanager__PConverter))
        return static_cast<void*>(const_cast< PConverter*>(this));
    return QWidget::qt_metacast(_clname);
}

int teramanager::PConverter::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    }
    return _id;
}

// SIGNAL 0
void teramanager::PConverter::sendProgressBarChanged(int _t1, int _t2, int _t3, const char * _t4)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE

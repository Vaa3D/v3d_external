/****************************************************************************
** Meta object code from reading C++ file 'm_PLog.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../mozak/m_terafly/src/presentation/m_PLog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'm_PLog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_teramanager__PLog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      22,   19,   18,   18, 0x05,

 // slots: signature, parameters, type, tag, flags
      40,   19,   18,   18, 0x0a,
      69,   67,   18,   18, 0x0a,
     112,   67,   18,   18, 0x0a,
     143,   67,   18,   18, 0x0a,
     170,   18,   18,   18, 0x0a,
     196,   18,   18,   18, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_teramanager__PLog[] = {
    "teramanager::PLog\0\0op\0sendAppend(void*)\0"
    "appendOperationVoid(void*)\0s\0"
    "enableIoCoreOperationsCheckBoxChanged(int)\0"
    "autoUpdateCheckBoxChanged(int)\0"
    "appendCheckBoxChanged(int)\0"
    "updatePushButtonClicked()\0reset()\0"
};

void teramanager::PLog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        PLog *_t = static_cast<PLog *>(_o);
        switch (_id) {
        case 0: _t->sendAppend((*reinterpret_cast< void*(*)>(_a[1]))); break;
        case 1: _t->appendOperationVoid((*reinterpret_cast< void*(*)>(_a[1]))); break;
        case 2: _t->enableIoCoreOperationsCheckBoxChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->autoUpdateCheckBoxChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->appendCheckBoxChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->updatePushButtonClicked(); break;
        case 6: _t->reset(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData teramanager::PLog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject teramanager::PLog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_teramanager__PLog,
      qt_meta_data_teramanager__PLog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &teramanager::PLog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *teramanager::PLog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *teramanager::PLog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_teramanager__PLog))
        return static_cast<void*>(const_cast< PLog*>(this));
    return QDialog::qt_metacast(_clname);
}

int teramanager::PLog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void teramanager::PLog::sendAppend(void * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE

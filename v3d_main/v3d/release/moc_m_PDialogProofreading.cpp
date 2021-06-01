/****************************************************************************
** Meta object code from reading C++ file 'm_PDialogProofreading.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../mozak/m_terafly/src/presentation/m_PDialogProofreading.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'm_PDialogProofreading.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_teramanager__PDialogProofreading[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      34,   33,   33,   33, 0x0a,
      55,   33,   33,   33, 0x0a,
      81,   33,   33,   33, 0x0a,
     112,  104,   99,   33, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_teramanager__PDialogProofreading[] = {
    "teramanager::PDialogProofreading\0\0"
    "startButtonClicked()\0showBlocksButtonClicked()\0"
    "updateBlocks(int)\0bool\0obj,evt\0"
    "eventFilter(QObject*,QEvent*)\0"
};

void teramanager::PDialogProofreading::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        PDialogProofreading *_t = static_cast<PDialogProofreading *>(_o);
        switch (_id) {
        case 0: _t->startButtonClicked(); break;
        case 1: _t->showBlocksButtonClicked(); break;
        case 2: _t->updateBlocks((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: { bool _r = _t->eventFilter((*reinterpret_cast< QObject*(*)>(_a[1])),(*reinterpret_cast< QEvent*(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        default: ;
        }
    }
}

const QMetaObjectExtraData teramanager::PDialogProofreading::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject teramanager::PDialogProofreading::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_teramanager__PDialogProofreading,
      qt_meta_data_teramanager__PDialogProofreading, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &teramanager::PDialogProofreading::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *teramanager::PDialogProofreading::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *teramanager::PDialogProofreading::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_teramanager__PDialogProofreading))
        return static_cast<void*>(const_cast< PDialogProofreading*>(this));
    return QWidget::qt_metacast(_clname);
}

int teramanager::PDialogProofreading::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}
QT_END_MOC_NAMESPACE

/****************************************************************************
** Meta object code from reading C++ file 'StagedFileLoader.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../neuron_annotator/data_model/StagedFileLoader.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'StagedFileLoader.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ProgressiveLoader[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      19,   18,   18,   18, 0x05,

 // slots: signature, parameters, type, tag, flags
      45,   37,   18,   18, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_ProgressiveLoader[] = {
    "ProgressiveLoader\0\0newFoldersFound()\0"
    "dirName\0reexamineResultDirectory(QString)\0"
};

void ProgressiveLoader::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ProgressiveLoader *_t = static_cast<ProgressiveLoader *>(_o);
        switch (_id) {
        case 0: _t->newFoldersFound(); break;
        case 1: _t->reexamineResultDirectory((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData ProgressiveLoader::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ProgressiveLoader::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_ProgressiveLoader,
      qt_meta_data_ProgressiveLoader, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ProgressiveLoader::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ProgressiveLoader::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ProgressiveLoader::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ProgressiveLoader))
        return static_cast<void*>(const_cast< ProgressiveLoader*>(this));
    if (!strcmp(_clname, "QList<ProgressiveLoadItem*>"))
        return static_cast< QList<ProgressiveLoadItem*>*>(const_cast< ProgressiveLoader*>(this));
    return QObject::qt_metacast(_clname);
}

int ProgressiveLoader::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void ProgressiveLoader::newFoldersFound()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE

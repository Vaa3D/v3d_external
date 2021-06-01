/****************************************************************************
** Meta object code from reading C++ file 'DataThread.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../neuron_annotator/utility/DataThread.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DataThread.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_DataThread[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      20,   12,   11,   11, 0x05,
      52,   44,   11,   11, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_DataThread[] = {
    "DataThread\0\0results\0gotResults(const void*)\0"
    "message\0gotError(QString)\0"
};

void DataThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DataThread *_t = static_cast<DataThread *>(_o);
        switch (_id) {
        case 0: _t->gotResults((*reinterpret_cast< const void*(*)>(_a[1]))); break;
        case 1: _t->gotError((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData DataThread::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject DataThread::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_DataThread,
      qt_meta_data_DataThread, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DataThread::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DataThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DataThread::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DataThread))
        return static_cast<void*>(const_cast< DataThread*>(this));
    return QThread::qt_metacast(_clname);
}

int DataThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
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
void DataThread::gotResults(const void * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void DataThread::gotError(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
static const uint qt_meta_data_GetOntologyThread[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_GetOntologyThread[] = {
    "GetOntologyThread\0"
};

void GetOntologyThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData GetOntologyThread::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject GetOntologyThread::staticMetaObject = {
    { &DataThread::staticMetaObject, qt_meta_stringdata_GetOntologyThread,
      qt_meta_data_GetOntologyThread, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &GetOntologyThread::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *GetOntologyThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *GetOntologyThread::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_GetOntologyThread))
        return static_cast<void*>(const_cast< GetOntologyThread*>(this));
    return DataThread::qt_metacast(_clname);
}

int GetOntologyThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DataThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_GetAnnotatedBranchThread[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_GetAnnotatedBranchThread[] = {
    "GetAnnotatedBranchThread\0"
};

void GetAnnotatedBranchThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData GetAnnotatedBranchThread::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject GetAnnotatedBranchThread::staticMetaObject = {
    { &DataThread::staticMetaObject, qt_meta_stringdata_GetAnnotatedBranchThread,
      qt_meta_data_GetAnnotatedBranchThread, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &GetAnnotatedBranchThread::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *GetAnnotatedBranchThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *GetAnnotatedBranchThread::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_GetAnnotatedBranchThread))
        return static_cast<void*>(const_cast< GetAnnotatedBranchThread*>(this));
    return DataThread::qt_metacast(_clname);
}

int GetAnnotatedBranchThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DataThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_GetEntityThread[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_GetEntityThread[] = {
    "GetEntityThread\0"
};

void GetEntityThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData GetEntityThread::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject GetEntityThread::staticMetaObject = {
    { &DataThread::staticMetaObject, qt_meta_stringdata_GetEntityThread,
      qt_meta_data_GetEntityThread, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &GetEntityThread::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *GetEntityThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *GetEntityThread::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_GetEntityThread))
        return static_cast<void*>(const_cast< GetEntityThread*>(this));
    return DataThread::qt_metacast(_clname);
}

int GetEntityThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DataThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_GetParentsThread[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_GetParentsThread[] = {
    "GetParentsThread\0"
};

void GetParentsThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData GetParentsThread::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject GetParentsThread::staticMetaObject = {
    { &DataThread::staticMetaObject, qt_meta_stringdata_GetParentsThread,
      qt_meta_data_GetParentsThread, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &GetParentsThread::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *GetParentsThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *GetParentsThread::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_GetParentsThread))
        return static_cast<void*>(const_cast< GetParentsThread*>(this));
    return DataThread::qt_metacast(_clname);
}

int GetParentsThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DataThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_GetAncestorThread[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_GetAncestorThread[] = {
    "GetAncestorThread\0"
};

void GetAncestorThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData GetAncestorThread::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject GetAncestorThread::staticMetaObject = {
    { &DataThread::staticMetaObject, qt_meta_stringdata_GetAncestorThread,
      qt_meta_data_GetAncestorThread, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &GetAncestorThread::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *GetAncestorThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *GetAncestorThread::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_GetAncestorThread))
        return static_cast<void*>(const_cast< GetAncestorThread*>(this));
    return DataThread::qt_metacast(_clname);
}

int GetAncestorThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DataThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_GetEntityAnnotationsThread[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_GetEntityAnnotationsThread[] = {
    "GetEntityAnnotationsThread\0"
};

void GetEntityAnnotationsThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData GetEntityAnnotationsThread::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject GetEntityAnnotationsThread::staticMetaObject = {
    { &DataThread::staticMetaObject, qt_meta_stringdata_GetEntityAnnotationsThread,
      qt_meta_data_GetEntityAnnotationsThread, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &GetEntityAnnotationsThread::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *GetEntityAnnotationsThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *GetEntityAnnotationsThread::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_GetEntityAnnotationsThread))
        return static_cast<void*>(const_cast< GetEntityAnnotationsThread*>(this));
    return DataThread::qt_metacast(_clname);
}

int GetEntityAnnotationsThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DataThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_CreateAnnotationThread[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_CreateAnnotationThread[] = {
    "CreateAnnotationThread\0"
};

void CreateAnnotationThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData CreateAnnotationThread::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject CreateAnnotationThread::staticMetaObject = {
    { &DataThread::staticMetaObject, qt_meta_stringdata_CreateAnnotationThread,
      qt_meta_data_CreateAnnotationThread, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &CreateAnnotationThread::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *CreateAnnotationThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *CreateAnnotationThread::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CreateAnnotationThread))
        return static_cast<void*>(const_cast< CreateAnnotationThread*>(this));
    return DataThread::qt_metacast(_clname);
}

int CreateAnnotationThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DataThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_RemoveAnnotationThread[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_RemoveAnnotationThread[] = {
    "RemoveAnnotationThread\0"
};

void RemoveAnnotationThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData RemoveAnnotationThread::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject RemoveAnnotationThread::staticMetaObject = {
    { &DataThread::staticMetaObject, qt_meta_stringdata_RemoveAnnotationThread,
      qt_meta_data_RemoveAnnotationThread, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &RemoveAnnotationThread::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *RemoveAnnotationThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *RemoveAnnotationThread::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_RemoveAnnotationThread))
        return static_cast<void*>(const_cast< RemoveAnnotationThread*>(this));
    return DataThread::qt_metacast(_clname);
}

int RemoveAnnotationThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DataThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_GetAnnotationSessionThread[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_GetAnnotationSessionThread[] = {
    "GetAnnotationSessionThread\0"
};

void GetAnnotationSessionThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData GetAnnotationSessionThread::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject GetAnnotationSessionThread::staticMetaObject = {
    { &DataThread::staticMetaObject, qt_meta_stringdata_GetAnnotationSessionThread,
      qt_meta_data_GetAnnotationSessionThread, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &GetAnnotationSessionThread::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *GetAnnotationSessionThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *GetAnnotationSessionThread::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_GetAnnotationSessionThread))
        return static_cast<void*>(const_cast< GetAnnotationSessionThread*>(this));
    return DataThread::qt_metacast(_clname);
}

int GetAnnotationSessionThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DataThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_SelectEntityThread[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_SelectEntityThread[] = {
    "SelectEntityThread\0"
};

void SelectEntityThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData SelectEntityThread::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject SelectEntityThread::staticMetaObject = {
    { &DataThread::staticMetaObject, qt_meta_stringdata_SelectEntityThread,
      qt_meta_data_SelectEntityThread, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SelectEntityThread::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SelectEntityThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SelectEntityThread::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SelectEntityThread))
        return static_cast<void*>(const_cast< SelectEntityThread*>(this));
    return DataThread::qt_metacast(_clname);
}

int SelectEntityThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DataThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
QT_END_MOC_NAMESPACE

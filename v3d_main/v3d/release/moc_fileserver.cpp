/****************************************************************************
** Meta object code from reading C++ file 'fileserver.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../terafly/src/presentation/fileserver.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'fileserver.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_FileServer[] = {

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
      21,   12,   11,   11, 0x05,

 // slots: signature, parameters, type, tag, flags
      43,   11,   11,   11, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_FileServer[] = {
    "FileServer\0\0filename\0receivedfile(QString)\0"
    "Socketdisconnect(QString)\0"
};

void FileServer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        FileServer *_t = static_cast<FileServer *>(_o);
        switch (_id) {
        case 0: _t->receivedfile((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->Socketdisconnect((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData FileServer::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject FileServer::staticMetaObject = {
    { &QTcpServer::staticMetaObject, qt_meta_stringdata_FileServer,
      qt_meta_data_FileServer, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &FileServer::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *FileServer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *FileServer::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_FileServer))
        return static_cast<void*>(const_cast< FileServer*>(this));
    return QTcpServer::qt_metacast(_clname);
}

int FileServer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTcpServer::qt_metacall(_c, _id, _a);
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
void FileServer::receivedfile(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_FileSocket_receive[] = {

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
      29,   20,   19,   19, 0x05,

 // slots: signature, parameters, type, tag, flags
      50,   19,   19,   19, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_FileSocket_receive[] = {
    "FileSocket_receive\0\0filename\0"
    "receivefile(QString)\0readFile()\0"
};

void FileSocket_receive::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        FileSocket_receive *_t = static_cast<FileSocket_receive *>(_o);
        switch (_id) {
        case 0: _t->receivefile((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->readFile(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData FileSocket_receive::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject FileSocket_receive::staticMetaObject = {
    { &QTcpSocket::staticMetaObject, qt_meta_stringdata_FileSocket_receive,
      qt_meta_data_FileSocket_receive, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &FileSocket_receive::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *FileSocket_receive::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *FileSocket_receive::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_FileSocket_receive))
        return static_cast<void*>(const_cast< FileSocket_receive*>(this));
    return QTcpSocket::qt_metacast(_clname);
}

int FileSocket_receive::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTcpSocket::qt_metacall(_c, _id, _a);
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
void FileSocket_receive::receivefile(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE

/****************************************************************************
** Meta object code from reading C++ file 'V3dR_Communicator.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../vrrenderer/V3dR_Communicator.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'V3dR_Communicator.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_FileSocket_send[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      17,   16,   16,   16, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_FileSocket_send[] = {
    "FileSocket_send\0\0readMSG()\0"
};

void FileSocket_send::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        FileSocket_send *_t = static_cast<FileSocket_send *>(_o);
        switch (_id) {
        case 0: _t->readMSG(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData FileSocket_send::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject FileSocket_send::staticMetaObject = {
    { &QTcpSocket::staticMetaObject, qt_meta_stringdata_FileSocket_send,
      qt_meta_data_FileSocket_send, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &FileSocket_send::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *FileSocket_send::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *FileSocket_send::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_FileSocket_send))
        return static_cast<void*>(const_cast< FileSocket_send*>(this));
    return QTcpSocket::qt_metacast(_clname);
}

int FileSocket_send::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTcpSocket::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
static const uint qt_meta_data_ManageSocket[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      31,   14,   13,   13, 0x05,
      74,   13,   13,   13, 0x05,

 // slots: signature, parameters, type, tag, flags
      91,   13,   13,   13, 0x0a,
     105,   13,   13,   13, 0x0a,
     129,   13,   13,   13, 0x0a,
     153,   13,   13,   13, 0x0a,
     175,  167,   13,   13, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_ManageSocket[] = {
    "ManageSocket\0\0ip,port,username\0"
    "makeMessageSocket(QString,QString,QString)\0"
    "loadANO(QString)\0onReadyRead()\0"
    "send1(QListWidgetItem*)\0send2(QListWidgetItem*)\0"
    "messageMade()\0anofile\0receivefile(QString)\0"
};

void ManageSocket::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ManageSocket *_t = static_cast<ManageSocket *>(_o);
        switch (_id) {
        case 0: _t->makeMessageSocket((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 1: _t->loadANO((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->onReadyRead(); break;
        case 3: _t->send1((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 4: _t->send2((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 5: _t->messageMade(); break;
        case 6: _t->receivefile((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData ManageSocket::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ManageSocket::staticMetaObject = {
    { &QTcpSocket::staticMetaObject, qt_meta_stringdata_ManageSocket,
      qt_meta_data_ManageSocket, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ManageSocket::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ManageSocket::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ManageSocket::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ManageSocket))
        return static_cast<void*>(const_cast< ManageSocket*>(this));
    return QTcpSocket::qt_metacast(_clname);
}

int ManageSocket::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTcpSocket::qt_metacall(_c, _id, _a);
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
void ManageSocket::makeMessageSocket(QString _t1, QString _t2, QString _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ManageSocket::loadANO(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
static const uint qt_meta_data_V3dR_Communicator[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      19,   18,   18,   18, 0x05,

 // slots: signature, parameters, type, tag, flags
      55,   38,   33,   18, 0x0a,
      97,   18,   18,   18, 0x0a,
     121,   18,   18,   18, 0x08,
     135,   18,   18,   18, 0x08,
     149,   18,   18,   18, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_V3dR_Communicator[] = {
    "V3dR_Communicator\0\0messageMade()\0bool\0"
    "ip,port,username\0"
    "SendLoginRequest(QString,QString,QString)\0"
    "CollaborationMainloop()\0onReadyRead()\0"
    "onConnected()\0onDisconnected()\0"
};

void V3dR_Communicator::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        V3dR_Communicator *_t = static_cast<V3dR_Communicator *>(_o);
        switch (_id) {
        case 0: _t->messageMade(); break;
        case 1: { bool _r = _t->SendLoginRequest((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 2: _t->CollaborationMainloop(); break;
        case 3: _t->onReadyRead(); break;
        case 4: _t->onConnected(); break;
        case 5: _t->onDisconnected(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData V3dR_Communicator::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject V3dR_Communicator::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_V3dR_Communicator,
      qt_meta_data_V3dR_Communicator, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &V3dR_Communicator::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *V3dR_Communicator::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *V3dR_Communicator::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_V3dR_Communicator))
        return static_cast<void*>(const_cast< V3dR_Communicator*>(this));
    return QWidget::qt_metacast(_clname);
}

int V3dR_Communicator::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void V3dR_Communicator::messageMade()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE

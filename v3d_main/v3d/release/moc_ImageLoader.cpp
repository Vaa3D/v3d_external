/****************************************************************************
** Meta object code from reading C++ file 'ImageLoader.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../neuron_annotator/utility/ImageLoader.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ImageLoader.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ImageLoader[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: signature, parameters, type, tag, flags
      36,   13,   12,   12, 0x05,
      80,   66,   12,   12, 0x05,
     102,   66,   12,   12, 0x05,
     131,  123,   12,   12, 0x05,
     163,   12,   12,   12, 0x05,

 // slots: signature, parameters, type, tag, flags
     174,   12,   12,   12, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_ImageLoader[] = {
    "ImageLoader\0\0progress,progressIndex\0"
    "progressValueChanged(int,int)\0"
    "progressIndex\0progressComplete(int)\0"
    "progressAborted(int)\0message\0"
    "progressMessageChanged(QString)\0"
    "canceled()\0cancel()\0"
};

void ImageLoader::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ImageLoader *_t = static_cast<ImageLoader *>(_o);
        switch (_id) {
        case 0: _t->progressValueChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->progressComplete((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->progressAborted((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->progressMessageChanged((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 4: _t->canceled(); break;
        case 5: _t->cancel(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData ImageLoader::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ImageLoader::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_ImageLoader,
      qt_meta_data_ImageLoader, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ImageLoader::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ImageLoader::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ImageLoader::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ImageLoader))
        return static_cast<void*>(const_cast< ImageLoader*>(this));
    if (!strcmp(_clname, "QRunnable"))
        return static_cast< QRunnable*>(const_cast< ImageLoader*>(this));
    if (!strcmp(_clname, "ImageLoaderBasic"))
        return static_cast< ImageLoaderBasic*>(const_cast< ImageLoader*>(this));
    return QObject::qt_metacast(_clname);
}

int ImageLoader::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
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
void ImageLoader::progressValueChanged(int _t1, int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ImageLoader::progressComplete(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void ImageLoader::progressAborted(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void ImageLoader::progressMessageChanged(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void ImageLoader::canceled()
{
    QMetaObject::activate(this, &staticMetaObject, 4, 0);
}
QT_END_MOC_NAMESPACE

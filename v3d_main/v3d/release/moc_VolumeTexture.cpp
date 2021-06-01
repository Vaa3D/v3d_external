/****************************************************************************
** Meta object code from reading C++ file 'VolumeTexture.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../neuron_annotator/data_model/VolumeTexture.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'VolumeTexture.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_jfrc__VolumeTexture[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      21,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      11,       // signalCount

 // signals: signature, parameters, type, tag, flags
      21,   20,   20,   20, 0x05,
      48,   20,   20,   20, 0x05,
      73,   20,   20,   20, 0x05,
      95,   20,   20,   20, 0x05,
     118,   20,   20,   20, 0x05,
     142,   20,   20,   20, 0x05,
     173,   20,   20,   20, 0x05,
     211,   20,   20,   20, 0x05,
     257,  241,   20,   20, 0x05,
     286,   20,   20,   20, 0x05,
     314,   20,   20,   20, 0x05,

 // slots: signature, parameters, type, tag, flags
     345,   20,  340,   20, 0x0a,
     360,   20,   20,   20, 0x0a,
     392,   20,  340,   20, 0x0a,
     416,   20,  340,   20, 0x0a,
     443,  435,   20,   20, 0x0a,
     468,  435,  340,   20, 0x0a,
     492,  435,  340,   20, 0x0a,
     519,   20,   20,   20, 0x0a,
     543,  539,  340,   20, 0x0a,
     571,   20,   20,   20, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_jfrc__VolumeTexture[] = {
    "jfrc::VolumeTexture\0\0visibilityTextureChanged()\0"
    "colorMapTextureChanged()\0labelTextureChanged()\0"
    "signalTextureChanged()\0signalMetadataChanged()\0"
    "benchmarkTimerResetRequested()\0"
    "benchmarkTimerPrintRequested(QString)\0"
    "volumeLoadSequenceCompleted()\0"
    "fileUrl,channel\0mpegQueueRequested(QUrl,int)\0"
    "mpegLoadSequenceRequested()\0"
    "loadNextVolumeRequested()\0bool\0"
    "updateVolume()\0updateNeuronVisibilityTexture()\0"
    "updateColorMapTexture()\0loadLabelPbdFile()\0"
    "fileUrl\0setLabelPbdFileUrl(QUrl)\0"
    "loadSignalRawFile(QUrl)\0"
    "loadReferenceRawFile(QUrl)\0"
    "loadStagedVolumes()\0url\0"
    "queueSeparationFolder(QUrl)\0"
    "queueVolumeData()\0"
};

void jfrc::VolumeTexture::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        VolumeTexture *_t = static_cast<VolumeTexture *>(_o);
        switch (_id) {
        case 0: _t->visibilityTextureChanged(); break;
        case 1: _t->colorMapTextureChanged(); break;
        case 2: _t->labelTextureChanged(); break;
        case 3: _t->signalTextureChanged(); break;
        case 4: _t->signalMetadataChanged(); break;
        case 5: _t->benchmarkTimerResetRequested(); break;
        case 6: _t->benchmarkTimerPrintRequested((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 7: _t->volumeLoadSequenceCompleted(); break;
        case 8: _t->mpegQueueRequested((*reinterpret_cast< QUrl(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 9: _t->mpegLoadSequenceRequested(); break;
        case 10: _t->loadNextVolumeRequested(); break;
        case 11: { bool _r = _t->updateVolume();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 12: _t->updateNeuronVisibilityTexture(); break;
        case 13: { bool _r = _t->updateColorMapTexture();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 14: { bool _r = _t->loadLabelPbdFile();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 15: _t->setLabelPbdFileUrl((*reinterpret_cast< QUrl(*)>(_a[1]))); break;
        case 16: { bool _r = _t->loadSignalRawFile((*reinterpret_cast< QUrl(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 17: { bool _r = _t->loadReferenceRawFile((*reinterpret_cast< QUrl(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 18: _t->loadStagedVolumes(); break;
        case 19: { bool _r = _t->queueSeparationFolder((*reinterpret_cast< QUrl(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 20: _t->queueVolumeData(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData jfrc::VolumeTexture::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject jfrc::VolumeTexture::staticMetaObject = {
    { &NaSharedDataModel<PrivateVolumeTexture>::staticMetaObject, qt_meta_stringdata_jfrc__VolumeTexture,
      qt_meta_data_jfrc__VolumeTexture, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &jfrc::VolumeTexture::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *jfrc::VolumeTexture::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *jfrc::VolumeTexture::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_jfrc__VolumeTexture))
        return static_cast<void*>(const_cast< VolumeTexture*>(this));
    return NaSharedDataModel<PrivateVolumeTexture>::qt_metacast(_clname);
}

int jfrc::VolumeTexture::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = NaSharedDataModel<PrivateVolumeTexture>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 21)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 21;
    }
    return _id;
}

// SIGNAL 0
void jfrc::VolumeTexture::visibilityTextureChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void jfrc::VolumeTexture::colorMapTextureChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void jfrc::VolumeTexture::labelTextureChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}

// SIGNAL 3
void jfrc::VolumeTexture::signalTextureChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, 0);
}

// SIGNAL 4
void jfrc::VolumeTexture::signalMetadataChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, 0);
}

// SIGNAL 5
void jfrc::VolumeTexture::benchmarkTimerResetRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 5, 0);
}

// SIGNAL 6
void jfrc::VolumeTexture::benchmarkTimerPrintRequested(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void jfrc::VolumeTexture::volumeLoadSequenceCompleted()
{
    QMetaObject::activate(this, &staticMetaObject, 7, 0);
}

// SIGNAL 8
void jfrc::VolumeTexture::mpegQueueRequested(QUrl _t1, int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void jfrc::VolumeTexture::mpegLoadSequenceRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 9, 0);
}

// SIGNAL 10
void jfrc::VolumeTexture::loadNextVolumeRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 10, 0);
}
QT_END_MOC_NAMESPACE

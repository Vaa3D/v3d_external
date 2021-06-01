/****************************************************************************
** Meta object code from reading C++ file 'GalleryButton.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../neuron_annotator/gui/GalleryButton.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'GalleryButton.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_GalleryButton[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      29,   15,   14,   14, 0x05,
      80,   65,   14,   14, 0x05,
     141,  115,   14,   14, 0x05,

 // slots: signature, parameters, type, tag, flags
     197,  189,  184,   14, 0x0a,
     234,  225,  184,   14, 0x0a,
     273,  261,  184,   14, 0x0a,
     302,   14,  184,   14, 0x0a,
     333,  321,   14,   14, 0x0a,
     364,  358,   14,   14, 0x0a,
     393,  388,   14,   14, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_GalleryButton[] = {
    "GalleryButton\0\0index,visible\0"
    "fragmentVisibilityChanged(int,bool)\0"
    "index,selected\0fragmentSelectionChanged(int,bool)\0"
    "fragmentIndex,highlighted\0"
    "fragmentHighlightChanged(NeuronIndex,bool)\0"
    "bool\0checked\0setFragmentVisibility(bool)\0"
    "selected\0setFragmentSelection(bool)\0"
    "highlighted\0setFragmentHighlighted(bool)\0"
    "updateVisibility()\0scaledImage\0"
    "setThumbnailIcon(QImage)\0point\0"
    "showContextMenu(QPoint)\0text\0"
    "setLabelText(QString)\0"
};

void GalleryButton::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        GalleryButton *_t = static_cast<GalleryButton *>(_o);
        switch (_id) {
        case 0: _t->fragmentVisibilityChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 1: _t->fragmentSelectionChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 2: _t->fragmentHighlightChanged((*reinterpret_cast< NeuronIndex(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 3: { bool _r = _t->setFragmentVisibility((*reinterpret_cast< bool(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 4: { bool _r = _t->setFragmentSelection((*reinterpret_cast< bool(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 5: { bool _r = _t->setFragmentHighlighted((*reinterpret_cast< bool(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 6: { bool _r = _t->updateVisibility();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 7: _t->setThumbnailIcon((*reinterpret_cast< const QImage(*)>(_a[1]))); break;
        case 8: _t->showContextMenu((*reinterpret_cast< QPoint(*)>(_a[1]))); break;
        case 9: _t->setLabelText((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData GalleryButton::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject GalleryButton::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_GalleryButton,
      qt_meta_data_GalleryButton, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &GalleryButton::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *GalleryButton::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *GalleryButton::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_GalleryButton))
        return static_cast<void*>(const_cast< GalleryButton*>(this));
    return QWidget::qt_metacast(_clname);
}

int GalleryButton::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void GalleryButton::fragmentVisibilityChanged(int _t1, bool _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void GalleryButton::fragmentSelectionChanged(int _t1, bool _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void GalleryButton::fragmentHighlightChanged(NeuronIndex _t1, bool _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_END_MOC_NAMESPACE

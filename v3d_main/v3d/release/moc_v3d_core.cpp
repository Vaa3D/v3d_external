/****************************************************************************
** Meta object code from reading C++ file 'v3d_core.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../v3d_core.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'v3d_core.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MyTextBrowser[] = {

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
      15,   14,   14,   14, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_MyTextBrowser[] = {
    "MyTextBrowser\0\0setText2FocusPointFeatureWidget()\0"
};

void MyTextBrowser::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MyTextBrowser *_t = static_cast<MyTextBrowser *>(_o);
        switch (_id) {
        case 0: _t->setText2FocusPointFeatureWidget(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData MyTextBrowser::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MyTextBrowser::staticMetaObject = {
    { &QTextBrowser::staticMetaObject, qt_meta_stringdata_MyTextBrowser,
      qt_meta_data_MyTextBrowser, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MyTextBrowser::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MyTextBrowser::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MyTextBrowser::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MyTextBrowser))
        return static_cast<void*>(const_cast< MyTextBrowser*>(this));
    return QTextBrowser::qt_metacast(_clname);
}

int MyTextBrowser::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTextBrowser::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
static const uint qt_meta_data_My4DImage[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      11,   10,   10,   10, 0x05,
      41,   10,   10,   10, 0x05,

 // slots: signature, parameters, type, tag, flags
      71,   10,   10,   10, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_My4DImage[] = {
    "My4DImage\0\0focusFeatureViewTextUpdated()\0"
    "signal_trace_history_append()\0"
    "setText2FocusPointFeatureWidget()\0"
};

void My4DImage::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        My4DImage *_t = static_cast<My4DImage *>(_o);
        switch (_id) {
        case 0: _t->focusFeatureViewTextUpdated(); break;
        case 1: _t->signal_trace_history_append(); break;
        case 2: _t->setText2FocusPointFeatureWidget(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData My4DImage::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject My4DImage::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_My4DImage,
      qt_meta_data_My4DImage, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &My4DImage::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *My4DImage::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *My4DImage::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_My4DImage))
        return static_cast<void*>(const_cast< My4DImage*>(this));
    if (!strcmp(_clname, "Image4DSimple"))
        return static_cast< Image4DSimple*>(const_cast< My4DImage*>(this));
    return QObject::qt_metacast(_clname);
}

int My4DImage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void My4DImage::focusFeatureViewTextUpdated()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void My4DImage::signal_trace_history_append()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}
static const uint qt_meta_data_XFormView[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       1,   94, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: signature, parameters, type, tag, flags
      11,   10,   10,   10, 0x05,
      29,   10,   10,   10, 0x05,
      58,   52,   10,   10, 0x05,
      82,   76,   10,   10, 0x05,
     107,  101,   10,   10, 0x05,
     132,  126,   10,   10, 0x05,

 // slots: signature, parameters, type, tag, flags
     162,  151,   10,   10, 0x0a,
     186,   10,   10,   10, 0x0a,
     204,   52,   10,   10, 0x0a,
     223,  221,   10,   10, 0x0a,
     254,  245,   10,   10, 0x0a,
     298,  221,   10,   10, 0x2a,
     337,   10,   10,   10, 0x0a,
     353,   10,   10,   10, 0x0a,
     361,   10,   10,   10, 0x0a,
     395,  390,   10,   10, 0x0a,

 // properties: name, type, flags
      52,  431, 0x06095003,

       0        // eod
};

static const char qt_meta_stringdata_XFormView[] = {
    "XFormView\0\0colorChanged(int)\0"
    "colorChangedGlass(int)\0scale\0"
    "scaleChanged(int)\0x_new\0focusXChanged(int)\0"
    "y_new\0focusYChanged(int)\0z_new\0"
    "focusZChanged(int)\0pxm,bGlass\0"
    "setPixmap(QPixmap,bool)\0updateViewPlane()\0"
    "changeScale(int)\0c\0changeFocusPlane(int)\0"
    "c,bGlass\0changeColorType(ImageDisplayColorType,bool)\0"
    "changeColorType(ImageDisplayColorType)\0"
    "setPixmapType()\0reset()\0"
    "popupImageProcessingDialog()\0item\0"
    "popupImageProcessingDialog(QString)\0"
    "double\0"
};

void XFormView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        XFormView *_t = static_cast<XFormView *>(_o);
        switch (_id) {
        case 0: _t->colorChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->colorChangedGlass((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->scaleChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->focusXChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->focusYChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->focusZChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->setPixmap((*reinterpret_cast< const QPixmap(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 7: _t->updateViewPlane(); break;
        case 8: _t->changeScale((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->changeFocusPlane((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->changeColorType((*reinterpret_cast< ImageDisplayColorType(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 11: _t->changeColorType((*reinterpret_cast< ImageDisplayColorType(*)>(_a[1]))); break;
        case 12: _t->setPixmapType(); break;
        case 13: _t->reset(); break;
        case 14: _t->popupImageProcessingDialog(); break;
        case 15: _t->popupImageProcessingDialog((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData XFormView::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject XFormView::staticMetaObject = {
    { &ArthurFrame::staticMetaObject, qt_meta_stringdata_XFormView,
      qt_meta_data_XFormView, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &XFormView::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *XFormView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *XFormView::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_XFormView))
        return static_cast<void*>(const_cast< XFormView*>(this));
    return ArthurFrame::qt_metacast(_clname);
}

int XFormView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = ArthurFrame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< double*>(_v) = scale(); break;
        }
        _id -= 1;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: changeScale(*reinterpret_cast< double*>(_v)); break;
        }
        _id -= 1;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 1;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void XFormView::colorChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void XFormView::colorChangedGlass(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void XFormView::scaleChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void XFormView::focusXChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void XFormView::focusYChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void XFormView::focusZChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_END_MOC_NAMESPACE

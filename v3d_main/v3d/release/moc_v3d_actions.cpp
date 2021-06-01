/****************************************************************************
** Meta object code from reading C++ file 'v3d_actions.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../v3d_actions.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'v3d_actions.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_v3d__BrowseToWebPageAction[] = {

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
      28,   27,   27,   27, 0x05,

 // slots: signature, parameters, type, tag, flags
      47,   27,   27,   27, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_v3d__BrowseToWebPageAction[] = {
    "v3d::BrowseToWebPageAction\0\0"
    "page_open_failed()\0openWebPage()\0"
};

void v3d::BrowseToWebPageAction::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        BrowseToWebPageAction *_t = static_cast<BrowseToWebPageAction *>(_o);
        switch (_id) {
        case 0: _t->page_open_failed(); break;
        case 1: _t->openWebPage(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData v3d::BrowseToWebPageAction::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject v3d::BrowseToWebPageAction::staticMetaObject = {
    { &QAction::staticMetaObject, qt_meta_stringdata_v3d__BrowseToWebPageAction,
      qt_meta_data_v3d__BrowseToWebPageAction, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &v3d::BrowseToWebPageAction::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *v3d::BrowseToWebPageAction::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *v3d::BrowseToWebPageAction::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_v3d__BrowseToWebPageAction))
        return static_cast<void*>(const_cast< BrowseToWebPageAction*>(this));
    return QAction::qt_metacast(_clname);
}

int v3d::BrowseToWebPageAction::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAction::qt_metacall(_c, _id, _a);
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
void v3d::BrowseToWebPageAction::page_open_failed()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
static const uint qt_meta_data_v3d__ShowV3dAboutDialogAction[] = {

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
      31,   30,   30,   30, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_v3d__ShowV3dAboutDialogAction[] = {
    "v3d::ShowV3dAboutDialogAction\0\0"
    "show_dialog()\0"
};

void v3d::ShowV3dAboutDialogAction::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ShowV3dAboutDialogAction *_t = static_cast<ShowV3dAboutDialogAction *>(_o);
        switch (_id) {
        case 0: _t->show_dialog(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData v3d::ShowV3dAboutDialogAction::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject v3d::ShowV3dAboutDialogAction::staticMetaObject = {
    { &QAction::staticMetaObject, qt_meta_stringdata_v3d__ShowV3dAboutDialogAction,
      qt_meta_data_v3d__ShowV3dAboutDialogAction, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &v3d::ShowV3dAboutDialogAction::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *v3d::ShowV3dAboutDialogAction::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *v3d::ShowV3dAboutDialogAction::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_v3d__ShowV3dAboutDialogAction))
        return static_cast<void*>(const_cast< ShowV3dAboutDialogAction*>(this));
    return QAction::qt_metacast(_clname);
}

int v3d::ShowV3dAboutDialogAction::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAction::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE

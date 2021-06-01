/****************************************************************************
** Meta object code from reading C++ file 'v3d_plugin_loader.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../plugin_loader/v3d_plugin_loader.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'v3d_plugin_loader.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_V3d_PluginLoader[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      18,   17,   17,   17, 0x0a,
      34,   17,   17,   17, 0x0a,
      50,   17,   17,   17, 0x0a,
      65,   17,   17,   17, 0x0a,
      95,   77,   17,   17, 0x0a,
     129,   17,   17,   17, 0x0a,
     147,   17,   17,   17, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_V3d_PluginLoader[] = {
    "V3d_PluginLoader\0\0rescanPlugins()\0"
    "populateMenus()\0aboutPlugins()\0"
    "runPlugin()\0loader,menuString\0"
    "runPlugin(QPluginLoader*,QString)\0"
    "runRecentPlugin()\0clear_recentPlugins()\0"
};

void V3d_PluginLoader::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        V3d_PluginLoader *_t = static_cast<V3d_PluginLoader *>(_o);
        switch (_id) {
        case 0: _t->rescanPlugins(); break;
        case 1: _t->populateMenus(); break;
        case 2: _t->aboutPlugins(); break;
        case 3: _t->runPlugin(); break;
        case 4: _t->runPlugin((*reinterpret_cast< QPluginLoader*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 5: _t->runRecentPlugin(); break;
        case 6: _t->clear_recentPlugins(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData V3d_PluginLoader::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject V3d_PluginLoader::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_V3d_PluginLoader,
      qt_meta_data_V3d_PluginLoader, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &V3d_PluginLoader::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *V3d_PluginLoader::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *V3d_PluginLoader::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_V3d_PluginLoader))
        return static_cast<void*>(const_cast< V3d_PluginLoader*>(this));
    if (!strcmp(_clname, "V3DPluginCallback2"))
        return static_cast< V3DPluginCallback2*>(const_cast< V3d_PluginLoader*>(this));
    return QObject::qt_metacast(_clname);
}

int V3d_PluginLoader::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}
QT_END_MOC_NAMESPACE

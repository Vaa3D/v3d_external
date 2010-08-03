/****************************************************************************
** Meta object code from reading C++ file 'wavelets.h'
**
** Created: Tue Aug 3 16:57:39 2010
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "wavelets.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'wavelets.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_WaveletPlugin[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // slots: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x0a,
      30,   24,   14,   14, 0x0a,
      48,   14,   14,   14, 0x0a,
      72,   14,   14,   14, 0x0a,
      99,   14,   14,   14, 0x0a,
     119,   14,   14,   14, 0x0a,
     139,   14,   14,   14, 0x0a,
     159,   14,   14,   14, 0x0a,
     179,   14,   14,   14, 0x0a,
     206,   14,   14,   14, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_WaveletPlugin[] = {
    "WaveletPlugin\0\0cancel()\0value\0"
    "sliderChange(int)\0addScaleButtonPressed()\0"
    "removeScaleButtonPressed()\0"
    "dev1ButtonPressed()\0dev2ButtonPressed()\0"
    "dev3ButtonPressed()\0dev4ButtonPressed()\0"
    "detectSpotsButtonPressed()\0"
    "denoiseButtonPressed()\0"
};

const QMetaObject WaveletPlugin::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_WaveletPlugin,
      qt_meta_data_WaveletPlugin, 0 }
};

const QMetaObject *WaveletPlugin::metaObject() const
{
    return &staticMetaObject;
}

void *WaveletPlugin::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_WaveletPlugin))
        return static_cast<void*>(const_cast< WaveletPlugin*>(this));
    if (!strcmp(_clname, "V3DPluginInterface"))
        return static_cast< V3DPluginInterface*>(const_cast< WaveletPlugin*>(this));
    if (!strcmp(_clname, "com.janelia.v3d.V3DPluginInterface/1.0"))
        return static_cast< V3DPluginInterface*>(const_cast< WaveletPlugin*>(this));
    return QObject::qt_metacast(_clname);
}

int WaveletPlugin::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: cancel(); break;
        case 1: sliderChange((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: addScaleButtonPressed(); break;
        case 3: removeScaleButtonPressed(); break;
        case 4: dev1ButtonPressed(); break;
        case 5: dev2ButtonPressed(); break;
        case 6: dev3ButtonPressed(); break;
        case 7: dev4ButtonPressed(); break;
        case 8: detectSpotsButtonPressed(); break;
        case 9: denoiseButtonPressed(); break;
        default: ;
        }
        _id -= 10;
    }
    return _id;
}
QT_END_MOC_NAMESPACE

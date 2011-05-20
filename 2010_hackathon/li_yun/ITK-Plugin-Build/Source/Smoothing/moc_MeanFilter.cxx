/****************************************************************************
** Meta object code from reading C++ file 'MeanFilter.h'
**
** Created: Tue Jun 21 10:17:54 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../ITK-V3D-Plugins/Source/Smoothing/MeanFilter.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MeanFilter.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ITKMeanFilterPlugin[] = {

 // content:
       5,       // revision
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

static const char qt_meta_stringdata_ITKMeanFilterPlugin[] = {
    "ITKMeanFilterPlugin\0"
};

const QMetaObject ITKMeanFilterPlugin::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_ITKMeanFilterPlugin,
      qt_meta_data_ITKMeanFilterPlugin, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ITKMeanFilterPlugin::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ITKMeanFilterPlugin::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ITKMeanFilterPlugin::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ITKMeanFilterPlugin))
        return static_cast<void*>(const_cast< ITKMeanFilterPlugin*>(this));
    if (!strcmp(_clname, "V3DPluginInterface"))
        return static_cast< V3DPluginInterface*>(const_cast< ITKMeanFilterPlugin*>(this));
    if (!strcmp(_clname, "com.janelia.v3d.V3DPluginInterface/1.1"))
        return static_cast< V3DPluginInterface*>(const_cast< ITKMeanFilterPlugin*>(this));
    return QObject::qt_metacast(_clname);
}

int ITKMeanFilterPlugin::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_ITKMeanFilterDialog[] = {

 // content:
       5,       // revision
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

static const char qt_meta_stringdata_ITKMeanFilterDialog[] = {
    "ITKMeanFilterDialog\0"
};

const QMetaObject ITKMeanFilterDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_ITKMeanFilterDialog,
      qt_meta_data_ITKMeanFilterDialog, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ITKMeanFilterDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ITKMeanFilterDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ITKMeanFilterDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ITKMeanFilterDialog))
        return static_cast<void*>(const_cast< ITKMeanFilterDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int ITKMeanFilterDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
QT_END_MOC_NAMESPACE

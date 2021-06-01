/****************************************************************************
** Meta object code from reading C++ file 'PTabVolumeInfo.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../terafly/src/presentation/PTabVolumeInfo.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PTabVolumeInfo.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_terafly__CUserInactivityFilter[] = {

 // content:
       6,       // revision
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

static const char qt_meta_stringdata_terafly__CUserInactivityFilter[] = {
    "terafly::CUserInactivityFilter\0"
};

void terafly::CUserInactivityFilter::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData terafly::CUserInactivityFilter::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject terafly::CUserInactivityFilter::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_terafly__CUserInactivityFilter,
      qt_meta_data_terafly__CUserInactivityFilter, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &terafly::CUserInactivityFilter::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *terafly::CUserInactivityFilter::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *terafly::CUserInactivityFilter::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_terafly__CUserInactivityFilter))
        return static_cast<void*>(const_cast< CUserInactivityFilter*>(this));
    return QObject::qt_metacast(_clname);
}

int terafly::CUserInactivityFilter::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_terafly__PTabVolumeInfo[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      18,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      25,   24,   24,   24, 0x0a,
      33,   24,   24,   24, 0x0a,
      40,   24,   24,   24, 0x0a,
      49,   24,   24,   24, 0x0a,
      71,   24,   24,   24, 0x0a,
      96,   24,   24,   24, 0x0a,
     121,  119,   24,   24, 0x0a,
     147,  119,   24,   24, 0x0a,
     181,  119,   24,   24, 0x0a,
     230,  216,   24,   24, 0x0a,
     261,   24,   24,   24, 0x2a,
     288,  119,   24,   24, 0x0a,
     329,  119,   24,   24, 0x0a,
     366,  119,   24,   24, 0x0a,
     404,  119,   24,   24, 0x0a,
     441,  119,   24,   24, 0x0a,
     481,  119,   24,   24, 0x0a,
     525,  119,   24,   24, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_terafly__PTabVolumeInfo[] = {
    "terafly::PTabVolumeInfo\0\0reset()\0"
    "init()\0update()\0open_button_clicked()\0"
    "recheck_button_clicked()\0"
    "clear_button_clicked()\0v\0"
    "ram_limit_changed(double)\0"
    "empty_combobox_index_changed(int)\0"
    "empty_intensity_value_changed(int)\0"
    "in_background\0vp_refill_button_clicked(bool)\0"
    "vp_refill_button_clicked()\0"
    "vp_refill_strategy_combobox_changed(int)\0"
    "vp_refill_times_spinbox_changed(int)\0"
    "vp_refill_auto_checkbox_changed(bool)\0"
    "vp_refill_stop_combobox_changed(int)\0"
    "vp_refill_coverage_spinbox_changed(int)\0"
    "vp_highest_res_cache_checkbox_changed(bool)\0"
    "vp_highest_res_freeze_checkbox_changed(bool)\0"
};

void terafly::PTabVolumeInfo::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        PTabVolumeInfo *_t = static_cast<PTabVolumeInfo *>(_o);
        switch (_id) {
        case 0: _t->reset(); break;
        case 1: _t->init(); break;
        case 2: _t->update(); break;
        case 3: _t->open_button_clicked(); break;
        case 4: _t->recheck_button_clicked(); break;
        case 5: _t->clear_button_clicked(); break;
        case 6: _t->ram_limit_changed((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 7: _t->empty_combobox_index_changed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->empty_intensity_value_changed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->vp_refill_button_clicked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 10: _t->vp_refill_button_clicked(); break;
        case 11: _t->vp_refill_strategy_combobox_changed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->vp_refill_times_spinbox_changed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: _t->vp_refill_auto_checkbox_changed((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 14: _t->vp_refill_stop_combobox_changed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 15: _t->vp_refill_coverage_spinbox_changed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 16: _t->vp_highest_res_cache_checkbox_changed((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 17: _t->vp_highest_res_freeze_checkbox_changed((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData terafly::PTabVolumeInfo::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject terafly::PTabVolumeInfo::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_terafly__PTabVolumeInfo,
      qt_meta_data_terafly__PTabVolumeInfo, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &terafly::PTabVolumeInfo::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *terafly::PTabVolumeInfo::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *terafly::PTabVolumeInfo::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_terafly__PTabVolumeInfo))
        return static_cast<void*>(const_cast< PTabVolumeInfo*>(this));
    return QWidget::qt_metacast(_clname);
}

int terafly::PTabVolumeInfo::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    }
    return _id;
}
QT_END_MOC_NAMESPACE

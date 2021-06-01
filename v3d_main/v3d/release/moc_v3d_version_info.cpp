/****************************************************************************
** Meta object code from reading C++ file 'v3d_version_info.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../v3d_version_info.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'v3d_version_info.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_v3d__UpdateItem[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      28,   17,   16,   16, 0x05,

 // slots: signature, parameters, type, tag, flags
      86,   80,   16,   16, 0x0a,
     102,   16,   16,   16, 0x0a,
     146,   16,   16,   16, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_v3d__UpdateItem[] = {
    "v3d::UpdateItem\0\0,succeeded\0"
    "updateComplete(v3d::DownloadingUpdatesDialog*,bool)\0"
    "state\0setInstall(int)\0"
    "startUpdate(v3d::DownloadingUpdatesDialog*)\0"
    "finishedDownloadSlot(QNetworkReply*)\0"
};

void v3d::UpdateItem::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        UpdateItem *_t = static_cast<UpdateItem *>(_o);
        switch (_id) {
        case 0: _t->updateComplete((*reinterpret_cast< v3d::DownloadingUpdatesDialog*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 1: _t->setInstall((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->startUpdate((*reinterpret_cast< v3d::DownloadingUpdatesDialog*(*)>(_a[1]))); break;
        case 3: _t->finishedDownloadSlot((*reinterpret_cast< QNetworkReply*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData v3d::UpdateItem::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject v3d::UpdateItem::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_v3d__UpdateItem,
      qt_meta_data_v3d__UpdateItem, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &v3d::UpdateItem::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *v3d::UpdateItem::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *v3d::UpdateItem::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_v3d__UpdateItem))
        return static_cast<void*>(const_cast< UpdateItem*>(this));
    return QObject::qt_metacast(_clname);
}

int v3d::UpdateItem::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void v3d::UpdateItem::updateComplete(v3d::DownloadingUpdatesDialog * _t1, bool _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_v3d__V3DVersionChecker[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      24,   23,   23,   23, 0x0a,
      49,   43,   23,   23, 0x08,
      76,   23,   23,   23, 0x08,
      95,   23,   23,   23, 0x08,
     128,  113,   23,   23, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_v3d__V3DVersionChecker[] = {
    "v3d::V3DVersionChecker\0\0createVersionXml()\0"
    "reply\0gotVersion(QNetworkReply*)\0"
    "show_update_list()\0install_updates()\0"
    "progressDialog\0"
    "finishUpdates(v3d::DownloadingUpdatesDialog*)\0"
};

void v3d::V3DVersionChecker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        V3DVersionChecker *_t = static_cast<V3DVersionChecker *>(_o);
        switch (_id) {
        case 0: _t->createVersionXml(); break;
        case 1: _t->gotVersion((*reinterpret_cast< QNetworkReply*(*)>(_a[1]))); break;
        case 2: _t->show_update_list(); break;
        case 3: _t->install_updates(); break;
        case 4: _t->finishUpdates((*reinterpret_cast< v3d::DownloadingUpdatesDialog*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData v3d::V3DVersionChecker::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject v3d::V3DVersionChecker::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_v3d__V3DVersionChecker,
      qt_meta_data_v3d__V3DVersionChecker, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &v3d::V3DVersionChecker::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *v3d::V3DVersionChecker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *v3d::V3DVersionChecker::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_v3d__V3DVersionChecker))
        return static_cast<void*>(const_cast< V3DVersionChecker*>(this));
    return QObject::qt_metacast(_clname);
}

int v3d::V3DVersionChecker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}
static const uint qt_meta_data_v3d__VersionUrlValidator[] = {

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

static const char qt_meta_stringdata_v3d__VersionUrlValidator[] = {
    "v3d::VersionUrlValidator\0"
};

void v3d::VersionUrlValidator::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData v3d::VersionUrlValidator::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject v3d::VersionUrlValidator::staticMetaObject = {
    { &QValidator::staticMetaObject, qt_meta_stringdata_v3d__VersionUrlValidator,
      qt_meta_data_v3d__VersionUrlValidator, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &v3d::VersionUrlValidator::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *v3d::VersionUrlValidator::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *v3d::VersionUrlValidator::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_v3d__VersionUrlValidator))
        return static_cast<void*>(const_cast< VersionUrlValidator*>(this));
    return QValidator::qt_metacast(_clname);
}

int v3d::VersionUrlValidator::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QValidator::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_v3d__CheckForUpdatesDialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      28,   27,   27,   27, 0x05,

 // slots: signature, parameters, type, tag, flags
      41,   27,   27,   27, 0x08,
      53,   27,   27,   27, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_v3d__CheckForUpdatesDialog[] = {
    "v3d::CheckForUpdatesDialog\0\0yes_update()\0"
    "check_now()\0show_options()\0"
};

void v3d::CheckForUpdatesDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        CheckForUpdatesDialog *_t = static_cast<CheckForUpdatesDialog *>(_o);
        switch (_id) {
        case 0: _t->yes_update(); break;
        case 1: _t->check_now(); break;
        case 2: _t->show_options(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData v3d::CheckForUpdatesDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject v3d::CheckForUpdatesDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_v3d__CheckForUpdatesDialog,
      qt_meta_data_v3d__CheckForUpdatesDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &v3d::CheckForUpdatesDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *v3d::CheckForUpdatesDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *v3d::CheckForUpdatesDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_v3d__CheckForUpdatesDialog))
        return static_cast<void*>(const_cast< CheckForUpdatesDialog*>(this));
    if (!strcmp(_clname, "Ui::dialog_update_v3d"))
        return static_cast< Ui::dialog_update_v3d*>(const_cast< CheckForUpdatesDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int v3d::CheckForUpdatesDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
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
void v3d::CheckForUpdatesDialog::yes_update()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
static const uint qt_meta_data_v3d__UpdatesListDialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      24,   23,   23,   23, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_v3d__UpdatesListDialog[] = {
    "v3d::UpdatesListDialog\0\0update_install()\0"
};

void v3d::UpdatesListDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        UpdatesListDialog *_t = static_cast<UpdatesListDialog *>(_o);
        switch (_id) {
        case 0: _t->update_install(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData v3d::UpdatesListDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject v3d::UpdatesListDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_v3d__UpdatesListDialog,
      qt_meta_data_v3d__UpdatesListDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &v3d::UpdatesListDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *v3d::UpdatesListDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *v3d::UpdatesListDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_v3d__UpdatesListDialog))
        return static_cast<void*>(const_cast< UpdatesListDialog*>(this));
    if (!strcmp(_clname, "Ui::dialog_update_list"))
        return static_cast< Ui::dialog_update_list*>(const_cast< UpdatesListDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int v3d::UpdatesListDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void v3d::UpdatesListDialog::update_install()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
static const uint qt_meta_data_v3d__UpdateOptionsDialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      26,   25,   25,   25, 0x08,
      58,   42,   25,   25, 0x08,
      99,   25,   25,   25, 0x08,
     129,   25,   25,   25, 0x08,
     155,   25,   25,   25, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_v3d__UpdateOptionsDialog[] = {
    "v3d::UpdateOptionsDialog\0\0save_xml_file()\0"
    "updateFrequency\0"
    "on_comboBox_currentIndexChanged(QString)\0"
    "on_lineEdit_editingFinished()\0"
    "on_lineEdit_textChanged()\0"
    "use_default_version_file()\0"
};

void v3d::UpdateOptionsDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        UpdateOptionsDialog *_t = static_cast<UpdateOptionsDialog *>(_o);
        switch (_id) {
        case 0: _t->save_xml_file(); break;
        case 1: _t->on_comboBox_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->on_lineEdit_editingFinished(); break;
        case 3: _t->on_lineEdit_textChanged(); break;
        case 4: _t->use_default_version_file(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData v3d::UpdateOptionsDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject v3d::UpdateOptionsDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_v3d__UpdateOptionsDialog,
      qt_meta_data_v3d__UpdateOptionsDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &v3d::UpdateOptionsDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *v3d::UpdateOptionsDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *v3d::UpdateOptionsDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_v3d__UpdateOptionsDialog))
        return static_cast<void*>(const_cast< UpdateOptionsDialog*>(this));
    if (!strcmp(_clname, "Ui::dialog_update_options"))
        return static_cast< Ui::dialog_update_options*>(const_cast< UpdateOptionsDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int v3d::UpdateOptionsDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}
static const uint qt_meta_data_v3d__CheckingForUpdatesDialog[] = {

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

static const char qt_meta_stringdata_v3d__CheckingForUpdatesDialog[] = {
    "v3d::CheckingForUpdatesDialog\0\0"
    "canceled()\0"
};

void v3d::CheckingForUpdatesDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        CheckingForUpdatesDialog *_t = static_cast<CheckingForUpdatesDialog *>(_o);
        switch (_id) {
        case 0: _t->canceled(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData v3d::CheckingForUpdatesDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject v3d::CheckingForUpdatesDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_v3d__CheckingForUpdatesDialog,
      qt_meta_data_v3d__CheckingForUpdatesDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &v3d::CheckingForUpdatesDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *v3d::CheckingForUpdatesDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *v3d::CheckingForUpdatesDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_v3d__CheckingForUpdatesDialog))
        return static_cast<void*>(const_cast< CheckingForUpdatesDialog*>(this));
    if (!strcmp(_clname, "Ui::checking_for_updates_dialog"))
        return static_cast< Ui::checking_for_updates_dialog*>(const_cast< CheckingForUpdatesDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int v3d::CheckingForUpdatesDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
static const uint qt_meta_data_v3d__UpdatesAvailableDialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      29,   28,   28,   28, 0x05,

 // slots: signature, parameters, type, tag, flags
      42,   28,   28,   28, 0x08,
      57,   28,   28,   28, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_v3d__UpdatesAvailableDialog[] = {
    "v3d::UpdatesAvailableDialog\0\0yes_update()\0"
    "never_update()\0remind_me_later()\0"
};

void v3d::UpdatesAvailableDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        UpdatesAvailableDialog *_t = static_cast<UpdatesAvailableDialog *>(_o);
        switch (_id) {
        case 0: _t->yes_update(); break;
        case 1: _t->never_update(); break;
        case 2: _t->remind_me_later(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData v3d::UpdatesAvailableDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject v3d::UpdatesAvailableDialog::staticMetaObject = {
    { &QMessageBox::staticMetaObject, qt_meta_stringdata_v3d__UpdatesAvailableDialog,
      qt_meta_data_v3d__UpdatesAvailableDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &v3d::UpdatesAvailableDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *v3d::UpdatesAvailableDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *v3d::UpdatesAvailableDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_v3d__UpdatesAvailableDialog))
        return static_cast<void*>(const_cast< UpdatesAvailableDialog*>(this));
    return QMessageBox::qt_metacast(_clname);
}

int v3d::UpdatesAvailableDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMessageBox::qt_metacall(_c, _id, _a);
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
void v3d::UpdatesAvailableDialog::yes_update()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
static const uint qt_meta_data_v3d__DownloadingUpdatesDialog[] = {

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

static const char qt_meta_stringdata_v3d__DownloadingUpdatesDialog[] = {
    "v3d::DownloadingUpdatesDialog\0\0"
    "canceled()\0"
};

void v3d::DownloadingUpdatesDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DownloadingUpdatesDialog *_t = static_cast<DownloadingUpdatesDialog *>(_o);
        switch (_id) {
        case 0: _t->canceled(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData v3d::DownloadingUpdatesDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject v3d::DownloadingUpdatesDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_v3d__DownloadingUpdatesDialog,
      qt_meta_data_v3d__DownloadingUpdatesDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &v3d::DownloadingUpdatesDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *v3d::DownloadingUpdatesDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *v3d::DownloadingUpdatesDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_v3d__DownloadingUpdatesDialog))
        return static_cast<void*>(const_cast< DownloadingUpdatesDialog*>(this));
    if (!strcmp(_clname, "Ui::downloading_updates_dialog"))
        return static_cast< Ui::downloading_updates_dialog*>(const_cast< DownloadingUpdatesDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int v3d::DownloadingUpdatesDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
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

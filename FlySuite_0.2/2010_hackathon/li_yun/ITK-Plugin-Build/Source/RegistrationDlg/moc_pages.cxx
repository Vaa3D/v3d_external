/****************************************************************************
** Meta object code from reading C++ file 'pages.h'
**
** Created: Sat May 7 21:47:02 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../ITK-V3D-Plugins/Source/RegistrationDlg/pages.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'pages.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_AutoPipePage[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      14,   13,   13,   13, 0x0a,
      29,   13,   13,   13, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_AutoPipePage[] = {
    "AutoPipePage\0\0CallPipeline()\0CreateIcon()\0"
};

const QMetaObject AutoPipePage::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_AutoPipePage,
      qt_meta_data_AutoPipePage, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &AutoPipePage::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *AutoPipePage::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *AutoPipePage::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_AutoPipePage))
        return static_cast<void*>(const_cast< AutoPipePage*>(this));
    return QWidget::qt_metacast(_clname);
}

int AutoPipePage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: CallPipeline(); break;
        case 1: CreateIcon(); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}
static const uint qt_meta_data_UserPipePage[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      14,   13,   13,   13, 0x05,

 // slots: signature, parameters, type, tag, flags
      29,   13,   13,   13, 0x0a,
      44,   13,   13,   13, 0x0a,
      54,   13,   13,   13, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_UserPipePage[] = {
    "UserPipePage\0\0countChanged()\0"
    "CallPipeline()\0ADDItem()\0CreatIcon()\0"
};

const QMetaObject UserPipePage::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_UserPipePage,
      qt_meta_data_UserPipePage, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &UserPipePage::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *UserPipePage::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *UserPipePage::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_UserPipePage))
        return static_cast<void*>(const_cast< UserPipePage*>(this));
    return QWidget::qt_metacast(_clname);
}

int UserPipePage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: countChanged(); break;
        case 1: CallPipeline(); break;
        case 2: ADDItem(); break;
        case 3: CreatIcon(); break;
        default: ;
        }
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void UserPipePage::countChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
static const uint qt_meta_data_UserFilterPage[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      16,   15,   15,   15, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_UserFilterPage[] = {
    "UserFilterPage\0\0CallFilter()\0"
};

const QMetaObject UserFilterPage::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_UserFilterPage,
      qt_meta_data_UserFilterPage, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &UserFilterPage::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *UserFilterPage::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *UserFilterPage::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_UserFilterPage))
        return static_cast<void*>(const_cast< UserFilterPage*>(this));
    return QWidget::qt_metacast(_clname);
}

int UserFilterPage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: CallFilter(); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE

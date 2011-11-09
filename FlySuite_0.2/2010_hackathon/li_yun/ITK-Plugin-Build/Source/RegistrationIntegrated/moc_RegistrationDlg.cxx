/****************************************************************************
** Meta object code from reading C++ file 'RegistrationDlg.h'
**
** Created: Fri Jul 8 02:46:56 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../ITK-V3D-Plugins/Source/RegistrationIntegrated/RegistrationDlg.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'RegistrationDlg.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_RegistrationDlg[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      19,   17,   16,   16, 0x0a,
      36,   17,   16,   16, 0x0a,
      56,   16,   16,   16, 0x0a,
      71,   16,   16,   16, 0x0a,
      79,   16,   16,   16, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_RegistrationDlg[] = {
    "RegistrationDlg\0\0i\0updateOptim(int)\0"
    "updateIntOptim(int)\0updateConfig()\0"
    "Start()\0Subtract()\0"
};

const QMetaObject RegistrationDlg::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_RegistrationDlg,
      qt_meta_data_RegistrationDlg, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &RegistrationDlg::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *RegistrationDlg::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *RegistrationDlg::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_RegistrationDlg))
        return static_cast<void*>(const_cast< RegistrationDlg*>(this));
    return QDialog::qt_metacast(_clname);
}

int RegistrationDlg::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: updateOptim((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: updateIntOptim((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: updateConfig(); break;
        case 3: Start(); break;
        case 4: Subtract(); break;
        default: ;
        }
        _id -= 5;
    }
    return _id;
}
QT_END_MOC_NAMESPACE

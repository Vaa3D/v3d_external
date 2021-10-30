/****************************************************************************
** Meta object code from reading C++ file 'q_paradialog_rigidaffine.h'
**
** Created: Tue Dec 6 11:56:31 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "q_paradialog_rigidaffine.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'q_paradialog_rigidaffine.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_CParaDialog_rigidaffine[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      25,   24,   24,   24, 0x08,
      46,   24,   24,   24, 0x08,
      67,   24,   24,   24, 0x08,
      92,   24,   24,   24, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_CParaDialog_rigidaffine[] = {
    "CParaDialog_rigidaffine\0\0_slots_openDlg_tar()\0"
    "_slots_openDlg_sub()\0_slots_saveDlg_sub2tar()\0"
    "_slots_saveDlg_grid()\0"
};

const QMetaObject CParaDialog_rigidaffine::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_CParaDialog_rigidaffine,
      qt_meta_data_CParaDialog_rigidaffine, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &CParaDialog_rigidaffine::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *CParaDialog_rigidaffine::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *CParaDialog_rigidaffine::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CParaDialog_rigidaffine))
        return static_cast<void*>(const_cast< CParaDialog_rigidaffine*>(this));
    if (!strcmp(_clname, "Ui::Paradialog_rigidaffine"))
        return static_cast< Ui::Paradialog_rigidaffine*>(const_cast< CParaDialog_rigidaffine*>(this));
    return QDialog::qt_metacast(_clname);
}

int CParaDialog_rigidaffine::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _slots_openDlg_tar(); break;
        case 1: _slots_openDlg_sub(); break;
        case 2: _slots_saveDlg_sub2tar(); break;
        case 3: _slots_saveDlg_grid(); break;
        default: ;
        }
        _id -= 4;
    }
    return _id;
}
QT_END_MOC_NAMESPACE

/****************************************************************************
** Meta object code from reading C++ file 'basic_thread.h'
**
** Created: Sat May 7 21:18:43 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../v3d_main/basic_c_fun/basic_thread.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'basic_thread.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_TransactionThread[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      19,   18,   18,   18, 0x05,
      40,   18,   18,   18, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_TransactionThread[] = {
    "TransactionThread\0\0transactionStarted()\0"
    "allTransactionsDone()\0"
};

const QMetaObject TransactionThread::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_TransactionThread,
      qt_meta_data_TransactionThread, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &TransactionThread::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *TransactionThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *TransactionThread::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_TransactionThread))
        return static_cast<void*>(const_cast< TransactionThread*>(this));
    return QThread::qt_metacast(_clname);
}

int TransactionThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: transactionStarted(); break;
        case 1: allTransactionsDone(); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void TransactionThread::transactionStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void TransactionThread::allTransactionsDone()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}
QT_END_MOC_NAMESPACE

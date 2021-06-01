/****************************************************************************
** Meta object code from reading C++ file 'ConsoleObserver.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../neuron_annotator/utility/ConsoleObserver.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ConsoleObserver.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ConsoleObserver[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      30,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       9,       // signalCount

 // signals: signature, parameters, type, tag, flags
      26,   17,   16,   16, 0x05,
      66,   50,   16,   16, 0x05,
     138,  104,   16,   16, 0x05,
     202,  194,   16,   16, 0x05,
     244,   16,   16,   16, 0x05,
     287,  269,   16,   16, 0x05,
     330,  317,   16,   16, 0x05,
     365,  358,   16,   16, 0x05,
     401,  394,   16,   16, 0x05,

 // slots: signature, parameters, type, tag, flags
     436,  429,   16,   16, 0x0a,
     461,  429,   16,   16, 0x0a,
     512,  485,   16,   16, 0x0a,
     567,  549,   16,   16, 0x0a,
     610,  601,   16,   16, 0x0a,
     638,  601,   16,   16, 0x0a,
     675,  665,   16,   16, 0x0a,
     699,   16,   16,   16, 0x0a,
     719,  601,   16,   16, 0x08,
     764,  756,   16,   16, 0x08,
     803,  797,   16,   16, 0x08,
     830,  756,   16,   16, 0x08,
     870,  797,   16,   16, 0x08,
     904,  756,   16,   16, 0x08,
     953,  797,   16,   16, 0x08,
     996,  756,   16,   16, 0x08,
    1046,  797,   16,   16, 0x08,
    1090,  756,   16,   16, 0x08,
    1129,  797,   16,   16, 0x08,
    1162,  756,   16,   16, 0x08,
    1204,  797,   16,   16, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_ConsoleObserver[] = {
    "ConsoleObserver\0\0ontology\0"
    "openOntology(Ontology*)\0annotatedBranch\0"
    "openAnnotatedBranch(AnnotatedBranch*)\0"
    "entityId,annotations,userColorMap\0"
    "updateAnnotations(qint64,AnnotationList*,UserColorMap*)\0"
    "session\0openAnnotationSession(AnnotationSession*)\0"
    "closeAnnotationSession()\0entityId,external\0"
    "selectEntityById(qint64,bool)\0"
    "errorMessage\0communicationError(QString)\0"
    "sample\0updateCurrentSample(Entity*)\0"
    "entity\0openStackWithVaa3d(Entity*)\0"
    "rootId\0ontologySelected(qint64)\0"
    "ontologyChanged(qint64)\0"
    "category,uniqueId,clearAll\0"
    "entitySelected(QString,QString,bool)\0"
    "category,uniqueId\0entityDeselected(QString,QString)\0"
    "entityId\0entityViewRequested(qint64)\0"
    "annotationsChanged(qint64)\0sessionId\0"
    "sessionSelected(qint64)\0sessionDeselected()\0"
    "annotatedBranchViewRequested(qint64)\0"
    "results\0loadOntologyResults(const void*)\0"
    "error\0loadOntologyError(QString)\0"
    "entityViewRequestedResults(const void*)\0"
    "entityViewRequestedError(QString)\0"
    "annotatedBranchViewRequestedResults(const void*)\0"
    "annotatedBranchViewRequestedError(QString)\0"
    "annotatedBranchViewRequested2Results(const void*)\0"
    "annotatedBranchViewRequested2Error(QString)\0"
    "annotationsChangedResults(const void*)\0"
    "annotationsChangedError(QString)\0"
    "loadAnnotationSessionResults(const void*)\0"
    "loadAnnotationSessionError(QString)\0"
};

void ConsoleObserver::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ConsoleObserver *_t = static_cast<ConsoleObserver *>(_o);
        switch (_id) {
        case 0: _t->openOntology((*reinterpret_cast< Ontology*(*)>(_a[1]))); break;
        case 1: _t->openAnnotatedBranch((*reinterpret_cast< AnnotatedBranch*(*)>(_a[1]))); break;
        case 2: _t->updateAnnotations((*reinterpret_cast< qint64(*)>(_a[1])),(*reinterpret_cast< AnnotationList*(*)>(_a[2])),(*reinterpret_cast< UserColorMap*(*)>(_a[3]))); break;
        case 3: _t->openAnnotationSession((*reinterpret_cast< AnnotationSession*(*)>(_a[1]))); break;
        case 4: _t->closeAnnotationSession(); break;
        case 5: _t->selectEntityById((*reinterpret_cast< const qint64(*)>(_a[1])),(*reinterpret_cast< const bool(*)>(_a[2]))); break;
        case 6: _t->communicationError((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->updateCurrentSample((*reinterpret_cast< Entity*(*)>(_a[1]))); break;
        case 8: _t->openStackWithVaa3d((*reinterpret_cast< Entity*(*)>(_a[1]))); break;
        case 9: _t->ontologySelected((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        case 10: _t->ontologyChanged((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        case 11: _t->entitySelected((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 12: _t->entityDeselected((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 13: _t->entityViewRequested((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        case 14: _t->annotationsChanged((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        case 15: _t->sessionSelected((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        case 16: _t->sessionDeselected(); break;
        case 17: _t->annotatedBranchViewRequested((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        case 18: _t->loadOntologyResults((*reinterpret_cast< const void*(*)>(_a[1]))); break;
        case 19: _t->loadOntologyError((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 20: _t->entityViewRequestedResults((*reinterpret_cast< const void*(*)>(_a[1]))); break;
        case 21: _t->entityViewRequestedError((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 22: _t->annotatedBranchViewRequestedResults((*reinterpret_cast< const void*(*)>(_a[1]))); break;
        case 23: _t->annotatedBranchViewRequestedError((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 24: _t->annotatedBranchViewRequested2Results((*reinterpret_cast< const void*(*)>(_a[1]))); break;
        case 25: _t->annotatedBranchViewRequested2Error((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 26: _t->annotationsChangedResults((*reinterpret_cast< const void*(*)>(_a[1]))); break;
        case 27: _t->annotationsChangedError((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 28: _t->loadAnnotationSessionResults((*reinterpret_cast< const void*(*)>(_a[1]))); break;
        case 29: _t->loadAnnotationSessionError((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData ConsoleObserver::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ConsoleObserver::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_ConsoleObserver,
      qt_meta_data_ConsoleObserver, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ConsoleObserver::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ConsoleObserver::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ConsoleObserver::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ConsoleObserver))
        return static_cast<void*>(const_cast< ConsoleObserver*>(this));
    return QObject::qt_metacast(_clname);
}

int ConsoleObserver::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 30)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 30;
    }
    return _id;
}

// SIGNAL 0
void ConsoleObserver::openOntology(Ontology * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ConsoleObserver::openAnnotatedBranch(AnnotatedBranch * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void ConsoleObserver::updateAnnotations(qint64 _t1, AnnotationList * _t2, UserColorMap * _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void ConsoleObserver::openAnnotationSession(AnnotationSession * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void ConsoleObserver::closeAnnotationSession()
{
    QMetaObject::activate(this, &staticMetaObject, 4, 0);
}

// SIGNAL 5
void ConsoleObserver::selectEntityById(const qint64 & _t1, const bool _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void ConsoleObserver::communicationError(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void ConsoleObserver::updateCurrentSample(Entity * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void ConsoleObserver::openStackWithVaa3d(Entity * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}
QT_END_MOC_NAMESPACE

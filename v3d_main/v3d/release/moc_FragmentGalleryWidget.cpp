/****************************************************************************
** Meta object code from reading C++ file 'FragmentGalleryWidget.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../neuron_annotator/gui/FragmentGalleryWidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'FragmentGalleryWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_FragmentGalleryWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      23,   22,   22,   22, 0x05,

 // slots: signature, parameters, type, tag, flags
      47,   22,   22,   22, 0x0a,
     105,   99,   22,   22, 0x0a,
     125,   22,   22,   22, 0x0a,
     139,   22,   22,   22, 0x0a,
     153,   22,   22,   22, 0x0a,
     166,   22,   22,   22, 0x0a,
     179,   22,   22,   22, 0x0a,
     201,   22,   22,   22, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_FragmentGalleryWidget[] = {
    "FragmentGalleryWidget\0\0scrollPixelChanged(int)\0"
    "scrollToFragment(NeuronSelectionModel::NeuronIndex)\0"
    "pixel\0setScrollPixel(int)\0sortByIndex()\0"
    "sortByColor()\0sortBySize()\0sortByName()\0"
    "updateNameSortTable()\0updateSortTables()\0"
};

void FragmentGalleryWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        FragmentGalleryWidget *_t = static_cast<FragmentGalleryWidget *>(_o);
        switch (_id) {
        case 0: _t->scrollPixelChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->scrollToFragment((*reinterpret_cast< NeuronSelectionModel::NeuronIndex(*)>(_a[1]))); break;
        case 2: _t->setScrollPixel((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->sortByIndex(); break;
        case 4: _t->sortByColor(); break;
        case 5: _t->sortBySize(); break;
        case 6: _t->sortByName(); break;
        case 7: _t->updateNameSortTable(); break;
        case 8: _t->updateSortTables(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData FragmentGalleryWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject FragmentGalleryWidget::staticMetaObject = {
    { &QAbstractScrollArea::staticMetaObject, qt_meta_stringdata_FragmentGalleryWidget,
      qt_meta_data_FragmentGalleryWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &FragmentGalleryWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *FragmentGalleryWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *FragmentGalleryWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_FragmentGalleryWidget))
        return static_cast<void*>(const_cast< FragmentGalleryWidget*>(this));
    return QAbstractScrollArea::qt_metacast(_clname);
}

int FragmentGalleryWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractScrollArea::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void FragmentGalleryWidget::scrollPixelChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE

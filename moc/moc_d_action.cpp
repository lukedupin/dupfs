/****************************************************************************
** Meta object code from reading C++ file 'd_action.h'
**
** Created: Thu Aug 20 00:12:06 2009
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../d_action.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'd_action.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_DAction[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // signals: signature, parameters, type, tag, flags
      40,    9,    8,    8, 0x05,

 // slots: signature, parameters, type, tag, flags
      65,    8,    8,    8, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_DAction[] = {
    "DAction\0\0project_id,hourly_rate_id,name\0"
    "clicked(int,int,QString)\0actionTriggered()\0"
};

const QMetaObject DAction::staticMetaObject = {
    { &QAction::staticMetaObject, qt_meta_stringdata_DAction,
      qt_meta_data_DAction, 0 }
};

const QMetaObject *DAction::metaObject() const
{
    return &staticMetaObject;
}

void *DAction::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DAction))
        return static_cast<void*>(const_cast< DAction*>(this));
    return QAction::qt_metacast(_clname);
}

int DAction::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAction::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: clicked((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 1: actionTriggered(); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void DAction::clicked(int _t1, int _t2, QString _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE

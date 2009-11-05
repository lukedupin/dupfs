/****************************************************************************
** Meta object code from reading C++ file 'network.h'
**
** Created: Thu Aug 20 00:12:04 2009
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../network.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'network.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Network[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // signals: signature, parameters, type, tag, flags
      19,    9,    8,    8, 0x05,

 // slots: signature, parameters, type, tag, flags
      54,   48,    8,    8, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_Network[] = {
    "Network\0\0idx,reply\0finished(int,QNetworkReply*)\0"
    "reply\0replyFinished(QNetworkReply*)\0"
};

const QMetaObject Network::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Network,
      qt_meta_data_Network, 0 }
};

const QMetaObject *Network::metaObject() const
{
    return &staticMetaObject;
}

void *Network::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Network))
        return static_cast<void*>(const_cast< Network*>(this));
    return QObject::qt_metacast(_clname);
}

int Network::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: finished((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QNetworkReply*(*)>(_a[2]))); break;
        case 1: replyFinished((*reinterpret_cast< QNetworkReply*(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void Network::finished(int _t1, QNetworkReply * _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE

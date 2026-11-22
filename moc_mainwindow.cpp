/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.19)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.19. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DepartmentChartWidget_t {
    QByteArrayData data[1];
    char stringdata0[22];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DepartmentChartWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DepartmentChartWidget_t qt_meta_stringdata_DepartmentChartWidget = {
    {
QT_MOC_LITERAL(0, 0, 21) // "DepartmentChartWidget"

    },
    "DepartmentChartWidget"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DepartmentChartWidget[] = {

 // content:
       8,       // revision
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

void DepartmentChartWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject DepartmentChartWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_DepartmentChartWidget.data,
    qt_meta_data_DepartmentChartWidget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *DepartmentChartWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DepartmentChartWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DepartmentChartWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int DepartmentChartWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    return _id;
}
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[20];
    char stringdata0[292];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 19), // "onNavigationChanged"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 9), // "pageIndex"
QT_MOC_LITERAL(4, 42, 8), // "onLogout"
QT_MOC_LITERAL(5, 51, 12), // "onAddFaculty"
QT_MOC_LITERAL(6, 64, 13), // "onEditFaculty"
QT_MOC_LITERAL(7, 78, 15), // "onDeleteFaculty"
QT_MOC_LITERAL(8, 94, 14), // "onAssignCourse"
QT_MOC_LITERAL(9, 109, 18), // "onDeleteAssignment"
QT_MOC_LITERAL(10, 128, 28), // "updateCalculatedHoursPreview"
QT_MOC_LITERAL(11, 157, 15), // "onSubmitRequest"
QT_MOC_LITERAL(12, 173, 16), // "onApproveRequest"
QT_MOC_LITERAL(13, 190, 15), // "onRejectRequest"
QT_MOC_LITERAL(14, 206, 12), // "onCreateUser"
QT_MOC_LITERAL(15, 219, 15), // "onResetPassword"
QT_MOC_LITERAL(16, 235, 12), // "onDeleteUser"
QT_MOC_LITERAL(17, 248, 11), // "onExportCsv"
QT_MOC_LITERAL(18, 260, 10), // "refreshAll"
QT_MOC_LITERAL(19, 271, 20) // "refreshTimetableGrid"

    },
    "MainWindow\0onNavigationChanged\0\0"
    "pageIndex\0onLogout\0onAddFaculty\0"
    "onEditFaculty\0onDeleteFaculty\0"
    "onAssignCourse\0onDeleteAssignment\0"
    "updateCalculatedHoursPreview\0"
    "onSubmitRequest\0onApproveRequest\0"
    "onRejectRequest\0onCreateUser\0"
    "onResetPassword\0onDeleteUser\0onExportCsv\0"
    "refreshAll\0refreshTimetableGrid"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      17,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   99,    2, 0x08 /* Private */,
       4,    0,  102,    2, 0x08 /* Private */,
       5,    0,  103,    2, 0x08 /* Private */,
       6,    0,  104,    2, 0x08 /* Private */,
       7,    0,  105,    2, 0x08 /* Private */,
       8,    0,  106,    2, 0x08 /* Private */,
       9,    0,  107,    2, 0x08 /* Private */,
      10,    0,  108,    2, 0x08 /* Private */,
      11,    0,  109,    2, 0x08 /* Private */,
      12,    0,  110,    2, 0x08 /* Private */,
      13,    0,  111,    2, 0x08 /* Private */,
      14,    0,  112,    2, 0x08 /* Private */,
      15,    0,  113,    2, 0x08 /* Private */,
      16,    0,  114,    2, 0x08 /* Private */,
      17,    0,  115,    2, 0x08 /* Private */,
      18,    0,  116,    2, 0x08 /* Private */,
      19,    0,  117,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onNavigationChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->onLogout(); break;
        case 2: _t->onAddFaculty(); break;
        case 3: _t->onEditFaculty(); break;
        case 4: _t->onDeleteFaculty(); break;
        case 5: _t->onAssignCourse(); break;
        case 6: _t->onDeleteAssignment(); break;
        case 7: _t->updateCalculatedHoursPreview(); break;
        case 8: _t->onSubmitRequest(); break;
        case 9: _t->onApproveRequest(); break;
        case 10: _t->onRejectRequest(); break;
        case 11: _t->onCreateUser(); break;
        case 12: _t->onResetPassword(); break;
        case 13: _t->onDeleteUser(); break;
        case 14: _t->onExportCsv(); break;
        case 15: _t->refreshAll(); break;
        case 16: _t->refreshTimetableGrid(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_MainWindow.data,
    qt_meta_data_MainWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 17)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 17;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 17)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 17;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE

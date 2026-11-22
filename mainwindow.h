#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QButtonGroup>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QTimeEdit>
#include <QTableWidget>
#include <QLabel>
#include <QGroupBox>
#include "database.h"

// Custom QPainter Bar Chart for Department Utilization Analytics
class DepartmentChartWidget : public QWidget {
    Q_OBJECT
public:
    explicit DepartmentChartWidget(QWidget *parent = nullptr);
    void setStats(const QList<DepartmentStat> &stats);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QList<DepartmentStat> m_stats;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(const UserAccount &currentUser, QWidget *parent = nullptr);
    bool isLogoutRequested() const { return m_logoutRequested; }

private slots:
    void onNavigationChanged(int pageIndex);
    void onLogout();

    // Faculty & Scheduling
    void onAddFaculty();
    void onEditFaculty();
    void onDeleteFaculty();
    void onAssignCourse();
    void onDeleteAssignment();
    void updateCalculatedHoursPreview();

    // Requests & Approvals
    void onSubmitRequest();
    void onApproveRequest();
    void onRejectRequest();

    // Admin User Management
    void onCreateUser();
    void onResetPassword();
    void onDeleteUser();

    // Global Refresh & Export
    void onExportCsv();
    void refreshAll();
    void refreshTimetableGrid();

private:
    void setupUi();
    void applyModernTheme();
    void applyRbacPolicies();

    void populateDepartmentFilter();
    void populateFacultyCombos();
    void populateSummaryTable();
    void populateAssignmentsTable();
    void populateRequestsTable();
    void populateUsersTable();
    void populateAuditTable();

    UserAccount m_user;
    bool m_logoutRequested = false;

    // Navigation & Header
    QButtonGroup *navButtonGroup;
    QStackedWidget *pageStack;
    QLabel *pageTitleLabel;
    QComboBox *semesterFilterCombo;
    QComboBox *deptFilterCombo;
    QLineEdit *searchEdit;

    // KPI Dashboard Cards
    QLabel *kpiFacultyValue;
    QLabel *kpiAvgUtilValue;
    QLabel *kpiOverloadValue;
    QLabel *kpiPendingReqValue;
    DepartmentChartWidget *deptChart;

    // Page 1: Faculty Summary & Registration
    QGroupBox *facultyRegGroup;
    QLineEdit *facultyNameEdit;
    QComboBox *departmentCombo;
    QComboBox *designationCombo;
    QDoubleSpinBox *maxHoursSpin;
    QTableWidget *summaryTable;
    QPushButton *editFacultyBtn;
    QPushButton *deleteFacultyBtn;

    // Page 2: Course Scheduler & Log
    QGroupBox *scheduleFormGroup;
    QComboBox *assignFacultyCombo;
    QLineEdit *courseCodeEdit;
    QLineEdit *courseNameEdit;
    QComboBox *activityTypeCombo;
    QComboBox *dayOfWeekCombo;
    QTimeEdit *startTimeEdit;
    QTimeEdit *endTimeEdit;
    QLineEdit *roomEdit;
    QComboBox *semesterCombo;
    QLabel *durationPreviewLabel;
    QTableWidget *assignmentsTable;
    QPushButton *deleteAssignBtn;

    // Page 3: Weekly Timetable
    QComboBox *timetableFacultyCombo;
    QTableWidget *timetableGrid;

    // Page 4: Adjustment Requests
    QComboBox *reqFacultyCombo;
    QComboBox *reqTypeCombo;
    QDoubleSpinBox *reqHoursSpin;
    QTextEdit *reqReasonEdit;
    QTableWidget *requestsTable;
    QPushButton *approveReqBtn;
    QPushButton *rejectReqBtn;

    // Page 5: Admin User Management
    QLineEdit *newUsernameEdit;
    QLineEdit *newPasswordEdit;
    QComboBox *newRoleCombo;
    QComboBox *newUserDeptCombo;
    QComboBox *newUserFacultyCombo;
    QTableWidget *usersTable;

    // Page 6: Admin Audit Logs
    QTableWidget *auditTable;
};

#endif // MAINWINDOW_H
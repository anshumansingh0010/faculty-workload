#ifndef DATABASE_H
#define DATABASE_H

#include <QString>
#include <QList>
#include <QTime>

struct UserAccount {
    int id = -1;
    QString username;
    QString role;         // "Admin", "HOD", "Faculty"
    QString department;   // Used when role == "HOD"
    int facultyId = -1;   // Linked faculty profile ID when role == "Faculty"
    QString linkedFacultyName;
};

struct Faculty {
    int id;
    QString name;
    QString department;
    QString designation;
    double maxHours;
};

struct Assignment {
    int id;
    int facultyId;
    QString facultyName;
    QString department;
    QString courseCode;
    QString courseName;
    QString activityType;
    double weightFactor;
    QString dayOfWeek;
    QTime startTime;
    QTime endTime;
    QString room;
    double rawHours;
    double effectiveHours;
    QString semester;
};

struct WorkloadSummary {
    int facultyId;
    QString name;
    QString department;
    QString designation;
    double maxHours;
    double rawHours;
    double effectiveHours;
    int courseCount;
};

struct DepartmentStat {
    QString department;
    int facultyCount;
    double totalEffectiveHours;
    double totalMaxCapacity;
    double avgUtilization;
};

struct AdjustmentRequest {
    int id;
    int facultyId;
    QString facultyName;
    QString department;
    QString requestType;
    double requestedHours;
    QString reason;
    QString status;       // "Pending", "Approved", "Rejected"
    QString reviewedBy;
    QString createdAt;
};

struct AuditEntry {
    int id;
    QString timestamp;
    QString username;
    QString role;
    QString action;
    QString details;
};

class Database {
public:
    static bool init();
    static void seedInitialData();

    // Authentication & RBAC User Management
    static QString hashPassword(const QString &password, const QString &salt);
    static bool authenticateUser(const QString &username, const QString &password, UserAccount &outUser);
    static bool createUser(const QString &username, const QString &password,
                           const QString &role, const QString &department,
                           int facultyId, QString &errorMsg);
    static bool resetUserPassword(int userId, const QString &newPassword);
    static bool deleteUser(int userId);
    static QList<UserAccount> getAllUsers();

    // Faculty CRUD
    static bool addFaculty(const QString &name, const QString &department,
                           const QString &designation, double maxHours);
    static bool updateFaculty(int facultyId, const QString &name,
                              const QString &department, const QString &designation,
                              double maxHours);
    static bool deleteFaculty(int facultyId);
    static QList<Faculty> getFacultyList(const QString &deptScope = "All Departments",
                                         int specificFacultyId = -1);
    static QStringList getDistinctDepartments();

    // Dual-Resource Clash Detection & Assignments
    static bool checkScheduleConflict(int facultyId, const QString &room,
                                      const QString &semester, const QString &dayOfWeek,
                                      const QTime &startTime, const QTime &endTime,
                                      QString &conflictDetails);

    static bool addAssignment(int facultyId, const QString &courseCode,
                              const QString &courseName, const QString &activityType,
                              double weightFactor, const QString &dayOfWeek,
                              const QTime &startTime, const QTime &endTime,
                              const QString &room, const QString &semester);

    static bool deleteAssignment(int assignmentId);

    static QList<Assignment> getAssignments(const QString &semesterFilter = "All Semesters",
                                            const QString &deptFilter = "All Departments",
                                            const QString &searchQuery = "",
                                            int specificFacultyId = -1);

    static QList<Assignment> getFacultySchedule(int facultyId, const QString &semester);

    // Aggregations & Analytics
    static QList<WorkloadSummary> getWorkloadSummaries(const QString &semesterFilter = "All Semesters",
                                                       const QString &deptFilter = "All Departments",
                                                       const QString &searchQuery = "",
                                                       int specificFacultyId = -1);

    static QList<DepartmentStat> getDepartmentStats(const QString &semesterFilter = "All Semesters",
                                                    const QString &deptFilter = "All Departments");

    // Leave / Workload Adjustment Workflow
    static bool submitAdjustmentRequest(int facultyId, const QString &requestType,
                                        double requestedHours, const QString &reason);
    static bool updateRequestStatus(int requestId, const QString &newStatus,
                                    const QString &reviewerUsername);
    static QList<AdjustmentRequest> getAdjustmentRequests(const QString &deptFilter = "All Departments",
                                                          int specificFacultyId = -1);

    // Audit Logging
    static void logAudit(const QString &username, const QString &role,
                         const QString &action, const QString &details);
    static QList<AuditEntry> getAuditLogs(int limit = 150);
};

#endif // DATABASE_H
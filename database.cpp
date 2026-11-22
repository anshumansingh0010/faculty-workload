#include "database.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>
#include <QUuid>
#include <QDateTime>
#include <QVariant>
#include <QDebug>

bool Database::init() {
    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        QSqlDatabase existing = QSqlDatabase::database();
        if (existing.isOpen()) return true;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("faculty_workload_v3.db");

    if (!db.open()) {
        qCritical() << "Database connection failed:" << db.lastError().text();
        return false;
    }

    QSqlQuery query;
    query.exec("PRAGMA foreign_keys = ON;");

    const QStringList ddlStatements = {
        R"(
        CREATE TABLE IF NOT EXISTS faculty (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            department TEXT NOT NULL,
            designation TEXT NOT NULL,
            max_hours REAL NOT NULL
        );
        )",
        R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            salt TEXT NOT NULL,
            role TEXT NOT NULL,
            department TEXT,
            faculty_id INTEGER,
            FOREIGN KEY (faculty_id) REFERENCES faculty(id) ON DELETE SET NULL
        );
        )",
        R"(
        CREATE TABLE IF NOT EXISTS assignments (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            faculty_id INTEGER NOT NULL,
            course_code TEXT NOT NULL,
            course_name TEXT NOT NULL,
            activity_type TEXT NOT NULL,
            weight_factor REAL NOT NULL,
            day_of_week TEXT NOT NULL,
            start_time TEXT NOT NULL,
            end_time TEXT NOT NULL,
            room TEXT NOT NULL,
            raw_hours REAL NOT NULL,
            effective_hours REAL NOT NULL,
            semester TEXT NOT NULL,
            FOREIGN KEY (faculty_id) REFERENCES faculty(id) ON DELETE CASCADE
        );
        )",
        R"(
        CREATE TABLE IF NOT EXISTS adjustment_requests (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            faculty_id INTEGER NOT NULL,
            request_type TEXT NOT NULL,
            requested_hours REAL NOT NULL,
            reason TEXT NOT NULL,
            status TEXT NOT NULL DEFAULT 'Pending',
            reviewed_by TEXT,
            created_at TEXT NOT NULL,
            FOREIGN KEY (faculty_id) REFERENCES faculty(id) ON DELETE CASCADE
        );
        )",
        R"(
        CREATE TABLE IF NOT EXISTS audit_logs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp TEXT NOT NULL,
            username TEXT NOT NULL,
            role TEXT NOT NULL,
            action TEXT NOT NULL,
            details TEXT NOT NULL
        );
        )"
    };

    for (const QString &ddl : ddlStatements) {
        if (!query.exec(ddl)) {
            qCritical() << "Schema initialization error:" << query.lastError().text();
            return false;
        }
    }

    seedInitialData();
    return true;
}

QString Database::hashPassword(const QString &password, const QString &salt) {
    QByteArray combined = (salt + password).toUtf8();
    return QString(QCryptographicHash::hash(combined, QCryptographicHash::Sha256).toHex());
}

void Database::seedInitialData() {
    QSqlQuery checkUsers("SELECT COUNT(*) FROM users");
    if (checkUsers.next() && checkUsers.value(0).toInt() > 0) {
        return; // Already initialized
    }

    // 1. Seed Faculty Members
    addFaculty("Dr. Ananya Sharma", "Computer Science", "Professor", 14.0);
    addFaculty("Prof. Rajesh Verma", "Computer Science", "Associate Professor", 16.0);
    addFaculty("Dr. Meera Nair", "Electronics", "Assistant Professor", 16.0);
    addFaculty("Dr. Vikram Singh", "Mathematics", "Professor", 12.0);
    addFaculty("Prof. Kavita Rao", "Mechanical", "Associate Professor", 15.0);

    const auto faculty = getFacultyList();
    int idAnanya = -1, idRajesh = -1, idMeera = -1, idVikram = -1, idKavita = -1;
    for (const auto &f : faculty) {
        if (f.name.contains("Ananya")) idAnanya = f.id;
        else if (f.name.contains("Rajesh")) idRajesh = f.id;
        else if (f.name.contains("Meera")) idMeera = f.id;
        else if (f.name.contains("Vikram")) idVikram = f.id;
        else if (f.name.contains("Kavita")) idKavita = f.id;
    }

    // 2. Seed RBAC User Accounts
    QString err;
    createUser("admin", "admin123", "Admin", "All Departments", -1, err);
    createUser("hod_cs", "hod123", "HOD", "Computer Science", -1, err);
    createUser("ananya", "fac123", "Faculty", "Computer Science", idAnanya, err);

    // 3. Seed Sample Course Assignments
    if (idAnanya != -1) {
        addAssignment(idAnanya, "CS401", "Machine Learning", "Lecture", 1.0,
                      "Monday", QTime(9, 0), QTime(11, 0), "LH-101", "Fall 2026");
        addAssignment(idAnanya, "CS401L", "Machine Learning Lab", "Lab", 0.75,
                      "Wednesday", QTime(14, 0), QTime(17, 0), "AI-Lab-2", "Fall 2026");
        addAssignment(idAnanya, "CS-IQAC", "Curriculum & Accreditation", "Admin/Research", 1.0,
                      "Friday", QTime(10, 0), QTime(12, 0), "Conf-Room-B", "Fall 2026");
    }

    if (idRajesh != -1) {
        addAssignment(idRajesh, "CS302", "Operating Systems", "Lecture", 1.0,
                      "Tuesday", QTime(9, 0), QTime(12, 0), "LH-204", "Fall 2026");
        addAssignment(idRajesh, "CS302L", "OS Kernel Lab", "Lab", 0.75,
                      "Thursday", QTime(13, 0), QTime(17, 0), "Sys-Lab-1", "Fall 2026");
        addAssignment(idRajesh, "CS510", "Distributed Systems", "Lecture", 1.0,
                      "Friday", QTime(9, 0), QTime(12, 0), "LH-301", "Fall 2026");
        addAssignment(idRajesh, "CS512", "Cloud Architecture", "Lecture", 1.0,
                      "Monday", QTime(13, 0), QTime(17, 0), "LH-302", "Fall 2026");
        addAssignment(idRajesh, "CS-PROJ", "Capstone Supervision", "Admin/Research", 1.0,
                      "Wednesday", QTime(10, 0), QTime(14, 0), "Lab-4", "Fall 2026");
    }

    if (idMeera != -1) {
        addAssignment(idMeera, "EC205", "Digital Signal Processing", "Lecture", 1.0,
                      "Monday", QTime(10, 0), QTime(13, 0), "EC-102", "Fall 2026");
        addAssignment(idMeera, "EC205L", "DSP Hardware Lab", "Lab", 0.75,
                      "Thursday", QTime(9, 0), QTime(13, 0), "DSP-Lab", "Fall 2026");
    }

    if (idVikram != -1) {
        addAssignment(idVikram, "MA102", "Linear Algebra & Matrices", "Lecture", 1.0,
                      "Tuesday", QTime(10, 0), QTime(13, 0), "Main-Hall-A", "Fall 2026");
        addAssignment(idVikram, "MA204", "Numerical Optimization", "Lecture", 1.0,
                      "Thursday", QTime(10, 0), QTime(13, 0), "Main-Hall-A", "Fall 2026");
    }

    if (idKavita != -1) {
        addAssignment(idKavita, "ME301", "Thermodynamics II", "Lecture", 1.0,
                      "Wednesday", QTime(9, 0), QTime(12, 0), "ME-201", "Fall 2026");
    }

    // 4. Seed Sample Adjustment Request
    if (idAnanya != -1) {
        submitAdjustmentRequest(idAnanya, "Conference / Academic Leave", 2.0,
                                "Presenting keynote paper at IEEE ICML Symposium (Nov 2026).");
    }

    logAudit("SYSTEM", "System", "INIT", "Initialized v3.0 database with RBAC roles and sample schedules.");
}

bool Database::authenticateUser(const QString &username, const QString &password, UserAccount &outUser) {
    QSqlQuery query;
    query.prepare(R"(
        SELECT u.id, u.username, u.password_hash, u.salt, u.role, u.department, u.faculty_id, f.name
        FROM users u
        LEFT JOIN faculty f ON u.faculty_id = f.id
        WHERE LOWER(u.username) = LOWER(:uname)
    )");
    query.bindValue(":uname", username.trimmed());

    if (!query.exec() || !query.next()) {
        logAudit(username.trimmed(), "Unknown", "LOGIN_FAILED", "Invalid username attempt.");
        return false;
    }

    QString storedHash = query.value(2).toString();
    QString salt = query.value(3).toString();
    QString computedHash = hashPassword(password, salt);

    if (storedHash != computedHash) {
        logAudit(username.trimmed(), query.value(4).toString(), "LOGIN_FAILED", "Incorrect password.");
        return false;
    }

    outUser.id = query.value(0).toInt();
    outUser.username = query.value(1).toString();
    outUser.role = query.value(4).toString();
    outUser.department = query.value(5).toString();
    outUser.facultyId = query.value(6).isNull() ? -1 : query.value(6).toInt();
    outUser.linkedFacultyName = query.value(7).toString();

    logAudit(outUser.username, outUser.role, "LOGIN_SUCCESS", "User authenticated successfully.");
    return true;
}

bool Database::createUser(const QString &username, const QString &password,
                          const QString &role, const QString &department,
                          int facultyId, QString &errorMsg) {
    if (username.trimmed().isEmpty() || password.length() < 4) {
        errorMsg = "Username is required and password must be at least 4 characters.";
        return false;
    }

    QString salt = QUuid::createUuid().toString(QUuid::WithoutBraces).left(16);
    QString passHash = hashPassword(password, salt);

    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO users (username, password_hash, salt, role, department, faculty_id)
        VALUES (:uname, :phash, :salt, :role, :dept, :fid)
    )");
    query.bindValue(":uname", username.trimmed());
    query.bindValue(":phash", passHash);
    query.bindValue(":salt", salt);
    query.bindValue(":role", role);
    query.bindValue(":dept", department);
    if (facultyId > 0) {
        query.bindValue(":fid", facultyId);
    } else {
        query.bindValue(":fid", QVariant());
    }

    if (!query.exec()) {
        errorMsg = "Username already exists or database constraint failed.";
        return false;
    }
    return true;
}

bool Database::resetUserPassword(int userId, const QString &newPassword) {
    QString salt = QUuid::createUuid().toString(QUuid::WithoutBraces).left(16);
    QString passHash = hashPassword(newPassword, salt);

    QSqlQuery query;
    query.prepare("UPDATE users SET password_hash = :phash, salt = :salt WHERE id = :id");
    query.bindValue(":phash", passHash);
    query.bindValue(":salt", salt);
    query.bindValue(":id", userId);
    return query.exec();
}

bool Database::deleteUser(int userId) {
    QSqlQuery query;
    query.prepare("DELETE FROM users WHERE id = :id");
    query.bindValue(":id", userId);
    return query.exec();
}

QList<UserAccount> Database::getAllUsers() {
    QList<UserAccount> list;
    QSqlQuery query(R"(
        SELECT u.id, u.username, u.role, u.department, u.faculty_id, f.name
        FROM users u
        LEFT JOIN faculty f ON u.faculty_id = f.id
        ORDER BY u.role ASC, u.username ASC
    )");
    while (query.next()) {
        UserAccount u;
        u.id = query.value(0).toInt();
        u.username = query.value(1).toString();
        u.role = query.value(2).toString();
        u.department = query.value(3).toString();
        u.facultyId = query.value(4).isNull() ? -1 : query.value(4).toInt();
        u.linkedFacultyName = query.value(5).toString();
        list.append(u);
    }
    return list;
}

bool Database::addFaculty(const QString &name, const QString &department,
                          const QString &designation, double maxHours) {
    QSqlQuery query;
    query.prepare("INSERT INTO faculty (name, department, designation, max_hours) "
                  "VALUES (:name, :dept, :desig, :max_hours)");
    query.bindValue(":name", name);
    query.bindValue(":dept", department);
    query.bindValue(":desig", designation);
    query.bindValue(":max_hours", maxHours);
    return query.exec();
}

bool Database::updateFaculty(int facultyId, const QString &name,
                             const QString &department, const QString &designation,
                             double maxHours) {
    QSqlQuery query;
    query.prepare("UPDATE faculty SET name = :name, department = :dept, "
                  "designation = :desig, max_hours = :max_hours WHERE id = :id");
    query.bindValue(":name", name);
    query.bindValue(":dept", department);
    query.bindValue(":desig", designation);
    query.bindValue(":max_hours", maxHours);
    query.bindValue(":id", facultyId);
    return query.exec();
}

bool Database::deleteFaculty(int facultyId) {
    QSqlQuery query;
    query.exec("PRAGMA foreign_keys = ON;");
    query.prepare("DELETE FROM faculty WHERE id = :id");
    query.bindValue(":id", facultyId);
    return query.exec();
}

QList<Faculty> Database::getFacultyList(const QString &deptScope, int specificFacultyId) {
    QList<Faculty> list;
    QString sql = "SELECT id, name, department, designation, max_hours FROM faculty WHERE 1=1";
    if (deptScope != "All Departments") {
        sql += " AND department = :dept";
    }
    if (specificFacultyId > 0) {
        sql += " AND id = :fid";
    }
    sql += " ORDER BY name ASC";

    QSqlQuery query;
    query.prepare(sql);
    if (deptScope != "All Departments") query.bindValue(":dept", deptScope);
    if (specificFacultyId > 0) query.bindValue(":fid", specificFacultyId);

    if (query.exec()) {
        while (query.next()) {
            Faculty f;
            f.id = query.value(0).toInt();
            f.name = query.value(1).toString();
            f.department = query.value(2).toString();
            f.designation = query.value(3).toString();
            f.maxHours = query.value(4).toDouble();
            list.append(f);
        }
    }
    return list;
}

QStringList Database::getDistinctDepartments() {
    QStringList depts;
    QSqlQuery query("SELECT DISTINCT department FROM faculty ORDER BY department ASC");
    while (query.next()) {
        depts << query.value(0).toString();
    }
    return depts;
}

bool Database::checkScheduleConflict(int facultyId, const QString &room,
                                     const QString &semester, const QString &dayOfWeek,
                                     const QTime &startTime, const QTime &endTime,
                                     QString &conflictDetails) {
    QSqlQuery query;
    query.prepare(R"(
        SELECT a.faculty_id, f.name, a.course_code, a.course_name, a.start_time, a.end_time, a.room
        FROM assignments a
        JOIN faculty f ON a.faculty_id = f.id
        WHERE a.semester = :sem AND a.day_of_week = :day
    )");
    query.bindValue(":sem", semester);
    query.bindValue(":day", dayOfWeek);

    if (!query.exec()) return false;

    const QString cleanRoom = room.trimmed();

    while (query.next()) {
        int existFid = query.value(0).toInt();
        QString existFacName = query.value(1).toString();
        QString existCode = query.value(2).toString();
        QString existTitle = query.value(3).toString();
        QTime existStart = QTime::fromString(query.value(4).toString(), "HH:mm");
        QTime existEnd = QTime::fromString(query.value(5).toString(), "HH:mm");
        QString existRoom = query.value(6).toString().trimmed();

        bool timeOverlaps = (startTime < existEnd && existStart < endTime);
        if (!timeOverlaps) continue;

        // 1. Check Faculty Double-Booking
        if (existFid == facultyId) {
            conflictDetails = QString("[Faculty Clash] %1 is already teaching %2 (%3) from %4 to %5 in %6.")
                                  .arg(existFacName, existCode, existTitle,
                                       existStart.toString("HH:mm"), existEnd.toString("HH:mm"), existRoom);
            return true;
        }

        // 2. Check Classroom / Lab Double-Booking
        if (!cleanRoom.isEmpty() && cleanRoom.compare("TBD", Qt::CaseInsensitive) != 0 &&
            existRoom.compare(cleanRoom, Qt::CaseInsensitive) == 0) {
            conflictDetails = QString("[Room Clash] Room '%1' is already booked by %2 for %3 (%4) from %5 to %6.")
                                  .arg(existRoom, existFacName, existCode, existTitle,
                                       existStart.toString("HH:mm"), existEnd.toString("HH:mm"));
            return true;
        }
    }
    return false;
}

bool Database::addAssignment(int facultyId, const QString &courseCode,
                             const QString &courseName, const QString &activityType,
                             double weightFactor, const QString &dayOfWeek,
                             const QTime &startTime, const QTime &endTime,
                             const QString &room, const QString &semester) {
    double rawHours = startTime.secsTo(endTime) / 3600.0;
    double effectiveHours = rawHours * weightFactor;

    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO assignments (
            faculty_id, course_code, course_name, activity_type, weight_factor,
            day_of_week, start_time, end_time, room, raw_hours, effective_hours, semester
        ) VALUES (
            :fid, :code, :cname, :atype, :weight,
            :day, :stime, :etime, :room, :raw, :eff, :sem
        )
    )");
    query.bindValue(":fid", facultyId);
    query.bindValue(":code", courseCode);
    query.bindValue(":cname", courseName);
    query.bindValue(":atype", activityType);
    query.bindValue(":weight", weightFactor);
    query.bindValue(":day", dayOfWeek);
    query.bindValue(":stime", startTime.toString("HH:mm"));
    query.bindValue(":etime", endTime.toString("HH:mm"));
    query.bindValue(":room", room);
    query.bindValue(":raw", rawHours);
    query.bindValue(":eff", effectiveHours);
    query.bindValue(":sem", semester);

    return query.exec();
}

bool Database::deleteAssignment(int assignmentId) {
    QSqlQuery query;
    query.prepare("DELETE FROM assignments WHERE id = :id");
    query.bindValue(":id", assignmentId);
    return query.exec();
}

QList<Assignment> Database::getAssignments(const QString &semesterFilter,
                                           const QString &deptFilter,
                                           const QString &searchQuery,
                                           int specificFacultyId) {
    QList<Assignment> list;
    QString sql = R"(
        SELECT a.id, a.faculty_id, f.name, f.department, a.course_code, a.course_name,
               a.activity_type, a.weight_factor, a.day_of_week, a.start_time, a.end_time,
               a.room, a.raw_hours, a.effective_hours, a.semester
        FROM assignments a
        JOIN faculty f ON a.faculty_id = f.id
        WHERE 1=1
    )";

    if (semesterFilter != "All Semesters") sql += " AND a.semester = :sem";
    if (deptFilter != "All Departments") sql += " AND f.department = :dept";
    if (specificFacultyId > 0) sql += " AND a.faculty_id = :fid";
    if (!searchQuery.trimmed().isEmpty()) {
        sql += " AND (f.name LIKE :search OR a.course_code LIKE :search OR a.course_name LIKE :search OR a.room LIKE :search)";
    }

    sql += R"(
        ORDER BY f.name ASC,
        CASE a.day_of_week
            WHEN 'Monday' THEN 1 WHEN 'Tuesday' THEN 2 WHEN 'Wednesday' THEN 3
            WHEN 'Thursday' THEN 4 WHEN 'Friday' THEN 5 WHEN 'Saturday' THEN 6 ELSE 7
        END,
        a.start_time ASC
    )";

    QSqlQuery query;
    query.prepare(sql);
    if (semesterFilter != "All Semesters") query.bindValue(":sem", semesterFilter);
    if (deptFilter != "All Departments") query.bindValue(":dept", deptFilter);
    if (specificFacultyId > 0) query.bindValue(":fid", specificFacultyId);
    if (!searchQuery.trimmed().isEmpty()) query.bindValue(":search", "%" + searchQuery.trimmed() + "%");

    if (query.exec()) {
        while (query.next()) {
            Assignment a;
            a.id = query.value(0).toInt();
            a.facultyId = query.value(1).toInt();
            a.facultyName = query.value(2).toString();
            a.department = query.value(3).toString();
            a.courseCode = query.value(4).toString();
            a.courseName = query.value(5).toString();
            a.activityType = query.value(6).toString();
            a.weightFactor = query.value(7).toDouble();
            a.dayOfWeek = query.value(8).toString();
            a.startTime = QTime::fromString(query.value(9).toString(), "HH:mm");
            a.endTime = QTime::fromString(query.value(10).toString(), "HH:mm");
            a.room = query.value(11).toString();
            a.rawHours = query.value(12).toDouble();
            a.effectiveHours = query.value(13).toDouble();
            a.semester = query.value(14).toString();
            list.append(a);
        }
    }
    return list;
}

QList<Assignment> Database::getFacultySchedule(int facultyId, const QString &semester) {
    return getAssignments(semester, "All Departments", "", facultyId);
}

QList<WorkloadSummary> Database::getWorkloadSummaries(const QString &semesterFilter,
                                                      const QString &deptFilter,
                                                      const QString &searchQuery,
                                                      int specificFacultyId) {
    QList<WorkloadSummary> list;

    QString joinCond = "f.id = a.faculty_id";
    if (semesterFilter != "All Semesters") {
        joinCond += " AND a.semester = :sem";
    }

    QString sql = QString(R"(
        SELECT f.id, f.name, f.department, f.designation, f.max_hours,
               COALESCE(SUM(a.raw_hours), 0),
               COALESCE(SUM(a.effective_hours), 0),
               COUNT(a.id)
        FROM faculty f
        LEFT JOIN assignments a ON %1
        WHERE 1=1
    )").arg(joinCond);

    if (deptFilter != "All Departments") sql += " AND f.department = :dept";
    if (specificFacultyId > 0) sql += " AND f.id = :fid";
    if (!searchQuery.trimmed().isEmpty()) {
        sql += " AND (f.name LIKE :search OR f.department LIKE :search OR f.designation LIKE :search)";
    }

    sql += " GROUP BY f.id ORDER BY f.name ASC";

    QSqlQuery query;
    query.prepare(sql);
    if (semesterFilter != "All Semesters") query.bindValue(":sem", semesterFilter);
    if (deptFilter != "All Departments") query.bindValue(":dept", deptFilter);
    if (specificFacultyId > 0) query.bindValue(":fid", specificFacultyId);
    if (!searchQuery.trimmed().isEmpty()) query.bindValue(":search", "%" + searchQuery.trimmed() + "%");

    if (query.exec()) {
        while (query.next()) {
            WorkloadSummary ws;
            ws.facultyId = query.value(0).toInt();
            ws.name = query.value(1).toString();
            ws.department = query.value(2).toString();
            ws.designation = query.value(3).toString();
            ws.maxHours = query.value(4).toDouble();
            ws.rawHours = query.value(5).toDouble();
            ws.effectiveHours = query.value(6).toDouble();
            ws.courseCount = query.value(7).toInt();
            list.append(ws);
        }
    }
    return list;
}

QList<DepartmentStat> Database::getDepartmentStats(const QString &semesterFilter,
                                                   const QString &deptFilter) {
    QList<DepartmentStat> stats;
    const auto summaries = getWorkloadSummaries(semesterFilter, deptFilter, "", -1);

    QMap<QString, DepartmentStat> map;
    for (const auto &ws : summaries) {
        auto &st = map[ws.department];
        st.department = ws.department;
        st.facultyCount += 1;
        st.totalEffectiveHours += ws.effectiveHours;
        st.totalMaxCapacity += ws.maxHours;
    }

    for (auto it = map.begin(); it != map.end(); ++it) {
        DepartmentStat st = it.value();
        st.avgUtilization = (st.totalMaxCapacity > 0)
                                ? (st.totalEffectiveHours / st.totalMaxCapacity) * 100.0
                                : 0.0;
        stats.append(st);
    }
    return stats;
}

bool Database::submitAdjustmentRequest(int facultyId, const QString &requestType,
                                       double requestedHours, const QString &reason) {
    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO adjustment_requests (faculty_id, request_type, requested_hours, reason, status, created_at)
        VALUES (:fid, :rtype, :hrs, :reason, 'Pending', :ts)
    )");
    query.bindValue(":fid", facultyId);
    query.bindValue(":rtype", requestType);
    query.bindValue(":hrs", requestedHours);
    query.bindValue(":reason", reason);
    query.bindValue(":ts", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm"));
    return query.exec();
}

bool Database::updateRequestStatus(int requestId, const QString &newStatus,
                                   const QString &reviewerUsername) {
    QSqlQuery query;
    query.prepare("UPDATE adjustment_requests SET status = :st, reviewed_by = :rev WHERE id = :id");
    query.bindValue(":st", newStatus);
    query.bindValue(":rev", reviewerUsername);
    query.bindValue(":id", requestId);
    return query.exec();
}

QList<AdjustmentRequest> Database::getAdjustmentRequests(const QString &deptFilter,
                                                         int specificFacultyId) {
    QList<AdjustmentRequest> list;
    QString sql = R"(
        SELECT r.id, r.faculty_id, f.name, f.department, r.request_type,
               r.requested_hours, r.reason, r.status, COALESCE(r.reviewed_by, '-'), r.created_at
        FROM adjustment_requests r
        JOIN faculty f ON r.faculty_id = f.id
        WHERE 1=1
    )";
    if (deptFilter != "All Departments") sql += " AND f.department = :dept";
    if (specificFacultyId > 0) sql += " AND r.faculty_id = :fid";
    sql += " ORDER BY r.id DESC";

    QSqlQuery query;
    query.prepare(sql);
    if (deptFilter != "All Departments") query.bindValue(":dept", deptFilter);
    if (specificFacultyId > 0) query.bindValue(":fid", specificFacultyId);

    if (query.exec()) {
        while (query.next()) {
            AdjustmentRequest req;
            req.id = query.value(0).toInt();
            req.facultyId = query.value(1).toInt();
            req.facultyName = query.value(2).toString();
            req.department = query.value(3).toString();
            req.requestType = query.value(4).toString();
            req.requestedHours = query.value(5).toDouble();
            req.reason = query.value(6).toString();
            req.status = query.value(7).toString();
            req.reviewedBy = query.value(8).toString();
            req.createdAt = query.value(9).toString();
            list.append(req);
        }
    }
    return list;
}

void Database::logAudit(const QString &username, const QString &role,
                        const QString &action, const QString &details) {
    QSqlQuery query;
    query.prepare("INSERT INTO audit_logs (timestamp, username, role, action, details) "
                  "VALUES (:ts, :uname, :role, :act, :det)");
    query.bindValue(":ts", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    query.bindValue(":uname", username);
    query.bindValue(":role", role);
    query.bindValue(":act", action);
    query.bindValue(":det", details);
    query.exec();
}

QList<AuditEntry> Database::getAuditLogs(int limit) {
    QList<AuditEntry> logs;
    QSqlQuery query;
    query.prepare("SELECT id, timestamp, username, role, action, details "
                  "FROM audit_logs ORDER BY id DESC LIMIT :lim");
    query.bindValue(":lim", limit);
    if (query.exec()) {
        while (query.next()) {
            AuditEntry e;
            e.id = query.value(0).toInt();
            e.timestamp = query.value(1).toString();
            e.username = query.value(2).toString();
            e.role = query.value(3).toString();
            e.action = query.value(4).toString();
            e.details = query.value(5).toString();
            logs.append(e);
        }
    }
    return logs;
}
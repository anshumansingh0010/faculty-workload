#include "mainwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFrame>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QProgressBar>
#include <QDialog>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QPainter>
#include <QPainterPath>

// ============================================================================
// CUSTOM QPAINTER DEPARTMENT ANALYTICS CHART
// ============================================================================

DepartmentChartWidget::DepartmentChartWidget(QWidget *parent) : QWidget(parent) {
    setMinimumHeight(185);
}

void DepartmentChartWidget::setStats(const QList<DepartmentStat> &stats) {
    m_stats = stats;
    update();
}

void DepartmentChartWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    p.fillRect(rect(), Qt::white);

    if (m_stats.isEmpty()) {
        p.setPen(QColor("#64748B"));
        p.drawText(rect(), Qt::AlignCenter, "No departmental workload data available for current filter.");
        return;
    }

    const int leftMargin = 145;
    const int rightMargin = 85;
    const int topMargin = 28;
    const int rowHeight = 32;
    const int barHeight = 18;
    const int chartWidth = width() - leftMargin - rightMargin;

    if (chartWidth <= 50) return;

    // Title & Legend
    p.setPen(QColor("#0F172A"));
    QFont titleFont = p.font();
    titleFont.setBold(true);
    titleFont.setPointSize(10);
    p.setFont(titleFont);
    p.drawText(12, 18, "Department Capacity Utilization (%) — Dashed Line = 100% Cap");

    // 100% Reference Line (at 80% of track width so >100% overload is visible)
    const double maxScale = 125.0;
    int capX = leftMargin + static_cast<int>((100.0 / maxScale) * chartWidth);
    QPen capPen(QColor("#DC2626"), 1.5, Qt::DashLine);
    p.setPen(capPen);
    p.drawLine(capX, topMargin - 4, capX, topMargin + m_stats.size() * rowHeight);

    QFont labelFont = p.font();
    labelFont.setBold(false);
    labelFont.setPointSize(9);
    p.setFont(labelFont);

    for (int i = 0; i < m_stats.size(); ++i) {
        const auto &st = m_stats[i];
        int y = topMargin + i * rowHeight;

        // Department Label
        p.setPen(QColor("#334155"));
        p.drawText(QRect(10, y, leftMargin - 18, barHeight + 4),
                   Qt::AlignVCenter | Qt::AlignRight, st.department);

        // Background Track
        QRect trackRect(leftMargin, y + 2, chartWidth, barHeight);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#F1F5F9"));
        p.drawRoundedRect(trackRect, 5, 5);

        // Filled Bar
        double clampedPct = qMin(maxScale, st.avgUtilization);
        int fillW = static_cast<int>((clampedPct / maxScale) * chartWidth);
        QColor barColor = (st.avgUtilization > 100.0) ? QColor("#DC2626")
                        : (st.avgUtilization >= 80.0) ? QColor("#16A34A")
                                                      : QColor("#2563EB");
        if (fillW > 0) {
            QRect fillRect(leftMargin, y + 2, fillW, barHeight);
            p.setBrush(barColor);
            p.drawRoundedRect(fillRect, 5, 5);
        }

        // Value Text
        p.setPen(QColor("#0F172A"));
        p.drawText(QRect(leftMargin + chartWidth + 8, y, rightMargin - 10, barHeight + 4),
                   Qt::AlignVCenter | Qt::AlignLeft,
                   QString("%1%").arg(st.avgUtilization, 0, 'f', 1));
    }
}

// ============================================================================
// MAIN WINDOW
// ============================================================================

MainWindow::MainWindow(const UserAccount &currentUser, QWidget *parent)
    : QMainWindow(parent), m_user(currentUser) {
    applyModernTheme();
    setupUi();
    applyRbacPolicies();
    updateCalculatedHoursPreview();
    refreshAll();
}

void MainWindow::applyModernTheme() {
    setStyleSheet(R"(
        QMainWindow { background-color: #F1F5F9; }
        QFrame#Sidebar {
            background-color: #0F172A;
            border-right: 1px solid #1E293B;
        }
        QPushButton.NavBtn {
            text-align: left;
            padding: 11px 16px;
            border: none;
            border-radius: 8px;
            color: #CBD5E1;
            font-size: 13px;
            font-weight: bold;
            background-color: transparent;
        }
        QPushButton.NavBtn:hover {
            background-color: #1E293B;
            color: #FFFFFF;
        }
        QPushButton.NavBtn:checked {
            background-color: #2563EB;
            color: #FFFFFF;
        }
        QFrame.Card, QGroupBox {
            background-color: #FFFFFF;
            border: 1px solid #E2E8F0;
            border-radius: 10px;
            font-weight: bold;
            color: #0F172A;
        }
        QGroupBox {
            margin-top: 12px;
            padding-top: 16px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 14px;
            padding: 0 6px;
            color: #1E293B;
            font-size: 13px;
        }
        QLineEdit, QComboBox, QDoubleSpinBox, QTimeEdit, QTextEdit {
            padding: 6px 9px;
            border: 1px solid #CBD5E1;
            border-radius: 6px;
            background-color: #F8FAFC;
            color: #0F172A;
            font-weight: normal;
        }
        QLineEdit:focus, QComboBox:focus, QDoubleSpinBox:focus, QTimeEdit:focus, QTextEdit:focus {
            border: 1.5px solid #2563EB;
            background-color: #FFFFFF;
        }
        QTableWidget {
            background-color: #FFFFFF;
            border: 1px solid #E2E8F0;
            border-radius: 8px;
            gridline-color: #F1F5F9;
            selection-background-color: #DBEAFE;
            selection-color: #1E3A8A;
        }
        QHeaderView::section {
            background-color: #F8FAFC;
            color: #334155;
            font-weight: bold;
            padding: 8px;
            border: none;
            border-bottom: 1.5px solid #E2E8F0;
        }
        QPushButton.PrimaryBtn {
            background-color: #2563EB;
            color: #FFFFFF;
            border: none;
            border-radius: 6px;
            padding: 8px 14px;
            font-weight: bold;
        }
        QPushButton.PrimaryBtn:hover { background-color: #1D4ED8; }
        QPushButton.SuccessBtn {
            background-color: #16A34A;
            color: #FFFFFF;
            border: none;
            border-radius: 6px;
            padding: 8px 14px;
            font-weight: bold;
        }
        QPushButton.SuccessBtn:hover { background-color: #15803D; }
        QPushButton.DangerBtn {
            background-color: #FEE2E2;
            color: #B91C1C;
            border: 1px solid #FCA5A5;
            border-radius: 6px;
            padding: 7px 12px;
            font-weight: bold;
        }
        QPushButton.DangerBtn:hover { background-color: #FECACA; }
        QPushButton.SecondaryBtn {
            background-color: #F1F5F9;
            color: #1E293B;
            border: 1px solid #CBD5E1;
            border-radius: 6px;
            padding: 7px 12px;
            font-weight: bold;
        }
        QPushButton.SecondaryBtn:hover { background-color: #E2E8F0; }
    )");
}

void MainWindow::setupUi() {
    setWindowTitle("Academic Workload & Timetable ERP v3.0");
    resize(1360, 820);

    auto *central = new QWidget(this);
    setCentralWidget(central);

    auto *rootLayout = new QHBoxLayout(central);
    rootLayout->setSpacing(0);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    // ========================================================================
    // LEFT SIDEBAR NAVIGATION
    // ========================================================================
    auto *sidebar = new QFrame();
    sidebar->setObjectName("Sidebar");
    sidebar->setFixedWidth(235);
    auto *sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(14, 20, 14, 18);
    sideLayout->setSpacing(6);

    auto *brandLabel = new QLabel("ACADEMIC ERP");
    brandLabel->setStyleSheet("color: #38BDF8; font-size: 16px; font-weight: bold; letter-spacing: 1px;");

    QString roleBadgeText = m_user.role.toUpper();
    if (m_user.role == "HOD") {
        roleBadgeText += " • " + m_user.department;
    }
    auto *roleBadge = new QLabel(roleBadgeText);
    roleBadge->setStyleSheet(
        "background-color: #1E293B; color: #93C5FD; padding: 4px 8px; "
        "border-radius: 5px; font-size: 11px; font-weight: bold;"
    );

    sideLayout->addWidget(brandLabel);
    sideLayout->addWidget(roleBadge);
    sideLayout->addSpacing(16);

    navButtonGroup = new QButtonGroup(this);
    navButtonGroup->setExclusive(true);

    auto addNavButton = [&](const QString &text, int index) -> QPushButton* {
        auto *btn = new QPushButton(text);
        btn->setProperty("class", "NavBtn");
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        navButtonGroup->addButton(btn, index);
        sideLayout->addWidget(btn);
        return btn;
    };

    auto *btnDash = addNavButton("1. Dashboard & Summary", 0);
    addNavButton("2. Course Scheduler", 1);
    addNavButton("3. Weekly Timetable", 2);
    addNavButton("4. Leave & Load Requests", 3);
    auto *btnUsers = addNavButton("5. User Management", 4);
    auto *btnAudit = addNavButton("6. System Audit Logs", 5);

    if (m_user.role != "Admin") {
        btnUsers->hide();
        btnAudit->hide();
    }

    btnDash->setChecked(true);
    sideLayout->addStretch();

    // Logged-In User Profile Card
    auto *userCard = new QFrame();
    userCard->setStyleSheet("background-color: #1E293B; border-radius: 8px; padding: 8px;");
    auto *userCardLayout = new QVBoxLayout(userCard);
    userCardLayout->setSpacing(4);

    QString displayName = m_user.linkedFacultyName.isEmpty() ? m_user.username : m_user.linkedFacultyName;
    auto *userNameLbl = new QLabel(displayName);
    userNameLbl->setStyleSheet("color: #F8FAFC; font-weight: bold; font-size: 12px;");
    userNameLbl->setWordWrap(true);

    auto *userSubLbl = new QLabel("User: @" + m_user.username);
    userSubLbl->setStyleSheet("color: #94A3B8; font-size: 11px;");

    auto *logoutBtn = new QPushButton("Logout / Switch Role");
    logoutBtn->setCursor(Qt::PointingHandCursor);
    logoutBtn->setStyleSheet(
        "QPushButton { background-color: #334155; color: #F8FAFC; border: none; "
        "border-radius: 6px; padding: 7px; font-weight: bold; font-size: 11px; margin-top: 4px; }"
        "QPushButton:hover { background-color: #DC2626; }"
    );

    userCardLayout->addWidget(userNameLbl);
    userCardLayout->addWidget(userSubLbl);
    userCardLayout->addWidget(logoutBtn);
    sideLayout->addWidget(userCard);

    // ========================================================================
    // RIGHT CONTENT AREA (TOP HEADER + STACKED PAGES)
    // ========================================================================
    auto *rightContainer = new QWidget();
    auto *rightLayout = new QVBoxLayout(rightContainer);
    rightLayout->setContentsMargins(18, 14, 18, 14);
    rightLayout->setSpacing(12);

    // Top Header & Global Filter Bar
    auto *headerFrame = new QFrame();
    headerFrame->setProperty("class", "Card");
    auto *headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setContentsMargins(14, 10, 14, 10);

    pageTitleLabel = new QLabel("Dashboard & Workload Summary");
    pageTitleLabel->setStyleSheet("font-size: 17px; font-weight: bold; color: #0F172A;");

    semesterFilterCombo = new QComboBox();
    semesterFilterCombo->addItems({"All Semesters", "Fall 2026", "Spring 2027", "Summer 2027"});

    deptFilterCombo = new QComboBox();
    deptFilterCombo->addItem("All Departments");

    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Search faculty, course, room...");
    searchEdit->setClearButtonEnabled(true);
    searchEdit->setMaximumWidth(230);

    auto *exportCsvBtn = new QPushButton("Export CSV");
    exportCsvBtn->setProperty("class", "SecondaryBtn");

    headerLayout->addWidget(pageTitleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(new QLabel("Semester:"));
    headerLayout->addWidget(semesterFilterCombo);
    headerLayout->addWidget(new QLabel("Dept:"));
    headerLayout->addWidget(deptFilterCombo);
    headerLayout->addWidget(searchEdit);
    headerLayout->addWidget(exportCsvBtn);

    rightLayout->addWidget(headerFrame);

    pageStack = new QStackedWidget();
    rightLayout->addWidget(pageStack, 1);

    // ------------------------------------------------------------------------
    // PAGE 0: DASHBOARD & WORKLOAD SUMMARY
    // ------------------------------------------------------------------------
    auto *page0 = new QWidget();
    auto *p0Layout = new QVBoxLayout(page0);
    p0Layout->setContentsMargins(0, 0, 0, 0);
    p0Layout->setSpacing(12);

    // KPI Cards Row
    auto *kpiRow = new QHBoxLayout();
    kpiRow->setSpacing(12);

    auto makeKpiCard = [&](const QString &title, QLabel *&valLabel, const QString &accentColor) {
        auto *card = new QFrame();
        card->setProperty("class", "Card");
        auto *cl = new QVBoxLayout(card);
        cl->setContentsMargins(14, 12, 14, 12);
        auto *t = new QLabel(title);
        t->setStyleSheet("color: #64748B; font-size: 11px; font-weight: bold;");
        valLabel = new QLabel("0");
        valLabel->setStyleSheet(QString("color: %1; font-size: 22px; font-weight: bold;").arg(accentColor));
        cl->addWidget(t);
        cl->addWidget(valLabel);
        kpiRow->addWidget(card);
    };

    makeKpiCard("ACTIVE FACULTY", kpiFacultyValue, "#0F172A");
    makeKpiCard("AVG UTILIZATION", kpiAvgUtilValue, "#2563EB");
    makeKpiCard("OVERLOADED FACULTY", kpiOverloadValue, "#DC2626");
    makeKpiCard("PENDING REQUESTS", kpiPendingReqValue, "#D97706");
    p0Layout->addLayout(kpiRow);

    // Department Bar Chart Card
    auto *chartCard = new QFrame();
    chartCard->setProperty("class", "Card");
    auto *chartLayout = new QVBoxLayout(chartCard);
    chartLayout->setContentsMargins(8, 8, 8, 8);
    deptChart = new DepartmentChartWidget();
    chartLayout->addWidget(deptChart);
    p0Layout->addWidget(chartCard);

    // Lower Split: Register Faculty Form + Summary Table
    auto *p0Split = new QHBoxLayout();
    p0Split->setSpacing(12);

    facultyRegGroup = new QGroupBox("Register New Faculty");
    facultyRegGroup->setFixedWidth(300);
    auto *facForm = new QFormLayout(facultyRegGroup);

    facultyNameEdit = new QLineEdit();
    facultyNameEdit->setPlaceholderText("e.g., Dr. Priya Nair");

    departmentCombo = new QComboBox();
    departmentCombo->addItems({"Computer Science", "Electronics", "Mechanical", "Mathematics", "Physics"});
    departmentCombo->setEditable(true);

    designationCombo = new QComboBox();
    designationCombo->addItems({"Professor", "Associate Professor", "Assistant Professor", "Lecturer"});

    maxHoursSpin = new QDoubleSpinBox();
    maxHoursSpin->setRange(1.0, 40.0);
    maxHoursSpin->setValue(16.0);
    maxHoursSpin->setSuffix(" hrs/wk");

    auto *addFacultyBtn = new QPushButton("Add Faculty Member");
    addFacultyBtn->setProperty("class", "PrimaryBtn");

    facForm->addRow("Name:", facultyNameEdit);
    facForm->addRow("Department:", departmentCombo);
    facForm->addRow("Designation:", designationCombo);
    facForm->addRow("Max Load:", maxHoursSpin);
    facForm->addRow(addFacultyBtn);

    auto *sumRight = new QVBoxLayout();
    summaryTable = new QTableWidget();
    summaryTable->setColumnCount(8);
    summaryTable->setHorizontalHeaderLabels({
        "Faculty Name", "Department", "Designation", "Courses",
        "Raw Hrs", "Eff. Hrs", "Max Cap", "Utilization & Status"
    });
    summaryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    summaryTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    summaryTable->setSelectionMode(QAbstractItemView::SingleSelection);
    summaryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto *sumActions = new QHBoxLayout();
    editFacultyBtn = new QPushButton("Edit Selected Faculty");
    editFacultyBtn->setProperty("class", "SecondaryBtn");
    deleteFacultyBtn = new QPushButton("Remove Selected Faculty");
    deleteFacultyBtn->setProperty("class", "DangerBtn");
    sumActions->addWidget(editFacultyBtn);
    sumActions->addWidget(deleteFacultyBtn);
    sumActions->addStretch();

    sumRight->addWidget(summaryTable);
    sumRight->addLayout(sumActions);

    p0Split->addWidget(facultyRegGroup);
    p0Split->addLayout(sumRight, 1);
    p0Layout->addLayout(p0Split, 1);

    pageStack->addWidget(page0);

    // ------------------------------------------------------------------------
    // PAGE 1: COURSE SCHEDULER & CLASH DETECTION
    // ------------------------------------------------------------------------
    auto *page1 = new QWidget();
    auto *p1Layout = new QHBoxLayout(page1);
    p1Layout->setContentsMargins(0, 0, 0, 0);
    p1Layout->setSpacing(12);

    scheduleFormGroup = new QGroupBox("Schedule Course (Faculty + Room Clash Checked)");
    scheduleFormGroup->setFixedWidth(340);
    auto *schForm = new QFormLayout(scheduleFormGroup);

    assignFacultyCombo = new QComboBox();
    courseCodeEdit = new QLineEdit();
    courseCodeEdit->setPlaceholderText("e.g., CS405");
    courseNameEdit = new QLineEdit();
    courseNameEdit->setPlaceholderText("e.g., Compiler Design");

    activityTypeCombo = new QComboBox();
    activityTypeCombo->addItem("Lecture (1.0x weight)", 1.0);
    activityTypeCombo->addItem("Lab / Practical (0.75x weight)", 0.75);
    activityTypeCombo->addItem("Admin / Research (1.0x weight)", 1.0);

    dayOfWeekCombo = new QComboBox();
    dayOfWeekCombo->addItems({"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"});

    startTimeEdit = new QTimeEdit(QTime(9, 0));
    startTimeEdit->setDisplayFormat("HH:mm");
    endTimeEdit = new QTimeEdit(QTime(11, 0));
    endTimeEdit->setDisplayFormat("HH:mm");

    auto *tRow = new QHBoxLayout();
    tRow->addWidget(startTimeEdit);
    tRow->addWidget(new QLabel("to"));
    tRow->addWidget(endTimeEdit);

    roomEdit = new QLineEdit();
    roomEdit->setPlaceholderText("e.g., LH-101 or AI-Lab-2");

    semesterCombo = new QComboBox();
    semesterCombo->addItems({"Fall 2026", "Spring 2027", "Summer 2027"});
    semesterCombo->setEditable(true);

    durationPreviewLabel = new QLabel();
    durationPreviewLabel->setStyleSheet("color: #16A34A; font-weight: bold;");

    auto *assignBtn = new QPushButton("Verify & Assign Schedule");
    assignBtn->setProperty("class", "SuccessBtn");

    schForm->addRow("Faculty:", assignFacultyCombo);
    schForm->addRow("Course Code:", courseCodeEdit);
    schForm->addRow("Course Title:", courseNameEdit);
    schForm->addRow("Duty Type:", activityTypeCombo);
    schForm->addRow("Day:", dayOfWeekCombo);
    schForm->addRow("Time Slot:", tRow);
    schForm->addRow("Room / Lab:", roomEdit);
    schForm->addRow("Semester:", semesterCombo);
    schForm->addRow("Load Preview:", durationPreviewLabel);
    schForm->addRow(assignBtn);

    auto *p1Right = new QVBoxLayout();
    assignmentsTable = new QTableWidget();
    assignmentsTable->setColumnCount(8);
    assignmentsTable->setHorizontalHeaderLabels({
        "Faculty", "Course", "Type", "Day & Time",
        "Room", "Raw Hrs", "Eff. Hrs", "Semester"
    });
    assignmentsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    assignmentsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    assignmentsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    assignmentsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto *p1BtnRow = new QHBoxLayout();
    deleteAssignBtn = new QPushButton("Remove Selected Allocation");
    deleteAssignBtn->setProperty("class", "DangerBtn");
    p1BtnRow->addWidget(deleteAssignBtn);
    p1BtnRow->addStretch();

    p1Right->addWidget(assignmentsTable);
    p1Right->addLayout(p1BtnRow);

    p1Layout->addWidget(scheduleFormGroup);
    p1Layout->addLayout(p1Right, 1);

    pageStack->addWidget(page1);

    // ------------------------------------------------------------------------
    // PAGE 2: WEEKLY TIMETABLE MATRIX
    // ------------------------------------------------------------------------
    auto *page2 = new QWidget();
    auto *p2Layout = new QVBoxLayout(page2);
    p2Layout->setContentsMargins(0, 0, 0, 0);

    auto *ttCard = new QFrame();
    ttCard->setProperty("class", "Card");
    auto *ttLayout = new QVBoxLayout(ttCard);

    auto *ttHeader = new QHBoxLayout();
    timetableFacultyCombo = new QComboBox();
    timetableFacultyCombo->setMinimumWidth(280);
    ttHeader->addWidget(new QLabel("Select Faculty Member:"));
    ttHeader->addWidget(timetableFacultyCombo);
    ttHeader->addStretch();
    ttHeader->addWidget(new QLabel("Legend:  [Blue = Lecture]   [Teal = Lab]   [Amber = Admin/Research]"));

    timetableGrid = new QTableWidget(10, 7);
    timetableGrid->setHorizontalHeaderLabels({
        "Time Slot", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
    });
    timetableGrid->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    timetableGrid->verticalHeader()->setVisible(false);
    timetableGrid->setEditTriggers(QAbstractItemView::NoEditTriggers);
    timetableGrid->setSelectionMode(QAbstractItemView::NoSelection);

    for (int r = 0; r < 10; ++r) {
        int hr = 8 + r;
        auto *slot = new QTableWidgetItem(
            QString("%1:00 - %2:00").arg(hr, 2, 10, QChar('0')).arg(hr + 1, 2, 10, QChar('0'))
        );
        slot->setTextAlignment(Qt::AlignCenter);
        slot->setBackground(QBrush(QColor("#F8FAFC")));
        timetableGrid->setItem(r, 0, slot);
        timetableGrid->setRowHeight(r, 50);
    }

    ttLayout->addLayout(ttHeader);
    ttLayout->addWidget(timetableGrid);
    p2Layout->addWidget(ttCard);

    pageStack->addWidget(page2);

    // ------------------------------------------------------------------------
    // PAGE 3: WORKLOAD ADJUSTMENT & LEAVE REQUESTS
    // ------------------------------------------------------------------------
    auto *page3 = new QWidget();
    auto *p3Layout = new QHBoxLayout(page3);
    p3Layout->setContentsMargins(0, 0, 0, 0);
    p3Layout->setSpacing(12);

    auto *reqFormGroup = new QGroupBox("Submit Workload / Leave Request");
    reqFormGroup->setFixedWidth(330);
    auto *reqForm = new QFormLayout(reqFormGroup);

    reqFacultyCombo = new QComboBox();
    reqTypeCombo = new QComboBox();
    reqTypeCombo->addItems({
        "Course Load Relief",
        "Conference / Academic Leave",
        "Research Grant Buyout",
        "Extra Teaching Credit"
    });

    reqHoursSpin = new QDoubleSpinBox();
    reqHoursSpin->setRange(0.5, 20.0);
    reqHoursSpin->setSingleStep(0.5);
    reqHoursSpin->setValue(2.0);
    reqHoursSpin->setSuffix(" hrs/wk");

    reqReasonEdit = new QTextEdit();
    reqReasonEdit->setPlaceholderText("Provide justification, dates, or substitute arrangement...");
    reqReasonEdit->setMaximumHeight(110);

    auto *submitReqBtn = new QPushButton("Submit Request");
    submitReqBtn->setProperty("class", "PrimaryBtn");

    reqForm->addRow("Faculty:", reqFacultyCombo);
    reqForm->addRow("Request Type:", reqTypeCombo);
    reqForm->addRow("Hours Impact:", reqHoursSpin);
    reqForm->addRow("Justification:", reqReasonEdit);
    reqForm->addRow(submitReqBtn);

    auto *p3Right = new QVBoxLayout();
    requestsTable = new QTableWidget();
    requestsTable->setColumnCount(7);
    requestsTable->setHorizontalHeaderLabels({
        "Faculty", "Department", "Request Type", "Hours", "Reason", "Status", "Reviewed By"
    });
    requestsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    requestsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    requestsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    requestsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto *reqActionRow = new QHBoxLayout();
    approveReqBtn = new QPushButton("Approve Selected Request");
    approveReqBtn->setProperty("class", "SuccessBtn");
    rejectReqBtn = new QPushButton("Reject Selected Request");
    rejectReqBtn->setProperty("class", "DangerBtn");

    reqActionRow->addWidget(approveReqBtn);
    reqActionRow->addWidget(rejectReqBtn);
    reqActionRow->addStretch();

    p3Right->addWidget(requestsTable);
    p3Right->addLayout(reqActionRow);

    p3Layout->addWidget(reqFormGroup);
    p3Layout->addLayout(p3Right, 1);

    pageStack->addWidget(page3);

    // ------------------------------------------------------------------------
    // PAGE 4: ADMIN USER MANAGEMENT
    // ------------------------------------------------------------------------
    auto *page4 = new QWidget();
    auto *p4Layout = new QHBoxLayout(page4);
    p4Layout->setContentsMargins(0, 0, 0, 0);
    p4Layout->setSpacing(12);

    auto *userFormGroup = new QGroupBox("Create RBAC User Account");
    userFormGroup->setFixedWidth(330);
    auto *uForm = new QFormLayout(userFormGroup);

    newUsernameEdit = new QLineEdit();
    newUsernameEdit->setPlaceholderText("e.g., hod_ece or prof_verma");

    newPasswordEdit = new QLineEdit();
    newPasswordEdit->setPlaceholderText("Min 4 characters");
    newPasswordEdit->setEchoMode(QLineEdit::Password);

    newRoleCombo = new QComboBox();
    newRoleCombo->addItems({"Faculty", "HOD", "Admin"});

    newUserDeptCombo = new QComboBox();
    newUserDeptCombo->addItems({"Computer Science", "Electronics", "Mechanical", "Mathematics", "Physics"});
    newUserDeptCombo->setEditable(true);

    newUserFacultyCombo = new QComboBox();

    auto *createUserBtn = new QPushButton("Create User Account");
    createUserBtn->setProperty("class", "PrimaryBtn");

    uForm->addRow("Username:", newUsernameEdit);
    uForm->addRow("Password:", newPasswordEdit);
    uForm->addRow("Access Role:", newRoleCombo);
    uForm->addRow("HOD Dept:", newUserDeptCombo);
    uForm->addRow("Link Faculty:", newUserFacultyCombo);
    uForm->addRow(createUserBtn);

    auto *p4Right = new QVBoxLayout();
    usersTable = new QTableWidget();
    usersTable->setColumnCount(5);
    usersTable->setHorizontalHeaderLabels({
        "User ID", "Username", "Role", "Scoped Department", "Linked Faculty Profile"
    });
    usersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    usersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    usersTable->setSelectionMode(QAbstractItemView::SingleSelection);
    usersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto *userBtnRow = new QHBoxLayout();
    auto *resetPwdBtn = new QPushButton("Reset Password");
    resetPwdBtn->setProperty("class", "SecondaryBtn");
    auto *delUserBtn = new QPushButton("Delete User");
    delUserBtn->setProperty("class", "DangerBtn");
    userBtnRow->addWidget(resetPwdBtn);
    userBtnRow->addWidget(delUserBtn);
    userBtnRow->addStretch();

    p4Right->addWidget(usersTable);
    p4Right->addLayout(userBtnRow);

    p4Layout->addWidget(userFormGroup);
    p4Layout->addLayout(p4Right, 1);

    pageStack->addWidget(page4);

    // ------------------------------------------------------------------------
    // PAGE 5: ADMIN SYSTEM AUDIT LOGS
    // ------------------------------------------------------------------------
    auto *page5 = new QWidget();
    auto *p5Layout = new QVBoxLayout(page5);
    p5Layout->setContentsMargins(0, 0, 0, 0);

    auditTable = new QTableWidget();
    auditTable->setColumnCount(5);
    auditTable->setHorizontalHeaderLabels({
        "Timestamp", "Username", "Role", "Action", "Event Details"
    });
    auditTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    auditTable->horizontalHeader()->setStretchLastSection(true);
    auditTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    p5Layout->addWidget(auditTable);
    pageStack->addWidget(page5);

    rootLayout->addWidget(sidebar);
    rootLayout->addWidget(rightContainer, 1);

    // ========================================================================
    // SIGNAL & SLOT CONNECTIONS
    // ========================================================================
    connect(navButtonGroup, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &MainWindow::onNavigationChanged);
    connect(logoutBtn, &QPushButton::clicked, this, &MainWindow::onLogout);

    connect(addFacultyBtn, &QPushButton::clicked, this, &MainWindow::onAddFaculty);
    connect(editFacultyBtn, &QPushButton::clicked, this, &MainWindow::onEditFaculty);
    connect(deleteFacultyBtn, &QPushButton::clicked, this, &MainWindow::onDeleteFaculty);

    connect(assignBtn, &QPushButton::clicked, this, &MainWindow::onAssignCourse);
    connect(deleteAssignBtn, &QPushButton::clicked, this, &MainWindow::onDeleteAssignment);

    connect(startTimeEdit, &QTimeEdit::timeChanged, this, &MainWindow::updateCalculatedHoursPreview);
    connect(endTimeEdit, &QTimeEdit::timeChanged, this, &MainWindow::updateCalculatedHoursPreview);
    connect(activityTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::updateCalculatedHoursPreview);

    connect(submitReqBtn, &QPushButton::clicked, this, &MainWindow::onSubmitRequest);
    connect(approveReqBtn, &QPushButton::clicked, this, &MainWindow::onApproveRequest);
    connect(rejectReqBtn, &QPushButton::clicked, this, &MainWindow::onRejectRequest);

    connect(createUserBtn, &QPushButton::clicked, this, &MainWindow::onCreateUser);
    connect(resetPwdBtn, &QPushButton::clicked, this, &MainWindow::onResetPassword);
    connect(delUserBtn, &QPushButton::clicked, this, &MainWindow::onDeleteUser);

    connect(semesterFilterCombo, &QComboBox::currentTextChanged, this, &MainWindow::refreshAll);
    connect(deptFilterCombo, &QComboBox::currentTextChanged, this, &MainWindow::refreshAll);
    connect(searchEdit, &QLineEdit::textChanged, this, &MainWindow::refreshAll);
    connect(timetableFacultyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::refreshTimetableGrid);
    connect(exportCsvBtn, &QPushButton::clicked, this, &MainWindow::onExportCsv);
}

void MainWindow::applyRbacPolicies() {
    if (m_user.role == "HOD") {
        // HOD is locked strictly to their own department
        deptFilterCombo->clear();
        deptFilterCombo->addItem(m_user.department);
        deptFilterCombo->setEnabled(false);

        departmentCombo->clear();
        departmentCombo->addItem(m_user.department);
        departmentCombo->setEnabled(false);
    } else if (m_user.role == "Faculty") {
        // Faculty members have read-only access to schedules & can submit requests
        deptFilterCombo->setEnabled(false);
        facultyRegGroup->hide();
        editFacultyBtn->hide();
        deleteFacultyBtn->hide();

        scheduleFormGroup->hide();
        deleteAssignBtn->hide();

        approveReqBtn->hide();
        rejectReqBtn->hide();
    }
}

void MainWindow::onNavigationChanged(int pageIndex) {
    pageStack->setCurrentIndex(pageIndex);
    const QStringList titles = {
        "Dashboard & Workload Summary",
        "Course Scheduler & Clash Detection",
        "Weekly Faculty Timetable Matrix",
        "Leave & Workload Adjustment Requests",
        "RBAC User & Role Management",
        "System Security & Activity Audit Log"
    };
    if (pageIndex >= 0 && pageIndex < titles.size()) {
        pageTitleLabel->setText(titles[pageIndex]);
    }
}

void MainWindow::onLogout() {
    Database::logAudit(m_user.username, m_user.role, "LOGOUT", "User signed out of session.");
    m_logoutRequested = true;
    close();
}

void MainWindow::updateCalculatedHoursPreview() {
    QTime start = startTimeEdit->time();
    QTime end = endTimeEdit->time();
    double weight = activityTypeCombo->currentData().toDouble();

    if (end <= start) {
        durationPreviewLabel->setStyleSheet("color: #DC2626; font-weight: bold;");
        durationPreviewLabel->setText("End time must be after Start time");
        return;
    }

    double rawHrs = start.secsTo(end) / 3600.0;
    double effHrs = rawHrs * weight;
    durationPreviewLabel->setStyleSheet("color: #16A34A; font-weight: bold;");
    durationPreviewLabel->setText(
        QString("%1 raw x %2 = %3 eff. hrs")
            .arg(rawHrs, 0, 'f', 2)
            .arg(weight, 0, 'f', 2)
            .arg(effHrs, 0, 'f', 2)
    );
}

void MainWindow::onAddFaculty() {
    const QString name = facultyNameEdit->text().trimmed();
    const QString dept = (m_user.role == "HOD") ? m_user.department : departmentCombo->currentText().trimmed();
    const QString desig = designationCombo->currentText().trimmed();
    const double maxHrs = maxHoursSpin->value();

    if (name.isEmpty() || dept.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Faculty name and department are required.");
        return;
    }

    if (Database::addFaculty(name, dept, desig, maxHrs)) {
        Database::logAudit(m_user.username, m_user.role, "ADD_FACULTY",
                           QString("Registered %1 (%2, Max %3 hrs)").arg(name, dept).arg(maxHrs));
        facultyNameEdit->clear();
        refreshAll();
    }
}

void MainWindow::onEditFaculty() {
    if (m_user.role == "Faculty") return;
    int row = summaryTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "No Selection", "Select a faculty member to edit.");
        return;
    }

    int facultyId = summaryTable->item(row, 0)->data(Qt::UserRole).toInt();
    QString currentName = summaryTable->item(row, 0)->text();
    QString currentDept = summaryTable->item(row, 1)->text();
    QString currentDesig = summaryTable->item(row, 2)->text();
    double currentMax = summaryTable->item(row, 6)->text().toDouble();

    QDialog dialog(this);
    dialog.setWindowTitle("Edit Faculty Profile");
    QFormLayout form(&dialog);

    QLineEdit nameEdit(currentName);
    QLineEdit deptEdit(currentDept);
    if (m_user.role == "HOD") deptEdit.setEnabled(false);

    QComboBox desigBox;
    desigBox.addItems({"Professor", "Associate Professor", "Assistant Professor", "Lecturer"});
    desigBox.setCurrentText(currentDesig);

    QDoubleSpinBox maxSpin;
    maxSpin.setRange(1.0, 40.0);
    maxSpin.setValue(currentMax);

    form.addRow("Full Name:", &nameEdit);
    form.addRow("Department:", &deptEdit);
    form.addRow("Designation:", &desigBox);
    form.addRow("Max Capacity:", &maxSpin);

    QDialogButtonBox btns(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form.addRow(&btns);
    connect(&btns, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&btns, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted && !nameEdit.text().trimmed().isEmpty()) {
        Database::updateFaculty(facultyId, nameEdit.text().trimmed(), deptEdit.text().trimmed(),
                                desigBox.currentText(), maxSpin.value());
        Database::logAudit(m_user.username, m_user.role, "EDIT_FACULTY",
                           QString("Updated %1 (Max %2 hrs)").arg(nameEdit.text().trimmed()).arg(maxSpin.value()));
        refreshAll();
    }
}

void MainWindow::onDeleteFaculty() {
    int row = summaryTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "No Selection", "Select a faculty row to remove.");
        return;
    }

    int facultyId = summaryTable->item(row, 0)->data(Qt::UserRole).toInt();
    QString name = summaryTable->item(row, 0)->text();

    if (QMessageBox::question(this, "Confirm Removal",
                              QString("Remove '%1' and all scheduled courses?").arg(name)) == QMessageBox::Yes) {
        Database::deleteFaculty(facultyId);
        Database::logAudit(m_user.username, m_user.role, "DELETE_FACULTY", "Removed faculty: " + name);
        refreshAll();
    }
}

void MainWindow::onAssignCourse() {
    if (assignFacultyCombo->count() == 0) {
        QMessageBox::warning(this, "No Faculty", "No eligible faculty available in your scope.");
        return;
    }

    int facultyId = assignFacultyCombo->currentData().toInt();
    const QString code = courseCodeEdit->text().trimmed().toUpper();
    const QString title = courseNameEdit->text().trimmed();
    const QString actType = activityTypeCombo->currentText().split(" (").first();
    const double weight = activityTypeCombo->currentData().toDouble();
    const QString day = dayOfWeekCombo->currentText();
    const QTime start = startTimeEdit->time();
    const QTime end = endTimeEdit->time();
    const QString room = roomEdit->text().trimmed().isEmpty() ? "TBD" : roomEdit->text().trimmed();
    const QString sem = semesterCombo->currentText().trimmed();

    if (code.isEmpty() || title.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Course code and title are required.");
        return;
    }
    if (end <= start) {
        QMessageBox::warning(this, "Invalid Slot", "End time must be strictly after start time.");
        return;
    }

    // Dual-Resource Clash Detection (Faculty + Classroom)
    QString conflictDetails;
    if (Database::checkScheduleConflict(facultyId, room, sem, day, start, end, conflictDetails)) {
        QMessageBox::critical(this, "Schedule Conflict Detected",
                              QString("Cannot schedule %1 on %2 (%3-%4):\n\n%5")
                                  .arg(code, day, start.toString("HH:mm"), end.toString("HH:mm"), conflictDetails));
        return;
    }

    if (Database::addAssignment(facultyId, code, title, actType, weight, day, start, end, room, sem)) {
        Database::logAudit(m_user.username, m_user.role, "ASSIGN_COURSE",
                           QString("Assigned %1 (%2 %3-%4, Room %5)").arg(code, day, start.toString("HH:mm"), end.toString("HH:mm"), room));
        courseCodeEdit->clear();
        courseNameEdit->clear();
        roomEdit->clear();
        refreshAll();
    }
}

void MainWindow::onDeleteAssignment() {
    int row = assignmentsTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "No Selection", "Select a schedule allocation to remove.");
        return;
    }

    int assignId = assignmentsTable->item(row, 0)->data(Qt::UserRole).toInt();
    QString courseInfo = assignmentsTable->item(row, 1)->text();
    Database::deleteAssignment(assignId);
    Database::logAudit(m_user.username, m_user.role, "REMOVE_COURSE", "Removed allocation: " + courseInfo);
    refreshAll();
}

void MainWindow::onSubmitRequest() {
    if (reqFacultyCombo->count() == 0) return;

    int facultyId = reqFacultyCombo->currentData().toInt();
    QString type = reqTypeCombo->currentText();
    double hours = reqHoursSpin->value();
    QString reason = reqReasonEdit->toPlainText().trimmed();

    if (reason.isEmpty()) {
        QMessageBox::warning(this, "Missing Reason", "Please enter a justification for this request.");
        return;
    }

    if (Database::submitAdjustmentRequest(facultyId, type, hours, reason)) {
        Database::logAudit(m_user.username, m_user.role, "SUBMIT_REQUEST",
                           QString("Submitted %1 (%2 hrs)").arg(type).arg(hours));
        reqReasonEdit->clear();
        refreshAll();
        QMessageBox::information(this, "Request Submitted", "Your workload adjustment request is now pending review.");
    }
}

void MainWindow::onApproveRequest() {
    int row = requestsTable->currentRow();
    if (row < 0) return;
    int reqId = requestsTable->item(row, 0)->data(Qt::UserRole).toInt();
    Database::updateRequestStatus(reqId, "Approved", m_user.username);
    Database::logAudit(m_user.username, m_user.role, "APPROVE_REQUEST", QString("Approved request #%1").arg(reqId));
    refreshAll();
}

void MainWindow::onRejectRequest() {
    int row = requestsTable->currentRow();
    if (row < 0) return;
    int reqId = requestsTable->item(row, 0)->data(Qt::UserRole).toInt();
    Database::updateRequestStatus(reqId, "Rejected", m_user.username);
    Database::logAudit(m_user.username, m_user.role, "REJECT_REQUEST", QString("Rejected request #%1").arg(reqId));
    refreshAll();
}

void MainWindow::onCreateUser() {
    QString uname = newUsernameEdit->text().trimmed();
    QString pwd = newPasswordEdit->text();
    QString role = newRoleCombo->currentText();
    QString dept = (role == "HOD") ? newUserDeptCombo->currentText().trimmed() : "All Departments";
    int linkedFid = (role == "Faculty") ? newUserFacultyCombo->currentData().toInt() : -1;

    QString err;
    if (Database::createUser(uname, pwd, role, dept, linkedFid, err)) {
        Database::logAudit(m_user.username, m_user.role, "CREATE_USER",
                           QString("Created user '%1' with role %2").arg(uname, role));
        newUsernameEdit->clear();
        newPasswordEdit->clear();
        refreshAll();
        QMessageBox::information(this, "User Created", "New RBAC account created successfully.");
    } else {
        QMessageBox::warning(this, "Creation Failed", err);
    }
}

void MainWindow::onResetPassword() {
    int row = usersTable->currentRow();
    if (row < 0) return;
    int userId = usersTable->item(row, 0)->text().toInt();
    QString uname = usersTable->item(row, 1)->text();

    bool ok = false;
    QString newPwd = QInputDialog::getText(this, "Reset Password",
                                           QString("Enter new password for '%1':").arg(uname),
                                           QLineEdit::Password, "", &ok);
    if (ok && newPwd.length() >= 4) {
        Database::resetUserPassword(userId, newPwd);
        Database::logAudit(m_user.username, m_user.role, "RESET_PASSWORD", "Reset password for @" + uname);
        QMessageBox::information(this, "Password Updated", "Password has been reset.");
    } else if (ok) {
        QMessageBox::warning(this, "Weak Password", "Password must be at least 4 characters.");
    }
}

void MainWindow::onDeleteUser() {
    int row = usersTable->currentRow();
    if (row < 0) return;
    int userId = usersTable->item(row, 0)->text().toInt();
    QString uname = usersTable->item(row, 1)->text();

    if (uname == m_user.username) {
        QMessageBox::warning(this, "Blocked", "You cannot delete your own active session account.");
        return;
    }

    if (QMessageBox::question(this, "Confirm Delete", "Delete user @" + uname + "?") == QMessageBox::Yes) {
        Database::deleteUser(userId);
        Database::logAudit(m_user.username, m_user.role, "DELETE_USER", "Deleted user @" + uname);
        refreshAll();
    }
}

void MainWindow::refreshAll() {
    populateDepartmentFilter();
    populateFacultyCombos();
    populateSummaryTable();
    populateAssignmentsTable();
    refreshTimetableGrid();
    populateRequestsTable();

    if (m_user.role == "Admin") {
        populateUsersTable();
        populateAuditTable();
    }
}

void MainWindow::populateDepartmentFilter() {
    if (m_user.role == "HOD") return; // Locked to HOD's department
    QString current = deptFilterCombo->currentText();
    deptFilterCombo->blockSignals(true);
    deptFilterCombo->clear();
    deptFilterCombo->addItem("All Departments");
    deptFilterCombo->addItems(Database::getDistinctDepartments());
    int idx = deptFilterCombo->findText(current);
    deptFilterCombo->setCurrentIndex(idx >= 0 ? idx : 0);
    deptFilterCombo->blockSignals(false);
}

void MainWindow::populateFacultyCombos() {
    QString scopeDept = (m_user.role == "HOD") ? m_user.department : "All Departments";
    int scopeFid = (m_user.role == "Faculty") ? m_user.facultyId : -1;

    const auto scopedList = Database::getFacultyList(scopeDept, scopeFid);
    const auto allList = Database::getFacultyList();

    auto fillCombo = [&](QComboBox *box, const QList<Faculty> &list) {
        int prev = box->currentData().toInt();
        box->blockSignals(true);
        box->clear();
        for (const auto &f : list) {
            box->addItem(QString("%1 (%2)").arg(f.name, f.department), f.id);
        }
        int idx = box->findData(prev);
        if (idx >= 0) box->setCurrentIndex(idx);
        box->blockSignals(false);
    };

    fillCombo(assignFacultyCombo, scopedList);
    fillCombo(timetableFacultyCombo, scopedList);
    fillCombo(reqFacultyCombo, scopedList);
    fillCombo(newUserFacultyCombo, allList);
}

void MainWindow::populateSummaryTable() {
    QString deptFilter = (m_user.role == "HOD") ? m_user.department : deptFilterCombo->currentText();
    int specificFid = (m_user.role == "Faculty") ? m_user.facultyId : -1;

    const auto summaries = Database::getWorkloadSummaries(
        semesterFilterCombo->currentText(), deptFilter, searchEdit->text(), specificFid
    );

    summaryTable->setRowCount(summaries.size());

    int overloadedCount = 0;
    double sumEff = 0.0;
    double sumMax = 0.0;

    for (int i = 0; i < summaries.size(); ++i) {
        const auto &ws = summaries[i];
        sumEff += ws.effectiveHours;
        sumMax += ws.maxHours;

        double util = (ws.maxHours > 0) ? (ws.effectiveHours / ws.maxHours) * 100.0 : 0.0;
        QString statusText;
        QString barColor;

        if (ws.effectiveHours > ws.maxHours) {
            statusText = "OVERLOADED";
            barColor = "#DC2626";
            overloadedCount++;
        } else if (ws.effectiveHours >= 0.8 * ws.maxHours) {
            statusText = "Optimal";
            barColor = "#16A34A";
        } else {
            statusText = "Underloaded";
            barColor = "#2563EB";
        }

        auto *nameItem = new QTableWidgetItem(ws.name);
        nameItem->setData(Qt::UserRole, ws.facultyId);

        summaryTable->setItem(i, 0, nameItem);
        summaryTable->setItem(i, 1, new QTableWidgetItem(ws.department));
        summaryTable->setItem(i, 2, new QTableWidgetItem(ws.designation));
        summaryTable->setItem(i, 3, new QTableWidgetItem(QString::number(ws.courseCount)));
        summaryTable->setItem(i, 4, new QTableWidgetItem(QString::number(ws.rawHours, 'f', 1)));
        summaryTable->setItem(i, 5, new QTableWidgetItem(QString::number(ws.effectiveHours, 'f', 2)));
        summaryTable->setItem(i, 6, new QTableWidgetItem(QString::number(ws.maxHours, 'f', 1)));

        auto *bar = new QProgressBar();
        bar->setRange(0, 100);
        bar->setValue(qMin(100, static_cast<int>(util)));
        bar->setFormat(QString("%1% (%2)").arg(util, 0, 'f', 0).arg(statusText));
        bar->setAlignment(Qt::AlignCenter);
        bar->setStyleSheet(QString(
            "QProgressBar { border: 1px solid #CBD5E1; border-radius: 5px; text-align: center; font-weight: bold; background: #F8FAFC; }"
            "QProgressBar::chunk { background-color: %1; border-radius: 4px; }"
        ).arg(barColor));

        summaryTable->setCellWidget(i, 7, bar);
    }

    double avgUtil = (sumMax > 0) ? (sumEff / sumMax) * 100.0 : 0.0;
    kpiFacultyValue->setText(QString::number(summaries.size()));
    kpiAvgUtilValue->setText(QString("%1%").arg(avgUtil, 0, 'f', 1));
    kpiOverloadValue->setText(QString::number(overloadedCount));

    deptChart->setStats(Database::getDepartmentStats(semesterFilterCombo->currentText(), deptFilter));
}

void MainWindow::populateAssignmentsTable() {
    QString deptFilter = (m_user.role == "HOD") ? m_user.department : deptFilterCombo->currentText();
    int specificFid = (m_user.role == "Faculty") ? m_user.facultyId : -1;

    const auto list = Database::getAssignments(
        semesterFilterCombo->currentText(), deptFilter, searchEdit->text(), specificFid
    );

    assignmentsTable->setRowCount(list.size());
    for (int i = 0; i < list.size(); ++i) {
        const auto &a = list[i];
        auto *facItem = new QTableWidgetItem(a.facultyName);
        facItem->setData(Qt::UserRole, a.id);

        assignmentsTable->setItem(i, 0, facItem);
        assignmentsTable->setItem(i, 1, new QTableWidgetItem(QString("%1 - %2").arg(a.courseCode, a.courseName)));
        assignmentsTable->setItem(i, 2, new QTableWidgetItem(a.activityType));
        assignmentsTable->setItem(i, 3, new QTableWidgetItem(
            QString("%1 (%2-%3)").arg(a.dayOfWeek.left(3), a.startTime.toString("HH:mm"), a.endTime.toString("HH:mm"))
        ));
        assignmentsTable->setItem(i, 4, new QTableWidgetItem(a.room));
        assignmentsTable->setItem(i, 5, new QTableWidgetItem(QString::number(a.rawHours, 'f', 1)));
        assignmentsTable->setItem(i, 6, new QTableWidgetItem(QString::number(a.effectiveHours, 'f', 2)));
        assignmentsTable->setItem(i, 7, new QTableWidgetItem(a.semester));
    }
}

void MainWindow::refreshTimetableGrid() {
    for (int r = 0; r < 10; ++r) {
        for (int c = 1; c <= 6; ++c) {
            timetableGrid->setItem(r, c, new QTableWidgetItem(""));
        }
    }

    if (timetableFacultyCombo->count() == 0) return;

    int facultyId = timetableFacultyCombo->currentData().toInt();
    const auto schedule = Database::getFacultySchedule(facultyId, semesterFilterCombo->currentText());
    const QStringList days = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

    for (const auto &a : schedule) {
        int col = days.indexOf(a.dayOfWeek) + 1;
        if (col <= 0) continue;

        QColor bg("#DBEAFE"), fg("#1E3A8A");
        if (a.activityType == "Lab") {
            bg = QColor("#CCFBF1");
            fg = QColor("#115E59");
        } else if (a.activityType.contains("Admin")) {
            bg = QColor("#FEF3C7");
            fg = QColor("#92400E");
        }

        for (int r = 0; r < 10; ++r) {
            QTime slotStart(8 + r, 0);
            QTime slotEnd(9 + r, 0);
            if (a.startTime < slotEnd && slotStart < a.endTime) {
                auto *item = new QTableWidgetItem(QString("%1\n[%2]").arg(a.courseCode, a.room));
                item->setTextAlignment(Qt::AlignCenter);
                item->setBackground(QBrush(bg));
                item->setForeground(QBrush(fg));
                timetableGrid->setItem(r, col, item);
            }
        }
    }
}

void MainWindow::populateRequestsTable() {
    QString deptFilter = (m_user.role == "HOD") ? m_user.department : deptFilterCombo->currentText();
    int specificFid = (m_user.role == "Faculty") ? m_user.facultyId : -1;

    const auto reqs = Database::getAdjustmentRequests(deptFilter, specificFid);
    requestsTable->setRowCount(reqs.size());

    int pendingCount = 0;
    for (int i = 0; i < reqs.size(); ++i) {
        const auto &r = reqs[i];
        if (r.status == "Pending") pendingCount++;

        auto *facItem = new QTableWidgetItem(r.facultyName);
        facItem->setData(Qt::UserRole, r.id);

        auto *statusItem = new QTableWidgetItem(r.status);
        statusItem->setTextAlignment(Qt::AlignCenter);
        if (r.status == "Approved") {
            statusItem->setBackground(QBrush(QColor("#DCFCE7")));
            statusItem->setForeground(QBrush(QColor("#166534")));
        } else if (r.status == "Rejected") {
            statusItem->setBackground(QBrush(QColor("#FEE2E2")));
            statusItem->setForeground(QBrush(QColor("#991B1B")));
        } else {
            statusItem->setBackground(QBrush(QColor("#FEF3C7")));
            statusItem->setForeground(QBrush(QColor("#92400E")));
        }

        requestsTable->setItem(i, 0, facItem);
        requestsTable->setItem(i, 1, new QTableWidgetItem(r.department));
        requestsTable->setItem(i, 2, new QTableWidgetItem(r.requestType));
        requestsTable->setItem(i, 3, new QTableWidgetItem(QString::number(r.requestedHours, 'f', 1) + " hrs"));
        requestsTable->setItem(i, 4, new QTableWidgetItem(r.reason));
        requestsTable->setItem(i, 5, statusItem);
        requestsTable->setItem(i, 6, new QTableWidgetItem(r.reviewedBy));
    }

    kpiPendingReqValue->setText(QString::number(pendingCount));
}

void MainWindow::populateUsersTable() {
    const auto users = Database::getAllUsers();
    usersTable->setRowCount(users.size());

    for (int i = 0; i < users.size(); ++i) {
        const auto &u = users[i];
        usersTable->setItem(i, 0, new QTableWidgetItem(QString::number(u.id)));
        usersTable->setItem(i, 1, new QTableWidgetItem(u.username));
        usersTable->setItem(i, 2, new QTableWidgetItem(u.role));
        usersTable->setItem(i, 3, new QTableWidgetItem(u.department));
        usersTable->setItem(i, 4, new QTableWidgetItem(
            u.linkedFacultyName.isEmpty() ? "-" : u.linkedFacultyName
        ));
    }
}

void MainWindow::populateAuditTable() {
    const auto logs = Database::getAuditLogs();
    auditTable->setRowCount(logs.size());

    for (int i = 0; i < logs.size(); ++i) {
        const auto &e = logs[i];
        auditTable->setItem(i, 0, new QTableWidgetItem(e.timestamp));
        auditTable->setItem(i, 1, new QTableWidgetItem(e.username));
        auditTable->setItem(i, 2, new QTableWidgetItem(e.role));
        auditTable->setItem(i, 3, new QTableWidgetItem(e.action));
        auditTable->setItem(i, 4, new QTableWidgetItem(e.details));
    }
}

void MainWindow::onExportCsv() {
    QString fileName = QFileDialog::getSaveFileName(
        this, "Export Workload Report", "faculty_workload_v3_report.csv", "CSV Files (*.csv)"
    );
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    QString deptFilter = (m_user.role == "HOD") ? m_user.department : deptFilterCombo->currentText();
    int specificFid = (m_user.role == "Faculty") ? m_user.facultyId : -1;

    const auto summaries = Database::getWorkloadSummaries(
        semesterFilterCombo->currentText(), deptFilter, searchEdit->text(), specificFid
    );

    QTextStream out(&file);
    out << "Faculty Name,Department,Designation,Courses,Raw Hours,Effective Hours,Max Capacity,Utilization %,Status\n";
    for (const auto &ws : summaries) {
        double util = (ws.maxHours > 0) ? (ws.effectiveHours / ws.maxHours) * 100.0 : 0.0;
        QString status = (ws.effectiveHours > ws.maxHours) ? "OVERLOADED"
                       : (ws.effectiveHours >= 0.8 * ws.maxHours) ? "Optimal"
                                                                  : "Underloaded";
        out << QString("\"%1\",\"%2\",\"%3\",%4,%5,%6,%7,%8,\"%9\"\n")
                   .arg(ws.name, ws.department, ws.designation)
                   .arg(ws.courseCount)
                   .arg(ws.rawHours, 0, 'f', 1)
                   .arg(ws.effectiveHours, 0, 'f', 2)
                   .arg(ws.maxHours, 0, 'f', 1)
                   .arg(util, 0, 'f', 1)
                   .arg(status);
    }
    file.close();
    Database::logAudit(m_user.username, m_user.role, "EXPORT_CSV", "Exported workload summary report.");
    QMessageBox::information(this, "Export Complete", "Workload report exported successfully.");
}
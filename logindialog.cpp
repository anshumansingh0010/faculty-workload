#include "logindialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFrame>

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Academic ERP — Secure Sign In");
    setFixedSize(440, 520);

    setStyleSheet(R"(
        QDialog { background-color: #0F172A; }
        QFrame#Card {
            background-color: #FFFFFF;
            border-radius: 14px;
            border: 1px solid #CBD5E1;
        }
        QLabel#Title {
            font-size: 20px;
            font-weight: bold;
            color: #0F172A;
        }
        QLabel#Subtitle {
            font-size: 12px;
            color: #64748B;
        }
        QLabel.FieldLabel {
            font-size: 12px;
            font-weight: bold;
            color: #334155;
        }
        QLineEdit {
            padding: 10px 12px;
            border: 1.5px solid #CBD5E1;
            border-radius: 8px;
            font-size: 13px;
            background-color: #F8FAFC;
            color: #0F172A;
        }
        QLineEdit:focus {
            border: 1.5px solid #2563EB;
            background-color: #FFFFFF;
        }
        QPushButton#SignInBtn {
            background-color: #2563EB;
            color: #FFFFFF;
            border: none;
            border-radius: 8px;
            padding: 11px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton#SignInBtn:hover { background-color: #1D4ED8; }
        QPushButton.DemoBtn {
            background-color: #F1F5F9;
            color: #1E293B;
            border: 1px solid #CBD5E1;
            border-radius: 6px;
            padding: 6px 8px;
            font-size: 11px;
            font-weight: bold;
        }
        QPushButton.DemoBtn:hover {
            background-color: #E2E8F0;
            border-color: #94A3B8;
        }
    )");

    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(24, 24, 24, 24);

    auto *card = new QFrame();
    card->setObjectName("Card");
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setSpacing(12);
    cardLayout->setContentsMargins(28, 28, 28, 24);

    auto *badge = new QLabel("FACULTY WORKLOAD & TIMETABLE ERP");
    badge->setStyleSheet("color: #2563EB; font-size: 11px; font-weight: bold; letter-spacing: 1px;");

    auto *title = new QLabel("Sign in to your account");
    title->setObjectName("Title");

    auto *subtitle = new QLabel("Role-Based Academic Resource & Scheduling Portal");
    subtitle->setObjectName("Subtitle");

    auto *userLbl = new QLabel("Username");
    userLbl->setProperty("class", "FieldLabel");
    usernameEdit = new QLineEdit();
    usernameEdit->setPlaceholderText("Enter username (e.g., admin, hod_cs, ananya)");

    auto *passLbl = new QLabel("Password");
    passLbl->setProperty("class", "FieldLabel");
    passwordEdit = new QLineEdit();
    passwordEdit->setPlaceholderText("Enter password");
    passwordEdit->setEchoMode(QLineEdit::Password);

    showPasswordCheck = new QCheckBox("Show password");
    showPasswordCheck->setStyleSheet("color: #475569; font-size: 12px;");

    errorLabel = new QLabel();
    errorLabel->setStyleSheet("color: #DC2626; font-size: 12px; font-weight: bold;");
    errorLabel->setWordWrap(true);
    errorLabel->hide();

    auto *signInBtn = new QPushButton("Sign In");
    signInBtn->setObjectName("SignInBtn");
    signInBtn->setCursor(Qt::PointingHandCursor);

    // Quick Demo Role Buttons
    auto *demoHeader = new QLabel("Quick-Fill Demo Role Credentials:");
    demoHeader->setStyleSheet("color: #64748B; font-size: 11px; margin-top: 6px;");

    auto *demoRow = new QHBoxLayout();
    demoRow->setSpacing(6);

    auto *adminBtn = new QPushButton("Admin");
    adminBtn->setProperty("class", "DemoBtn");
    adminBtn->setCursor(Qt::PointingHandCursor);

    auto *hodBtn = new QPushButton("HOD (CS)");
    hodBtn->setProperty("class", "DemoBtn");
    hodBtn->setCursor(Qt::PointingHandCursor);

    auto *facBtn = new QPushButton("Faculty");
    facBtn->setProperty("class", "DemoBtn");
    facBtn->setCursor(Qt::PointingHandCursor);

    demoRow->addWidget(adminBtn);
    demoRow->addWidget(hodBtn);
    demoRow->addWidget(facBtn);

    cardLayout->addWidget(badge);
    cardLayout->addWidget(title);
    cardLayout->addWidget(subtitle);
    cardLayout->addSpacing(6);
    cardLayout->addWidget(userLbl);
    cardLayout->addWidget(usernameEdit);
    cardLayout->addWidget(passLbl);
    cardLayout->addWidget(passwordEdit);
    cardLayout->addWidget(showPasswordCheck);
    cardLayout->addWidget(errorLabel);
    cardLayout->addSpacing(4);
    cardLayout->addWidget(signInBtn);
    cardLayout->addWidget(demoHeader);
    cardLayout->addLayout(demoRow);

    outerLayout->addWidget(card);

    connect(showPasswordCheck, &QCheckBox::toggled, this, [this](bool checked) {
        passwordEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
    });
    connect(signInBtn, &QPushButton::clicked, this, &LoginDialog::handleLogin);
    connect(passwordEdit, &QLineEdit::returnPressed, this, &LoginDialog::handleLogin);
    connect(adminBtn, &QPushButton::clicked, this, &LoginDialog::fillAdminDemo);
    connect(hodBtn, &QPushButton::clicked, this, &LoginDialog::fillHodDemo);
    connect(facBtn, &QPushButton::clicked, this, &LoginDialog::fillFacultyDemo);
}

void LoginDialog::fillAdminDemo() {
    usernameEdit->setText("admin");
    passwordEdit->setText("admin123");
    errorLabel->hide();
}

void LoginDialog::fillHodDemo() {
    usernameEdit->setText("hod_cs");
    passwordEdit->setText("hod123");
    errorLabel->hide();
}

void LoginDialog::fillFacultyDemo() {
    usernameEdit->setText("ananya");
    passwordEdit->setText("fac123");
    errorLabel->hide();
}

void LoginDialog::handleLogin() {
    const QString user = usernameEdit->text().trimmed();
    const QString pass = passwordEdit->text();

    if (user.isEmpty() || pass.isEmpty()) {
        errorLabel->setText("Please enter both username and password.");
        errorLabel->show();
        return;
    }

    if (Database::authenticateUser(user, pass, m_user)) {
        accept();
    } else {
        errorLabel->setText("Invalid username or password. Try a demo button below.");
        errorLabel->show();
        passwordEdit->clear();
        passwordEdit->setFocus();
    }
}
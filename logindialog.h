#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QCheckBox>
#include "database.h"

class LoginDialog : public QDialog {
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    UserAccount authenticatedUser() const { return m_user; }

private slots:
    void handleLogin();
    void fillAdminDemo();
    void fillHodDemo();
    void fillFacultyDemo();

private:
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QCheckBox *showPasswordCheck;
    QLabel *errorLabel;
    UserAccount m_user;
};

#endif // LOGINDIALOG_H
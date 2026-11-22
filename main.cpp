#include <QApplication>
#include <QStyleFactory>
#include <QMessageBox>
#include "database.h"
#include "logindialog.h"
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    if (!Database::init()) {
        QMessageBox::critical(nullptr, "Startup Error",
                              "Failed to initialize SQLite database.");
        return 1;
    }

    // Session loop allows seamless Logout & Role Switching without restarting
    while (true) {
        LoginDialog login;
        if (login.exec() != QDialog::Accepted) {
            break;
        }

        MainWindow window(login.authenticatedUser());
        window.show();
        app.exec();

        if (!window.isLogoutRequested()) {
            break;
        }
    }

    return 0;
}
#include <QApplication>
#include "loginwindow.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    LoginWindow loginWindow;

    // connect signal // handle slot
    QObject::connect(&loginWindow, &LoginWindow::loginSuccessful, [&](const QString &username) {
        MainWindow *mainWindow = new MainWindow(username);
        mainWindow->show();
    });

    loginWindow.show();

    return a.exec();// event loop
}

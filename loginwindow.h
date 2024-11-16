#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>


namespace Ui {
class LoginWindow;
}

class LoginWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();
    void onFirebaseReply();

signals:
    void loginSuccessful(const QString &username);

private slots:
    void on_signUp_but_clicked();

    void on_signIn_but_clicked();

private:
    Ui::LoginWindow *ui;

};

#endif

#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QLineEdit>

#include <QMainWindow>

namespace Ui {
class mainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(const QString &username, QWidget *parent = nullptr);
    ~MainWindow();
    void loadUserData(const QString &username);
private slots:
    void on_saveButton_clicked();

    void on_quit_button_clicked();

    void on_clear_button_clicked();

    void on_changeIMG_but_clicked();

    void on_delete_button_clicked();

private:
    Ui::mainWindow *ui;
    QString username;
    void checkInput(QLineEdit *lineEdit, const QRegularExpression &regExp);
};
#endif

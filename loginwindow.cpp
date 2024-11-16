#include "loginwindow.h"
#include "ui_loginwindow.h"
#include "mainwindow.h"
#include <QMessageBox>
#include <QCryptographicHash>
#include <QtNetwork>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>

LoginWindow::LoginWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::LoginWindow)
{
    ui->setupUi(this);
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

// Hash password
QString hashPassword(const QString &password) {
    QByteArray hashedPassword = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    return hashedPassword.toHex();
}


void LoginWindow::on_signIn_but_clicked()
{
    QString username = ui->user_input->text();
    QString password = ui->pw_input->text();

    // Hash the password
    QString hashedPassword = hashPassword(password);

    // Firebase Account URL
    QString firebaseURL = "https://data-20655-default-rtdb.firebaseio.com/Account.json";

    // Create a network manager
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    // Create a request for Firebase (GET request)
    QNetworkRequest request;
    request.setUrl(QUrl(firebaseURL));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "Data/json");

    // Send GET request to retrieve the accounts data
    QNetworkReply* reply = manager->get(request);

    // Connect the reply finished signal to a slot
    connect(reply, &QNetworkReply::finished, this, &LoginWindow::onFirebaseReply);
}

void LoginWindow::onFirebaseReply()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) return;

    if (reply->error() != QNetworkReply::NoError) {
        QMessageBox::warning(this, "Login", "Failed to connect to Firebase: " + reply->errorString());
        return;
    }

    // Parse the response
    QByteArray responseData = reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
    QJsonObject jsonObject = jsonResponse.object();

    QString storedUsername;
    QString storedPassword;
    bool loginSuccessful = false;

    // Iterate over the retrieved JSON data (accounts data)
    for (auto key : jsonObject.keys()) {
        QJsonObject account = jsonObject[key].toObject();
        storedUsername = account["username"].toString();
        storedPassword = account["password"].toString();

        // Compare the username and hashed password
        if (storedUsername == ui->user_input->text() && storedPassword == hashPassword(ui->pw_input->text())) {
            loginSuccessful = true;
            break;
        }
    }

    // Close the reply object
    reply->deleteLater();

    // Handle the login result
    if (loginSuccessful) {
        // Login successful, open MainWindow
        MainWindow *mainWindow = new MainWindow(storedUsername);
        mainWindow->loadUserData(storedUsername);
        mainWindow->show();
        this->close();
    } else {
        QMessageBox::warning(this, "Login", "Incorrect username or password!\nPlease try again!");
    }
}


void LoginWindow::on_signUp_but_clicked()
{
    QString username = ui->user_input->text();
    QString password = ui->pw_input->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Sign Up", "Username and password cannot be blank!");
        return;
    }



    // Hash Password
    QString hashedPassword = hashPassword(password);

    // Firebase Account-user URL
    QString firebaseURL = "https://data-20655-default-rtdb.firebaseio.com/Account/" + username + ".json";

    // create json object
    QJsonObject newUser;
    newUser["username"] = username;
    newUser["password"] = hashedPassword;

    //create network manager
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    //request
    QNetworkRequest request;
    request.setUrl(QUrl(firebaseURL));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Put data - Firebase (PUT request)
    QNetworkReply* reply = manager->put(request, QJsonDocument(newUser).toJson());

    // Xử lý phản hồi từ Firebase
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            QMessageBox::information(this, "Sign Up", "Account created successfully!");
        } else {
            QMessageBox::warning(this, "Sign Up", "Failed to create account: " + reply->errorString());
        }
        reply->deleteLater();
    });
}

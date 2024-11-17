#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QPixmap>
#include <QLabel>
#include <QImage>
#include <QImageReader>
#include <QByteArray>
#include <QBuffer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpressionValidator>
#include <QLineEdit>
#include <QDebug>


MainWindow::MainWindow(const QString &username, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::mainWindow),
    username(username)
{
    ui->setupUi(this);
    this->showMaximized();

    QRegularExpression nameRegExp("[A-Za-z\\s]+");
    QRegularExpression phoneRegExp("\\d{10}");
    QRegularExpression cccdRegExp("\\d{12}");

    auto nameValidator = new QRegularExpressionValidator(nameRegExp, this);
    auto phoneValidator = new QRegularExpressionValidator(phoneRegExp, this);
    auto cccdValidator = new QRegularExpressionValidator(cccdRegExp, this);

    ui->name_input->setValidator(nameValidator);
    ui->placeofbirth_input->setValidator(nameValidator);
    ui->address_input->setValidator(nameValidator);
    ui->phonenum_input->setValidator(phoneValidator);
    ui->ci_input->setValidator(cccdValidator);
}

MainWindow::~MainWindow()
{
    delete ui;
}



QString imageBase64Temp = "";

// Load user data from database
void MainWindow::loadUserData(const QString &username)
{
    // user URL
    QString firebaseURL = QString("https://data-20655-default-rtdb.firebaseio.com/User Data/%1.json").arg(username);

    // Get data
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QNetworkRequest request;
    request.setUrl(QUrl(firebaseURL));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "Data/json");

    QNetworkReply* reply = manager->get(request);

    // handle reply
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox::warning(this, "Error", "Failed to load data: " + reply->errorString());
            reply->deleteLater();
            return;
        }

        // Parse Json data
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
        QJsonObject userData = jsonResponse.object();

        // show data
        ui->name_input->setText(userData["Name"].toString());
        ui->ci_input->setText(userData["Citizen identification"].toString());
        ui->dateofbirth_input->setDate(QDate::fromString(userData["Date of birth"].toString(), "dd/MM/yyyy"));
        ui->placeofbirth_input->setText(userData["Place of birth"].toString());
        ui->email_input->setText(userData["Email"].toString());
        ui->address_input->setText(userData["Address"].toString());
        ui->phonenum_input->setText(userData["Phone number"].toString());

        QString gender = userData["Gender"].toString();
        if (gender == "male") {
            ui->male_input->setChecked(true);
        } else {
            ui->female_input->setChecked(true);
        }

        // Load image
        if (userData.contains("Image")) {
            QByteArray byteArray = QByteArray::fromBase64(userData["Image"].toString().toUtf8());
            QImage image;
            image.loadFromData(byteArray);
            if (!image.isNull()) {
                ui->picture_label->setPixmap(QPixmap::fromImage(image));
                ui->picture_label->setScaledContents(true);
                ui->picture_label->setMinimumSize(150, 200);
            }
        }

        reply->deleteLater();
    });
}


void MainWindow::on_saveButton_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::warning(this, "Warning!", "Your information will be provided to the administrator.", QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
    QString name = ui->name_input->text();
    QString CI = ui->ci_input->text();
    QString dateOfBirth = ui->dateofbirth_input->text();
    QString placeOfBirth = ui->placeofbirth_input->text();
    QString email = ui->email_input->text();
    QString address = ui->address_input->text();
    QString gender = ui->male_input->isChecked() ? "male" : "female";
    QString phoneNumber = ui->phonenum_input->text();

    if (name.isEmpty() || CI.isEmpty() || phoneNumber.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all required fields!");
        return;
    }

    QJsonObject userData;
    userData["Name"] = name;
    userData["Citizen identification"] = CI;
    userData["Date of birth"] = dateOfBirth;
    userData["Place of birth"] = placeOfBirth;
    userData["Gender"] = gender;
    userData["Email"] = email;
    userData["Address"] = address;
    userData["Phone number"] = phoneNumber;

    // Image base64
    if (!imageBase64Temp.isEmpty()) {
        userData["Image"] = imageBase64Temp;
    }

    // user data URL
    QString firebaseURL = QString("https://data-20655-default-rtdb.firebaseio.com/User Data/%1.json").arg(username);

    // Request
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QNetworkRequest request;
    request.setUrl(QUrl(firebaseURL));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "Data/json");

    QNetworkReply* reply = manager->put(request, QJsonDocument(userData).toJson());

    // Handle reply
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            QMessageBox::information(this, "Save", "Data saved successfully!");
        } else {
            QMessageBox::warning(this, "Save", "Failed to save data: " + reply->errorString());
        }
        reply->deleteLater();
    });
    }
}


void MainWindow::on_quit_button_clicked()
{
    this->close();
}

void MainWindow::on_clear_button_clicked()
{
    ui->name_input->clear();
    ui->ci_input->clear();
    ui->dateofbirth_input->setDate(QDate::currentDate());
    ui->placeofbirth_input->clear();
    ui->male_input->setChecked(false);
    ui->female_input->setChecked(false);
    ui->email_input->clear();
    ui->address_input->clear();
    ui->phonenum_input->clear();
}


void MainWindow::on_changeIMG_but_clicked()
{
    // chose image
    QString imagePath = QFileDialog::getOpenFileName(this, "Select Image", "", "Images (*.png *.xpm *.jpg *.bmp)");

    // check select
    if (!imagePath.isEmpty()) {
        QPixmap pixmap(imagePath);

        // check image file
        if (pixmap.isNull()) {
            QMessageBox::warning(this, "ERROR", "Cannot open the image file!");
            return;
        }

        // show image in Qlabel
        ui->picture_label->setPixmap(pixmap);
        ui->picture_label->setScaledContents(true);
        ui->picture_label->setMinimumSize(150, 200);

        // Encode img -> base64
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        buffer.open(QIODevice::WriteOnly);
        pixmap.toImage().save(&buffer, "PNG");
        imageBase64Temp = byteArray.toBase64();
    }
}


void MainWindow::on_delete_button_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::warning(this, "Warning!", "Your account will be deleted!", QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // URL Firebase User Data
        QString firebaseUserDataURL = QString("https://data-20655-default-rtdb.firebaseio.com/User Data/%1.json").arg(username);

        // URL Firebase Account
        QString firebaseAccountURL = QString("https://data-20655-default-rtdb.firebaseio.com/Account/%1.json").arg(username);

        QNetworkAccessManager* manager = new QNetworkAccessManager(this);

        // delete User Data
        QNetworkRequest userDataRequest;
        userDataRequest.setUrl(QUrl(firebaseUserDataURL));
        userDataRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QNetworkReply* userDataReply = manager->deleteResource(userDataRequest);

        // Handle delete User Data
        connect(userDataReply, &QNetworkReply::finished, this, [=]() {
            if (userDataReply->error() == QNetworkReply::NoError) {
                // Delete Account
                QNetworkRequest accountRequest;
                accountRequest.setUrl(QUrl(firebaseAccountURL));
                accountRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

                QNetworkReply* accountReply = manager->deleteResource(accountRequest);

                connect(accountReply, &QNetworkReply::finished, this, [=]() {
                    if (accountReply->error() == QNetworkReply::NoError) {
                        QMessageBox::information(this, "Success", "Your account was deleted!");
                        this->close();
                    } else {
                        QMessageBox::warning(this, "Error", "Failed to delete account: " + accountReply->errorString());
                    }
                    accountReply->deleteLater();
                });

            } else {
                QMessageBox::warning(this, "Error", "Failed to delete user data: " + userDataReply->errorString());
            }
            userDataReply->deleteLater();
        });
    }
}

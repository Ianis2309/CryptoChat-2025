#include "mainwindow.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScreen>
#include <QApplication>
#include <QRegularExpression>
#include <QPainter>
#include <QMessageBox>
#include "chatwindow.h"
#include <QDebug>
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(QApplication::primaryScreen()->availableGeometry().size());
    move(0, 0);

    stack = new QStackedWidget(this);
    stack->addWidget(loginPage());
    stack->addWidget(signupPage());

    setCentralWidget(stack);
    stack->setCurrentIndex(0);
}

void MainWindow::paintEvent(QPaintEvent *event){
    QPainter painter(this);

    QPixmap background(":/CCBackground2.png");

    if (background.isNull()) {
        qWarning() << "Background image couldn't be loaded.";
        return;
    }

    QPixmap scaled = background.scaled(this->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    painter.drawPixmap(0, 0, scaled);
}

QWidget* MainWindow::loginPage()
{
    QWidget *page = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout(page);

    QWidget *form = new QWidget;
    QVBoxLayout *formLayout = new QVBoxLayout(form);

    QLabel *title = new QLabel("CryptoChat - Log In");
    title->setAlignment(Qt::AlignCenter);
    title->setFont(QFont("Arial", 24, QFont::Bold));

    QLineEdit *phoneEdit = new QLineEdit;
    phoneEdit->setPlaceholderText("Phone number");
    phoneEdit->setFixedWidth(300);
    phoneEdit->setFixedHeight(30);

    QLabel *phoneErrorLabel = new QLabel;
    phoneErrorLabel->setStyleSheet("color: red;");
    phoneErrorLabel->setVisible(false);

    QWidget *passwordField = new QWidget;
    QHBoxLayout* passwordLayout = new QHBoxLayout(passwordField);
    passwordLayout->setContentsMargins(0, 0, 0, 0);

    QLineEdit *passwordEdit = new QLineEdit;
    passwordEdit->setPlaceholderText("Password");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setFixedWidth(300);
    passwordEdit->setFixedHeight(30);

    QPushButton *toggleBtn = new QPushButton;
    toggleBtn->setCheckable(true);
    toggleBtn->setFixedSize(30, 30);
    toggleBtn->setStyleSheet("border: none;");
    toggleBtn->setIcon(QIcon(":/eyeclosed.png"));
    toggleBtn->setIconSize(QSize(20, 20));

    passwordLayout->addWidget(passwordEdit);
    passwordLayout->addWidget(toggleBtn);

    connect(toggleBtn, &QPushButton::toggled, this, [=](bool checked){
        passwordEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
        toggleBtn->setIcon(QIcon(checked ? ":/eyeopen.png" : ":/eyeclosed.png"));
    });

    passwordField->setContentsMargins(35, 0, 0, 0);

    QLabel* errorLabel = new QLabel;
    errorLabel->setStyleSheet("color: red;");
    errorLabel->setVisible(false);

    QPushButton *loginBtn = new QPushButton("Log In");
    loginBtn->setFixedWidth(200);
    loginBtn->setFixedHeight(30);

    QPushButton *toSignupBtn = new QPushButton("Don't have an account? Sign Up");
    toSignupBtn->setFixedWidth(200);
    toSignupBtn->setFixedHeight(30);

    connect(toSignupBtn, &QPushButton::clicked, this, &MainWindow::showSignup);

    connect(loginBtn, &QPushButton::clicked, this, [=]() {
        QString phone = phoneEdit->text();
        QRegularExpression regexPhone("^07[0-9]{8}$");

        if(phoneEdit->text().isEmpty() || passwordEdit->text().isEmpty()){
            phoneErrorLabel->setVisible(false);
            errorLabel->setText("Oops! Looks like you missed some fields.");
            errorLabel->setVisible(true);
        }
        else if(!regexPhone.match(phone).hasMatch()){
            errorLabel->setVisible(false);
            phoneErrorLabel->setText("Invalid phone number");
            phoneErrorLabel->setVisible(true);
        }
        else{
            phoneErrorLabel->setVisible(false);
            errorLabel->setVisible(false);
            try {
                socket->sendLogIn(phone.toStdString(), passwordEdit->text().toStdString());

                std::string response = socket->receiveData();
                //std::cout<<response;
                if (socket->parseHeader(response) == Header::ERROR__) {
                    QMessageBox::warning(this, "Login failed", "Wrong credentials.");
                    return;
                }

                std::string username;
                if (!socket->handleLoginOK(response, username)) {
                    QMessageBox::warning(this, "Login failed", "Invalid response.");
                    return;
                }

                ChatWindow* chat = new ChatWindow();
                chat->setUserData(QString::fromStdString(username), phone);
                chat->setSocket(socket);

                std::string agendaData = socket->receiveData();
                std::vector<Contact*> contacts;
                if (!socket->handleAgenda(agendaData, contacts)) {
                    QMessageBox::warning(this, "Error", "Failed to load agenda.");
                    return;
                }
                chat->setContacts(contacts);
                socket->sendData("timeout");

                qDebug() << contacts.size();

                for (size_t i = 0; i < contacts.size(); ++i) {
                    std::string talkData = socket->receiveData();
                    qDebug() << talkData;
                    if (socket->parseHeader(talkData) != Header::TALKS) {
                        QMessageBox::warning(this, "Error", "Expected TALKS packet, got something else.");
                        break;
                    }

                    if (talkData.size()>12){
                        QString contactPhone = QString::fromStdString(talkData.substr(1, 10));
                        QString allMessages = QString::fromLatin1(talkData.c_str() + 11, talkData.size() - 11);
                        chat->loadMessagesFromServer(contactPhone, allMessages);
                    }
                    socket->sendData("timeout");
                }

                socket->startListening([this, chat](const std::string& data) {
                    QMetaObject::invokeMethod(this, [this, data, chat]() {
                        chat->handleServerMessage(data);
                    }, Qt::QueuedConnection);
                });

                chat->show();
                this->close();

            } catch (const std::exception& e) {
                QMessageBox::critical(this, "Error", e.what());
            }
            //ChatWindow *chat = new ChatWindow();
            //chat->setUserData("user", phoneEdit->text());
            //chat->show();
            //this->close();
        }
    });

    formLayout->addWidget(title);
    formLayout->addSpacing(30);
    formLayout->addWidget(phoneEdit, 0, Qt::AlignHCenter);
    formLayout->addWidget(phoneErrorLabel, 0, Qt::AlignHCenter);
    formLayout->addWidget(passwordField, 0, Qt::AlignHCenter);
    formLayout->addWidget(errorLabel, 0, Qt::AlignHCenter);
    formLayout->addWidget(loginBtn, 0, Qt::AlignHCenter);
    formLayout->addWidget(toSignupBtn, 0, Qt::AlignHCenter);

    mainLayout->addStretch();
    mainLayout->addWidget(form, 0, Qt::AlignCenter);
    mainLayout->addStretch();

    return page;
}

QWidget* MainWindow::signupPage()
{
    QWidget *page = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout(page);

    QWidget *form = new QWidget;
    QVBoxLayout *formLayout = new QVBoxLayout(form);

    QLabel *title = new QLabel("CryptoChat - Sign Up");
    title->setAlignment(Qt::AlignCenter);
    title->setFont(QFont("Arial", 24, QFont::Bold));

    QLineEdit *phoneEdit = new QLineEdit;
    phoneEdit->setPlaceholderText("Phone number");
    phoneEdit->setFixedWidth(300);
    phoneEdit->setFixedHeight(30);

    QLabel *phoneErrorLabel = new QLabel;
    phoneErrorLabel->setStyleSheet("color: red;");
    phoneErrorLabel->setVisible(false);

    QLineEdit *usernameEdit = new QLineEdit;
    usernameEdit->setPlaceholderText("Username");
    usernameEdit->setFixedWidth(300);
    usernameEdit->setFixedHeight(30);

    QWidget *passwordField = new QWidget;
    QHBoxLayout* passwordLayout = new QHBoxLayout(passwordField);
    passwordLayout->setContentsMargins(0, 0, 0, 0);

    QLineEdit *passwordEdit = new QLineEdit;
    passwordEdit->setPlaceholderText("Password");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setFixedWidth(300);
    passwordEdit->setFixedHeight(30);

    QPushButton *toggleBtn = new QPushButton;
    toggleBtn->setCheckable(true);
    toggleBtn->setFixedSize(30, 30);
    toggleBtn->setStyleSheet("border: none;");
    toggleBtn->setIcon(QIcon(":/eyeclosed.png"));
    toggleBtn->setIconSize(QSize(20, 20));

    passwordLayout->addWidget(passwordEdit);
    passwordLayout->addWidget(toggleBtn);

    connect(toggleBtn, &QPushButton::toggled, this, [=](bool checked){
        passwordEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
        toggleBtn->setIcon(QIcon(checked ? ":/eyeopen.png" : ":/eyeclosed.png"));
    });

    passwordField->setContentsMargins(35, 0, 0, 0);

    QLabel* errorLabel = new QLabel;
    errorLabel->setStyleSheet("color: red;");
    errorLabel->setVisible(false);

    QPushButton *signupBtn = new QPushButton("Create Account");
    signupBtn->setFixedWidth(200);
    signupBtn->setFixedHeight(30);

    QLabel* accountCreatedLabel = new QLabel;
    accountCreatedLabel->setStyleSheet("color: green;");
    accountCreatedLabel->setVisible(false);

    QPushButton *toLoginBtn = new QPushButton("Already have an account? Log In");
    toLoginBtn->setFixedWidth(200);
    toLoginBtn->setFixedHeight(30);

    connect(signupBtn, &QPushButton::clicked, this, [=]() {
        QString phone=phoneEdit->text();
        QRegularExpression regexPhone("^07[0-9]{8}$");

        if(phoneEdit->text().isEmpty() || usernameEdit->text().isEmpty() || passwordEdit->text().isEmpty()){
            phoneErrorLabel->setVisible(false);
            accountCreatedLabel->setVisible(false);
            errorLabel->setText("Oops! Looks like you missed some fields.");
            errorLabel->setVisible(true);
        }
        else if(!regexPhone.match(phone).hasMatch()){
            errorLabel->setVisible(false);
            accountCreatedLabel->setVisible(false);
            phoneErrorLabel->setText("Invalid phone number.");
            phoneErrorLabel->setVisible(true);
        }
        else{
            phoneErrorLabel->setVisible(false);
            errorLabel->setVisible(false);
            accountCreatedLabel->setText("Account created successfully");
            accountCreatedLabel->setVisible(true);
            try {
                socket->sendSignUp(phone.toStdString(), usernameEdit->text().toStdString(), passwordEdit->text().toStdString());

                std::string response = socket->receiveData();
                std::cout<<response;
                if (socket->parseHeader(response) == Header::ERROR__) {
                    QMessageBox::warning(this, "SignUp failed", "Account already exists");
                    return;
                }

                if (socket->parseHeader(response) != Header::_OK_) {
                    QMessageBox::warning(this, "SignUp failed", "Invalid response.");
                    return;
                }

                ChatWindow* chat = new ChatWindow();
                chat->setUserData(usernameEdit->text(), phone);
                chat->setSocket(socket);
                socket->startListening([this, chat](const std::string& data) {
                    QMetaObject::invokeMethod(this, [this, data, chat]() {
                        chat->handleServerMessage(data);
                    }, Qt::QueuedConnection);
                });

                /*std::string agendaData = socket->receiveData();
                std::vector<Contact*> contacts;
                if (socket->handleAgenda(agendaData, contacts)) {
                    chat->setContacts(contacts);
                }*/
                chat->show();
                this->close();
            } catch (const std::exception& e) {
                QMessageBox::critical(this, "Error", e.what());
            }
        }
    });

    connect(toLoginBtn, &QPushButton::clicked, this, &MainWindow::showLogin);

    formLayout->addWidget(title);
    formLayout->addSpacing(30);
    formLayout->addWidget(phoneEdit, 0, Qt::AlignHCenter);
    formLayout->addWidget(phoneErrorLabel, 0, Qt::AlignHCenter);
    formLayout->addWidget(usernameEdit, 0, Qt::AlignHCenter);
    formLayout->addWidget(passwordField, 0, Qt::AlignHCenter);
    formLayout->addWidget(errorLabel, 0, Qt::AlignHCenter);
    formLayout->addWidget(signupBtn, 0, Qt::AlignHCenter);
    formLayout->addWidget(accountCreatedLabel, 0, Qt::AlignHCenter);
    formLayout->addWidget(toLoginBtn, 0, Qt::AlignHCenter);

    mainLayout->addStretch();
    mainLayout->addWidget(form, 0, Qt::AlignCenter);
    mainLayout->addStretch();

    return page;
}

void MainWindow::showSignup() {
    stack->setCurrentIndex(1);
}

void MainWindow::showLogin() {
    stack->setCurrentIndex(0);
}

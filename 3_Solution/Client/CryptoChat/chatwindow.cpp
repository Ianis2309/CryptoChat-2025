#include "chatwindow.h"
#include <QMessageBox>
#include <QPushButton>
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QPainter>
#include <QPainterPath>
#include <QTime>
#include <QTimer>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QAbstractTextDocumentLayout>
#include <QLineEdit>
#include <QFile>
#include <QFileDialog>
#include <QDesktopServices>
#include <QStandardPaths>
#include <mainwindow.h>
#include <QMenu>
#include "BackgroundScrollArea.h"
//#include "crypto_utils.h"

ChatWindow::ChatWindow(QWidget *parent)
    : QMainWindow(parent)
{
    showMaximized();

    QWidget *central = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(central);

    contactListWidget = new QWidget;
    contactListLayout = new QVBoxLayout(contactListWidget);
    contactListLayout->setAlignment(Qt::AlignTop);

    QPushButton* settingsBtn = new QPushButton("⚙");
    settingsBtn->setFixedSize(30, 30);
    settingsBtn->setStyleSheet(R"(
    QPushButton {
        font-size: 20px;
        padding-bottom: 5px;
        border: none;
        color: white;
        background-color: transparent;
    }
    QPushButton:hover {
        background-color: #444;
        border-radius: 6px;
    }
)");
    connect(settingsBtn, &QPushButton::clicked, this, &ChatWindow::showSettingsDialog);

    QPushButton *addContactButton = new QPushButton("+");
    addContactButton->setFixedSize(30, 30);
    addContactButton->setStyleSheet(R"(
        QPushButton {
            font-size: 20px;
            padding-bottom: 5px;
            border: none;
            background-color: #444;
            color: white;
            border-radius: 15px;
        }
        QPushButton:hover {
            background-color: #666;
        }
    )");
    connect(addContactButton, &QPushButton::clicked, this, &ChatWindow::showAddContactDialog);

    QWidget* topBar = new QWidget;
    QHBoxLayout* topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(5, 5, 5, 5);
    topLayout->setSpacing(10);

    QLabel* chatsLabel = new QLabel("Chats");
    chatsLabel->setStyleSheet("font-weight: bold; font-size: 22px; color: white;");
    chatsLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    topLayout->addWidget(chatsLabel, 0, Qt::AlignVCenter);
    topLayout->addStretch();
    topLayout->addWidget(settingsBtn, 0, Qt::AlignVCenter);
    topLayout->addWidget(addContactButton, 0, Qt::AlignVCenter);

    contactListLayout->addWidget(topBar);

    searchInput = new QLineEdit;
    searchInput->setPlaceholderText("Search contacts...");
    searchInput->setStyleSheet("padding: 5px; font-size: 13px;");
    searchInput->setClearButtonEnabled(true);
    contactListLayout->addWidget(searchInput);
    connect(searchInput, &QLineEdit::textChanged, this, &ChatWindow::filterContacts);

    contactScrollArea = new QScrollArea;
    contactScrollArea->setWidgetResizable(true);
    contactScrollArea->setWidget(contactListWidget);
    contactScrollArea->setFixedWidth(300);

    //addContact("Jonescu", "0743828787");
    //addContact("Birsan", "0746514289");
    //addContact("Despina", "07498658");

    chatArea = new QWidget;
    chatLayout = new QVBoxLayout(chatArea);

    placeholderWidget = new QWidget;
    QVBoxLayout *placeholderLayout = new QVBoxLayout(placeholderWidget);
    placeholderLayout->setAlignment(Qt::AlignCenter);

    placeholderLayout->addStretch();

    QLabel *logoLabel = new QLabel;
    QPixmap logoPixmap(":/logo2 (1).png");
    logoLabel->setPixmap(logoPixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logoLabel->setAlignment(Qt::AlignCenter);

    QLabel *textLabel1 = new QLabel("Welcome to CryptoChat!");
    textLabel1->setAlignment(Qt::AlignCenter);
    textLabel1->setStyleSheet("font-size: 22px; color: gray; font-weight: bold;");

    placeholderLayout->addWidget(logoLabel);
    placeholderLayout->addWidget(textLabel1);
    placeholderLayout->addStretch();

    QLabel *textLabel2 = new QLabel("End-to-end encrypted");
    textLabel2->setAlignment(Qt::AlignCenter);
    textLabel2->setStyleSheet("font-size: 14px; color: gray;");

    placeholderLayout->addWidget(textLabel2);

    chatLayout->addWidget(placeholderWidget);

    chatMessagesWidget = new QWidget;
    chatMessagesLayout = new QVBoxLayout(chatMessagesWidget);
    chatMessagesLayout->setAlignment(Qt::AlignTop);

    headerWidget = new QWidget;
    headerWidget->setStyleSheet(R"(
        background-color: rgb(35, 60, 100);
        border-radius: 10px;
    )");

    headerWidget->setFixedHeight(70);
    headerWidget->setVisible(false);
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(10, 5, 10, 5);

    headerImage = new QLabel;
    headerImage->setFixedSize(48, 48);
    headerImage->setScaledContents(true);
    headerImage->setPixmap(QPixmap(":/userimg.png")); // default
    headerImage->setStyleSheet("border-radius: 24px;");

    QVBoxLayout *namePhoneLayout = new QVBoxLayout;
    namePhoneLayout->setContentsMargins(10, 0, 0, 0);
    headerUsernameLabel = new QLabel("Username");
    headerUsernameLabel->setStyleSheet("font-weight: bold; font-size: 16px; color: white;");
    headerPhoneLabel = new QLabel("07xxxxxxxx");
    headerPhoneLabel->setStyleSheet("font-size: 12px; color: gray;");
    namePhoneLayout->addWidget(headerUsernameLabel);
    namePhoneLayout->addWidget(headerPhoneLabel);

    optionsButton = new QPushButton("⋮");
    optionsButton->setFixedSize(24, 24);
    optionsButton->setStyleSheet(R"(
    QPushButton {
        font-size: 26px;
        color: white;
        background-color: transparent;
        border: none;
    }
    QPushButton:hover {
        background-color: #444;
        border-radius: 4px;
    }
)");

    connect(optionsButton, &QPushButton::clicked, this, [this]() {
        QMenu menu;
        QAction* deleteAction = menu.addAction("Delete contact");
        QAction* chosen = menu.exec(QCursor::pos());
        if (chosen == deleteAction) {
            if (!currentContact.isEmpty()) {
                socket->sendDelAccount(currentContact.toStdString());
                if(del){
                    removeContact(currentContact);
                    QMessageBox::information(this, "Delete contact", "Contact deleted successfully.");
                }
                else
                    QMessageBox::warning(this, "Delete contact", "Couldn't delete contact.");
            }
        }
    });

    headerLayout->addWidget(headerImage);
    headerLayout->addLayout(namePhoneLayout);
    headerLayout->addStretch();
    headerLayout->addWidget(optionsButton);
    headerWidget->setVisible(false);

    chatLayout->insertWidget(0, headerWidget);


    chatScrollArea = new BackgroundScrollArea;
    static_cast<BackgroundScrollArea*>(chatScrollArea)->setBackgroundImage(":/CCbg.png");
    chatScrollArea->setWidgetResizable(true);
    chatScrollArea->setWidget(chatMessagesWidget);
    chatScrollArea->hide();

    messageInput = new QTextEdit;
    messageInput->setPlaceholderText("Type a message");
    messageInput->setFixedHeight(50);
    connect(messageInput, &QTextEdit::textChanged, this, &ChatWindow::handleAutoWrapInput);
    messageInput->hide();

    sendButton = new QPushButton("Send");
    sendButton->setFixedWidth(100);
    sendButton->hide();

    attachButton = new QPushButton("📎");
    attachButton->setFixedSize(40, 40);
    attachButton->setStyleSheet(R"(
        QPushButton {
           font-size: 22px;
            border: none;
            background-color: transparent;
            color: white;
        }
        QPushButton:hover {
           background-color: #444;
           border-radius: 6px;
        }
    )");
    connect(attachButton, &QPushButton::clicked, this, &ChatWindow::handleAttachFile);
    attachButton->hide();

    QHBoxLayout *inputLayout = new QHBoxLayout;
    inputLayout->addWidget(attachButton);
    inputLayout->addWidget(messageInput);
    inputLayout->addWidget(sendButton);

    chatLayout->addWidget(chatScrollArea);
    chatLayout->addLayout(inputLayout);

    mainLayout->addWidget(contactScrollArea);
    mainLayout->addWidget(chatArea);

    connect(sendButton, &QPushButton::clicked, this, &ChatWindow::sendMessage);

    setCentralWidget(central);
    setWindowTitle("CryptoChat - Chat");
}

void ChatWindow::handleServerMessage(const std::string& data) {
    Header h = socket->parseHeader(data);
    switch (h) {
    case Header::LOGOUT: {
        if(socket->parseSecondHeader(data) == Header::_OK_){
            QMessageBox::information(this, "Logout", "Logged out successfully.");
            logout = true;
        }
        else{
            QMessageBox::warning(this, "Logout", "Couldn't log out.");
            logout = false;
        }
        break;
    }
    case Header::ADDCONT: {
        std::string username;
        if (!socket->handleAddContactOK(data, username)) {
            QMessageBox::warning(this, "Add contact failed", "Invalid response.");
            return;
        }

        addContact(QString::fromStdString(username), pendingPhone);
        break;
    }
    case Header::DELCONT: {
        Header h2 = socket->parseSecondHeader(data);
        if(h2 == Header::_OK_)
            del = true;
        else
            del = false;
        break;
    }
    case Header::TEXT: {
        std::string sender, message;
        socket->handleText(data, sender, message);

        QString msg = QString::fromStdString(message);
        QString senderPhone = QString::fromStdString(sender);
        //QString decrypted = QString::fromStdString(Crypt(msg.toStdString(), currentUser.phoneNumber.toStdString()));

        chatHistory[senderPhone].append({false, false, msg, "", {}, {}});
        if (senderPhone == currentContact) {
            addMessageBubble(msg, false, false, {});
        }
        break;
    }
    /*case Header::TALKS: {
        QString contactPhone = QString::fromStdString(data.substr(1, 10));
        QString allMessages = QString::fromStdString(data.substr(11));
        //loadMessagesFromServer(contactPhone, allMessages);
        break;
    }*/
    case Header::ERROR__: {
        std::string message = std::string(data.begin()+1, data.end());
        QMessageBox::warning(this, "Add contact failed", message.c_str());
        break;
    }
    default:
        qDebug() << "Header necunoscut primit de la server";
        break;
    }
}

void ChatWindow::loadMessagesFromServer(const QString& phone, const QString& rawData)
{
    qDebug() << phone;
    QString cleanData = rawData;

    int startIndex = cleanData.indexOf('[');
    if (startIndex > 0) {
        cleanData = cleanData.mid(startIndex);
    }

    QStringList messageList = cleanData.split(QChar(0xFF), Qt::SkipEmptyParts);
    QRegularExpression regex(R"(\[(.*?)\]\s+(\d{10})\s+([\s\S]*))");

    for (QString rawMsg : messageList) {
        qDebug() << "MESAJ INDIVIDUAL: " << rawMsg;
        rawMsg = rawMsg.trimmed();

        QRegularExpressionMatch match = regex.match(rawMsg);
        if (match.hasMatch()) {
            QString timestamp = match.captured(1);
            QString sender = match.captured(2);
            QString content = match.captured(3);

            bool isUser = (sender == currentUser.phoneNumber);

            //QString decrypted = QString::fromStdString(Crypt(content.toStdString(), currentUser.phoneNumber.toStdString()));
            chatHistory[phone].append({
                isUser,
                false,
                content,
                "",
                {},
                timestamp
            });
        }
    }
}

void ChatWindow::filterContacts(const QString& text) {
    QString filter = text.toLower();

    for (auto it = contactWidgets.begin(); it != contactWidgets.end(); ++it) {
        Contact* contact = it.key();
        QFrame* frame = it.value();

        QString username = QString::fromStdString(contact->username).toLower();
        QString phone = QString::fromStdString(contact->phone).toLower();

        bool visible = username.contains(filter) || phone.contains(filter);
        frame->setVisible(visible);
    }
}

void ChatWindow::handleAttachFile() {
    QString filePath = QFileDialog::getOpenFileName(this, "Choose file");
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Error", "Could not open file.");
        return;
    }

    QByteArray fileData = file.readAll();
    file.close();

    QString fileName = QFileInfo(filePath).fileName();

    addFileBubbleCard(fileName, fileData, true, true);

    chatHistory[currentContact].append({true, true, "", fileName, fileData, {}});

    //std::vector<uint8_t> bytes(fileData.begin(), fileData.end());
    //socket->sendBinary(fileName.toStdString(), bytes);
}

void ChatWindow::addFileBubbleCard(const QString& filename, const QByteArray& fileData, bool isUser, bool animate) {
    QWidget* bubbleWidget = new QWidget;
    QHBoxLayout* bubbleLayout = new QHBoxLayout(bubbleWidget);
    bubbleLayout->setContentsMargins(10, 5, 10, 5);

    QWidget* messageContainer = new QWidget;
    QHBoxLayout* messageLayout = new QHBoxLayout(messageContainer);
    messageLayout->setContentsMargins(0, 0, 0, 0);

    QWidget* fileBubble = new QWidget;
    QVBoxLayout* bubbleContent = new QVBoxLayout(fileBubble);
    bubbleContent->setContentsMargins(10, 10, 10, 10);

    QLabel* nameLabel = new QLabel(filename);
    nameLabel->setStyleSheet("font-weight: bold; color: white; font-size: 14px;");

    QString fileType = QFileInfo(filename).suffix().toUpper();
    QString sizeStr = QString::number(fileData.size() / 1024.0 / 1024.0, 'f', 2) + " MB";
    QLabel* infoLabel = new QLabel(sizeStr + ", " + fileType + " file");
    infoLabel->setStyleSheet("color: lightgray; font-size: 12px;");

    QHBoxLayout* btnLayout = new QHBoxLayout;
    QPushButton* openBtn = new QPushButton("Open");
    QPushButton* saveBtn = new QPushButton("Save as...");
    btnLayout->addWidget(openBtn);
    btnLayout->addWidget(saveBtn);
    openBtn->setStyleSheet("padding: 4px;");
    saveBtn->setStyleSheet("padding: 4px;");

    bubbleContent->addWidget(nameLabel);
    bubbleContent->addWidget(infoLabel);
    bubbleContent->addLayout(btnLayout);

    fileBubble->setStyleSheet(QString(R"(
        background-color: %1;
        border-radius: 12px;
    )").arg(isUser ? "#552ae2" : "#1e9274"));
    fileBubble->setMaximumWidth(700);
    fileBubble->setMinimumWidth(120);

    QHBoxLayout* infoLayout = new QHBoxLayout;
    infoLayout->setContentsMargins(0, 0, 0, 0);
    infoLayout->setSpacing(5);

    QLabel* timeLabel = new QLabel(QTime::currentTime().toString("hh:mm"));
    timeLabel->setStyleSheet("font-size: 10px; color: lightgray;");
    infoLayout->addStretch();
    infoLayout->addWidget(timeLabel);

    if (isUser) {
        QLabel* checkLabel = new QLabel(animate ? "✔" : "✔✔");
        checkLabel->setStyleSheet("font-size: 10px; color: lightgray;");
        infoLayout->addWidget(checkLabel);

        if (animate) {
            QTimer::singleShot(500, this, [checkLabel]() {
                checkLabel->setText("✔✔");
                auto* effect = new QGraphicsOpacityEffect(checkLabel);
                checkLabel->setGraphicsEffect(effect);
                auto* animation = new QPropertyAnimation(effect, "opacity");
                animation->setDuration(500);
                animation->setStartValue(0.0);
                animation->setEndValue(1.0);
                animation->start(QAbstractAnimation::DeleteWhenStopped);
            });
        }
    }

    messageLayout->addWidget(fileBubble);
    messageLayout->addLayout(infoLayout);

    if (isUser) {
        bubbleLayout->addStretch();
        bubbleLayout->addWidget(messageContainer, 0, Qt::AlignRight);
    } else {
        bubbleLayout->addWidget(messageContainer, 0, Qt::AlignLeft);
        bubbleLayout->addStretch();
    }

    chatMessagesLayout->addWidget(bubbleWidget);
    QCoreApplication::processEvents();
    chatScrollArea->verticalScrollBar()->setValue(chatScrollArea->verticalScrollBar()->maximum());

    connect(openBtn, &QPushButton::clicked, this, [=]() {
        QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/" + filename;
        QFile file(tempPath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(fileData);
            file.close();
            QDesktopServices::openUrl(QUrl::fromLocalFile(tempPath));
        } else {
            QMessageBox::warning(this, "Open failed", "Could not open file.");
        }
    });

    connect(saveBtn, &QPushButton::clicked, this, [=]() {
        QString savePath = QFileDialog::getSaveFileName(this, "Save file as...", filename);
        if (!savePath.isEmpty()) {
            QFile file(savePath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(fileData);
                file.close();
                QMessageBox::information(this, "Saved", "File saved successfully.");
            } else {
                QMessageBox::warning(this, "Save failed", "Could not save file.");
            }
        }
    });
}

void ChatWindow::showSettingsDialog() {
    QDialog dialog(this);
    dialog.setWindowTitle("Settings");
    dialog.setFixedSize(400, 500);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    avatarImageLabel = new QLabel;
    QPixmap pic(currentUser.avatarPath.isEmpty() ? ":/userimg.png" : currentUser.avatarPath);
    avatarImageLabel->setPixmap(pic.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    avatarImageLabel->setAlignment(Qt::AlignCenter);

    editOverlay = new QLabel("✏️", avatarImageLabel);
    editOverlay->setAlignment(Qt::AlignCenter);
    editOverlay->setStyleSheet(R"(
        QLabel {
            background-color: rgba(0, 0, 0, 100);
            border-radius: 32px;
            font-size: 24px;
            color: white;
        }
    )");
    editOverlay->hide();

    avatarWrapper = new QWidget;
    QStackedLayout* avatarStack = new QStackedLayout(avatarWrapper);
    avatarStack->setStackingMode(QStackedLayout::StackAll);
    avatarStack->addWidget(avatarImageLabel);
    avatarStack->addWidget(editOverlay);

    avatarWrapper->setAttribute(Qt::WA_Hover);
    avatarWrapper->installEventFilter(this);
    layout->addWidget(avatarWrapper);

    connect(editOverlay, &QLabel::linkActivated, this, [=](const QString&) {
        QString imagePath = QFileDialog::getOpenFileName(this, "Choose Profile Picture", "", "Images (*.png *.jpg *.jpeg)");
        if (!imagePath.isEmpty()) {
            currentUser.avatarPath = imagePath;
            QPixmap newPic(imagePath);
            avatarImageLabel->setPixmap(newPic.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    });

    QLabel* usernameLabel = new QLabel("Username: " + currentUser.username);
    QLabel* phoneLabel = new QLabel("Phone: " + currentUser.phoneNumber);
    usernameLabel->setStyleSheet("font-size: 14px; color: white;");
    phoneLabel->setStyleSheet("font-size: 14px; color: white;");

    QPushButton* changeUsernameBtn = new QPushButton("Change Username");
    QPushButton* changePhoneBtn = new QPushButton("Change Phone Number");
    QPushButton* changePasswordBtn = new QPushButton("Change Password");

    for (auto btn : {changeUsernameBtn, changePhoneBtn, changePasswordBtn}) {
        btn->setStyleSheet("padding: 6px; font-size: 13px;");
    }

    QPushButton* logoutBtn = new QPushButton("Log Out");
    logoutBtn->setStyleSheet("color: red; font-weight: bold; padding: 6px;");

    layout->addSpacing(10);
    layout->addWidget(usernameLabel);
    layout->addWidget(phoneLabel);
    layout->addSpacing(10);
    layout->addWidget(changeUsernameBtn);
    layout->addWidget(changePhoneBtn);
    layout->addWidget(changePasswordBtn);
    layout->addStretch();
    layout->addWidget(logoutBtn, 0, Qt::AlignCenter);

    dialog.setStyleSheet(R"(
        QDialog { background-color: #2e2e2e; color: white; }
        QPushButton { background-color: #444; border: none; border-radius: 6px; }
        QPushButton:hover { background-color: #666; }
    )");

    connect(changeUsernameBtn, &QPushButton::clicked, this, [=]() {
        QDialog usernameDialog(this);
        usernameDialog.setWindowTitle("Change Username");
        QVBoxLayout* layout = new QVBoxLayout(&usernameDialog);
        QLineEdit* newUsername = new QLineEdit;
        newUsername->setPlaceholderText("New username");
        QPushButton* saveBtn = new QPushButton("Save");

        layout->addWidget(newUsername);
        layout->addWidget(saveBtn);
        connect(saveBtn, &QPushButton::clicked, [&]() {
            if (!newUsername->text().isEmpty()) {
                currentUser.username = newUsername->text();
                usernameLabel->setText("Username: " + currentUser.username);
                QMessageBox::information(this, "Success", "Username changed.");
                usernameDialog.accept();
            }
            else
                QMessageBox::warning(&usernameDialog, "Error", "Fill in all fields.");
        });
        usernameDialog.exec();
    });

    connect(changePhoneBtn, &QPushButton::clicked, this, [=]() {
        QDialog phoneDialog(this);
        phoneDialog.setWindowTitle("Change Phone Number");
        QVBoxLayout* layout = new QVBoxLayout(&phoneDialog);
        QLineEdit* newPhone = new QLineEdit;
        newPhone->setPlaceholderText("New phone number (07XXXXXXXX)");
        QPushButton* saveBtn = new QPushButton("Save");

        layout->addWidget(newPhone);
        layout->addWidget(saveBtn);
        connect(saveBtn, &QPushButton::clicked, [&]() {
            QRegularExpression regex("^07\\d{8}$");
            if (regex.match(newPhone->text()).hasMatch()) {
                currentUser.phoneNumber = newPhone->text();
                phoneLabel->setText("Phone: " + currentUser.phoneNumber);
                QMessageBox::information(this, "Success", "Phone number changed.");
                phoneDialog.accept();
            } else {
                QMessageBox::warning(this, "Invalid", "Invalid phone number.");
            }
        });
        phoneDialog.exec();
    });

    connect(changePasswordBtn, &QPushButton::clicked, this, [=]() {
        QDialog passDialog(this);
        passDialog.setWindowTitle("Change Password");
        QVBoxLayout* layout = new QVBoxLayout(&passDialog);

        QLineEdit* oldPass = new QLineEdit;
        QLineEdit* newPass = new QLineEdit;
        oldPass->setEchoMode(QLineEdit::Password);
        newPass->setEchoMode(QLineEdit::Password);
        oldPass->setPlaceholderText("Current password");
        newPass->setPlaceholderText("New password");

        QPushButton* saveBtn = new QPushButton("Save");

        layout->addWidget(oldPass);
        layout->addWidget(newPass);
        layout->addWidget(saveBtn);

        connect(saveBtn, &QPushButton::clicked, [&]() {
            if (oldPass->text().isEmpty() || newPass->text().isEmpty()) {
                QMessageBox::warning(&passDialog, "Error", "Fill in all fields.");
            } else {
                QMessageBox::information(&passDialog, "Success", "Password changed.");
                passDialog.accept();
            }
        });

        passDialog.exec();
    });

    connect(logoutBtn, &QPushButton::clicked, this, [this, &dialog]() {
        socket->sendLogOut();

        /*try {
            std::string response = socket->receiveData();
            Header header = socket->parseHeader(response);

            if (header == Header::_OK_) {
                QMessageBox::information(this, "Logout", "Logged out successfully.");

                dialog.accept();
                this->close();

                MainWindow* mainWindow = new MainWindow();
                mainWindow->setSocket(socket);
                mainWindow->show();
            } else {
                QMessageBox::warning(this, "Logout", "Server reported logout error.");
            }
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Error", QString("Communication error: %1").arg(e.what()));
        }*/
        dialog.accept();
        if(logout){
            socket->stopListening();
            this->close();

            MainWindow* mainWindow = new MainWindow();
            mainWindow->setSocket(socket);
            mainWindow->show();
        }
    });

    dialog.exec();
}

void ChatWindow::addContact(const QString &name, const QString &phoneNumber)
{
    auto* contact = new Contact{name.toStdString(), phoneNumber.toStdString()};

    QFrame *contactFrame = new QFrame;
    contactFrame->setFrameShape(QFrame::StyledPanel);
    contactFrame->setStyleSheet(R"(
        QFrame {
           padding: 5px;
        }
        QFrame:hover {
            background-color: #393938;
            border-radius: 10px;
        }
    )");
    contactFrame->setProperty("phoneNumber", phoneNumber);

    QHBoxLayout *layout = new QHBoxLayout(contactFrame);
    layout->setContentsMargins(3, 3, 3, 3);

    QLabel *avatar = new QLabel;
    QPixmap original(":/userimg.png");
    original = original.scaled(32, 32, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    /*QPixmap rounded(32,32);
    rounded.fill(Qt::transparent);

    QPainter painter(&rounded);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addEllipse(0, 0, 32, 32);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, original);*/

    avatar->setPixmap(original);

    QVBoxLayout *textLayout = new QVBoxLayout;
    QLabel *nameLabel = new QLabel(name);
    nameLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    nameLabel->setAlignment(Qt::AlignLeft);
    QLabel *phoneLabel = new QLabel(phoneNumber);
    phoneLabel->setStyleSheet("font-size: 10px; color: gray;");
    phoneLabel->setAlignment(Qt::AlignLeft);

    textLayout->addWidget(nameLabel);
    textLayout->addWidget(phoneLabel);

    layout->addWidget(avatar);
    layout->addLayout(textLayout);
    layout->addStretch();

    contactFrame->installEventFilter(this);
    contactListLayout->addWidget(contactFrame);
    contactWidgets[contact] = contactFrame;
}

void ChatWindow::removeContact(const QString& phoneNumber) {
    for (auto it = contactWidgets.begin(); it != contactWidgets.end(); ++it) {
        if (QString::fromStdString(it.key()->phone) == phoneNumber) {
            QWidget* frame = it.value();
            contactListLayout->removeWidget(frame);
            frame->deleteLater();

            delete it.key();
            contactWidgets.erase(it);
            break;
        }
    }

    chatHistory.remove(phoneNumber);
    if (currentContact == phoneNumber) {
        currentContact.clear();
        selectedContactWidget = nullptr;

        headerWidget->hide();
        chatScrollArea->hide();
        messageInput->hide();
        sendButton->hide();
        attachButton->hide();
        placeholderWidget->show();
    }
}

void ChatWindow::showAddContactDialog() {
    QDialog dialog(this);
    dialog.setWindowTitle("Add Contact");
    dialog.setFixedSize(300, 150);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLineEdit *phoneInput = new QLineEdit;
    phoneInput->setPlaceholderText("Phone number (07XXXXXXXX)");
    phoneInput->setMaxLength(10);

    QLabel *errorLabel = new QLabel;
    errorLabel->setStyleSheet("color: red;");
    errorLabel->setVisible(false);

    QPushButton *addBtn = new QPushButton("Add");
    connect(addBtn, &QPushButton::clicked, [&]() {
        QString phone = phoneInput->text();
        QRegularExpression regex("^07\\d{8}$");
        if (!regex.match(phone).hasMatch()) {
            errorLabel->setText("Invalid phone number");
            errorLabel->setVisible(true);
            return;
        }
        errorLabel->setVisible(false);

        pendingPhone = phone;
        socket->sendAddContact(phone.toStdString());

        /*std::string response = socket->receiveData();
        if(socket->parseHeader(response) == Header::ERROR__){
            std::string message = std::string(response.begin()+1, response.end());
            QMessageBox::warning(this, "Add contact failed", message.c_str());
            return;
        }

        std::string username;
        if (!socket->handleAddContactOK(response, username)) {
            QMessageBox::warning(this, "Add contact failed", "Invalid response.");
            return;
        }

        addContact(QString::fromStdString(username), phone);*/
        dialog.accept();
    });

    layout->addWidget(phoneInput);
    layout->addWidget(errorLabel);
    layout->addStretch();
    layout->addWidget(addBtn, 0, Qt::AlignCenter);

    dialog.exec();
}

void ChatWindow::addMessageBubble(const QString &message, bool isUser, bool animate, const QString& timestamp)
{
    QWidget *bubbleWidget = new QWidget;
    QHBoxLayout *bubbleLayout = new QHBoxLayout(bubbleWidget);
    bubbleLayout->setContentsMargins(10, 5, 10, 5);

    QWidget* messageContainer = new QWidget;
    QHBoxLayout* messageLayout = new QHBoxLayout(messageContainer);
    messageLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *bubble = new QLabel(message);
    //bubble->setWordWrap(true);
    bubble->setTextFormat(Qt::PlainText);
    bubble->setTextInteractionFlags(Qt::TextSelectableByMouse);
    bubble->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    bubble->setStyleSheet(QString(R"(
        background-color: %1;
        border-radius: 12px;
        padding: 10px;
        font-size: 14px;
        color: white;
    )").arg(isUser ? "#552ae2" : "#1e9274"));

    bubble->setMaximumWidth(800);
    bubble->setMinimumWidth(50);

    QHBoxLayout *infoLayout = new QHBoxLayout;
    infoLayout->setContentsMargins(0, 0, 0, 0);
    infoLayout->setSpacing(5);

    QString displayTime = timestamp.isEmpty()
        ? QTime::currentTime().toString("hh:mm")
        : QTime::fromString(timestamp.section(' ', 1, 1), "hh:mm:ss").toString("hh:mm");

    QLabel *timeLabel = new QLabel(displayTime);
    timeLabel->setStyleSheet("font-size: 10px; color: lightgray;");
    infoLayout->addStretch();
    infoLayout->addWidget(timeLabel);

    if (isUser) {
        QLabel *checkLabel = new QLabel;
        checkLabel->setStyleSheet("font-size: 10px; color: lightgray;");
        checkLabel->setText(animate ? "✔" : "✔✔");
        infoLayout->addWidget(checkLabel);

        if(animate){
            QTimer::singleShot(500, this, [checkLabel]() {
                checkLabel->setText("✔✔");

                auto *effect = new QGraphicsOpacityEffect(checkLabel);
                checkLabel->setGraphicsEffect(effect);

                auto *animation = new QPropertyAnimation(effect, "opacity");
                animation->setDuration(500);
                animation->setStartValue(0.0);
                animation->setEndValue(1.0);
                animation->start(QAbstractAnimation::DeleteWhenStopped);
            });
        }
    }


    messageLayout->addWidget(bubble);
    messageLayout->addLayout(infoLayout);

    if (isUser) {
        bubbleLayout->addStretch();
        bubbleLayout->addWidget(messageContainer, 0, Qt::AlignRight);
    } else {
        bubbleLayout->addWidget(messageContainer, 0, Qt::AlignLeft);
        bubbleLayout->addStretch();
    }

    chatMessagesLayout->addWidget(bubbleWidget);
    QCoreApplication::processEvents();
    chatScrollArea->verticalScrollBar()->setValue(chatScrollArea->verticalScrollBar()->maximum());
}

void ChatWindow::handleAutoWrapInput() {
    static bool processing = false;
    if (processing) return;
    processing = true;

    QString text = messageInput->toPlainText();
    QStringList lines = text.split('\n');
    QStringList newLines;

    const int maxCharsPerLine = 100;

    for (const QString &line : lines) {
        QString current = line;
        while (current.length() > maxCharsPerLine) {
            newLines << current.left(maxCharsPerLine);
            current = current.mid(maxCharsPerLine);
        }
        newLines << current;
    }

    QString newText = newLines.join('\n');
    if (newText != text) {
        messageInput->blockSignals(true);
        messageInput->setPlainText(newText);
        QTextCursor cursor = messageInput->textCursor();
        cursor.movePosition(QTextCursor::End);
        messageInput->setTextCursor(cursor);
        messageInput->blockSignals(false);
    }

    processing = false;
}

bool ChatWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == avatarWrapper) {
        if (event->type() == QEvent::Enter) {
            editOverlay->show();
            auto* effect = new QGraphicsOpacityEffect(this);
            effect->setOpacity(0.5);
            avatarImageLabel->setGraphicsEffect(effect);
        } else if (event->type() == QEvent::Leave) {
            editOverlay->hide();
            avatarImageLabel->setGraphicsEffect(nullptr);
        } else if (event->type() == QEvent::MouseButtonPress) {
            QString imagePath = QFileDialog::getOpenFileName(this, "Choose Profile Picture", "", "Images (*.png *.jpg *.jpeg)");
            if (!imagePath.isEmpty()) {
                currentUser.avatarPath = imagePath;
                QPixmap newPic(imagePath);
                avatarImageLabel->setPixmap(newPic.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            }
        }
        return true;
    }

    if (event->type() == QEvent::MouseButtonPress) {
        QFrame *clickedContact = qobject_cast<QFrame *>(obj);
        if(clickedContact == selectedContactWidget)
            return true;
        if (clickedContact) {
            if (selectedContactWidget) {
                selectedContactWidget->setStyleSheet(R"(
                    QFrame {
                        padding: 5px;
                    }
                    QFrame:hover {
                        background-color: #393938;
                        border-radius: 10px;
                    }
                )");
            }

            clickedContact->setStyleSheet(R"(
                QFrame {
                    padding: 5px;
                    background-color: #393938;
                    border-radius: 10px;
                }
                QFrame:hover {
                    background-color: #393938;
                }
            )");

            selectedContactWidget = clickedContact;
            currentContact = clickedContact->property("phoneNumber").toString();
            selectContact();
        }
    }

    return QObject::eventFilter(obj, event);
}

void ChatWindow::selectContact()
{
    while (QLayoutItem *item = chatMessagesLayout->takeAt(0)) {
        if (QWidget *widget = item->widget()) {
            widget->setParent(nullptr);
            widget->deleteLater();
        }
        delete item;
    }
    QCoreApplication::processEvents();

    for (auto it = contactWidgets.begin(); it != contactWidgets.end(); ++it) {
        if (it.value() == selectedContactWidget) {
            Contact* contact = it.key();

            QPixmap pic(currentUser.avatarPath.isEmpty() ? ":/userimg.png" : currentUser.avatarPath);
            headerImage->setPixmap(pic.scaled(40, 40, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));

            headerUsernameLabel->setText(QString::fromStdString(contact->username));
            headerPhoneLabel->setText(QString::fromStdString(contact->phone));

            break;
        }
    }

    headerWidget->setVisible(true);

    const auto& entries = chatHistory.value(currentContact);
    for (const auto& entry : entries) {
        if (entry.isFile)
            addFileBubbleCard(entry.fileName, entry.fileData, entry.isUser, false);
        else
            addMessageBubble(entry.text, entry.isUser, false, entry.timestamp);
    }

    if (placeholderWidget->isVisible()) {
        placeholderWidget->hide();
        chatScrollArea->show();
        attachButton->show();
        messageInput->show();
        sendButton->show();
    }
}

void ChatWindow::sendMessage()
{
    QString message = messageInput->toPlainText().trimmed();
    if (message.isEmpty()) return;

    if (currentContact.isEmpty()) {
        QMessageBox::warning(this, "Attention", "Select a contact before you send a message.");
        return;
    }

    socket->sendText(currentUser.phoneNumber.toStdString(), currentContact.toStdString(), message.toStdString());

    addMessageBubble(message, true, true, {});

    chatHistory[currentContact].append({true, false, message, "", {}, ""});

    messageInput->clear();
}

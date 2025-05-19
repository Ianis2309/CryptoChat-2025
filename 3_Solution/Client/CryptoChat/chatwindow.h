#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QFrame>
#include <QPixmap>
#include <QMap>
#include <QStackedLayout>
#include <QPropertyAnimation>
#include "csocket.h"

struct MessageEntry {
    bool isUser;
    bool isFile;
    QString text;
    QString fileName;
    QByteArray fileData;
    QString timestamp;
};

struct User {
    QString username;
    QString phoneNumber;
    QString avatarPath;
};

class ChatWindow : public QMainWindow
{
    Q_OBJECT

public:
    ChatWindow(QWidget *parent = nullptr);
    void setUserData(const QString& username, const QString& phone) {
        currentUser.username = username;
        currentUser.phoneNumber = phone;
    }
    void setContacts(const std::vector<Contact*>& contacts) {
        for (Contact* c : contacts) {
            addContact(QString::fromStdString(c->username),
                       QString::fromStdString(c->phone));
        }
    }
    void setSocket(CSocket* s) { socket = s; }
    void handleServerMessage(const std::string& data);
    void loadMessagesFromServer(const QString& phone, const QString& rawData);

private slots:
    void sendMessage();
    void selectContact();
    void handleAutoWrapInput();

    void addContact(const QString &name, const QString &phoneNumber);
    void removeContact(const QString& phoneNumber);
    void addMessageBubble(const QString &message, bool isUser, bool animate, const QString& timestamp);

    void filterContacts(const QString& text);
    void showAddContactDialog();

    void handleAttachFile();
    void addFileBubbleCard(const QString& filename, const QByteArray& fileData, bool isUser, bool animate);

    void showSettingsDialog();

private:
    QWidget *contactListWidget;
    QVBoxLayout *contactListLayout;
    QMap<Contact*, QFrame*> contactWidgets;
    QLineEdit* searchInput;
    QScrollArea *contactScrollArea;

    QWidget* headerWidget = nullptr;
    QLabel* headerImage = nullptr;
    QLabel* headerUsernameLabel = nullptr;
    QLabel* headerPhoneLabel = nullptr;
    QPushButton* optionsButton = nullptr;

    QScrollArea *chatScrollArea;
    QWidget *chatMessagesWidget;
    QVBoxLayout *chatMessagesLayout;

    QPushButton* attachButton;
    QTextEdit *messageInput;
    QPushButton *sendButton;

    QWidget *chatArea;
    QVBoxLayout *chatLayout;
    QWidget *placeholderWidget;

    QString currentContact;
    QMap<QString, QList<MessageEntry>> chatHistory;

    QWidget *selectedContactWidget = nullptr;

    User currentUser;
    QWidget* avatarWrapper = nullptr;
    QLabel* editOverlay = nullptr;
    QLabel* avatarImageLabel = nullptr;

    CSocket* socket = nullptr;

    QString pendingPhone;
    bool del = true;
    bool logout = true;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif

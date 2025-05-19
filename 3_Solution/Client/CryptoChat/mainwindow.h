#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include "csocket.h"
#include <QCloseEvent>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void setSocket(CSocket* s){
        socket = s;
    }

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void showSignup();
    void showLogin();

private:
    QStackedWidget *stack;
    QWidget *loginPage();
    QWidget *signupPage();

    CSocket* socket = nullptr;
};

#endif

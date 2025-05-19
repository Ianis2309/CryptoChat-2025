#include <QApplication>
#include <QTimer>
#include "mainwindow.h"
#include "splashscreen.h"
#include "csocket.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/CCicon.ico"));

    SplashScreen splash;
    splash.show();

    QTimer::singleShot(4500, &app, [&]() {
        splash.close();
        CSocket* socket = new CSocket("172.20.10.2", 51234);
        socket->connectToServer();
        MainWindow *w = new MainWindow;
        w->setSocket(socket);
        w->show();
    });

    return app.exec();
}

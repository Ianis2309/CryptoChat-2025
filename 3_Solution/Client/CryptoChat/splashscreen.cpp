#include "splashscreen.h"

SplashScreen::SplashScreen(QWidget *parent)
    : QWidget(parent)
{
    showFullScreen();
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    label = new QLabel(this);
    label->setAlignment(Qt::AlignCenter);
    label->setGeometry(this->rect());

    movie = new QMovie(":/LoadingScreen.gif");
    label->setMovie(movie);
    movie->start();
}

SplashScreen::~SplashScreen()
{
    movie->stop();
}

#include "BackgroundScrollArea.h"
#include <QPainter>

BackgroundScrollArea::BackgroundScrollArea(QWidget *parent)
    : QScrollArea(parent)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    setWidgetResizable(true);
}

void BackgroundScrollArea::setBackgroundImage(const QString &imagePath)
{
    background.load(imagePath);
    viewport()->update();
}

void BackgroundScrollArea::paintEvent(QPaintEvent *event)
{
    if (!background.isNull()) {
        QPainter painter(viewport());
        QPixmap scaled = background.scaled(viewport()->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        painter.drawPixmap(0, 0, scaled);
    }

    QScrollArea::paintEvent(event);
}

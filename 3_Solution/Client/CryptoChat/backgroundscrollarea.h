#ifndef BACKGROUNDSCROLLAREA_H
#define BACKGROUNDSCROLLAREA_H

#include <QScrollArea>
#include <QPixmap>

class BackgroundScrollArea : public QScrollArea
{
    Q_OBJECT

public:
    explicit BackgroundScrollArea(QWidget *parent = nullptr);
    void setBackgroundImage(const QString &imagePath);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPixmap background;
};

#endif

#ifndef CIRCLE_H
#define CIRCLE_H

#include <QGraphicsItem>

class QGraphicsTextItem;


class Circle : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    Circle(int id);

    QRectF boundingRect() const; //这个函数是vitual必须重新实现，否则所定义的类难以实例化

signals:
    void closeSignal(bool close);

private:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

    int nodeId;
    QGraphicsTextItem *idtext;
};

#endif // CIRCLE_H

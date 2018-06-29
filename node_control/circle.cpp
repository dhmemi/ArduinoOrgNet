#include "circle.h"

#include <QPainter>
#include <QGraphicsTextItem>

Circle::Circle(int id)
{
    nodeId = id;
    idtext = new QGraphicsTextItem(this);
    idtext->setFont(QFont("Yahei", 20));
    idtext->setPlainText(QString::number(id));
    idtext->setDefaultTextColor(Qt::red);
    idtext->setParentItem(this);
    idtext->setPos(-10, -15);
    idtext->setZValue(500);
    setZValue(250);
}

QRectF Circle::boundingRect() const
{
    return QRectF(-30,-30,60,60);
}

void Circle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->save();
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setPen(Qt::black);
    painter->setBrush(Qt::gray);
    if(nodeId == 0){
        painter->setBrush(Qt::yellow);
    }
    painter->drawEllipse(-30,-30,60,60);
    painter->restore();
}

void Circle::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    Q_UNUSED(event);
}

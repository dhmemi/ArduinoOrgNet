#include "linkitem.h"
#include "circle.h"

#include <QPainter>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <QDebug>
#include <QGraphicsSceneContextMenuEvent>

LinkItem::LinkItem(int from, int to,
                   QMap<int, Circle *> *node, QGraphicsItem *parent)
    :fromId(from), toId(to), nodes(node), QGraphicsItem(parent)
{
    connected = false;
    dataFlow = false;
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &LinkItem::onTimerOut);
}

LinkItem::~LinkItem()
{
    delete timer;
}

QRectF LinkItem::boundingRect() const
{
    QPointF vga;
    vga.setX(((*nodes)[fromId]->x()+(*nodes)[toId]->x())/2);
    vga.setY(((*nodes)[fromId]->y()+(*nodes)[toId]->y())/2);
    return QRectF(vga.x()-5, vga.y()-5, 10, 10);
}

void LinkItem::setConnectState(bool conn)
{
    connected = conn;
    update();
}

void LinkItem::setDataFlow()
{
    if(!connected){
        return;
    }

    dataFlow = true;
    update();
    if(timer->isActive()){
        timer->stop();
    }
    timer->start(2000);
}

void LinkItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->save();
    Q_UNUSED(option);
    Q_UNUSED(widget);
    QPen pen = painter->pen();
    pen.setWidth(8);
    QColor color;
    if(connected){
        color = QColor(50, 50, 50);
        setZValue(100);
    }else{
        color = QColor(200, 230, 200);
        setZValue(50);
    }
    if(fromId == 0){
        color = QColor(200, 100, 150);
        setZValue(100);
    }
    if(dataFlow){
        color = QColor(50, 200, 50);
        setZValue(150);
    }
    pen.setColor(color);
    painter->setPen(pen);

    QLineF line((*nodes)[fromId]->pos(),(*nodes)[toId]->pos());
    painter->drawLine(line);
    painter->restore();
}

void LinkItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if(fromId == 0 || toId == 0){
        return;
    }

    QMenu menu(event->widget());
    if(connected){
        menu.addAction(QString("断开连接"), this, &LinkItem::changeState);
    }else{
        menu.addAction(QString("连接"), this, &LinkItem::changeState);
    }
    menu.exec(event->screenPos());
}

void LinkItem::changeState(bool trigger)
{
    Q_UNUSED(trigger);
    connected = !connected;
    update();
    emit connectChaned(fromId, toId, connected);
}

void LinkItem::onTimerOut()
{
    dataFlow = false;
    update();
}

#ifndef LINKITEM_H
#define LINKITEM_H

#include <QGraphicsItem>
#include <QMap>

class QTimer;
class Circle;

class LinkItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    LinkItem(int from, int to, QMap<int, Circle*> *node, QGraphicsItem *parent = nullptr);
    ~LinkItem();

    QRectF boundingRect() const; //这个函数是vitual必须重新实现，否则所定义的类难以实例化

    void setConnectState(bool conn);
    void setDataFlow();

signals:
    void connectChaned(int from, int to, bool connect);

private slots:
    void changeState(bool trigger);
    void onTimerOut();

private:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

    int fromId, toId;
    bool connected;
    bool dataFlow;
    QMap<int, Circle*> *nodes;
    QTimer *timer;
};

#endif // LINKITEM_H

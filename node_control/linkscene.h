#ifndef LinkScene_H
#define LinkScene_H
#include "define.h"

#include <QGraphicsScene>
#include <QMap>

class Circle;
class LinkItem;
class QTimer;

class LinkScene : public QGraphicsScene
{
    Q_OBJECT
public:
    LinkScene(QObject *parent = nullptr);
    ~LinkScene();

    void changedLinkState(const QByteArray &frame);

signals:
    sendLinkData(const QByteArray &data);

private slots:
    void connectChanged(int from, int to, bool connect);
    void timerOut();

private:
    void structCtrlData();
    quint8 getCheck(QByteArray &frame);
    void updateAll();

    QTimer *timer;
    QMap<int, Circle*> nodes;
    QMap<int, int> nodesAlive;
    QMap<int, QMap<int, LinkItem*> > links;
    bool ctrlAble[NSZ][NSZ];
    bool routeAble[NSZ][NSZ];
};

#endif // LinkScene_H

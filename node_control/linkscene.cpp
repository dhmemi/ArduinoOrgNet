#include "linkscene.h"
#include "circle.h"
#include "linkitem.h"

#include <QTimer>
#include <QDebug>

#define TIMER 8000

LinkScene::LinkScene(QObject *parent)
    : QGraphicsScene(parent)
{
    nodes.insert(0, new Circle(0));
    this->addItem(nodes[0]);
    nodesAlive.insert(0, MXTMR);
    links.insert(0, QMap<int, LinkItem*>());
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &LinkScene::timerOut);
    timer->start(TIMER);

    for(int i = 0; i < NSZ; i++){
        for(int j = 0; j < NSZ; j++){
            ctrlAble[i][j] = true;
        }
    }
    updateAll();
}

LinkScene::~LinkScene()
{
    delete timer;
    for(int i : nodes.keys()){
        delete nodes[i];
    }
    for(int m : links.keys()){
        for(int n : links[m].keys()){
            delete links[m][n];
        }
    }
}

void LinkScene::changedLinkState(const QByteArray &frame)
{
    int srcId = int(quint8(frame[1]));
    int dstId = int(quint8(frame[2]));
    if((!nodes.contains(srcId)) && srcId > 0 && srcId < NSZ){
        nodes.insert(srcId, new Circle(srcId));
        addItem(nodes[srcId]);
        nodesAlive.insert(srcId, MXTMR);
        links.insert(srcId, QMap<int, LinkItem*>());

        for(int i : nodes.keys()){
            if(i < srcId){
                links[i].insert(srcId, new LinkItem(i, srcId, &nodes));
                connect(links[i][srcId], &LinkItem::connectChaned, this, &LinkScene::connectChanged);
                addItem(links[i][srcId]);
            }else if(i > srcId){
                links[srcId].insert(i, new LinkItem(srcId, i, &nodes));
                connect(links[srcId][i], &LinkItem::connectChaned, this, &LinkScene::connectChanged);
                addItem(links[srcId][i]);
            }
        }
    }

    if(quint8(frame[9]) == quint8(CMD::Broadcast)){
        for(int i = 0; i < NSZ; i++){
            for(int j = 0; j < NSZ; j++){
                routeAble[i][j] = quint8(frame[i*NSZ+j+10]) > 0 ? true : false;
                if(nodes.contains(i+1) && nodes.contains(j+1) && i < j){
                    links[i+1][j+1]->setConnectState(routeAble[i][j]);
                }
            }
        }

        bool tag = true;
        for(int i = 0; tag && i < NSZ; i++){
            for(int j = 0; tag && j < NSZ; j++){
                if(ctrlAble[i][j] == false && routeAble[i][j] == true){
                    structCtrlData();
                    tag = false;
                }
            }
        }
    }else if(nodes.contains(srcId) && nodes.contains(dstId)){
        if(srcId < dstId){
            links[srcId][dstId]->setDataFlow();
        }else{
            links[dstId][srcId]->setDataFlow();
        }
    }
    updateAll();
}

void LinkScene::connectChanged(int from, int to, bool connect)
{
    ctrlAble[from-1][to-1] = connect;
    ctrlAble[to-1][from-1] = connect;
    structCtrlData();
    updateAll();
}

void LinkScene::timerOut()
{
    timer->stop();
    for(int i : nodesAlive.keys()){
        if(i > 0){
            if(nodesAlive[i] > 0){
                nodesAlive[i] --;
            }else{
                for(int m : links.keys()){
                    if(m == i){
                        for( int n : links[m].keys()){
                            removeItem(links[m][n]);
                            links[m][n]->update();
                        }
                        links.remove(m);
                    }else{
                        if(links[m].contains(i)){
                            removeItem(links[m][i]);
                            links[m][i]->update();
                            links[m].remove(i);
                        }
                    }
                }

                removeItem(nodes[i]);
                nodes[i]->update();
                //delete nodes[i];
                nodes.remove(i);
                nodesAlive.remove(i);
            }
        }
    }
    updateAll();
    timer->start(TIMER);
}

void LinkScene::structCtrlData()
{
    QByteArray data(10, char(0xFE));
    data[1] = char(0x00);
    data[2] = char(0xFF);
    data[3] = char(0xAA);
    data[4] = char(0x00);
    data[5] = char(NSZ*NSZ+4);
    data[6] = char(0x00);
    data[7] = char(0xFF);
    data[8] = char(0x00);
    data[9] = char(CMD::LinkCtrl);
    for(int i =0; i< NSZ; i++){
        for(int j = 0; j < NSZ; j++){
            data.append(quint8(ctrlAble[i][j]));
        }
    }
    data.append(quint8(0x01));
    data[data.size()-1] = getCheck(data);
    emit sendLinkData(data);
}

quint8 LinkScene::getCheck(QByteArray &frame)
{
    quint16 checksum = 0;
    for(int i = 0; i < frame.size()-1; i++){
        checksum += frame[i];
    }
    return quint8(~checksum);
}

void LinkScene::updateAll()
{
    int count = 0;
    for(int i : nodes.keys()){
        int x = cos(2*3.14159/nodes.size()*count - 1.5708)*200;
        int y = sin(2*3.14159/nodes.size()*count - 1.5708)*200;
        nodes[i]->setPos(QPointF(x,y));
        nodes[i]->update();
        count ++;
    }
    for(int m : links.keys()){
        for(int n : links[m].keys()){
            links[m][n]->update();
        }
    }
    update();
}

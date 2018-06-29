#include "msgtree.h"
#include "define.h"

#include <QDebug>
#include <QScrollBar>

MsgTree::MsgTree(QWidget *parent)
    :QTreeWidget(parent)
{
    curId = NSZ+1;
}

void MsgTree::addMsg(const QByteArray &frame)
{
    QTreeWidgetItem *frameItem, *fromItem, *toItem, *cmdItem, *dataItem;

    QStringList strings;
    strings << QString("帧数据:");
    QString text;
    for(int i = 0; i < frame.length(); i++){
        text += QString("%1").arg(quint8(frame[i]),2,16,QLatin1Char('0'));
        text += " ";
    }
    strings << text;
    frameItem = new QTreeWidgetItem(this, strings);
    frameItem->setData(1, Qt::UserRole, frame);

    strings.clear();
    strings << QString("from:");
    strings << QString::number(quint8(frame[1]),10);
    fromItem = new QTreeWidgetItem(frameItem, strings);

    strings.clear();
    strings << QString("to:");
    strings << QString::number(quint8(frame[2]),10);
    toItem = new QTreeWidgetItem(frameItem, strings);

    switch(quint8(frame[9])){
    case quint8(CMD::Broadcast):
        strings.clear();
        strings << QString("指令:");
        strings << QString("路由广播");
        cmdItem = new QTreeWidgetItem(frameItem, strings);

        strings.clear();
        strings << QString("数据:");
        text.clear();
        for(int i = 10; i < frame.length()-1; i++){
            text += QString("%1").arg(quint8(frame[i]),2,16,QLatin1Char('0'));
            text += " ";
        }
        strings << text;
        dataItem = new QTreeWidgetItem(frameItem, strings);

        text.clear();
        for(int i = 0; i < NSZ; i++){
            text += "[";
            for(int j = 0; j < NSZ; j++){
                text += QString("%1").arg(quint8(frame[i*NSZ+j+10]),2,16,QLatin1Char('0'));
                text += " ";
            }
            text += "]\n";
        }
        dataItem->setToolTip(1, text);
        frameItem->setToolTip(1, text);
        break;

    case quint8(CMD::SendData):
        strings.clear();
        strings << QString("指令:");
        strings << QString("数据传输");
        cmdItem = new QTreeWidgetItem(frameItem, strings);

        strings.clear();
        strings << QString("数据:");
        strings << QString(frame.mid(10, frame[5]-4));
        dataItem = new QTreeWidgetItem(frameItem, strings);
        dataItem->setToolTip(1, QString("from[%1] to[%2]: %3")
                                .arg(QString::number(quint8(frame[6])))
                                .arg(QString::number(quint8(frame[7])))
                                .arg(strings[1]));
        frameItem->setToolTip(1, strings[1]);
        break;
    case quint8(CMD::LinkCtrl):
        strings.clear();
        strings << QString("指令:");
        strings << QString("链路控制");
        cmdItem = new QTreeWidgetItem(frameItem, strings);

        strings.clear();
        strings << QString("数据:");
        text.clear();
        for(int i = 10; i < frame.length()-1; i++){
            text += QString("%1").arg(quint8(frame[i]),2,16,QLatin1Char('0'));
            text += " ";
        }
        strings << text;
        dataItem = new QTreeWidgetItem(frameItem, strings);

        text.clear();
        for(int i = 0; i < NSZ; i++){
            text += "[";
            for(int j = 0; j < NSZ; j++){
                text += QString("%1").arg(quint8(frame[i*NSZ+j+10]),2,16,QLatin1Char('0'));
                text += " ";
            }
            text += "]\n";
        }
        dataItem->setToolTip(1, text);
        frameItem->setToolTip(1, text);
        break;
    case quint8(CMD::SendCtrl):
        strings.clear();
        strings << QString("指令:");
        strings << QString("发送控制");
        cmdItem = new QTreeWidgetItem(frameItem, strings);

        strings.clear();
        strings << QString("数据:");
        text = "from[" + QString::number(frame[2], 10) + "] to[" + QString::number(frame[10],10) +
                "]: " + QString(frame.mid(11, frame[5]-5));
        strings << text;
        dataItem = new QTreeWidgetItem(frameItem, strings);

        dataItem->setToolTip(1, strings[1]);
        frameItem->setToolTip(1, strings[1]);
        break;
    default:
        strings.clear();
        strings << QString("指令:");
        strings << QString("未知指令");
        cmdItem = new QTreeWidgetItem(frameItem, strings);

        strings.clear();
        strings << QString("数据:");
        text.clear();
        for(int i = 10; i < frame.length()-1; i++){
            text += QString("%1").arg(quint8(frame[i]),2,16,QLatin1Char('0'));
            text += " ";
        }
        strings << text;
        dataItem = new QTreeWidgetItem(frameItem, strings);
    }

    if(curId > NSZ || curId == quint8(frame[1]) || curId == quint8(frame[2])){
        frameItem->setHidden(false);
    }else{
        frameItem->setHidden(true);
    }

    if(topLevelItemCount() > 200){
        QTreeWidgetItem *tmp =  takeTopLevelItem(0);
        while(tmp->childCount() > 0){
            delete tmp->takeChild(0);
        }
        delete tmp;
    }

    QScrollBar *bar = verticalScrollBar();
    bar->setValue(bar->maximum());
}

void MsgTree::showOnly(int nodeId)
{
    if(nodeId > NSZ){
        for(int i = 0; i < topLevelItemCount(); i++){
            topLevelItem(i)->setHidden(false);
        }
    }else{
        for(int i = 0; i < topLevelItemCount(); i++){
            topLevelItem(i)->setHidden(
                        topLevelItem(i)->data(1, Qt::UserRole).toByteArray().at(1) == nodeId ||
                        topLevelItem(i)->data(1, Qt::UserRole).toByteArray().at(2) == nodeId);
        }
    }
    curId = nodeId;
}

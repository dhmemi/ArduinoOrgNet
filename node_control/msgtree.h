#ifndef MSGTREE_H
#define MSGTREE_H

#include <QTreeWidget>

class MsgTree : public QTreeWidget
{
public:
    MsgTree(QWidget *parent = nullptr);
    void addMsg(const QByteArray &frame);
    void showOnly(int nodeId);

private:
    int curId;
};

#endif // MSGTREE_H

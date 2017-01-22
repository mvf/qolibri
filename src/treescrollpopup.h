#ifndef TREESCROLLPOPUP_H
#define TREESCROLLPOPUP_H

#include <QWidget>

class QTreeWidgetItem;

class TreeScrollPopup : public QWidget
{
    Q_OBJECT
public:
    TreeScrollPopup(QTreeWidgetItem *item, QWidget *parent);

private slots:
    void changeExpand(int value)
    {
        expand(topItem, 0, value);
    }

private:
    void countDepth(QTreeWidgetItem *item, int cur_depth, int *max_depth);
    void expand(QTreeWidgetItem *item, int cur_depth, int set_depth);
    QTreeWidgetItem *topItem;
};

#endif // TREESCROLLPOPUP_H

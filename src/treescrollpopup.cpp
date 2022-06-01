#include "treescrollpopup.h"

#include <QScrollBar>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

TreeScrollPopup::TreeScrollPopup(QTreeWidgetItem *item, QWidget *parent)
    : QWidget(parent), topItem(item)
{
    int depth = 0;

    countDepth(item, 0, &depth);

    QScrollBar *sbar = new QScrollBar(Qt::Horizontal, this);
    sbar->setRange(0, depth);
    sbar->setPageStep(1);
    sbar->setFixedWidth(170);
    if (item->isExpanded()) {
        changeExpand(1);
        sbar->setValue(1);
    } else {
        changeExpand(0);
        sbar->setValue(0);
    }
    QVBoxLayout *v = new QVBoxLayout();
    v->setContentsMargins(5, 5, 5, 5);
    v->setSpacing(5);
    v->addWidget(sbar);
    setLayout(v);

    setWindowFlags(Qt::Popup);

    connect(sbar, SIGNAL(valueChanged(int)), SLOT(changeExpand(int)));
}

void TreeScrollPopup::countDepth(QTreeWidgetItem *item,
                                 int cur_depth, int *max_depth)
{
    if (cur_depth > *max_depth)
        *max_depth = cur_depth;

    cur_depth++;
    for (int i = 0; i < item->childCount(); i++) {
        countDepth(item->child(i), cur_depth, max_depth);
    }
}

void TreeScrollPopup::expand(QTreeWidgetItem *item,
                             int cur_depth, int set_depth)
{
    if (set_depth > cur_depth)
        item->setExpanded(true);
    else
        item->setExpanded(false);

    cur_depth++;
    for (int i = 0; i < item->childCount(); i++) {
        expand(item->child(i), cur_depth, set_depth);
    }
}


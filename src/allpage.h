#ifndef ALLPAGE_H
#define ALLPAGE_H

#include "pagewidget.h"

#include <ebu/eb.h>

class AllPage : public PageWidget
{
    Q_OBJECT
public:
    AllPage(QWidget *parent, const SearchMethod &);
    RET_SEARCH search(const Query&);

private slots:
    void changePage(QTreeWidgetItem*, int column);

private:
    RET_SEARCH readPage(int page);
    RET_SEARCH initSeqHits();
    QList <EB_Position> seqHits;

    int curPage;
};

#endif // ALLPAGE_H

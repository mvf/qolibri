#ifndef SEARCHPAGE_H
#define SEARCHPAGE_H

#include "pagewidget.h"

class PageItems;

class SearchPage : public PageWidget
{
    Q_OBJECT

public:
    SearchPage(QWidget *parent, const SearchMethod&);
    RET_SEARCH search(const Query&);

private:
    RET_SEARCH doSearch(const Query& query, PageItems &items, int &itemIndex);
};

#endif // SEARCHPAGE_H

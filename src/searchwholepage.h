#ifndef SEARCHWHOLEPAGE_H
#define SEARCHWHOLEPAGE_H

#include "pagewidget.h"

class SearchWholePage : public PageWidget
{
    Q_OBJECT

public:
    SearchWholePage(QWidget *parent, const SearchMethod&);
    RET_SEARCH search(const Query&);
};

#endif // SEARCHWHOLEPAGE_H

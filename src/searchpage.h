#ifndef SEARCHPAGE_H
#define SEARCHPAGE_H

#include "pagewidget.h"

class SearchPage : public PageWidget
{
public:
    SearchPage(QWidget *parent, const SearchMethod&);
    RET_SEARCH search(const Query&);
};

#endif // SEARCHPAGE_H

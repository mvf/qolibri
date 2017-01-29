#ifndef INFOPAGE_H
#define INFOPAGE_H

#include "pagewidget.h"

class InfoPage : public PageWidget
{
    Q_OBJECT

public:
    InfoPage(QWidget *parent, const SearchMethod&);
    RET_SEARCH search(const Query&);

private:
    QString convSpecialChar(const QString&) const;
};

#endif // INFOPAGE_H

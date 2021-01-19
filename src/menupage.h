#ifndef MENUPAGE_H
#define MENUPAGE_H

#include "ebcore.h"
#include "pagewidget.h"

#include <ebu/eb.h>

class EbMenu;
class PageItems;

class MenuPage : public PageWidget
{
    Q_OBJECT
public:
    MenuPage(QWidget *parent, const SearchMethod&);
    RET_SEARCH search(const Query&);

private slots:
    void changePage(QTreeWidgetItem*, int column);

private:
    RET_SEARCH fullMenuPage();
    void selectMenuPage(int index);
    RET_SEARCH getMenus(EbMenu *eb, const EB_Position &pos, PageItems *items,
                  int count);
    QList <CandItem> topCands;
    int menuCount;
    bool checkMax;
};

#endif // MENUPAGE_H

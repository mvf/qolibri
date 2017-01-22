#ifndef SEARCHPAGEBUILDER_H
#define SEARCHPAGEBUILDER_H

#include "pageitems.h"
#include "method.h"

#include <QObject>

class BookBrowser;

class SearchPageBuilder : public QObject
{
    Q_OBJECT
public:
    SearchPageBuilder(BookBrowser *browser);
    RET_SEARCH search(const Query& query);
    QList <QTreeWidgetItem*> treeItems();
    QString text();
    int textLength();
    void expand();

signals:
    void statusRequested(const QString &str);

private:
    RET_SEARCH search1(const Query& query);

    int bookIndex;
    int itemIndex;
    PageItems items;
    BookBrowser *bookBrowser_;
};

#endif // SEARCHPAGEBUILDER_H

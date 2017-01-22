#ifndef PAGEITEMS_H
#define PAGEITEMS_H

#include <QList>
#include <QString>

class QTreeWidgetItem;

class PageItems
{

public:
    PageItems(const QString &ssheet);

    void composeHeader(const QString &ssheet);
    void composeError(const QString &anchor, const QString &text);
    QTreeWidgetItem* composeHLine(int num, const QString &anchor, const QString &title_l,
                      const QString &title_t, const QString &text);
    QTreeWidgetItem* composeHLine(int num, const QString &anchor, const QString &title,
                      const QString &text = QString());
    void composeTrail();
    QTreeWidgetItem* addHItem(int num, const QString &anchor, const QString &title);
    void addHtmlStr(const QString &html) { text_ += html; }
    void addTextStr(const QString &str) { text_ += str; }
    void expand(int level);
    QTreeWidgetItem* curItem() { return curItem_; }
    QTreeWidgetItem* item(int level) { return itemP_[level]; }

    QString text() { return text_; }
    QList <QTreeWidgetItem*> topItems() { return topItems_; }
    int textLength() { return textLength_; }

private:
    QString text_;
    QList <QTreeWidgetItem*> topItems_;
    QTreeWidgetItem* itemP_[7];
    QTreeWidgetItem* curItem_;
    int textLength_;

};

#endif // PAGEITEMS_H

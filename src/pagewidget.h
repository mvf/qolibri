#ifndef PAGEWIDGET_H
#define PAGEWIDGET_H

#include "method.h"

#include <QSplitter>
#include <QTreeWidget>

class BookBrowser;

class PageWidget : public QSplitter
{
    Q_OBJECT
public:
    PageWidget(QWidget *parent, const SearchMethod&);

    virtual RET_SEARCH search(const Query&) = 0;

    void collapseBookTree();

    void zoomIn();
    void zoomOut();
    BookBrowser* bookBrowser()
    {
        return bookBrowser_;
    }

    inline SearchMethod method() const
    {
        return method_;
    }

public slots:
    void setBookTreeVisible(bool visible);

protected:
    BookBrowser *bookBrowser_;
    QTreeWidget *bookTree;
    SearchMethod method_;

    static const QString CutString;
    static const QString IntString;

    static bool checkStop();
    static QString toAnchor(const QString &str, int num);

signals:
    void statusRequested(const QString &str);
    void selectionRequested(const QString& str);

private:
    void scrollTo(QTreeWidgetItem *to);

private slots:
    void scrollTo(QTreeWidgetItem *to, int)
    {
        scrollTo(to);
    }
    void scrollTo(QTreeWidgetItem *to, QTreeWidgetItem*)
    {
        scrollTo(to);
    }
    void changeFont(const QFont &font);
    void popupSlide(const QPoint &pos);
};

#endif // PAGEWIDGET_H

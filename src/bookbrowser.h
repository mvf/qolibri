#ifndef BOOKBROWSER_H
#define BOOKBROWSER_H

#include "method.h"

#include <QList>
#include <QTextBrowser>

class Book;

class BookBrowser : public QTextBrowser
{
    Q_OBJECT
public:
    BookBrowser(QWidget *parent = 0);
    void setSource(const QUrl &name);
    int fontSize() const;
    void addBookList(Book *book);
    void setBrowser(const QString &str);

protected:
    QList<Book*> bookList_;
    void contextMenuEvent(QContextMenuEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

signals:
    void searchRequested(SearchDirection, const QString &);
    //void menuRequested(Book*, int, int);
    void pasteRequested(const QString&);
    void processRequested(const QString &program, const QStringList &arguments);
    void soundRequested(const QString& fname);
    void videoRequested(const QString &fileName);
    void selectionRequested(const QString& str);
    void statusRequested(const QString &str);

private slots:
    void changeTextSelection();
    void pasteSearchText();
};

#endif // BOOKBROWSER_H

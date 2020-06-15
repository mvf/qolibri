#include "referencepopup.h"
#include "book.h"
#include "bookbrowserpopup.h"
#include "configure.h"
#include "ebook.h"

#include <QGuiApplication>
#include <QScreen>
#include <QScrollBar>
#include <QToolButton>
#include <QVBoxLayout>

ReferencePopup::ReferencePopup(Book *book, const EB_Position &pos,
                               QWidget *parent, bool menu_flag)
    : QWidget(parent), menuFlag(menu_flag)
{
#ifdef FIXED_POPUP
    bookBrowser_ = new BookBrowserPopup(parent);
#else
    bookBrowser_ = new BookBrowser(parent);
#endif
#ifdef FIXED_POPUP
    QToolButton *close_button = new QToolButton(this);
    close_button->setIcon(QIcon(":images/closetab.png"));
    bookBrowser_->setCornerWidget(close_button);
    connect(close_button, SIGNAL(clicked()), SLOT(close()));

#endif
    //bookBrowser_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    bookBrowser_->addBookList(book);
#ifdef FIXED_POPUP
    bookBrowser_->addTitle("<a class=cls href=close>" + book->name() + "</a>");
#endif

    QVBoxLayout *v = new QVBoxLayout();
    v->addWidget(bookBrowser_);
    v->setMargin(0);
    v->setSpacing(0);
    setLayout(v);

    bookBrowser_->setBrowser(browserText(book, pos));
    //bookBrowser_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //bookBrowser_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    setWindowFlags(Qt::Popup);
    if ( parent->objectName() == "main_browser") {
        setFixedWidth((parent->width() / 4) * 3);
    } else {
        setFixedWidth(parent->width());
    }

}

BookBrowser *ReferencePopup::bookBrowser()
{
    return bookBrowser_;
}

void ReferencePopup::showEvent(QShowEvent*)
{
#ifdef FIXED_POPUP
    if (QWidget *p = qobject_cast<QWidget *>(parent())) {
        move(p->mapToGlobal(QPoint(0, 0)));
        resize(p->size() - QSize(0, 0));
    }
#else

    QSize sz = bookBrowser_->size();
    QScrollBar *hb = bookBrowser_->horizontalScrollBar();
    if ( hb && hb->maximum() > 0) {
        sz.setWidth(hb->maximum() - hb->minimum() + hb->pageStep() + 10);
        resize(sz);
    }
    QScrollBar *vb = bookBrowser_->verticalScrollBar();
    const QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
    if ( vb && vb->maximum() > 0) {
        int h = vb->maximum() - vb->minimum() + vb->pageStep() + 10;
        int ph = screenGeometry.height() - 50;
        if ( h > ph ) {
            h = ph;
        }
        sz.setHeight(h);
    }

    int posx = QCursor::pos().x();
    if ((posx + sz.width()) > screenGeometry.width())
        posx = screenGeometry.width() - sz.width();

    int posy = QCursor::pos().y();
    if ((posy + sz.height()) > screenGeometry.height())
        posy = screenGeometry.height() - sz.height();

    move(QPoint(posx, posy));
    resize(sz);
#endif

}

QString ReferencePopup::browserText(Book *book, const EB_Position &pos)
{
    EBook eb(HookText);

    eb.initBook(book->path(), book->bookNo());
    eb.initHook(16, book->fontList());
    bookBrowser_->setSearchPaths(QStringList() << EbCache::cachePath);
    QString text = eb.text(pos);
    QString ttl;
    if (!menuFlag) {
#ifdef FIXED_POPUP
        QString heading = text.left(text.indexOf('\n'));
        foreach(QString s, bookBrowser_->titles()) {
            ttl += s + " > ";
        }
        ttl += heading;
        QString addr = QString("book|%1|%2|%3|%4").arg(0).arg(pos.page)
                               .arg(pos.offset)
                               .arg(bookBrowser_->titles().count());
        bookBrowser_->addTitle("<a class=ref href=" + addr +
                          " >" + heading + "</a>");
#else
        ttl = text.left(text.indexOf('\n'));
#endif
    } else {
#ifdef FIXED_POPUP
        ttl = bookBrowser_->titles()[0];
#else
        ttl = text.left(text.indexOf('\n'));
#endif
    }

    QString txt =
        "<html>\n"
        "<head>\n"
        "<meta http-equiv=\"Content-Type\""
        " content=\"text/html; charset=utf-8\" />\n"
        "<style type=\"text/css\">\n" + CONF->dictSheet +
        "pre { font-family: " + CONF->browserFont.family() + "; }\n"
        "</style>\n"
        "</head>\n"
        "<body>\n"
        "<h2>" + ttl +
        "</h2>\n<pre>" + eb.text(pos) + "</pre>" +
        "\n</body>\n"
        "</html>\n";
    return txt;
}

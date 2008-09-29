/**************************************************************************
*   Copyright (C) 2007 by BOP                                             *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#include <QtGui>

#include "book.h"
#include "bookview.h"
#include "ebook.h"
#include "configure.h"
#include "textcodec.h"

static bool stopFlag = false;
static QWidget *mainWin = 0;

const char* CutString = "----- cut -----";
const char* IntString = "----- interrupted -----";

//#define FIXED_POPUP

#define SJIStoUTF(q_bytearray) \
    QTextCodec::codecForName("Shift-JIS")->toUnicode(q_bytearray)         

#define FONT_RESTORE_DURING_ZOOM

bool checkStop()
{
    QEventLoop().processEvents();
    return stopFlag;
}


BookBrowser::BookBrowser(QWidget *parent)
    : QTextBrowser(parent)
{
    setSearchPaths(QStringList() << EbCache::cachePath);
    document()->setDefaultFont(CONF->browserFont);

    connect(this, SIGNAL(statusRequested(QString)),
            mainWin, SLOT(showStatus(QString)));
    connect(this, SIGNAL(searchRequested(SearchDirection, QString)),
            mainWin, SLOT(viewSearch(SearchDirection, QString)));
    connect(this, SIGNAL(pasteRequested(QString)),
            mainWin, SLOT(pasteSearchText(QString)));
    connect(this, SIGNAL(processRequested(QString)),
            mainWin, SLOT(execProcess(QString)));
    connect(this, SIGNAL(soundRequested(QString)),
            mainWin, SLOT(execSound(QString)));
    connect(this, SIGNAL(selectionChanged()),
            this, SLOT(changeTextSelection()));
    connect(this, SIGNAL(selectionRequested(QString)),
            mainWin, SLOT(changeOptSearchButtonText(QString)));
}

void BookBrowser::setSource(const QUrl &name)
{

    QStringList args = name.toString().split('?');

    if (args[0] == "sound") {
        // args[1] : wave file
        if (!CONF->waveProcess.isEmpty()) {
            emit processRequested(CONF->waveProcess + ' ' + args[1]);
        } else if (QSound::isAvailable()) {
            emit soundRequested(args[1]);
        } else {
            qWarning() << "Can't play sound" << CONF->waveProcess << args[1];
            emit statusRequested("Can't play sound");
        }
    } else if (args[0] == "book" || args[0] == "menu") {
        // args[1] : book index
        // args[2] : page
        // args[3] : offset
        if ( args.count() == 4) {
            int index = args[1].toInt();
            if (index >= bookList_.count()) {
                qWarning() << "Invalid book index" << args[1];
                emit statusRequested("ERROR : Invalid book index: " + args[1]);
                return;
            }
            bool mflag = (args[0] == "menu") ? true : false;
            EB_Position pos;
            pos.page = args[2].toInt();
            pos.offset = args[3].toInt();
             
            ReferencePopup *popup =
                new ReferencePopup(bookList_[index], pos, this, mflag);
            popup->show();
        } else {
            qWarning() << "Invalid Reference Parameter" << args.count();
        }
    } else if (args[0] == "mpeg") {
        // args[1] : mpeg file
        if (!CONF->mpegProcess.isEmpty()) {
            emit processRequested(CONF->mpegProcess + ' ' + args[1]);
        } else {
            qWarning() << "Can't play moview";
            emit statusRequested("Can't play Movie");
        }
    } else if (args[0] == "close") {
        parentWidget()->close();
    } else {
        qWarning() << "Invalid Command" << args[0];
    }
}

void BookBrowser::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu *menu = createStandardContextMenu();

    if (textCursor().hasSelection()) {
        menu->addSeparator();
        addDirectionMenu(menu);
        menu->addSeparator();
        menu->addAction(QObject::tr("&Paste selected string to edit line"),
		        this, SLOT(pasteSearchText()));
    }
    QAction *a = menu->exec(event->globalPos());
    if (a && a->data().isValid()){
       // qDebug() << a->data().typeName();
        SearchDirection d = (SearchDirection)a->data().toInt();
        if (d <= MenuRead) {
            emit searchRequested(d, textCursor().selectedText());
        } else {
            QString addr;
            if (d == GoogleSearch) {
                addr = CONF->googleUrl;
            } else if (d == WikipediaSearch) {
                addr = CONF->wikipediaUrl;
            } else if (d == Option1Search) {
                addr = CONF->userDefUrl;
            } else {
                qWarning() << "Selected Invalid Menu?";
                delete menu;
                return;
            }
            foreach(const char c, textCursor().selectedText().toUtf8()) {
                addr += "%" + QString::number((ushort)((uchar)c), 16);
            }
            emit processRequested(CONF->browserProcess + ' ' + addr);
        }
    }
    delete menu;
}

#ifdef Q_WS_X11
// Implement for linux.
// For "selectionChanged" SIGNAL not allways invoked at mouse move and
// release timing.
void BookBrowser::mouseReleaseEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton)
        changeTextSelection();

    QTextEdit::mouseReleaseEvent(ev);
}
#endif

void BookBrowser::changeTextSelection()
{
    emit selectionRequested(textCursor().selectedText());
}

void BookBrowser::pasteSearchText()
{
    emit pasteRequested(textCursor().selectedText());
}

ReferencePopup::ReferencePopup(Book *book, const EB_Position &pos,
                               QWidget *parent, bool menu_flag)
    : QWidget(parent), menuFlag(menu_flag)
{
#ifdef FIXED_POPUP
    bookBrowser = new BookBrowserPopup(parent);
#else
    bookBrowser = new BookBrowser(parent);
#endif
#ifdef FIXED_POPUP
    QToolButton *close_button = new QToolButton(this);
    close_button->setIcon(QIcon(":images/closetab.png"));
    bookBrowser->setCornerWidget(close_button, Qt::TopRightCorner);
    connect(close_button, SIGNAL(clicked()), this, SLOT(close()));

#endif
    //bookBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    bookBrowser->addBookList(book);
#ifdef FIXED_POPUP
    bookBrowser->addTitle("<a class=cls href=close>" + book->name() + "</a>");
#endif

    QVBoxLayout *v = new QVBoxLayout();
    v->addWidget(bookBrowser);
    v->setMargin(0);
    v->setSpacing(0);
    setLayout(v);

    bookBrowser->setBrowser(browserText(book, pos));
    //bookBrowser->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //bookBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    setWindowFlags(Qt::Popup);
    if ( parent->objectName() == "main_browser") {
        setFixedWidth((parent->width() / 4) * 3);
    } else {
        setFixedWidth(parent->width());
    }

}

void ReferencePopup::showEvent(QShowEvent*)
{
#ifdef FIXED_POPUP
    move(parent->mapToGlobal(QPoint(0, 0)));
    resize(parent->size() - QSize(0, 0));
#else

    QSize sz = bookBrowser->size();
    QScrollBar *hb = bookBrowser->horizontalScrollBar();
    if ( hb && hb->maximum() > 0) {
        sz.setWidth(hb->maximum() - hb->minimum() + hb->pageStep() + 10);
        resize(sz);
    }
    QScrollBar *vb = bookBrowser->verticalScrollBar();
    if ( vb && vb->maximum() > 0) {
        int h = vb->maximum() - vb->minimum() + vb->pageStep() + 10;
        int ph = QDesktopWidget().screenGeometry().height() - 50;
        if ( h > ph ) {
            h = ph;
        }
        sz.setHeight(h);
    }

    int posx = QCursor::pos().x();
    if ((posx + sz.width()) > QDesktopWidget().screenGeometry().width())
        posx = QDesktopWidget().screenGeometry().width() - sz.width();

    int posy = QCursor::pos().y();
    if ((posy + sz.height()) > QDesktopWidget().screenGeometry().height())
        posy = QDesktopWidget().screenGeometry().height() - sz.height();

    move(QPoint(posx, posy));
    resize(sz);
#endif

}


QString ReferencePopup::browserText(Book *book, const EB_Position &pos)
{
    EBook eb;

    eb.initBook(book->path(), book->bookNo());
    eb.initHook(16, book->fontList(), CONF->indentOffset);
    bookBrowser->setSearchPaths(QStringList() << EbCache::cachePath);
    QString text = eb.text(pos);
    QString ttl;
    if (!menuFlag) {
#ifdef FIXED_POPUP
        QString heading = text.left(text.indexOf('\n'));
        foreach(QString s, bookBrowser->titles()) {
            ttl += s + " > ";
        }
        ttl += heading;
        QString addr = QString("book|%1|%2|%3|%4").arg(0).arg(pos.page)
                               .arg(pos.offset)
                               .arg(bookBrowser->titles().count());
        bookBrowser->addTitle("<a class=ref href=" + addr +
                          " >" + heading + "</a>");
#else
        ttl = text.left(text.indexOf('\n'));
#endif
    } else {
#ifdef FIXED_POPUP
        ttl = bookBrowser->titles()[0];
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
        "</style>\n"
        "</head>\n"
        "<body>\n"
        "<h2>" + ttl + "</h2>\n" +
        "<pre>" + eb.text(pos) + "</pre>\n"
        "</body>\n"
        "</html>\n";
    return txt;
}

#ifdef FIXED_POPUP
void BookBrowserPopup::setSource(const QUrl &name)
{
    QStringList args = name.toString().split('|');

    if ((args[0] == "book" || args[0] == "menu") && args.count() <= 5) {
        // args[1] : book index
        // args[2] : page
        // args[3] : offset
        // args[4] : title index
        int index = args[1].toInt();
        if (index >= bookList_.count()) {
            qWarning() << "Invalid book index" << index;
            return;
        }
        if (args.count() == 5) {
            for (int i = titles_.count(); i > args[4].toInt(); i--) {
                titles_.removeLast();
            }
        }
        EB_Position pos;
        pos.page = args[2].toInt();
        pos.offset = args[3].toInt();
        ReferencePopup *popup = (ReferencePopup*)parentWidget();
        QString str = popup->browserText(bookList_[index], pos );
        setBrowser(str);
        return;
    }
    BookBrowser::setSource(name);
    return;
}
#endif

TreeScrollPopup::TreeScrollPopup(QTreeWidgetItem *item, QWidget *parent)
    : QWidget(parent), topItem(item)
{
    int depth = 0;

    countDepth(item, 0, &depth);

    QScrollBar *sbar = new QScrollBar(Qt::Horizontal, this);
    sbar->setRange(0, depth);
    sbar->setPageStep(1);
    sbar->setFixedWidth(170);
    if (item->isExpanded()) {
        changeExpand(1);
        sbar->setValue(1);
    } else {
        changeExpand(0);
        sbar->setValue(0);
    }
    QVBoxLayout *v = new QVBoxLayout();
    v->setMargin(5);
    v->setSpacing(5);
    v->addWidget(sbar);
    setLayout(v);

    setWindowFlags(Qt::Popup);

    connect(sbar, SIGNAL(valueChanged(int)), this, SLOT(changeExpand(int)));
}

void TreeScrollPopup::countDepth(QTreeWidgetItem *item,
                                 int cur_depth, int *max_depth)
{
    if (cur_depth > *max_depth)
        *max_depth = cur_depth;

    cur_depth++;
    for (int i = 0; i < item->childCount(); i++) {
        countDepth(item->child(i), cur_depth, max_depth);
    }
}

void TreeScrollPopup::expand(QTreeWidgetItem *item,
                             int cur_depth, int set_depth)
{
    if (set_depth > cur_depth)
        item->setExpanded(true);
    else
        item->setExpanded(false);

    cur_depth++;
    for (int i = 0; i < item->childCount(); i++) {
        expand(item->child(i), cur_depth, set_depth);
    }
}

PageItems::PageItems(const QString &ssheet)
    : QObject()
{
    composeHeader(ssheet);
    itemP_[0] = itemP_[1] = itemP_[2] = itemP_[3] = itemP_[4] =
    itemP_[5] = itemP_[6]  = 0;
}

void PageItems::composeHeader(const QString &ssheet)
{
    text_ = "<html>\n"
            "<head>\n"
            "<meta http-equiv=\"Content-Type\""
            " content=\"text/html; charset=utf-8\" />\n"
            "<style type=\"text/css\">\n" + ssheet +
            "</style>\n"
            "</head>\n"
            "<body>\n";
    textLength_ = text_.length();
}
void PageItems::composeHLine(int num, const QString &anchor,
                             const QString &title_l, const QString &title_t,
                             const QString &text)
{
    QString snum = QString::number(num);
    QString str;
    if (!text.isEmpty())
        str =  "<a name=" + anchor + " />"
               "<h" + snum + ">&nbsp;" + title_t +
               "</h" + snum + "><pre>" + text +
               "</pre>";
    else
        str =  "<a name=" + anchor + " />"
               "<h" + snum + ">&nbsp;" + title_t + "</h" + snum + ">\n";

    textLength_ += str.length();
    text_ += str;

    addHItem(num, anchor, title_l);

}

void PageItems::composeHLine(int num, const QString &anchor,
                                 const QString &title, const QString &text)
{
    QString snum = QString::number(num);
    QString str;
    if (!text.isEmpty())
        str =  "<a name=" + anchor + " />"
               "<h" + snum + ">&nbsp;" + title +
               "</h" + snum + "><pre>" + text +
               "</pre>";
    else
        str =  "<a name=" + anchor + " />"
               "<h" + snum + ">&nbsp;" + title + "</h" + snum + ">\n";

    textLength_ += str.length();
    text_ += str;

    addHItem(num, anchor, title);

}

void PageItems::composeError(const QString &anchor,
                                 const QString &text)
{
    QString str = "<p><a name=" + anchor + " /><em class=err>" + text +
	          "</em></p>";
    textLength_ += str.length();
    text_ += str;
    QTreeWidgetItem *i = new QTreeWidgetItem(items_[0],
	                        QStringList() << text << anchor);
    i->setForeground(0, QColor("#886666"));
    items_ << i;

}

void PageItems::addHItem(int num, const QString &anchor, const QString &title)
{

    QTreeWidgetItem *parent = (num > 0) ? itemP_[num-1] : 0;
    curItem_ = new QTreeWidgetItem(parent, QStringList() << title << anchor);

    items_ << curItem_;
    itemP_[num] = curItem_;
}

void PageItems::composeTrail()
{
    textLength_ += 18;
    text_ += "\n</body></html>\n";
}

void PageItems::expand(int level)
{
    /*
       level = 0 : expand top only
       level = 1 : expand top and first children
       level = 2 : expand all
    */
    if (level == 2) {
        foreach(QTreeWidgetItem * i, items_) {
            i->setExpanded(true);
        }
    } else {
        QTreeWidgetItem *ip = items_[0];
        ip->setExpanded(true);
        if (level == 1) {
            for (int i = 0; i < ip->childCount(); i++) {
                ip->child(i)->setExpanded(true);
            }
        }
    }
}


PageWidget::PageWidget(QWidget *parent, const SearchMethod &method)
    : QSplitter(parent), method_(method)
{
    setObjectName("dicpage");
    bookTree = new QTreeWidget();
    bookTree->header()->hide();
    bookTree->setColumnCount(2);
    bookTree->setColumnHidden(1, true);
    bookTree->setFont(qApp->font());
    bookTree->setContextMenuPolicy(Qt::CustomContextMenu);
    bookTree->setIndentation(15);

    bookBrowser = new BookBrowser(this);
    bookBrowser->setObjectName("main_browser");

    addWidget(bookTree);
    addWidget(bookBrowser);
    setStretchFactor(indexOf(bookBrowser), 1);

    connect(this, SIGNAL(statusRequested(QString)),
            mainWin, SLOT(showStatus(QString)));
    connect(this, SIGNAL(selectionRequested(QString)),
            mainWin, SLOT(changeOptSearchButtonText(QString)));
    connect(mainWin, SIGNAL(viewFontChanged(QFont)),
            this, SLOT(changeFont(QFont)));
    connect(bookTree,
            SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            this, SLOT(scrollTo(QTreeWidgetItem*,QTreeWidgetItem*)));
    connect(bookTree,
            SIGNAL(itemPressed(QTreeWidgetItem*,int)),
            this, SLOT(scrollTo(QTreeWidgetItem*,int)));
    connect(bookTree, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(popupSlide(QPoint)));

    totalCount = 0;
    retStatus = NORMAL;

}

void PageWidget::scrollTo(QTreeWidgetItem *to)
{
    if (to && to->text(1).at(0) != 'P' ) {
        bookBrowser->scrollToAnchor(to->text(1));
        emit selectionRequested(to->text(0));
    }
}

void PageWidget::changeFont(const QFont &font)
{
    bookBrowser->document()->setDefaultFont(font);
    bookBrowser->setFont(font);
}

void PageWidget::popupSlide(const QPoint &pos)
{
    QTreeWidgetItem *item = bookTree->itemAt(pos);

    if (!item || !item->childCount())
        return;

    bookTree->setCurrentItem(item);
    TreeScrollPopup *popup = new TreeScrollPopup(item, 0);
    popup->move(bookTree->viewport()->mapToGlobal(pos));
    popup->show();
}

QString PageWidget::emphasize(const QString &str, const QString &word)
{
    enum { NO_SKIP=0, SKIP_TAG=0x01, SKIP_ENT=0x02 };
    QString ret;
    int slen = str.length();
    int wlen = word.length();
    unsigned int skip = NO_SKIP;

    for (int i = 0; i < slen; i++) {
        QChar a = str[i];
        if ((slen - i) < wlen) {
            ret += a;
            continue;
        }
        if (a == '<') {
            skip |= SKIP_TAG;
            ret += a;
            continue;
        }
        if (a == '&') {
            skip |= SKIP_ENT;
            ret += a;
            continue;
        }
        if (skip) {
            if (a == '>')
                skip &= ~SKIP_TAG;
	    else if (a == ';')
		skip &= ~SKIP_ENT;
            ret += a;
            continue;
        }
        if (a.isSpace()) {
            ret += a;
            continue;
        }
        QString cmp = str.mid(i, wlen);
        if (!QString::compare(cmp, word, Qt::CaseInsensitive)) {
            ret += "<span class=sel>" + cmp + "</span>";
            i += wlen - 1;
        } else {
            ret += a;
        }
    }

    return ret;
}

bool PageWidget::isMatch( const QString &str, const QStringList &list,
                          NarrowingLogic logic )
{
    if (list.count() == 1)
        return (str.contains(list[0], Qt::CaseInsensitive));

    foreach(QString s, list) {
        if (str.contains(s, Qt::CaseInsensitive)) {
            if (logic == LogicOR)
                return true;
        } else {
            if (logic == LogicAND)
                return false;
        }
    }

    return (logic == LogicAND);
}

//QRegExp regRep1("<img[^>]+>");
//QRegExp regRep2("<[^>]+>");

bool PageWidget::getText(EBook *eb, int index, QString *head_l, QString *head_v,
                         QString *text)
{
    QString t_v = eb->hitText(index);
    QString h_v = eb->hitHeading(index);

    int p = t_v.indexOf('\n');
    if (h_v.isEmpty()) {
        h_v = t_v.left(p);
        t_v = t_v.mid(p+1);
    } else if (h_v == t_v.left(p)) {
        t_v = t_v.mid(p+1);
    }
    QString h_l = h_v;

    if (h_l.contains('<')) {
        h_l.replace(QRegExp("<img[^>]+>"), "?");
        //h_l.replace(regRep1, "?");
        if (h_l.contains('<')) {
            h_l.replace(QRegExp("<[^>]+>"), "");
            //h_l.replace(regRep2, "");
	}
    }

    int sp = 0;
    while((sp = h_l.indexOf('&', sp)) >= 0) {
	if (h_l.mid(sp+1, 3) == "lt;") 
            h_l.replace(sp, 4, '<');
        else if (h_l.mid(sp+1, 3) == "gt;")
            h_l.replace(sp, 4, '>');
        else if(h_l.mid(sp+1, 4) == "amp;")
            h_l.replace(sp, 5, '&');
        else {
            int ep = h_l.indexOf(';', sp+1);
            if (ep < 0) break;
            h_l.replace(sp, ep-sp+1, '?');
        }
        sp++;
    }

    *head_l = h_l;
    *head_v = h_v;
    *text = t_v;

    return !(t_v.isEmpty());

}

bool PageWidget::getMatch(EBook *eb, int index, const QStringList &slist,
                          NarrowingLogic logic, 
                          QString *head_l, QString *head_v, QString *text)
{

    if (slist.count()) {
        QString t_i = eb->hitText(index, false);
        if (!isMatch(t_i, slist, logic))
            return false;
    }
    getText(eb, index, head_l, head_v, text);
    totalCount++;
    matchCount++;

    return true;

}

RET_SEARCH PageWidget::checkLimit(int text_length)
{

    if (totalCount >= method_.limitTotal)
        return LIMIT_TOTAL;
    if (matchCount >= method_.limitBook)
        return LIMIT_BOOK;
    if (text_length >= CONF->limitBrowserChar)
        return LIMIT_CHAR;

    return NORMAL;
}

InfoPage::InfoPage(QWidget *parent, const SearchMethod &method)
    : PageWidget(parent, method)
{
    Book *book = method.book;

    PageItems items(CONF->dictSheet);

    items.composeHLine(1, "TOP", book->name());

    bookBrowser->addBookList(book);
    EBook eb;
    if (eb.initBook(book->path(), book->bookNo()) < 0) {
        retStatus = NO_BOOK;
        return;
    }
    eb.initHook(bookBrowser->fontSize(), book->fontList(),
                  CONF->indentOffset,  method.ruby);

    QString mstr = "<b>";

    if (eb.isHaveText())
        mstr += QObject::tr("Text") + " ";
    if (eb.isHaveWordSearch())
        mstr += QObject::tr("Forward search") + " ";
    if (eb.isHaveEndwordSearch())
        mstr += QObject::tr("Backward search") + " ";
    if (eb.isHaveKeywordSearch())
        mstr += QObject::tr("Keyword search") + " ";
    if (eb.isHaveCrossSearch())
        mstr += QObject::tr("Cross search") + " ";

    mstr += "</b>";

    QString str = QString(tr("Title: <b>%1</b>\nSearch Method: %2"))
                          .arg(eb.title()).arg(mstr);
    items.composeHLine(2, "BOOK", eb.path(), str);

    if (eb.isHaveMenu()) {
        items.composeHLine(2, "MENU", "Menu", eb.menu());
    }

    if (eb.isHaveCopyright()) {
        items.composeHLine(2, "COPYRIGHT", "Copyright", eb.copyright());
    }

    QStringList sfile;
    sfile << "READM*" << "*.TXT" << "*.HTM" << "*.HTML" << "COPYRIGHT"
	  << "VERSION" << "PREFACE";
    QStringList resultFiles = QDir(book->path()).entryList(sfile, QDir::Files);
    for(int i=0; i<resultFiles.count(); i++) {
        QString fname = resultFiles[i];
        QFile file(book->path() + "/" + fname);
        file.open(QIODevice::ReadOnly);
        QString str = SJIStoUTF(file.readAll());
        str.remove("\r");
        if (!fname.right(4).compare(".htm", Qt::CaseInsensitive) ||
            !fname.right(5).compare(".html", Qt::CaseInsensitive)) {
            QRegExp reg("(<body[^>]*>|</body>)", Qt::CaseInsensitive);
            QStringList list = str.split(reg);
            if (list.count() < 3) continue;
            items.composeHLine(2, toAnchor("F", i), fname);
	    items.addHtmlStr(list[1]);
        } else {
            str = convSpecialChar(str);
            items.composeHLine(2, toAnchor("F", i), fname, str);
        }
    }
    items.composeTrail();
    //qDebug() << txt;
    bookBrowser->setBrowser(items.text());
    bookBrowser->setSearchPaths(QStringList() << book->path() << 
                                EbCache::cachePath);
    bookTree->insertTopLevelItems(0, items.items());
    bookTree->setCurrentItem(items.items()[0]);

    return;
}

QString InfoPage::convSpecialChar(const QString &str) const
{
    QString wrk = str;

    wrk.replace(QRegExp("<(?!lt;)"), "&lt;");
    wrk.replace(QRegExp(">(?!gt;)"), "&gt;");
    return wrk;
}

MenuPage::MenuPage(QWidget *parent, const SearchMethod &method)
    : PageWidget(parent, method)
{
    bookBrowser->addBookList(method.bookReader);
    connect(bookTree, SIGNAL(itemActivated(QTreeWidgetItem *, int)),
            this, SLOT(changePage(QTreeWidgetItem *, int)));
    fullMenuPage();

    if (retStatus == LIMIT_MENU)
        selectMenuPage(-1);

    return;
}

void MenuPage::fullMenuPage()
{
    checkMax = true;
    EbMenu eb;
    if (eb.initBook(method_.bookReader->path(), method_.bookReader->bookNo()) <
        0) {
        retStatus = NO_BOOK;
    }
    eb.initHook(bookBrowser->fontSize(), method_.bookReader->fontList(),
                CONF->indentOffset, method_.ruby);

    EB_Position pos;
    if (!eb.menu(&pos)) {
        retStatus = NO_MENU;
        return;
    }

    PageItems items(CONF->bookSheet);

    items.addHItem(0, toAnchor("B", 0), method_.bookReader->name());
    menuCount = 0;

    getMenus(&eb, pos, &items, 0);
    if (retStatus == LIMIT_MENU) {
        //while (!item.item().isEmpty())
        //    delete item.time().takeLast();
        return;
    }

    items.composeTrail();
    bookTree->insertTopLevelItems(0, items.items());
    items.expand(2);

    bookTree->setCurrentItem(items.topItem());

    bookBrowser->setBrowser(items.text());

    return;
}

void MenuPage::selectMenuPage(int index)
{
    checkMax = false;
    retStatus = NORMAL;
    EbMenu eb;
    eb.initBook(method_.bookReader->path(), method_.bookReader->bookNo());
    eb.initHook(bookBrowser->fontSize(), method_.bookReader->fontList(),
                CONF->indentOffset, method_.ruby);
    if (index < 0) {
        topCands = eb.topMenu();
        index = 0;
    }
   
    PageItems items(CONF->bookSheet);

    QString name = method_.bookReader->name();
    items.addHItem(0, toAnchor("B", 0), name);
    menuCount = 0;
    QTreeWidgetItem *next = 0;

    for (int i = 0; i < topCands.count(); i++) {
        if (i == index) {
            items.composeHLine(1, toAnchor("H", menuCount), topCands[i].title);
	    next = items.curItem();
            getMenus(&eb, topCands[i].position, &items, 1);
        } else {
	    items.addHItem(1, toAnchor("P", i), topCands[i].title);
	    items.curItem()->setForeground(0, QColor("#666688"));
        }
    }

    items.composeTrail();
    bookTree->clear();
    bookTree->insertTopLevelItems(0, items.items());
    items.expand(1);

    if (next) {
        bookTree->scrollToItem(next);
        bookTree->setCurrentItem(next);
    }

    bookBrowser->setBrowser(items.text());
    emit statusRequested(QString("%1").arg(menuCount));
}

void MenuPage::changePage(QTreeWidgetItem *item, int)
{
    QString anc = item->text(1);

    if (anc.at(0) == 'P') {
        int page = anc.mid(1).toInt();
        bookTree->setCurrentItem(NULL);
        selectMenuPage(page);
    }
}

void MenuPage::getMenus(EbMenu *eb, const EB_Position &pos, PageItems *items,
                        int count)
{
    count++;
    if ((menuCount % 100) == 0) {
        emit statusRequested(QString("%1").arg(menuCount));
        if (checkStop()) {
            retStatus = INTERRUPTED;
            return;
        }
    }
    QString c_text;
    QList <CandItems> list = eb->candidate(pos, &c_text);
    if (list.count()) {
        foreach(CandItems i, list) {
            menuCount++;
            if (checkMax && menuCount >= CONF->limitMenuHit) {
                retStatus = LIMIT_MENU;
                break;
            }
            EB_Position next = i.position;
            items->composeHLine(count, toAnchor("H", menuCount), i.title);
            getMenus(eb, next, items, count);
        }
    } else {
        QString text_v;
        if (count > 1) {
            int p = c_text.indexOf('\n');
            if (p >= 0 ) text_v = c_text.mid(p + 1);
        } else {
            text_v = c_text;
        }
        items->addTextStr("<pre>" + text_v + "</pre>\n");
    }
    return;
}

AllPage::AllPage(QWidget *parent, const SearchMethod &method)
    : PageWidget(parent, method)
{
    connect(bookTree, SIGNAL(itemActivated(QTreeWidgetItem *, int)),
            this, SLOT(changePage(QTreeWidgetItem *, int)));
    retStatus = initSeqHits();
    if (retStatus != NORMAL)
        return;

    readPage(0);

    return;
}

RET_SEARCH AllPage::readPage(int page)
{
    curPage = page;

    RET_SEARCH ret = NORMAL;

    PageItems item(CONF->dictSheet);

    int seq_hits = seqHits.count();
    if (seq_hits == 1) {
        item.composeHLine(1, toAnchor("B", 0), method_.bookReader->name());
    } else {
        QString str =  method_.bookReader->name() + '(' +
                QString::number(seqHits.count()) + ')';
	item.addHItem(0, "TOP", str);
    }
    QTreeWidgetItem *top_tree = item.curItem();

    EbAll eb;
    if(eb.initBook(method_.bookReader->path(), method_.bookReader->bookNo()) <
       0) {
        return NO_BOOK;
    }
    eb.initHook(bookBrowser->fontSize(), method_.bookReader->fontList(),
                  CONF->indentOffset, method_.ruby);
    bookBrowser->addBookList(method_.bookReader);

    QTreeWidgetItem *current_item = top_tree;
    QTreeWidgetItem *page_tree;
    int limit = CONF->limitMenuHit;

    for (int i = 0; i < seq_hits; i++) {
        if (i == page) {
            eb.setStartHit(seqHits[i]);
            int hit_num = eb.hitFull(limit);
            if (seqHits.count() > 1) {
                QString name = method_.bookReader->name() + '(' +
                               QString::number(i * limit + 1) + '-' +
                               QString::number(i * limit + hit_num) + ')';
                item.composeHLine(1, toAnchor("HIT", i), name);
		current_item = page_tree = item.curItem();
            }
            for (int j = 0; j < hit_num; j++) {
                if ((j % 100) == 0) {
                    emit statusRequested(method_.bookReader->name() + '(' +
                                         QString::number(j) + "page)");
                    if (checkStop()) break;
                }
                QString head_l;
                QString head_v;
                QString text_v;
                getText(&eb, j, &head_l, &head_v, &text_v);
                item.composeHLine(2, toAnchor("H", j), head_l, head_v, text_v);
                if (item.textLength() > CONF->limitBrowserChar) {
                    item.composeError(toAnchor("CUT", 1), CutString);
                    ret = LIMIT_CHAR;
                    break;
                }
            }
        } else {
            QString t = eb.text(seqHits[i],  false);
            QString head_i = t.left(t.indexOf('\n'));
            QString name = "(" + QString::number(i * limit + 1) + "-) " +
                           head_i;
            item.addHItem(1, toAnchor("P", i), name);
	    page_tree = item.curItem();
            page_tree->setForeground(0, QColor(50, 50, 150));
        }
    }

    if (checkStop()) {
        item.composeError("LAST", IntString);
        ret = INTERRUPTED;
    }
    item.composeTrail();

    bookTree->clear();
    bookTree->insertTopLevelItems(0, item.items());
    item.expand(1);

    bookTree->setCurrentItem(current_item);

    bookBrowser->setBrowser(item.text());
    return ret;
}

void AllPage::changePage(QTreeWidgetItem *item, int)
{
    QString anc = item->text(1);

    if (anc.at(0) == 'P') {
        int page = anc.mid(1).toInt();
        readPage(page);
    }
}

RET_SEARCH AllPage::initSeqHits()
{
    EbAll eb;

    if (eb.initBook(method_.bookReader->path(), method_.bookReader->bookNo()) <
        0)
        return NO_BOOK;

    int cnt = 0;
    for (;;) {
        emit statusRequested( "Correnting index data");
        if (checkStop()) break;

        int hit_num = eb.hitFull(CONF->limitMenuHit);
        if (hit_num)
            seqHits << eb.hit(0).text;
        if (hit_num < CONF->limitMenuHit) break;

        cnt++;
        if (cnt > 20000) break;
    }
    return NORMAL;
}


SearchPage::SearchPage(QWidget *parent, const QStringList &slist,
                       const SearchMethod &method)
    : PageWidget(parent, method)
{

    QStringList search_list;
    if (method.direction != KeywordSearch &&
        method.direction != CrossSearch ) {
        search_list = slist;
        search_list.removeFirst();
    }

    PageItems items(CONF->dictSheet);
    items.addHItem(0, "TOP", "tmp");

    EBook eb;
    int book_count = 0;
    int req_cnt = method.limitBook;
    if (search_list.count())
        req_cnt = -1;

    bool break_flag = false;

    foreach(Book *book, method.group->bookList()) {
        if (book->bookType() != BookEpwingLocal) continue;
        if (book->checkState() != Qt::Checked) continue;

        emit statusRequested(book->name() + ":(" +
                             QString::number(totalCount) + ")");
        if (checkStop() || break_flag) break;

        if ( eb.initBook(book->path(), book->bookNo(), book_count) < 0) continue;

        eb.initHook(bookBrowser->fontSize(), book->fontList(),
                      CONF->indentOffset, method.ruby);

        int hit_num = 0;
        if (method.direction == KeywordSearch) {
            hit_num = eb.hitMultiWord(req_cnt, slist, SearchKeyWord);
        } else if (method.direction == CrossSearch) {
            hit_num = eb.hitMultiWord(req_cnt, slist, SearchCrossWord);
        } else if (method.direction == ExactWordSearch) {
            hit_num = eb.hitWord(req_cnt, slist[0], SearchExactWord);
        } else if (method.direction == ForwardSearch) {
            hit_num = eb.hitWord(req_cnt, slist[0], SearchWord);
        } else if (method.direction == BackwardSearch) {
            hit_num = eb.hitWord(req_cnt, slist[0], SearchEndWord);
        } else {
            qWarning() << "Invalid Search Method" << method.direction;
        }
        if (hit_num <= 0) {
            eb.unsetBook();
            continue;
        }

        matchCount = 0;
        for (int i = 0; i < hit_num; i++) {
            if (checkStop()) break;

            QString head_i;
            QString head_v;
            QString text_v;
            if (!getMatch(&eb, i, search_list, method.logic,
                             &head_i, &head_v, &text_v)) continue;

            if (matchCount == 1) {
                bookBrowser->addBookList(book);
                book_count++;
                items.composeHLine(1, toAnchor("B", book_count), book->name());
            }
            if (CONF->highlightMatch) {
                foreach(QString s, slist) {
                    head_v = emphasize(head_v, s);
                    text_v = emphasize(text_v, s);
                }
            }
            items.composeHLine(2, toAnchor("H", totalCount), head_i, head_v, text_v);

            RET_SEARCH chk = checkLimit(items.textLength());
            if (chk != NORMAL) {
                items.composeError(toAnchor("CUT", totalCount), CutString);
                if (chk != LIMIT_BOOK) {
                    break_flag = true;
                    retStatus = chk;
                }
                break;
            }
        }
        if (matchCount == 0) {
            eb.unsetBook();
            continue;
        }
	items.item(1)->setText(0, book->name() + " (" +
                                  QString::number(matchCount)  + ')');

        eb.unsetBook();
    }
    if (totalCount == 0) {
        retStatus = (checkStop()) ? NOT_HIT_INTERRUPTED : NOT_HIT;
        return;
    }
    if (checkStop()) {
        items.composeError("LAST", IntString);
        retStatus = INTERRUPTED;
    }
    items.composeTrail();

    emit statusRequested(QString("List (%1 items)").arg(totalCount));
    checkStop();

    QTreeWidgetItem *top_item = items.topItem();
    QString top_title = toLogicString(slist, method);
    top_item->setText(0, QString("%1(%2)").arg(top_title).arg(totalCount));

    bookTree->insertTopLevelItems(0, items.items());
    if (totalCount <= 100 || top_item->childCount() == 1) {
        items.expand(1);
    } else {
        items.expand(0);
    }
    bookTree->setCurrentItem(top_item);
    emit statusRequested(QString("Browser (%1 character)")
                         .arg(items.textLength()));
    checkStop();

    bookBrowser->setBrowser(items.text());
    //qDebug() << items.text();

    return;
}

SearchWholePage::SearchWholePage(QWidget *parent, const QStringList &slist, 
                               const SearchMethod &method)
    : PageWidget(parent, method)
{
    EbAll eb;

    int search_total = 0;
    int book_count = 0;

    PageItems item(CONF->dictSheet);
    item.addHItem(0, "TOP", "tmp");

    bool break_flag = false;
    RET_SEARCH break_check = NORMAL;

    foreach(Book * book, method.group->bookList()) {
        if (book->bookType() != BookEpwingLocal) continue;
        if (checkStop() || break_flag) break;

        if (book->checkState() != Qt::Checked) continue;

        matchCount = 0;

        if (eb.initBook(book->path(), book->bookNo(), book_count) < 0) {
            qDebug() << "Can't open the book" << book->path() << book->bookNo();
            continue;
        }
        eb.initSeek();
        if (!eb.isHaveText()) {
            eb.unsetBook();
            continue;
        }
        eb.initHook(bookBrowser->fontSize(), book->fontList(),
                      CONF->indentOffset, method.ruby);
        bookBrowser->addBookList(book);
        book_count++;

        int search_count = 0;
        for (;;) {
            emit statusRequested(book->name() + ":(" +
                                 QString::number(matchCount) + '/' +
                                 QString::number(search_count) + ") Total:(" +
                                 QString::number(totalCount) + '/' +
                                 QString::number(search_total) + ')');
            
            if (checkStop() || break_flag) break;

            int hit_num = eb.hitFull(3000);
            search_count += hit_num;
            search_total += hit_num;

            for (int i = 0; i < hit_num; i++) {
                QString head_i;
                QString head_v;
                QString text_v;
                if (!getMatch(&eb, i, slist, method.logic,
                                 &head_i, &head_v, &text_v)) continue;

                if (matchCount == 1) {

                    item.composeHLine(1, toAnchor("B", book_count),
                                      book->name());
                }
                if (CONF->highlightMatch) {
                    foreach(QString s, slist) {
                        head_v = emphasize(head_v, s);
                        text_v = emphasize(text_v, s);
                    }
                }
                item.composeHLine(2, toAnchor("H", totalCount), head_i,
                                  head_v, text_v);
                break_check = checkLimit(item.textLength());
                if (break_check != NORMAL) {
                    item.composeError(toAnchor("C", totalCount), CutString);
                    break;
                }
            }
            if (break_check != NORMAL) {
                if (break_check == LIMIT_BOOK) {
                     break_check = NORMAL;
                } else {
                    break_flag = true;
                    retStatus = break_check;
                }
                break;
            }

            if (hit_num < 3000) break;
        }
        eb.unsetBook();
        if (matchCount == 0) continue;

	item.item(1)->setText(0, QString("%1(%2)")
                                 .arg(book->name()).arg(matchCount));
    }
    if (totalCount == 0) {
        delete item.topItem();
        retStatus = (checkStop()) ? NOT_HIT_INTERRUPTED : NOT_HIT;
        return;
    }
    if (checkStop()) {
        item.composeError("LAST", IntString);
        retStatus = INTERRUPTED;
    }
    item.composeTrail();

    emit statusRequested(QString("List (%1 items)").arg(totalCount));
    checkStop();

    bookTree->insertTopLevelItems(0, item.items());
    QString top_title = toLogicString(slist, method);
    QTreeWidgetItem *top_tree = item.topItem();
    top_tree->setText(0, QString("%1(%2)").arg(top_title).arg(totalCount));
    if (totalCount <= 100 || top_tree->childCount() == 1) {
        item.expand(1);
    } else {
        item.expand(0);
    }

    bookTree->setCurrentItem(top_tree);

    checkStop();
    emit statusRequested(QString("Browser (%1 characters)")
                         .arg(item.textLength()));
    checkStop();

    bookBrowser->setBrowser(item.text());

    return;
}

WebPage::WebPage(QWidget *parent, const QString &url,
                 const SearchMethod &meth,
                 const QStringList &slist)
    : QWebView(parent), method_(meth)
{
    setObjectName("webpage");
    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    tabBar_ = 0;
    QString str = url;
    connect(this, SIGNAL(loadStarted()),
            this, SLOT(progressStart()));
    connect(this, SIGNAL(loadFinished(bool)),
            this, SLOT(progressFinished(bool)));
    connect(this, SIGNAL(loadProgress(int)),
            this, SLOT(progress(int)));
    connect(this, SIGNAL(linkClicked(const QUrl&)),
            this, SLOT(openLink(const QUrl&)));
    connect(this, SIGNAL(linkRequested(QString)),
            mainWin, SLOT(execProcess(QString)));

    QByteArray enc = encString(url);
    QString ustr = setSearchString(url, enc, slist);
    QString sdir = directionString(url);
    if (!sdir.isEmpty()) {
        ustr = setDirectionString(ustr, sdir, method_.direction);
    }

    qDebug() << ustr;
                     
    QWebSettings *ws = settings();
    ws->setAttribute(QWebSettings::JavascriptEnabled, true);
    ws->setAttribute(QWebSettings::JavaEnabled, true);
    ws->setAttribute(QWebSettings::PluginsEnabled, true);
    if (enc.isEmpty()) {
        load(QUrl::fromEncoded(ustr.toAscii()));
    } else {
        load(QUrl::fromEncoded(QTextCodec::codecForName(enc)->fromUnicode(ustr)));
    }

    show();
}

WebPage::WebPage(QWidget *parent, const QString &url)
    : QWebView(parent)
{
    setObjectName("webpage");
    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    tabBar_ = 0;
    connect(this, SIGNAL(loadStarted()),
            this, SLOT(progressStart()));
    connect(this, SIGNAL(loadFinished(bool)),
            this, SLOT(progressFinished(bool)));
    connect(this, SIGNAL(loadProgress(int)),
            this, SLOT(progress(int)));
    connect(this, SIGNAL(linkClicked(const QUrl&)),
            this, SLOT(openLink(const QUrl&)));
    connect(this, SIGNAL(linkRequested(QString)),
            mainWin, SLOT(execProcess(QString)));

    qDebug() << url;

    QWebSettings *ws = settings();
    ws->setAttribute(QWebSettings::JavascriptEnabled, true);
    ws->setAttribute(QWebSettings::JavaEnabled, true);
    ws->setAttribute(QWebSettings::PluginsEnabled, true);

    load(url);
    /*
    if (enc.isEmpty()) {
        load(QUrl::fromEncoded(ustr.toAscii()));
    } else {
        load(QUrl::fromEncoded(QTextCodec::codecForName(enc)->fromUnicode(ustr)));
    }
    */

    show();
}

QByteArray WebPage::encString(const QString &url)
{
    if (!url.contains(QRegExp("\\{.*\\}"))) {
        return QByteArray();
    } else {
        QString w1 = url.mid(url.indexOf('{')+1);
        QString w2 = w1.left(w1.indexOf('}'));
        w2.trimmed();
        return w2.toAscii();
    }
}

QString WebPage::setSearchString(const QString &url, const QByteArray &enc,
                                 const QStringList &slist)
{
    QString str;
    for (int i=0; i<slist.count(); i++) {
        str += slist[i];
        if (i < (slist.count() - 1)){
            str += " ";
        }
    }
    QByteArray bstr;
    if (enc.isEmpty()) {
        bstr = str.toUtf8();
    } else {
        bstr = QTextCodec::codecForName(enc)->fromUnicode(str);
    }
    QString fstr("");
    foreach(const char c, bstr) {
        fstr += "%" + QString::number((ushort)((uchar)c), 16).toUpper();
    }
    QString ustr = url;
    QRegExp rx("\\{.*\\}");
    if (!url.contains(rx)) {
        ustr += fstr;
    } else {
        ustr.replace(rx, fstr);
    }
    return ustr;
}
QString WebPage::directionString(const QString &url)
{
    if (!url.contains(QRegExp("\\[.*\\]"))) {
        return QString();
    } else {
        QString w1 = url.mid(url.indexOf('[')+1);
        QString w2 = w1.left(w1.indexOf(']'));
        w2.trimmed();
        return w2;
    }
}
QString WebPage::setDirectionString(const QString &url, const QString &dstr,
                                    SearchDirection &direc)
{

    QChar cdirec;
    switch(direc) {
        case ExactWordSearch :
            cdirec = 'E'; break;
        case ForwardSearch :
            cdirec = 'F'; break;
        case BackwardSearch :
            cdirec = 'B'; break;
        case FullTextSearch :
            cdirec = 'W'; break;
        default:
            cdirec = 'E';
    }

    QString udirec;

    bool first = true;
    foreach(QString s, dstr.split(',')) {

        // check format
        if (s.indexOf(':') != 1) {
            qWarning() << "Url Search Type Error" << dstr;
            return url;
        }

        if (first || s[0] == cdirec) {
            first = false;
            udirec = s.mid(2);
            break;
        }

    }
    
    if (udirec.isEmpty()) {
        qWarning() << "Url Search Type Error" << dstr;
        return url;
    }

    return QString(url).replace(QRegExp("\\[.*\\]"), udirec);

}


void WebPage::progressStart()
{
    if (tabBar_) tabBar_->setTabIcon(tabIndex_, QIcon(":/images/web2.png"));

    progressCount_ = 0;
}

void WebPage::progress(int)
{
    if (!tabBar_) return;
    if (progressCount_ == 0)  {
        tabBar_->setTabIcon(tabIndex_, QIcon(":/images/web3.png"));
        progressCount_ = 1;
    } else {
        tabBar_->setTabIcon(tabIndex_, QIcon(":/images/web2.png"));
        progressCount_ = 0;
    }
}

void WebPage::progressFinished(bool)
{
    if (tabBar_) tabBar_->setTabIcon(tabIndex_, QIcon(":/images/web1.png"));
}

void WebPage::openLink(const QUrl &url)
{
    QUrl u = QUrl::fromEncoded(url.toEncoded(), QUrl::TolerantMode);
    qDebug() << url.toEncoded();
    qDebug() << u.toString();

    //emit linkRequested(CONF->browserProcess + ' ' + u.toString());
    emit linkRequested(CONF->browserProcess + ' ' +
                       QString::fromAscii(url.toEncoded()));
}

BookView::BookView(QWidget *parent)
    : QTabWidget(parent)
{
    mainWin = parent;
    setUsesScrollButtons(true);
    QToolButton *close_button = new QToolButton(this);
    setCornerWidget(close_button, Qt::TopRightCorner);
    QToolButton *close_all_button = new QToolButton(this);
    setCornerWidget(close_all_button, Qt::TopLeftCorner);
    close_button->setCursor(Qt::ArrowCursor);
    close_button->setAutoRaise(true);
    close_button->setIcon(QIcon(":images/closetab.png"));
    close_button->setToolTip(tr("Close page"));
    close_button->setEnabled(true);
    close_all_button->setCursor(Qt::ArrowCursor);
    close_all_button->setAutoRaise(true);
    close_all_button->setIcon(QIcon(":images/closealltab.png"));
    close_all_button->setToolTip(tr("Close All Page"));
    close_all_button->setEnabled(true);
    connect(close_button, SIGNAL(clicked()), this, SLOT(closeTab()));
    connect(close_all_button, SIGNAL(clicked()), this, SLOT(closeAllTab()));
    connect(this, SIGNAL(tabChanged(int)),
            mainWin, SLOT(changeViewTabCount(int)));
    connect(this, SIGNAL(currentChanged(int)),
            mainWin, SLOT(showTabInfo(int)));
    tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tabBar(), SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(showTabBarMenu(const QPoint&)));
    connect(this, SIGNAL(statusRequested(QString)),
            mainWin, SLOT(showStatus(QString)));
}


RET_SEARCH BookView::newPage(const QStringList &list,
                             const SearchMethod &method, bool newTab)
{
    stopFlag = false;

    PageWidget *page;

    if (method.direction == FullTextSearch)
        page = new SearchWholePage(this, list, method);
    else if (method.direction == WholeRead)
        page = new AllPage(this, method);
    else if (method.direction == MenuRead)
        page = new MenuPage(this, method);
    else if (method.direction == BookInfo)
        page = new InfoPage(this, method);
    else
        // ExactWordSearch, ForwardSearch, BackwardSearch,
        // KeywordSearch, CrossSearch,
        page = new SearchPage(this, list, method);

    RET_SEARCH ret = page->retStatus;
    QWidget *focus_page = 0;
    if (ret == NOT_HIT || ret == NOT_HIT_INTERRUPTED || ret == NO_MENU ||
        ret == NO_BOOK) {
        delete page;
        bool rflag = true;
        foreach(Book *book, method.group->bookList()) {
            if (book->bookType() == BookWeb) {
                ret = NORMAL;
                rflag = false;
            }
        }
        if (rflag) return ret;
    } else {

        QString tab_title = toLogicString(list, method, false);
        if (newTab || count() == 0) {
            addTab(page, QIcon(":/images/sbook.png"), tab_title);
        } else {
            int index = currentIndex();
            closeTab();
            insertTab(index, page, QIcon(":/images/sbook.png"), tab_title);
        }
        focus_page = (QWidget*)page;
        connect(this, SIGNAL(fontChanged(QFont)), page, SLOT(changeFont(QFont)));
        emit tabChanged(count());
    }

    foreach(Book *book, method.group->bookList()) {
        if (book->bookType() == BookWeb &&
                book->checkState() == Qt::Checked) {
            WebPage *wpage = new WebPage(this, book->path(), method, list);
            QString vtitle = QString("%1(%2)").arg(list[0]).arg(book->name());
            int idx = addTab(wpage, QIcon(":/images/web2.png"), vtitle);
            wpage->setTabIndex(idx);
            wpage->setTabBar(tabBar());
            if (!focus_page) {
                focus_page = (QWidget *)wpage;
            }
        }
    }
    if (focus_page) {
        setCurrentWidget(focus_page);
    }

    return ret;
}

RET_SEARCH BookView::newWebPage(const QString &name, const QString &url)
{
    WebPage *wpage = new WebPage(this, url);
    int idx = addTab(wpage, QIcon(":/images/web2.png"), name);
    wpage->setTabIndex(idx);
    wpage->setTabBar(tabBar());
    setCurrentWidget(wpage);
    emit tabChanged(count());

    return NORMAL;
}

void BookView::showTabBarMenu(const QPoint& pnt)
{
    QTabBar *bar = tabBar();
    int index = 0;
    int tabnum = bar->count();
    for (; index < tabnum; index++) {
        if (bar->tabRect(index).contains(pnt))
            break;
    }
    QMenu menu("", bar);
    QAction *close_other_page = 0; 
    QAction *close_left_page = 0;
    QAction *close_right_page = 0;
    QAction *close_all_page = 0;
    QAction *close_this_page = menu.addAction(tr("Close This Page"));
    if (tabnum > 1)
        close_other_page = menu.addAction(tr("Close Other Pages"));
    if (index > 0)
        close_left_page = menu.addAction(tr("Close Left Pages"));
    if (index < (tabnum-1))
        close_right_page = menu.addAction(tr("Close Right Pages"));
    if (tabnum > 1)
        close_all_page = menu.addAction(tr("Close All Pages"));

    QAction *sel_act = menu.exec(bar->mapToGlobal(pnt));
    if (!sel_act)
        return;
    if(sel_act == close_this_page) {
        closeTab1(index);
    } else if (sel_act == close_other_page) {
        for (int i=tabnum-1; i>index; i--) {
            closeTab1(i);
        }
        for (int i=index-1; i>=0; i--) {
            closeTab1(i);
        }
    } else if (sel_act == close_left_page) {
        for (int i=index-1; i>=0; i--) {
            closeTab1(i);
        }
    } else if (sel_act == close_right_page) {
        for (int i=tabnum-1; i>index; i--) {
            closeTab1(i);
        }
    } else if (sel_act == close_all_page) {
        for (int i=tabnum-1; i>=0; i--) {
            closeTab1(i);
        }
    }
    emit tabChanged(count());
}

void BookView::closeTab1(int index)
{
    QWidget *wgt = widget(index);
    removeTab(index);
    //wgt->close();
    QTimer::singleShot(0, wgt, SLOT(deleteLater()));

}

void BookView::closeAllTab()
{
    QTabBar *bar = tabBar();
    for (int i=bar->count()-1;  i >= 0; i--) {
        closeTab1(i);
    }

    emit tabChanged(count());
}

void BookView::closeTab()
{
//    int idx = currentIndex();
//    QWidget *wgt = currentWidget();

//    removeTab(idx);
//    wgt->close();
//    delete wgt;

    closeTab1(currentIndex());

    emit tabChanged(count());
}
BookType BookView::pageType(int index)
{
    QWidget *w = widget(index);
    if (w->objectName() == "dicpage"){
        return BookEpwingLocal;
    } else if (w->objectName() == "webpage") {
        return BookWeb;
    } else {
        return BookWeb;
    }

}
BookType BookView::currentPageType()
{
    return pageType(currentIndex());
}

SearchMethod BookView::pageMethod(int index)
{
    QWidget *w = widget(index);
    BookType t = pageType(index);
    if (t == BookEpwingLocal) {
        return ((PageWidget*)w)->method();
    } else if (t == BookWeb) {
        return ((WebPage*)w)->method();
    } else {
        return ((WebPage*)w)->method();
    }
}

SearchMethod BookView::currentPageMethod()
{
    return pageMethod(currentIndex());
}

void BookView::stopSearch()
{
    stopFlag = true;
}

void BookView::zoomIn()
{
    int fsize = CONF->browserFont.pointSize();

    CONF->browserFont.setPointSize(fsize + 1);
    emit fontChanged(CONF->browserFont);
}

void BookView::zoomOut()
{
    int fsize = CONF->browserFont.pointSize();

    if (fsize > 5) {
        CONF->browserFont.setPointSize(fsize - 1);
        emit fontChanged(CONF->browserFont);
    }
}


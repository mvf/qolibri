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

#include "mainwindow.h"
#include "book.h"
#include "bookview.h"
#include "ebook.h"
#include "configure.h"

static bool stopFlag = false;
static MainWindow *mainWin = 0;

const char* CutString = "----- cut -----";
const char* IntString = "----- interrupted -----";

#define SJIStoUTF(q_bytearray) \
    QTextCodec::codecForName("Shift-JIS")->toUnicode(q_bytearray)         

#define FONT_RESTORE_DURING_ZOOM

BookBrowser::BookBrowser(QWidget *parent)
    : QTextBrowser(parent)
{
    setOpenExternalLinks(false);
    setSearchPaths(QStringList() << EBook::cachePath);
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
    QStringList args = name.toString().split('|');

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
             
            ReferencePopup *popup =
                new ReferencePopup(bookList_[index], args[2].toInt(),
                                   args[3].toInt(), this, mflag);
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

    QList <QAction*> alist;
    QList <SearchDirection> dlist;
    if (textCursor().hasSelection()) {
        menu->addSeparator();
        alist << menu->addAction(QObject::tr("&Exact word search"));
        dlist << ExactWordSearch;
        alist << menu->addAction(QObject::tr("&Forward search"));
        dlist << ForwardSearch;
        alist << menu->addAction(QObject::tr("&Keyword search"));
        dlist << KeywordSearch;
        alist << menu->addAction(QObject::tr("&Cross search"));
        dlist << CrossSearch;
        menu->addSeparator();
        alist << menu->addAction(QObject::tr("&Google search"));
        dlist << GoogleSearch;
        alist << menu->addAction(QObject::tr("&WikiPedia search"));
        dlist << WikipediaSearch;
        alist << menu->addAction(QObject::tr("&User defined URL search"));
        dlist << Option1Search;
        menu->addSeparator();
        menu->addAction(QObject::tr("&Paste selected string to edit line"),
		        this, SLOT(pasteSearchText()));
    }
    QAction *a = menu->exec(event->globalPos());
    if (a && alist.indexOf(a) >= 0) {
        int i = alist.indexOf(a);
        SearchDirection d = (SearchDirection)dlist[i];
        if (d <= MenuRead) {
            emit searchRequested(dlist[i], textCursor().selectedText());
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

ReferencePopup::ReferencePopup(Book *book, int page, int offset,
                               QWidget *parent, bool menu_flag)
    : QWidget(parent), menuFlag(menu_flag)
{
    bookBrowser = new BookBrowserPopup(parent);
    QToolButton *close_button = new QToolButton(this);
    close_button->setIcon(QIcon(":images/closetab.png"));
    connect(close_button, SIGNAL(clicked()), this, SLOT(close()));
    bookBrowser->setCornerWidget(close_button);
    //bookBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    bookBrowser->addBookList(book);
    bookBrowser->addTitle("<a class=cls href=close>" + book->name() + "</a>");


    QVBoxLayout *v = new QVBoxLayout();
    v->addWidget(bookBrowser);
    v->setMargin(0);
    v->setSpacing(0);
    setLayout(v);

    bookBrowser->setBrowser(browserText(book, page, offset));

    setWindowFlags(Qt::Popup);

    move(parent->mapToGlobal(QPoint(0, 0)));
    resize(parent->size() - QSize(0, 0));
}


QString ReferencePopup::browserText(Book *book, int page, int offset)
{
    EBook eb;

    eb.setBook(book->path(), book->bookNo());
    eb.initSearch(16, book->fontList(), CONF->indentOffset);
    bookBrowser->setSearchPaths(QStringList() << eb.cachePath);
    QString text = eb.text(page, offset);
    QString ttl = QString();
    if (!menuFlag) {
        QString heading = text.left(text.indexOf('\n'));

        foreach(QString s, bookBrowser->titles()) {
            ttl += s + " > ";
        }
        ttl += heading;
        QString addr = QString("book|%1|%2|%3|%4").arg(0).arg(page).arg(offset)
                               .arg(bookBrowser->titles().count());
        bookBrowser->addTitle("<a class=ref href=" + addr +
                          " >" + heading + "</a>");
    } else {
        ttl = bookBrowser->titles()[0];
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
        "<h1>" + ttl + "</h1>\n" +
        "<pre>" + eb.text(page, offset) + "</pre>\n"
        "</body>\n"
        "</html>\n";
    return txt;
}

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
        ReferencePopup *popup = (ReferencePopup*)parentWidget();
        QString str = popup->browserText(bookList_[index],
                                            args[2].toInt(), args[3].toInt());
        setBrowser(str);
        return;
    }
    BookBrowser::setSource(name);
    return;
}

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

void PageItems::setTopItem(const QString &anchor, const QString &title)
{
    curItem_  = new QTreeWidgetItem((QTreeWidget*)0,
                                    QStringList() << title << anchor);
    items_ << curItem_;
    itemP_[0] = curItem_;

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

    curItem_ = new QTreeWidgetItem(itemP_[num-1],
	                              QStringList() << title << anchor);
    items_ << curItem_;
    itemP_[num] = curItem_;
}

void PageItems::composeTrail()
{
    textLength_ += 18;
    text_ += "\n</body></html>\n";
}


PageWidget::PageWidget(QWidget *parent, const SearchMethod &method)
    : QSplitter(parent), method_(method)
{
    bookTree = new QTreeWidget();
    bookTree->header()->hide();
    bookTree->setColumnCount(2);
    bookTree->setColumnHidden(1, true);
    bookTree->setFont(qApp->font());
    bookTree->setContextMenuPolicy(Qt::CustomContextMenu);
    bookTree->setIndentation(15);

    bookBrowser = new BookBrowser(this);

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

bool PageWidget::checkStop()
{
    QEventLoop().processEvents();
    return stopFlag;
}

void PageWidget::scrollTo(QTreeWidgetItem *to)
{
    if (to && to->text(1).left(4) != "PAGE" ) {
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
    QString ret = QString();
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

bool PageWidget::getText(EBook *eb, int index, QString *head_l, QString *head_v,
                         QString *text)
{
    QString t_v = eb->text(index);
    QString h_v = eb->heading(index);

    int p = t_v.indexOf('\n');
    if (h_v.isEmpty()) {
        h_v = t_v.left(p);
        t_v = t_v.mid(p+1);
    } else if (h_v == t_v.left(p)) {
        t_v = t_v.mid(p+1);
    }
    QString h_l = h_v;

    if (h_l.contains('<')) {
        h_l.replace(QRegExp("<img[^>]*>"), "?");
        if (h_l.contains('<')) {
            h_l.replace(QRegExp("<[^>]*>"), " ");
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
        QString t_i = eb->text(index, false);
        if (!isMatch(t_i, slist, logic))
            return false;
    }
    getText(eb, index, head_l, head_v, text);
    totalCount++;
    matchCount++;

    return true;

}

RET_SEARCH PageWidget::checkLimit(int image_cnt, int text_length)
{

    if (totalCount >= method_.limitTotal)
        return LIMIT_TOTAL;
    if (matchCount >= method_.limitBook)
        return LIMIT_BOOK;
    if (text_length >= CONF->limitBrowserChar)
        return LIMIT_CHAR;
    if (image_cnt >= CONF->limitImageNum)
        return LIMIT_IMAGE;

    return NORMAL;
}

InfoPage::InfoPage(QWidget *parent, const SearchMethod &method)
    : PageWidget(parent, method)
{
    Book *book = method.book;

    PageItems items(CONF->dictSheet);

    items.composeHLine(1, "TOP", book->name(), QString());

    bookBrowser->addBookList(book);
    EBook eb;
    if (eb.setBook(book->path(), book->bookNo()) < 0) {
        retStatus = NO_BOOK;
        return;
    }
    eb.initSearch(bookBrowser->fontSize(), book->fontList(),
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
        QString anchor = toAnchor("FILE", i);
        QFile file(book->path() + "/" + fname);
        file.open(QIODevice::ReadOnly);
        QByteArray data = file.readAll();
        QString str = SJIStoUTF(data);
        str.remove("\r");
        if (!fname.right(4).compare(".htm", Qt::CaseInsensitive) ||
            !fname.right(5).compare(".html", Qt::CaseInsensitive)) {
            QRegExp reg("(<body[^>]*>|</body>)", Qt::CaseInsensitive);
            QStringList list = str.split(reg);
            if (list.count() < 3) continue;
            items.composeHLine(2, anchor, fname, QString());
	    items.addHtmlStr(list[1]);
        } else {
            str = convSpecialChar(str);
            items.composeHLine(2, anchor, fname, str);
        }
    }
    items.composeTrail();
    //qDebug() << txt;
    bookBrowser->setBrowser(items.text());
    bookBrowser->setSearchPaths(QStringList() << book->path() << eb.cachePath);
    bookTree->insertTopLevelItems(0, items.items());
    items.items()[0]->setExpanded(true);
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
    EBook eb(HookMenu);
    if (eb.setBook(method_.bookReader->path(), method_.bookReader->bookNo()) <
        0) {
        retStatus = NO_BOOK;
    }
    eb.initSearch(bookBrowser->fontSize(), method_.bookReader->fontList(),
                  CONF->indentOffset, method_.ruby);
    int page;
    int offset;
    if (!eb.menu(&page, &offset)) {
        retStatus = NO_MENU;
        return;
    }

    PageItems items(CONF->bookSheet);

    QString anchor = toAnchor("BOOK", 0);
    items.setTopItem(anchor, method_.bookReader->name());
    menuCount = 0;

    getMenus(&eb, page, offset, &items, 0);
    if (retStatus == LIMIT_MENU) {
        //while (!item.item().isEmpty())
        //    delete item.time().takeLast();
        return;
    }

    items.composeTrail();
    bookTree->insertTopLevelItems(0, items.items());
    foreach(QTreeWidgetItem * i, items.items()) {
        i->setExpanded(true);
    }

    bookTree->setCurrentItem(items.topItem());

    bookBrowser->setBrowser(items.text());

    return;
}

void MenuPage::selectMenuPage(int index)
{
    checkMax = false;
    retStatus = NORMAL;
    EBook eb(HookMenu);
    eb.setBook(method_.bookReader->path(), method_.bookReader->bookNo());
    eb.initSearch(bookBrowser->fontSize(), method_.bookReader->fontList(),
                  CONF->indentOffset, method_.ruby);
    int page;
    int offset;
    eb.menu(&page, &offset);
    if (index < 0) {
        getTopMenu(&eb, page, offset);
        index = 0;
    }

   
    PageItems items(CONF->bookSheet);

    QString anchor = toAnchor("BOOK", 0);
    QString name = method_.bookReader->name();
    items.setTopItem(anchor, name);
    QTreeWidgetItem *top_tree = items.curItem();
    menuCount = 0;
    QTreeWidgetItem *next = 0;

    for (int i = 0; i < topMenus.count(); i++) {
        if (i == index) {
            QString anchor = toAnchor("H", menuCount);
            items.composeHLine(1, anchor, topTitles[i], QString());
	    next = items.curItem();
            getMenus(&eb, topMenus[i].page, topMenus[i].offset, &items, 1);
        } else {
            QString anchor = toAnchor("PAGE", i);
	    items.addHItem(1, anchor, topTitles[i]);
	    items.curItem()->setForeground(0, QColor("#666688"));
        }
    }

    items.composeTrail();
    bookTree->clear();
    bookTree->insertTopLevelItems(0, items.items());
    bookTree->expandItem(top_tree);
    for (int i = 0; i < top_tree->childCount(); i++) {
        top_tree->child(i)->setExpanded(true);
    }

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

    if (anc.left(4) == "PAGE") {
        int page = anc.mid(4).toInt();
        bookTree->setCurrentItem(NULL);
        selectMenuPage(page);
    }
}

void MenuPage::getTopMenu(EBook *eb, int page, int offset)
{
    QString t = QString();
    QStringList list = eb->candidate(page, offset, &t);

    foreach(QString s, list) {
        QStringList cand = s.split("&|");

        topTitles << cand[0];
        EB_Position p;
        p.page = cand[1].toInt();
        p.offset = cand[2].toInt();
        topMenus << p;
    }
}

void MenuPage::getMenus(EBook *eb, int page, int offset, PageItems *items,
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
    QStringList list = eb->candidate(page, offset, &c_text);
    if (list.count()) {
        foreach(QString s, list) {
            QStringList cand = s.split("&|");
            menuCount++;
            if (checkMax && menuCount >= CONF->limitMenuHit) {
                retStatus = LIMIT_MENU;
                break;
            }
            QString anchor = toAnchor("H", menuCount);
            //qDebug() << anchor;
            int next_page = cand[1].toInt();
            int next_offset = cand[2].toInt();
            items->composeHLine(count, anchor, cand[0], QString());
            getMenus(eb, next_page, next_offset, items, count);
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

WholePage::WholePage(QWidget *parent, const SearchMethod &method)
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

RET_SEARCH WholePage::readPage(int page)
{
    curPage = page;

    RET_SEARCH ret = NORMAL;

    PageItems item(CONF->dictSheet);

    int seq_hits = seqHits.count();
    if (seq_hits == 1) {
        QString anchor = toAnchor("BOOK", 0);
        item.composeHLine(1, anchor, method_.bookReader->name(), QString());
    } else {
        QString str =  method_.bookReader->name() + '(' +
                QString::number(seqHits.count()) + ')';
	item.setTopItem("TOP", str);
    }
    QTreeWidgetItem *top_tree = item.curItem();

    EBook eb;
    if(eb.setBook(method_.bookReader->path(), method_.bookReader->bookNo()) <
       0) {
        return NO_BOOK;
    }
    eb.initSearch(bookBrowser->fontSize(), method_.bookReader->fontList(),
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
                QString anchor = toAnchor("HIT", i);
                QString name = method_.bookReader->name() + '(' +
                               QString::number(i * limit + 1) + '-' +
                               QString::number(i * limit + hit_num) + ')';
                item.composeHLine(1, anchor, name, QString());
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
                QString anchor = toAnchor("H", j);
                item.composeHLine(2, anchor, head_l, head_v, text_v);
                if (item.textLength() > CONF->limitBrowserChar ||
                    eb.imageCount() > CONF->limitImageNum) {
                    QString anchor = toAnchor("CUT", 1);
                    item.composeError(anchor, CutString);
                    ret = (eb.imageCount() > CONF->limitImageNum) ? 
                        LIMIT_IMAGE : LIMIT_CHAR;
                    break;
                }
            }
        } else {
            QString t = eb.text(seqHits[i].page,
                                seqHits[i].offset, false);
            QString head_i = t.left(t.indexOf('\n'));
            QString anchor = toAnchor("PAGE", i);
            QString name = "(" + QString::number(i * limit + 1) + "-) " +
                           head_i;
            item.addHItem(1, anchor, name);
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
    bookTree->expandItem(top_tree);
    for (int i = 0; i < top_tree->childCount(); i++) {
        top_tree->child(i)->setExpanded(true);
    }

    bookTree->setCurrentItem(current_item);

    bookBrowser->setBrowser(item.text());
    return ret;
}

void WholePage::changePage(QTreeWidgetItem *item, int)
{
    QString anc = item->text(1);

    if (anc.left(4) == "PAGE") {
        int page = anc.mid(4).toInt();
        readPage(page);
    }
}

RET_SEARCH WholePage::initSeqHits()
{
    EBook eb;

    if (eb.setBook(method_.bookReader->path(), method_.bookReader->bookNo()) <
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
    items.setTopItem("TOP", "tmp");

    EBook eb;
    int book_count = 0;
    int req_cnt = method.limitBook;
    if (search_list.count())
        req_cnt = -1;

    bool break_flag = false;

    foreach(Book *book, method.group->bookList()) {
        if (book->checkState() != Qt::Checked) continue;

        emit statusRequested(book->name() + ":(" +
                             QString::number(totalCount) + ")");
        if (checkStop() || break_flag) break;

        if ( eb.setBook(book->path(), book->bookNo(), book_count) < 0) continue;

        eb.initSearch(bookBrowser->fontSize(), book->fontList(),
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

	QTreeWidgetItem *book_item = 0;
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
                QString anchor_book = toAnchor("BOOK", book_count);
                items.composeHLine(1, anchor_book, book->name(), QString());
		book_item = items.curItem();
            }
            if (CONF->highlightMatch) {
                foreach(QString s, slist) {
                    head_v = emphasize(head_v, s);
                    text_v = emphasize(text_v, s);
                }
            }
            QString anchor = toAnchor("H", totalCount);
            items.composeHLine(2, anchor, head_i, head_v, text_v);

            RET_SEARCH chk = checkLimit(eb.imageCount(), items.textLength());
            if (chk != NORMAL) {
                //QString anchor = toAnchor("CUT", totalCount);
                items.composeError(anchor, CutString);
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
	book_item->setText(0, book->name() + " (" +
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

    bookTree->insertTopLevelItems(0, items.items());
    QString top_title = toLogicString(slist, method);
    QTreeWidgetItem *top_item = items.topItem();
    top_item->setText(0, QString("%1(%2)").arg(top_title).arg(totalCount));
    bookTree->expandItem(top_item);
    if (totalCount <= 100 || top_item->childCount() == 1) {
        for (int i = 0; i < top_item->childCount(); i++) {
            top_item->child(i)->setExpanded(true);
        }
    }
    bookTree->setCurrentItem(top_item);
    emit statusRequested(QString("Browser (%1 character)")
                         .arg(items.textLength()));
    checkStop();

    bookBrowser->setBrowser(items.text());

    return;
}

SearchWholePage::SearchWholePage(QWidget *parent, const QStringList &slist, 
                               const SearchMethod &method)
    : PageWidget(parent, method)
{
    EBook eb;

    int search_total = 0;
    int book_count = 0;

    PageItems item(CONF->dictSheet);
    item.setTopItem("TOP", "tmp");

    bool break_flag = false;
    RET_SEARCH break_check = NORMAL;

    foreach(Book * book, method.group->bookList()) {
        if (checkStop() || break_flag) break;

        if (book->checkState() != Qt::Checked) continue;

        matchCount = 0;

        if (eb.setBook(book->path(), book->bookNo(), book_count) < 0) {
            qDebug() << "Can't open the book" << book->path() << book->bookNo();
            continue;
        }
        if (!eb.isHaveText()) {
            eb.unsetBook();
            continue;
        }
        eb.initSearch(bookBrowser->fontSize(), book->fontList(),
                      CONF->indentOffset, method.ruby);
        bookBrowser->addBookList(book);
        book_count++;

        QTreeWidgetItem *book_tree = 0;

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
                    QString anchor = toAnchor("BOOK", book_count);

                    item.composeHLine(1, anchor, book->name(), QString());
		    book_tree =item.curItem();
                }
                if (CONF->highlightMatch) {
                    foreach(QString s, slist) {
                        head_v = emphasize(head_v, s);
                        text_v = emphasize(text_v, s);
                    }
                }
                QString anchor = toAnchor("H", totalCount);
                item.composeHLine(2, anchor, head_i, head_v, text_v);
                break_check = checkLimit(eb.imageCount(), item.textLength());
                if (break_check != NORMAL) {
                    QString anchor = toAnchor("CUT", totalCount);
                    item.composeError(anchor, CutString);
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

        book_tree->setText(0, QString("%1(%2)")
                           .arg(book_tree->text(0)).arg(matchCount));
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
    bookTree->expandItem(top_tree);
    if (totalCount <= 100 || top_tree->childCount() == 1) {
        for (int i = 0; i < top_tree->childCount(); i++) {
            top_tree->child(i)->setExpanded(true);
        }
    }

    bookTree->setCurrentItem(top_tree);

    checkStop();
    emit statusRequested(QString("Browser (%1 characters)")
                         .arg(item.textLength()));
    checkStop();

    bookBrowser->setBrowser(item.text());

    return;
}

BookView::BookView(QWidget *parent)
    : QTabWidget(parent)
{
    mainWin = (MainWindow *)parent;
    QToolButton *close_button = new QToolButton(this);
    setCornerWidget(close_button, Qt::TopRightCorner);
    close_button->setCursor(Qt::ArrowCursor);
    close_button->setAutoRaise(true);
    close_button->setIcon(QIcon(":images/closetab.png"));
    close_button->setToolTip(tr("Close page"));
    close_button->setEnabled(true);
    connect(close_button, SIGNAL(clicked()), this, SLOT(closeTab()));
    connect(this, SIGNAL(tabChanged(int)),
            mainWin, SLOT(changeViewTabCount(int)));
}


RET_SEARCH BookView::newPage(const QStringList &list,
                             const SearchMethod &method, bool newTab)
{
    stopFlag = false;

    PageWidget *page;

    if (method.direction == FullTextSearch)
        page = new SearchWholePage(this, list, method);
    else if (method.direction == WholeRead)
        page = new WholePage(this, method);
    else if (method.direction == MenuRead)
        page = new MenuPage(this, method);
    else if (method.direction == BookInfo)
        page = new InfoPage(this, method);
    else
        // ExactWordSearch, ForwardSearch, BackwardSearch,
        // KeywordSearch, CrossSearch,
        page = new SearchPage(this, list, method);

    RET_SEARCH ret = page->retStatus;
    if (ret == NOT_HIT || ret == NOT_HIT_INTERRUPTED || ret == NO_MENU ||
        ret == NO_BOOK) {
        delete page;
        return ret;
    }

    QString tab_title = toLogicString(list, method, false);
    if (newTab || count() == 0) {
        addTab(page, tab_title);
    } else {
        int index = currentIndex();
        closeTab();
        insertTab(index, page, tab_title);
    }
    setCurrentWidget(page);

    connect(this, SIGNAL(fontChanged(QFont)), page, SLOT(changeFont(QFont)));

    emit tabChanged(count());

    return ret;
}


void BookView::closeTab()
{
    int idx = currentIndex();
    QWidget *wgt = currentWidget();

    removeTab(idx);
    wgt->close();
    delete wgt;

    emit tabChanged(count());
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


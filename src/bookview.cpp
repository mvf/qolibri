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

#include "book.h"
#include "bookview.h"
#include "ebook.h"
#include "configure.h"
#include "textcodec.h"

#include <QContextMenuEvent>
#include <QDesktopWidget>
#include <QDir>
#include <QHeaderView>
#include <QMenu>
#include <QMouseEvent>
#include <QScrollBar>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QAudioDeviceInfo>
#else
#include <QSound>
#endif

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

static QString toAnchor(const QString &str, int num)
{
    return str + QString::number(num);
}

BookBrowser::BookBrowser(QWidget *parent)
    : QTextBrowser(parent)
{
    setSearchPaths(QStringList() << EbCache::cachePath);
    document()->setDefaultFont(CONF->browserFont);

    connect(this, SIGNAL(selectionChanged()), SLOT(changeTextSelection()));
}

void BookBrowser::setSource(const QUrl &name)
{

    QStringList args = name.toString().split('?');

    if (args[0] == "sound") {
        // args[1] : wave file
        if (!CONF->waveProcess.isEmpty()) {
            emit processRequested(CONF->waveProcess + ' ' + args[1]);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
	} else if (!QAudioDeviceInfo::availableDevices(QAudio::AudioOutput).isEmpty()) {
#else
        } else if (QSound::isAvailable()) {
#endif
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
                emit statusRequested("ERROR: Invalid book index: " + args[1]);
                return;
            }
            bool mflag = (args[0] == "menu") ? true : false;
            EB_Position pos;
            pos.page = args[2].toInt();
            pos.offset = args[3].toInt();

            ReferencePopup *popup =
                new ReferencePopup(bookList_[index], pos, this, mflag);
            connect(popup->bookBrowser(), SIGNAL(statusRequested(QString)),
                    SIGNAL(statusRequested(QString)));
            connect(popup->bookBrowser(),
                    SIGNAL(searchRequested(SearchDirection,QString)),
                    SIGNAL(searchRequested(SearchDirection,QString)));
            connect(popup->bookBrowser(), SIGNAL(pasteRequested(QString)),
                    SIGNAL(pasteRequested(QString)));
            connect(popup->bookBrowser(), SIGNAL(processRequested(QString)),
                    SIGNAL(processRequested(QString)));
            connect(popup->bookBrowser(), SIGNAL(soundRequested(QString)),
                    SIGNAL(soundRequested(QString)));
            connect(popup->bookBrowser(), SIGNAL(selectionRequested(QString)),
                    SIGNAL(selectionRequested(QString)));
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

    //if (textCursor().hasSelection() &&
    //    parent()->parent()->objectName() == "main_browser") {
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

//#ifdef Q_WS_X11
// Implement for linux.
// For "selectionChanged" SIGNAL not allways invoked at mouse move and
// release timing.
void BookBrowser::mouseReleaseEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton)
        changeTextSelection();

    QTextEdit::mouseReleaseEvent(ev);
}
//#endif

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
    bookBrowser_ = new BookBrowserPopup(parent);
#else
    bookBrowser_ = new BookBrowser(parent);
#endif
#ifdef FIXED_POPUP
    QToolButton *close_button = new QToolButton(this);
    close_button->setIcon(QIcon(":images/closetab.png"));
    bookBrowser_->setCornerWidget(close_button, Qt::TopRightCorner);
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

void ReferencePopup::showEvent(QShowEvent*)
{
#ifdef FIXED_POPUP
    move(parent->mapToGlobal(QPoint(0, 0)));
    resize(parent->size() - QSize(0, 0));
#else

    QSize sz = bookBrowser_->size();
    QScrollBar *hb = bookBrowser_->horizontalScrollBar();
    if ( hb && hb->maximum() > 0) {
        sz.setWidth(hb->maximum() - hb->minimum() + hb->pageStep() + 10);
        resize(sz);
    }
    QScrollBar *vb = bookBrowser_->verticalScrollBar();
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

    connect(sbar, SIGNAL(valueChanged(int)), SLOT(changeExpand(int)));
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
            "pre { font-family: " + CONF->browserFont.family() + "; }\n"
            "</style>\n"
            "</head>\n"
            "<body>\n";
    textLength_ = text_.length();
}

QTreeWidgetItem* PageItems::composeHLine(int num, const QString &anchor,
                             const QString &title_l, const QString &title_t,
                             const QString &text)
{
    QString snum = QString::number(num);
    QString str;
    if (!text.isEmpty())
        str =  "<a name=" + anchor + " />"
               "<h" + snum + ">&nbsp;" + title_t +
               "</h" + snum + "><pre>" + text + "</pre>";
    else
        str =  "<a name=" + anchor + " />"
               "<h" + snum + ">&nbsp;" + title_t + "</h" + snum + ">\n";

    textLength_ += str.length();
    text_ += str;

    return addHItem(num, anchor, title_l);
}

QTreeWidgetItem* PageItems::composeHLine(int num, const QString &anchor,
                                 const QString &title, const QString &text)
{
    QString snum = QString::number(num);
    QString str;
    if (!text.isEmpty())
        str =  "<a name=" + anchor + " />"
               "<h" + snum + ">&nbsp;" + title +
               "</h" + snum + "><pre>" + text + "</pre>";
    else
        str =  "<a name=" + anchor + " />"
               "<h" + snum + ">&nbsp;" + title + "</h" + snum + ">\n";

    textLength_ += str.length();
    text_ += str;

    return addHItem(num, anchor, title);
}

void PageItems::composeError(const QString &anchor,
                                 const QString &text)
{
    QString str = "<p><a name=" + anchor + " /><em class=err>" + text +
	          "</em></p>";
    textLength_ += str.length();
    text_ += str;
    QTreeWidgetItem *i = new QTreeWidgetItem(itemP_[0],
	                        QStringList() << text << anchor);
    i->setForeground(0, QColor(0x886666));
}

QTreeWidgetItem* PageItems::addHItem(int num, const QString &anchor, const QString &title)
{

    QTreeWidgetItem *parent = (num > 0) ? itemP_[num-1] : 0;
    curItem_ = new QTreeWidgetItem(parent, QStringList() << title << anchor);

    if (num == 0)
        topItems_ << curItem_;
    itemP_[num] = curItem_;
    return curItem_;
}

void PageItems::composeTrail()
{
    textLength_ += 18;
    text_ += "\n</body></html>\n";
}

void PageItems::expand(int level)
{
    foreach(QTreeWidgetItem *ip, topItems_) {
        ip->setExpanded(true);
        if (level >= 1) {
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

    bookBrowser_ = new BookBrowser(this);
    bookBrowser_->setObjectName("main_browser");

    addWidget(bookTree);
    addWidget(bookBrowser_);
    setStretchFactor(indexOf(bookBrowser_), 1);

    connect(this, SIGNAL(statusRequested(QString)),
            mainWin, SLOT(showStatus(QString)));
    connect(bookTree,
            SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            SLOT(scrollTo(QTreeWidgetItem*,QTreeWidgetItem*)));
    connect(bookTree,
            SIGNAL(itemPressed(QTreeWidgetItem*,int)),
            SLOT(scrollTo(QTreeWidgetItem*,int)));
    connect(bookTree, SIGNAL(customContextMenuRequested(QPoint)),
            SLOT(popupSlide(QPoint)));
}

void PageWidget::scrollTo(QTreeWidgetItem *to)
{
    if (to && to->text(1).at(0) != 'P' ) {
        bookBrowser_->scrollToAnchor(to->text(1));
        emit selectionRequested(to->text(0));
    }
}

void PageWidget::changeFont(const QFont &font)
{
    bookBrowser_->document()->setDefaultFont(font);
    bookBrowser_->setFont(font);
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

static RET_SEARCH checkLimit(const SearchMethod &method, int totalCount, int matchCount, int text_length)
{

    if (totalCount >= method.limitTotal)
        return LIMIT_TOTAL;
    if (matchCount >= method.limitBook)
        return LIMIT_BOOK;
    if (text_length >= CONF->limitBrowserChar)
        return LIMIT_CHAR;

    return NORMAL;
}

void PageWidget::zoomIn()
{
    QFont font = bookBrowser_->currentFont();
    int fsz = font.pointSize();
    font.setPointSize(fsz + 1);

    bookBrowser_->document()->setDefaultFont(font);
    bookBrowser_->setFont(font);
}

void PageWidget::zoomOut()
{
    QFont font = bookBrowser_->currentFont();
    int fsz = font.pointSize();
    font.setPointSize(fsz - 1);

    bookBrowser_->document()->setDefaultFont(font);
    bookBrowser_->setFont(font);
}

InfoPage::InfoPage(QWidget *parent, const SearchMethod &method)
    : PageWidget(parent, method)
{
}

RET_SEARCH InfoPage::search(const Query& query)
{
    Book *book = query.method.book;

    PageItems items(CONF->dictSheet);

    items.composeHLine(1, "TOP", book->name());

    bookBrowser_->addBookList(book);
    EBook eb;
    if (eb.initBook(book->path(), book->bookNo()) < 0) {
        return NO_BOOK;
    }
    eb.initHook(bookBrowser_->fontSize(), book->fontList(),
                  CONF->indentOffset,  query.method.ruby);

    QString mstr;

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

    QString astr;
    EB_Subbook_Code code = eb.subAppendixList[book->bookNo()];
    if(eb.isHaveAppendixSubbook(code)) {
        eb.setAppendixSubbook(code);
        astr += eb.appendixPath() + "/" + eb.appendixSubbookDirectory();
        int s1, s2;
        eb.stopCode(&s1, &s2);
        astr += "\nStop Code=0x" + QString::number(s1,16) + " 0x" +
            QString::number(s2,16);
        if (eb.isHaveNarrowAlt()) {
            int st = eb.narrowAltStart();
            int en = eb.narrowAltEnd();
            astr += "\nAlternate Narrow Font=" + QString::number(st) +
                    " - " + QString::number(en);
        }
        if (eb.isHaveWideAlt()) {
            int st = eb.wideAltStart();
            int en = eb.wideAltEnd();
            astr += "\nAlternate Wide Font=" + QString::number(st) +
                    " - " + QString::number(en);
        }
    } else {
        astr += QObject::tr("No Appendix");
    }

    QString str = "<table><tr><td>Title </td><td> " + eb.subbookTitle() +
                  "</td></tr>\n";
    str += "<tr><td>Search_Method </td><td> " + mstr + "</td></tr>\n";
    str += "<tr><td>Appendix </td><td> " + astr + "</td></tr></table><br>";
    items.composeHLine(2, "BOOK", eb.path(), str);

    if (eb.isHaveMenu()) {
        items.composeHLine(2, "MENU", "Menu", eb.getMenu());
    }

    if (eb.isHaveCopyright()) {
        items.composeHLine(2, "COPYRIGHT", "Copyright", eb.getCopyright());
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
        if (!fname.rightRef(4).compare(QStringLiteral(".htm"), Qt::CaseInsensitive) ||
            !fname.rightRef(5).compare(QStringLiteral(".html"), Qt::CaseInsensitive)) {
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
    bookBrowser_->setBrowser(items.text());
    bookBrowser_->setSearchPaths(QStringList() << book->path() <<
                                EbCache::cachePath);
    bookTree->insertTopLevelItems(0, items.topItems());
    bookTree->setCurrentItem(bookTree->topLevelItem(0));

    return NORMAL;
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
    bookBrowser_->addBookList(method.bookReader);
    connect(bookTree, SIGNAL(itemActivated(QTreeWidgetItem *, int)),
            SLOT(changePage(QTreeWidgetItem *, int)));
}

RET_SEARCH MenuPage::search(const Query&)
{
    RET_SEARCH retStatus = fullMenuPage();

    if (retStatus == LIMIT_MENU)
        selectMenuPage(-1);
    return retStatus;
}


RET_SEARCH MenuPage::fullMenuPage()
{
    checkMax = true;
    EbMenu eb;
    if (eb.initBook(method_.bookReader->path(), method_.bookReader->bookNo()) <
        0) {
        return NO_BOOK;
    }
    eb.initHook(bookBrowser_->fontSize(), method_.bookReader->fontList(),
                CONF->indentOffset, method_.ruby);

    EB_Position pos = eb.menu();
    if (!eb.isValidPosition(pos)){
        return NO_MENU;
    }

    PageItems items(CONF->bookSheet);

    items.addHItem(0, toAnchor("B", 0), method_.bookReader->name());
    menuCount = 0;

    RET_SEARCH retStatus = getMenus(&eb, pos, &items, 0);
    if (retStatus == LIMIT_MENU) {
        //while (!item.item().isEmpty())
        //    delete item.time().takeLast();
        return retStatus;
    }

    items.composeTrail();
    bookTree->insertTopLevelItems(0, items.topItems());
    items.expand(2);

    bookTree->setCurrentItem(bookTree->topLevelItem(0));

    bookBrowser_->setBrowser(items.text());

    return retStatus;
}

void MenuPage::selectMenuPage(int index)
{
    checkMax = false;
    EbMenu eb;
    eb.initBook(method_.bookReader->path(), method_.bookReader->bookNo());
    eb.initHook(bookBrowser_->fontSize(), method_.bookReader->fontList(),
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
	    items.curItem()->setForeground(0, QColor(0x666688));
        }
    }

    items.composeTrail();
    bookTree->clear();
    bookTree->insertTopLevelItems(0, items.topItems());
    items.expand(1);

    if (next) {
        bookTree->scrollToItem(next);
        bookTree->setCurrentItem(next);
    }

    bookBrowser_->setBrowser(items.text());
    emit statusRequested(QString("%1").arg(menuCount));
}

void MenuPage::changePage(QTreeWidgetItem *item, int)
{
    QString anc = item->text(1);

    if (anc.at(0) == 'P') {
        int page = anc.midRef(1).toInt();
        bookTree->setCurrentItem(NULL);
        selectMenuPage(page);
    }
}

RET_SEARCH MenuPage::getMenus(EbMenu *eb, const EB_Position &pos, PageItems *items,
                              int count)
{
    RET_SEARCH retStatus = NORMAL;
    count++;
    if ((menuCount % 100) == 0) {
        emit statusRequested(QString("%1").arg(menuCount));
        if (checkStop()) {
            return INTERRUPTED;
        }
    }
    QString c_text;
    QList <CandItem> list = eb->candidate(pos, &c_text);
    if (list.count()) {
        foreach(const CandItem &i, list) {
            menuCount++;
            if (checkMax && menuCount >= CONF->limitMenuHit) {
                retStatus = LIMIT_MENU;
                break;
            }
            EB_Position next = i.position;
            items->composeHLine(count, toAnchor("H", menuCount), i.title);
            retStatus = getMenus(eb, next, items, count);
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
    return retStatus;
}

AllPage::AllPage(QWidget *parent, const SearchMethod &method)
    : PageWidget(parent, method)
{
    connect(bookTree, SIGNAL(itemActivated(QTreeWidgetItem *, int)),
            SLOT(changePage(QTreeWidgetItem *, int)));
}

RET_SEARCH AllPage::search(const Query&)
{
    RET_SEARCH retStatus = initSeqHits();
    if (retStatus != NORMAL)
        return retStatus;

    readPage(0);
    return retStatus;
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
    eb.initHook(bookBrowser_->fontSize(), method_.bookReader->fontList(),
                  CONF->indentOffset, method_.ruby);
    bookBrowser_->addBookList(method_.bookReader);

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
                eb.getText(j, &head_l, &head_v, &text_v);
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
    bookTree->insertTopLevelItems(0, item.topItems());
    item.expand(1);

    bookTree->setCurrentItem(current_item);

    bookBrowser_->setBrowser(item.text());
    return ret;
}

void AllPage::changePage(QTreeWidgetItem *item, int)
{
    QString anc = item->text(1);

    if (anc.at(0) == 'P') {
        int page = anc.midRef(1).toInt();
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

SearchPageBuilder::SearchPageBuilder(BookBrowser *browser)
    : bookIndex(0)
    , itemIndex(0)
    , items(CONF->dictSheet)
    , bookBrowser_(browser)
{
}

RET_SEARCH SearchPageBuilder::search(const Query& query)
{
    QStringList queries;
    queries << query.query;
    switch (query.method.direction) {
    case ExactWordSearch:
    case ForwardSearch:
        queries << stemWords(query.query);
        break;
    default:
        break;
    }
    RET_SEARCH ret = NOT_HIT;
    foreach (const QString &q, queries) {
        ret = search1(Query(q, query.method));
    }
    return ret;
}

RET_SEARCH SearchPageBuilder::search1(const Query& query)
{
    RET_SEARCH retStatus = NORMAL;

    items.addHItem(0, "TOP", "tmp");

    EBook eb;
    int req_cnt = query.method.limitBook;
    bool break_flag = false;

    SearchType type = SearchExactWord;
    switch (query.method.direction) {
    case KeywordSearch:   type = SearchKeyWord; break;
    case CrossSearch:     type = SearchCrossWord; break;
    case ExactWordSearch: type = SearchExactWord; break;
    case ForwardSearch:   type = SearchWord; break;
    case BackwardSearch:  type = SearchEndWord; break;
    default:
        Q_ASSERT(0);
        qWarning() << "Invalid Search Method" << query.method.direction;
    }

    int totalMatchCount = 0;

    foreach(Book *book, query.method.group->bookList()) {
        if (book->bookType() != BookLocal) continue;
        if (book->checkState() != Qt::Checked) continue;

        emit statusRequested(book->name() + ":(" +
                             QString::number(itemIndex) + ")");
        if (checkStop() || break_flag) break;

        if ( eb.initBook(book->path(), book->bookNo(), bookIndex) < 0) continue;

        eb.initHook(bookBrowser_->fontSize(), book->fontList(),
                      CONF->indentOffset, query.method.ruby);

        int hit_num = 0;
        hit_num = eb.searchQuery(req_cnt, query.query, type);
        if (hit_num <= 0) {
            eb.unsetSubbook();
            continue;
        }

        int matchCount = 0;
        for (int i = 0; i < hit_num; i++) {
            if (checkStop()) break;

            QString head_i;
            QString head_v;
            QString text_v;
            eb.getMatch(i, &head_i, &head_v, &text_v, CONF->highlightMatch);
            itemIndex++;
            matchCount++;

            if (matchCount == 1) {
                bookBrowser_->addBookList(book);
                bookIndex++;
                items.composeHLine(1, toAnchor("B", bookIndex), book->name());
            }
            items.composeHLine(2, toAnchor("H", itemIndex), head_i, head_v, text_v);

            RET_SEARCH chk = checkLimit(query.method, itemIndex, matchCount, items.textLength());
            if (chk != NORMAL) {
                items.composeError(toAnchor("CUT", itemIndex), CutString);
                if (chk != LIMIT_BOOK) {
                    break_flag = true;
                    retStatus = chk;
                }
                break;
            }
        }
        if (matchCount == 0) {
            eb.unsetSubbook();
            continue;
        }
	items.item(1)->setText(0, book->name() + " (" +
                                  QString::number(matchCount)  + ')');

        eb.unsetSubbook();
        totalMatchCount += matchCount;
    }
    if (totalMatchCount == 0) {
        retStatus = (checkStop()) ? NOT_HIT_INTERRUPTED : NOT_HIT;
    }
    else if (checkStop()) {
        items.composeError("LAST", IntString);
        retStatus = INTERRUPTED;
    }
    items.composeTrail();

    emit statusRequested(QString("List (%1 items)").arg(itemIndex));
    checkStop();

    QTreeWidgetItem *top_item = items.item(0);
    QString top_title = query.toLogicString();
    top_item->setText(0, QString("%1(%2)").arg(top_title).arg(totalMatchCount));

    return retStatus;
}

QList <QTreeWidgetItem*> SearchPageBuilder::treeItems()
{
    return items.topItems();
}

QString SearchPageBuilder::text()
{
    return items.text();
}

int SearchPageBuilder::textLength()
{
    return items.textLength();
}

void SearchPageBuilder::expand()
{
    if (itemIndex <= 100 || items.topItems().count() == 1) {
        items.expand(1);
    } else {
        items.expand(0);
    }
}

SearchPage::SearchPage(QWidget *parent, const SearchMethod &method)
    : PageWidget(parent, method)
{
}

RET_SEARCH SearchPage::search(const Query& query)
{
    SearchPageBuilder builder(bookBrowser_);
    connect(&builder, SIGNAL(statusRequested(const QString&)), SIGNAL(statusRequested(const QString&)));
    RET_SEARCH retStatus = builder.search(query);

    bookTree->insertTopLevelItems(0, builder.treeItems());
    bookTree->setCurrentItem(bookTree->topLevelItem(0));
    builder.expand();
    emit statusRequested(QString("Browser (%1 character)").arg(builder.textLength()));

    bookBrowser_->setBrowser(builder.text());
    //qDebug() << items.text();

    return retStatus;
}

SearchWholePage::SearchWholePage(QWidget *parent, const SearchMethod &method)
    : PageWidget(parent, method)
{
}

RET_SEARCH SearchWholePage::search(const Query& query)
{
    RET_SEARCH retStatus = NORMAL;
    EbAll eb;

    int search_total = 0;
    int book_count = 0;
    int totalCount = 0;
    int matchCount = 0;

    PageItems item(CONF->dictSheet);
    item.addHItem(0, "TOP", "tmp");

    bool break_flag = false;
    RET_SEARCH break_check = NORMAL;

    foreach(Book * book, query.method.group->bookList()) {
        if (book->bookType() != BookLocal) continue;
        if (checkStop() || break_flag) break;

        if (book->checkState() != Qt::Checked) continue;

        matchCount = 0;

        if (eb.initBook(book->path(), book->bookNo(), book_count) < 0) {
            qDebug() << "Can't open the book" << book->path() << book->bookNo();
            continue;
        }
        eb.initSeek();
        if (!eb.isHaveText()) {
            eb.unsetSubbook();
            continue;
        }
        eb.initHook(bookBrowser_->fontSize(), book->fontList(),
                      CONF->indentOffset, query.method.ruby);
        bookBrowser_->addBookList(book);
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
                eb.getMatch(i, &head_i, &head_v, &text_v, CONF->highlightMatch);
                totalCount++;
                matchCount++;

                if (matchCount == 1) {

                    item.composeHLine(1, toAnchor("B", book_count),
                                      book->name());
                }
                item.composeHLine(2, toAnchor("H", totalCount), head_i,
                                  head_v, text_v);
                break_check = checkLimit(query.method, totalCount, matchCount, item.textLength());
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
        eb.unsetSubbook();
        if (matchCount == 0) continue;

	item.item(1)->setText(0, QString("%1(%2)")
                                 .arg(book->name()).arg(matchCount));
    }
    if (totalCount == 0) {
        delete item.topItems().at(0);
        retStatus = (checkStop()) ? NOT_HIT_INTERRUPTED : NOT_HIT;
        return retStatus;
    }
    if (checkStop()) {
        item.composeError("LAST", IntString);
        retStatus = INTERRUPTED;
    }
    item.composeTrail();

    emit statusRequested(QString("List (%1 items)").arg(totalCount));
    checkStop();

    bookTree->insertTopLevelItems(0, item.topItems());
    QString top_title = query.toLogicString();
    QTreeWidgetItem *top_tree = item.item(0);
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

    bookBrowser_->setBrowser(item.text());

    return retStatus;
}

WebPage::WebPage(QWidget *parent, const QString &url, const Query& query)
    : QWebView(parent), method_(query.method)
{
    loading_ = true;
    setObjectName("webpage");
    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    tabBar_ = 0;
    connect(this, SIGNAL(loadStarted()), SLOT(progressStart()));
    connect(this, SIGNAL(loadFinished(bool)), SLOT(progressFinished(bool)));
    connect(this, SIGNAL(loadProgress(int)), SLOT(progress(int)));
    connect(this, SIGNAL(linkClicked(const QUrl&)),
            SLOT(openLink(const QUrl&)));
    //connect(this, SIGNAL(linkRequested(QString)), SLOT(execProcess(QString)));

    QByteArray enc = encString(url);
    QString ustr = setSearchString(url, enc, query.query);
    QString sdir = directionString(url);
    if (!sdir.isEmpty()) {
        ustr = setDirectionString(ustr, sdir, method_.direction);
    }

    QWebSettings *ws = settings();
    ws->setAttribute(QWebSettings::JavascriptEnabled, true);
    ws->setAttribute(QWebSettings::JavaEnabled, true);
    ws->setAttribute(QWebSettings::PluginsEnabled, true);
    if (enc.isEmpty()) {
        load(QUrl::fromEncoded(ustr.toLatin1()));
    } else {
        load(QUrl::fromEncoded(QTextCodec::codecForName(enc)->fromUnicode(ustr)));
    }
    QAction *newWinAct = pageAction(QWebPage::OpenLinkInNewWindow);
    connect(newWinAct, SIGNAL(triggered()), SLOT(openNewWin()));
    connect(page(), SIGNAL(linkHovered(QString,QString,QString)),
            SLOT(copyHoveredLink(QString,QString,QString)));

    show();
}

WebPage::WebPage(QWidget *parent, const QString &url)
    : QWebView(parent)
{
    loading_ = true;
    setObjectName("webpage");
    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    tabBar_ = 0;
    connect(this, SIGNAL(loadStarted()), SLOT(progressStart()));
    connect(this, SIGNAL(loadFinished(bool)), SLOT(progressFinished(bool)));
    connect(this, SIGNAL(loadProgress(int)), SLOT(progress(int)));
    connect(this, SIGNAL(linkClicked(const QUrl&)),
            SLOT(openLink(const QUrl&)));
    QAction *newWinAct = pageAction(QWebPage::OpenLinkInNewWindow);
    connect(newWinAct, SIGNAL(triggered()), SLOT(openNewWin()));
    connect(page(), SIGNAL(linkHovered(QString,QString,QString)),
            SLOT(copyHoveredLink(QString,QString,QString)));

    //qDebug() << url;

    QWebSettings *ws = settings();
    ws->setAttribute(QWebSettings::JavascriptEnabled, true);
    ws->setAttribute(QWebSettings::JavaEnabled, true);
    ws->setAttribute(QWebSettings::PluginsEnabled, true);

    load(url);

    show();
}
void WebPage::openNewWin()
{
    emit linkRequested(CONF->browserProcess + ' ' + hoveredLink);
}

void WebPage::copyHoveredLink(const QString &link, const QString&,
                              const QString&)
{
    if (!link.isEmpty()) {
        hoveredLink = link;
    }
}

void WebPage::contextMenuEvent(QContextMenuEvent* event)
{
    QWebView::contextMenuEvent(event);
    //QMenu *menu = createStandardContextMenu();
    //QAction *a = menu->exec(event->globalPos());
    //delete menu;

}

QByteArray WebPage::encString(const QString &url)
{
    if (!url.contains(QRegExp("\\{.*\\}"))) {
        return QByteArray();
    } else {
        QString w1 = url.mid(url.indexOf('{')+1);
        QString w2 = w1.left(w1.indexOf('}'));
        return w2.trimmed().toLatin1();
    }
}

QString WebPage::setSearchString(const QString &url, const QByteArray &enc,
                                 const QString &query)
{
    QByteArray bstr;
    if (enc.isEmpty()) {
        bstr = query.toUtf8();
    } else {
        bstr = QTextCodec::codecForName(enc)->fromUnicode(query);
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
        return w2.trimmed();
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
    loading_ = true;
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
    loading_ = false;
    if (tabBar_) tabBar_->setTabIcon(tabIndex_, QIcon(":/images/web1.png"));
}

void WebPage::openLink(const QUrl &url)
{
    if (!popupBrowser_) {
        QUrl u = QUrl::fromEncoded(url.toEncoded(), QUrl::TolerantMode);
        qDebug() << url.toEncoded();
        qDebug() << u.toString();
        load(u);
    } else {
        emit linkRequested(CONF->browserProcess + ' ' + url.toString());
        //emit linkRequested(CONF->browserProcess + ' ' +
        //                   QString::fromAscii(url.toEncoded()));
    }

}

void WebPage::changeFont(const QFont &font)
{

    qDebug() << "WebPage::changeFont" << font.family();
    settings()->setFontFamily(QWebSettings::StandardFont, font.family());

}

void WebPage::zoomIn()
{
    QWebSettings *s = settings();
    int dsz = s->fontSize(QWebSettings::DefaultFontSize);
    int fsz = s->fontSize(QWebSettings::DefaultFixedFontSize);
    s->setFontSize(QWebSettings::DefaultFontSize, dsz + 1);
    s->setFontSize(QWebSettings::DefaultFixedFontSize, fsz + 1);
    reload();
}

void WebPage::zoomOut()
{
    QWebSettings *s = settings();
    int dsz = s->fontSize(QWebSettings::DefaultFontSize);
    int fsz = s->fontSize(QWebSettings::DefaultFixedFontSize);
    s->setFontSize(QWebSettings::DefaultFontSize, dsz - 1);
    s->setFontSize(QWebSettings::DefaultFixedFontSize, fsz - 1);
    reload();
}

void WebPage::setPopupBrowser(bool popup)
{
    popupBrowser_ = popup;
}

BookView::BookView(QWidget *parent)
    : QTabWidget(parent)
{
    mainWin = parent;
    setObjectName("bookview");
    setUsesScrollButtons(true);
    QToolButton *close_button = new QToolButton(this);
    setCornerWidget(close_button, Qt::TopRightCorner);
    //QToolButton *close_all_button = new QToolButton(this);
    //setCornerWidget(close_all_button, Qt::TopLeftCorner);
    close_button->setCursor(Qt::ArrowCursor);
    close_button->setAutoRaise(true);
    close_button->setIcon(QIcon(":images/closetab.png"));
    close_button->setToolTip(tr("Close page"));
    close_button->setEnabled(true);
    //close_all_button->setCursor(Qt::ArrowCursor);
    //close_all_button->setAutoRaise(true);
    //close_all_button->setIcon(QIcon(":images/closealltab.png"));
    //close_all_button->setToolTip(tr("Close All Page"));
    //close_all_button->setEnabled(true);
    connect(close_button, SIGNAL(clicked()), SLOT(closeTab()));
    //connect(close_all_button, SIGNAL(clicked()), SLOT(closeAllTab()));
    tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tabBar(), SIGNAL(customContextMenuRequested(const QPoint&)),
            SLOT(showTabBarMenu(const QPoint&)));
}

RET_SEARCH BookView::newPage(QWidget *parent, const Query& query, bool newTab,
                             bool popup_browser)
{
    stopFlag = false;

    int dcount = 0;
    if (query.method.direction != MenuRead && query.method.direction != BookInfo) {
        foreach(Book *book, query.method.group->bookList()) {
            if (book->bookType() == BookWeb &&
                    book->checkState() == Qt::Checked) {
                dcount++;
            }
        }
    }

    PageWidget *page = 0;
    switch (query.method.direction) {
    case FullTextSearch:
        page = new SearchWholePage(this, query.method);
        break;
    case WholeRead:
        page = new AllPage(this, query.method);
        break;
    case MenuRead:
        page = new MenuPage(this, query.method);
        break;
    case BookInfo:
        page = new InfoPage(this, query.method);
        break;
    case ExactWordSearch:
    case ForwardSearch:
    case BackwardSearch:
    case KeywordSearch:
    case CrossSearch:
        page = new SearchPage(this, query.method);
        break;
    default:
        Q_ASSERT(0);
    }
    RET_SEARCH retStatus = page->search(query);

    connect(page->bookBrowser(), SIGNAL(statusRequested(QString)),
            SIGNAL(statusRequested(QString)));
    connect(page->bookBrowser(),
            SIGNAL(searchRequested(SearchDirection,QString)),
            SIGNAL(searchRequested(SearchDirection,QString)));
    connect(page->bookBrowser(), SIGNAL(pasteRequested(QString)),
            SIGNAL(pasteRequested(QString)));
    connect(page->bookBrowser(), SIGNAL(processRequested(QString)),
            SIGNAL(processRequested(QString)));
    connect(page->bookBrowser(), SIGNAL(soundRequested(QString)),
            SIGNAL(soundRequested(QString)));
    connect(page, SIGNAL(selectionRequested(QString)),
            SIGNAL(selectionRequested(QString)));
    connect(page->bookBrowser(), SIGNAL(selectionRequested(QString)),
            SIGNAL(selectionRequested(QString)));
    connect(parent, SIGNAL(viewFontChanged(QFont)), page,
            SLOT(changeFont(QFont)));

    QWidget *focus_page = 0;
    BookView *view = this;
    if (retStatus == NORMAL) {
        dcount++;
    }
    if (dcount > 1) {
        view = new BookView(parent);
        QString tab_title = query.toLogicString();
        if (newTab || count() == 0) {
            addTab(view, QIcon(":/images/stabs.png"), tab_title);
        } else {
            int index = currentIndex();
            closeTab();
            insertTab(index, view, QIcon(":/images/stabs.png"),
                      tab_title);
        }
        connect(view, SIGNAL(currentChanged(int)), SLOT(viewTabChanged(int)));
        setCurrentWidget(view);
    } else {
        if (checkLoaded()) {
            emit allWebLoaded();
        }
    }
    if (retStatus != NORMAL) {
        delete page;
        if (dcount == 0) return retStatus;
        retStatus = NORMAL;
    } else {
        QString tab_title = query.toLogicString();
        if (dcount > 1 || newTab || view->count() == 0) {
            view->addTab(page, QIcon(":/images/sbook.png"), tab_title);
        } else {
            int index = view->currentIndex();
            view->closeTab();
            view->insertTab(index, page, QIcon(":/images/sbook.png"),
                            tab_title);
        }
        focus_page = (QWidget*)page;
        connect(this, SIGNAL(fontChanged(QFont)), page,
                SLOT(changeFont(QFont)));
        if (query.method.direction == MenuRead || query.method.direction == BookInfo) {
            view->setCurrentWidget(focus_page);
            return NORMAL;
        }
    }

    foreach(Book *book, query.method.group->bookList()) {
        if (book->bookType() == BookWeb &&
                book->checkState() == Qt::Checked) {
            WebPage *wpage = new WebPage(this, book->path(), query);
            connect(wpage, SIGNAL(loadFinished(bool)),
                    SLOT(webViewFinished(bool)));
            connect(wpage, SIGNAL(linkRequested(QString)),
                    SIGNAL(processRequested(QString)));
            connect(parent, SIGNAL(viewFontChanged(QFont)), wpage,
                    SLOT(changeFont(QFont)));
            connect(this, SIGNAL(popupBrowserSet(bool)), wpage,
                    SLOT(setPopupBrowser(bool)));
            int idx = view->addTab(wpage, QIcon(":/images/web2.png"),
                    book->name());
            wpage->setTabIndex(idx);
            wpage->setTabBar(view->tabBar());
            if (!focus_page) {
                focus_page = (QWidget *)wpage;
            }
        }
    }
    emit popupBrowserSet(popup_browser);
    if (focus_page) {
        view->setCurrentWidget(focus_page);
    }
    emit tabChanged(count());

    return retStatus;
}

RET_SEARCH BookView::newWebPage(const QString &name, const QString &url,
                                bool popup_browser)
{
    WebPage *wpage = new WebPage(this, url);
    connect(wpage, SIGNAL(loadFinished(bool)), SLOT(webViewFinished(bool)));
    connect(wpage, SIGNAL(linkRequested(QString)),
            SIGNAL(processRequested(QString)));
    connect(this, SIGNAL(popupBrowserSet(bool)), wpage,
            SLOT(setPopupBrowser(bool)));
    emit popupBrowserSet(popup_browser);
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
    if (index < 0) {
        return BookUnset;
    }
    QString oname = widget(index)->objectName();
    if (oname == "dicpage"){
        return BookLocal;
    } else if (oname == "webpage") {
        return BookWeb;
    } else if (oname == "bookview") {
        BookView *v = (BookView*)widget(index);
        return v->currentPageType();
    } else {
        return BookUnset;
    }

}
BookType BookView::currentPageType()
{
    return pageType(currentIndex());
}

QWidget* BookView::pageWidget(int index)
{
    if (index < 0) {
        return 0;
    }
    QString oname = widget(index)->objectName();
    if (oname == "bookview") {
            BookView *v = (BookView*)widget(index);
            return v->pageWidget(v->currentIndex());
    } else {
        return widget(index);
    }

}
QWidget* BookView::currentPageWidget()
{
    return pageWidget(currentIndex());
}

SearchMethod BookView::pageMethod(int index)
{
    if (index < 0) {
        return SearchMethod();
    }
    QWidget *w = widget(index);
    QString oname = w->objectName();
    if (oname == "dicpage"){
        return ((PageWidget*)w)->method();
    } else if (oname == "webpage") {
        return ((WebPage*)w)->method();
    } else if (oname == "bookview") {
        BookView *v = (BookView*)widget(index);
        return v->currentPageMethod();
    } else {
        return SearchMethod();
    }
}

SearchMethod BookView::currentPageMethod()
{
    return pageMethod(currentIndex());
}

void BookView::stopSearch()
{
    stopFlag = true;
    stopAllLoading();
}

void BookView::zoomIn()
{
    //int fsize = CONF->browserFont.pointSize();

    //CONF->browserFont.setPointSize(fsize + 1);
    //emit fontChanged(CONF->browserFont);
    QWidget *w = currentPageWidget();
    if (w->objectName() == "dicpage") {
        ((PageWidget*)w)->zoomIn();
    }else{
        ((WebPage*)w)->zoomIn();
    }
}

void BookView::zoomOut()
{
    //int fsize = CONF->browserFont.pointSize();

    //if (fsize > 5) {
    //    CONF->browserFont.setPointSize(fsize - 1);
    //    emit fontChanged(CONF->browserFont);
    //}
    QWidget *w = currentPageWidget();
    if (w->objectName() == "dicpage") {
        ((PageWidget*)w)->zoomOut();
    }else{
        ((WebPage*)w)->zoomOut();
    }
}

void BookView::showTabBar(int new_tab)
{
    int tcount = count();
    if (!new_tab) {
        if (tcount > 0) {
            if (tcount > 1){
                int index = currentIndex();
                for (int i=tcount-1; i>index; i--) {
                    closeTab1(i);
                }
                for (int i=index-1; i>=0; i--) {
                    closeTab1(i);
                }
            }
            tabBar()->hide();
        }
    } else {
        if (tcount > 0) {
            tabBar()->show();
        }
    }
}

void BookView::viewTabChanged(int index) {
    if (index < 0) {
        closeTab1(currentIndex());
        emit tabChanged(count());
    } else {
        emit currentChanged(currentIndex());
    }
}

void BookView::stopAllLoading()
{
    for(int i=0; i < count(); i++) {
        if (widget(i)->objectName() == "webpage") {
            WebPage *w = (WebPage*)widget(i);
            w->stop();
        } else if (widget(i)->objectName() == "bookview") {
            BookView *w = (BookView*)widget(i);
            w->stopAllLoading();
        }
    }
}

bool BookView::checkLoaded() {
    bool all_loaded = true;
    for(int i=0; i < count(); i++) {
        if (widget(i)->objectName() == "webpage") {
            WebPage *w = (WebPage*)widget(i);
            if (w->loading()) {
                all_loaded = false;
                break;
            }
        } else if (widget(i)->objectName() == "bookview") {
            BookView *w = (BookView*)widget(i);
            if (!w->checkLoaded()){
                all_loaded = false;
                break;
            }
        }
    }
    return all_loaded;
}

void BookView::webViewFinished(bool) {
    if (checkLoaded()) {
        emit allWebLoaded();
    }
    emit currentChanged(currentIndex());
}

void BookView::setPopupBrowser(bool popup)
{
    emit popupBrowserSet(popup);
}

#include "menupage.h"
#include "book.h"
#include "bookbrowser.h"
#include "configure.h"
#include "ebook.h"
#include "pageitems.h"

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
    eb.initHook(bookBrowser_->fontSize(), method_.bookReader->fontList());

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
    eb.initHook(bookBrowser_->fontSize(), method_.bookReader->fontList());
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
        int page = QStringView{anc}.mid(1).toInt();
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



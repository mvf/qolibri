#include "allpage.h"
#include "book.h"
#include "bookbrowser.h"
#include "configure.h"
#include "ebook.h"
#include "pageitems.h"

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
    eb.initHook(bookBrowser_->fontSize(), method_.bookReader->fontList());
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


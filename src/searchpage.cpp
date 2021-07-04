#include "searchpage.h"
#include "book.h"
#include "bookbrowser.h"
#include "configure.h"
#include "ebook.h"
#include "pageitems.h"

SearchPage::SearchPage(QWidget *parent, const SearchMethod &method)
    : PageWidget(parent, method)
{
}

RET_SEARCH SearchPage::search(const Query& query)
{
    PageItems items(CONF->dictSheet);
    int itemIndex = 0;
    const RET_SEARCH retStatus = doSearch(query, items, itemIndex);

    bookTree->insertTopLevelItems(0, items.topItems());
    bookTree->setCurrentItem(bookTree->topLevelItem(0));
    items.expand(itemIndex <= 100 || items.topItems().count() == 1);
    emit statusRequested(QString("Browser (%1 character)").arg(items.textLength()));

    bookBrowser_->setBrowser(items.text());
    //qDebug() << items.text();

    return retStatus;
}

RET_SEARCH SearchPage::doSearch(const Query& query, PageItems &items, int &itemIndex)
{
    RET_SEARCH retStatus = NORMAL;

    items.addHItem(0, "TOP", "tmp");

    EBook eb(HookText);
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
    int bookIndex = 0;

    foreach(Book *book, query.method.group->bookList()) {
        if (book->bookType() != BookLocal) continue;
        if (book->checkState() != Qt::Checked) continue;

        emit statusRequested(book->name() + ":(" +
                             QString::number(itemIndex) + ")");
        if (checkStop() || break_flag) break;

        if ( eb.initBook(book->path(), book->bookNo(), bookIndex) < 0) continue;

        eb.initHook(bookBrowser_->fontSize(), book->fontList());

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

            RET_SEARCH chk = query.method.checkLimit(itemIndex, matchCount, items.textLength());
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

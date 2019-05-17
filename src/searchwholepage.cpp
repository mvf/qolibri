#include "searchwholepage.h"
#include "book.h"
#include "bookbrowser.h"
#include "ebook.h"
#include "configure.h"
#include "pageitems.h"

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
        eb.initHook(bookBrowser_->fontSize(), book->fontList(), CONF->indentOffset);
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
                break_check = query.method.checkLimit(totalCount, matchCount, item.textLength());
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


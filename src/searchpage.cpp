#include "searchpage.h"
#include "bookbrowser.h"
#include "searchpagebuilder.h"

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


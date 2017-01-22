#include "infopage.h"
#include "book.h"
#include "bookbrowser.h"
#include "bookviewglobals.h"
#include "configure.h"
#include "ebook.h"
#include "pageitems.h"

#include <QDir>

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


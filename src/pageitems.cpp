#include "pageitems.h"
#include "configure.h"

#include <QStringList>
#include <QTreeWidgetItem>

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

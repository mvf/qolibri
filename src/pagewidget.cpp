#include "pagewidget.h"
#include "bookbrowser.h"
#include "treescrollpopup.h"

#include <QApplication>
#include <QHeaderView>

bool stopFlag = false;

const QString PageWidget::CutString("----- cut -----");
const QString PageWidget::IntString("----- interrupted -----");

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

    connect(bookTree,
            SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            SLOT(scrollTo(QTreeWidgetItem*,QTreeWidgetItem*)));
    connect(bookTree,
            SIGNAL(itemPressed(QTreeWidgetItem*,int)),
            SLOT(scrollTo(QTreeWidgetItem*,int)));
    connect(bookTree, SIGNAL(customContextMenuRequested(QPoint)),
            SLOT(popupSlide(QPoint)));
}

void PageWidget::collapseBookTree()
{
    try {
        QTreeWidgetItem* root = bookTree->topLevelItem(0);
        for (int i = 0; i < root->childCount(); i++) {
                root->child(i)->setExpanded(false);
        }
    } catch (...) {};
}

bool PageWidget::checkStop()
{
    QEventLoop().processEvents();
    return stopFlag;
}

QString PageWidget::toAnchor(const QString &str, int num)
{
    return str + QString::number(num);
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

void PageWidget::setBookTreeVisible(bool visible)
{
    bookTree->setVisible(visible);
}

#include "webview.h"
#include "webpage.h"

#include <QAction>
#include <QContextMenuEvent>
#include <QDebug>
#include <QMenu>
#include <QRegularExpression>
#include <QTabBar>
#include <QTextCodec>
#include <QWebEngineSettings>

WebView::WebView(QWidget *parent)
    : QWebEngineView(parent)
    , loading_(false)
    , progressCount_(0)
    , tabBar_(0)
{
    setObjectName("webpage");

    WebPage *const webPage = new WebPage(this);
    connect(webPage, SIGNAL(linkClicked(const QUrl&)), SLOT(openLink(const QUrl&)));
    connect(webPage, SIGNAL(linkHovered(QString)), SLOT(copyHoveredLink(QString)));
    connect(webPage->action(QWebEnginePage::OpenLinkInNewWindow), SIGNAL(triggered()), SLOT(openNewWin()));
    setPage(webPage);

    connect(this, SIGNAL(loadStarted()), SLOT(progressStart()));
    connect(this, SIGNAL(loadFinished(bool)), SLOT(progressFinished(bool)));
    connect(this, SIGNAL(loadProgress(int)), SLOT(progress(int)));

    QWebEngineSettings *ws = settings();
    ws->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    ws->setAttribute(QWebEngineSettings::PluginsEnabled, true);
}

void WebView::openNewWin()
{
    emit externalLinkRequested(hoveredLink);
}

void WebView::copyHoveredLink(const QString &link)
{
    if (!link.isEmpty()) {
        hoveredLink = link;
    }
}

void WebView::load(const QString &url, const Query &query)
{
    loading_ = true;
    method_ = query.method;
    QByteArray enc = encString(url);
    QString ustr = setSearchString(url, enc, query.query);
    QString sdir = directionString(url);
    if (!sdir.isEmpty()) {
        ustr = setDirectionString(ustr, sdir, method_.direction);
    }
    if (enc.isEmpty()) {
        load(QUrl::fromEncoded(ustr.toLatin1()));
    } else {
        load(QUrl::fromEncoded(QTextCodec::codecForName(enc)->fromUnicode(ustr)));
    }
}

void WebView::contextMenuEvent(QContextMenuEvent* event)
{
    if (auto *webPage = qobject_cast<WebPage *>(page())) {
        if (auto *menu = webPage->createContextMenu(*this)) {
            menu->popup(event->globalPos());
        }
    }
}

QByteArray WebView::encString(const QString &url)
{
    if (!url.contains(QRegularExpression{"\\{.*\\}"})) {
        return QByteArray();
    } else {
        QString w1 = url.mid(url.indexOf('{')+1);
        QString w2 = w1.left(w1.indexOf('}'));
        return w2.trimmed().toLatin1();
    }
}

QString WebView::setSearchString(const QString &url, const QByteArray &enc,
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
    QRegularExpression rx{"\\{.*\\}"};
    if (!url.contains(rx)) {
        ustr += fstr;
    } else {
        ustr.replace(rx, fstr);
    }
    return ustr;
}
QString WebView::directionString(const QString &url)
{
    if (!url.contains(QRegularExpression{"\\[.*\\]"})) {
        return QString();
    } else {
        QString w1 = url.mid(url.indexOf('[')+1);
        QString w2 = w1.left(w1.indexOf(']'));
        return w2.trimmed();
    }
}
QString WebView::setDirectionString(const QString &url, const QString &dstr,
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

    return QString{url}.replace(QRegularExpression{"\\[.*\\]"}, udirec);
}

void WebView::changeFontSize(int delta)
{
    QWebEngineSettings *s = settings();
    int dsz = s->fontSize(QWebEngineSettings::DefaultFontSize);
    int fsz = s->fontSize(QWebEngineSettings::DefaultFixedFontSize);
    s->setFontSize(QWebEngineSettings::DefaultFontSize, dsz + delta);
    s->setFontSize(QWebEngineSettings::DefaultFixedFontSize, fsz + delta);
}

void WebView::progressStart()
{
    loading_ = true;
    if (tabBar_) tabBar_->setTabIcon(tabIndex_, QIcon(":/images/web2.png"));

    progressCount_ = 0;
}

void WebView::progress(int)
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

void WebView::progressFinished(bool)
{
    loading_ = false;
    if (tabBar_) tabBar_->setTabIcon(tabIndex_, QIcon(":/images/web1.png"));
}

void WebView::openLink(const QUrl &url)
{
    emit externalLinkRequested(url.toString());
}

void WebView::changeFont(const QFont &font)
{

    qDebug() << "WebPage::changeFont" << font.family();
    settings()->setFontFamily(QWebEngineSettings::StandardFont, font.family());

}

void WebView::zoomIn()
{
    changeFontSize(1);
}

void WebView::zoomOut()
{
    changeFontSize(-1);
}

void WebView::setPopupBrowser(bool popup)
{
    if (WebPage *webPage = qobject_cast<WebPage *>(page())) {
        webPage->setDelegateLinks(popup);
    }
}


#include "webview.h"
#include "configure.h"

#include <QAction>
#include <QDebug>
#include <QTabBar>
#include <QTextCodec>

WebView::WebView(QWidget *parent, const QString &url, const Query& query)
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

WebView::WebView(QWidget *parent, const QString &url)
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
void WebView::openNewWin()
{
    emit processRequested(CONF->browserProcess, QStringList(hoveredLink));
}

void WebView::copyHoveredLink(const QString &link, const QString&,
                              const QString&)
{
    if (!link.isEmpty()) {
        hoveredLink = link;
    }
}

void WebView::contextMenuEvent(QContextMenuEvent* event)
{
    QWebView::contextMenuEvent(event);
    //QMenu *menu = createStandardContextMenu();
    //QAction *a = menu->exec(event->globalPos());
    //delete menu;

}

QByteArray WebView::encString(const QString &url)
{
    if (!url.contains(QRegExp("\\{.*\\}"))) {
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
    QRegExp rx("\\{.*\\}");
    if (!url.contains(rx)) {
        ustr += fstr;
    } else {
        ustr.replace(rx, fstr);
    }
    return ustr;
}
QString WebView::directionString(const QString &url)
{
    if (!url.contains(QRegExp("\\[.*\\]"))) {
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

    return QString(url).replace(QRegExp("\\[.*\\]"), udirec);

}

void WebView::changeFontSize(int delta)
{
    QWebSettings *s = settings();
    int dsz = s->fontSize(QWebSettings::DefaultFontSize);
    int fsz = s->fontSize(QWebSettings::DefaultFixedFontSize);
    s->setFontSize(QWebSettings::DefaultFontSize, dsz + delta);
    s->setFontSize(QWebSettings::DefaultFixedFontSize, fsz + delta);
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
    if (!popupBrowser_) {
        QUrl u = QUrl::fromEncoded(url.toEncoded(), QUrl::TolerantMode);
        qDebug() << url.toEncoded();
        qDebug() << u.toString();
        load(u);
    } else {
        emit processRequested(CONF->browserProcess, QStringList(url.toString()));
    }

}

void WebView::changeFont(const QFont &font)
{

    qDebug() << "WebPage::changeFont" << font.family();
    settings()->setFontFamily(QWebSettings::StandardFont, font.family());

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
    popupBrowser_ = popup;
}


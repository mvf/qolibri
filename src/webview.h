#ifndef WEBVIEW_H
#define WEBVIEW_H

#include "method.h"

#include <QWebView>

struct QTabBar;
struct Query;

class WebView : public QWebView
{
    Q_OBJECT
public:
    WebView(QWidget *parent, const QString &url, const Query& query);
    WebView(QWidget *parent, const QString &url);
    void zoomIn();
    void zoomOut();
    void setTabIndex(int index) { tabIndex_ = index; }
    void setTabBar(QTabBar *bar) { tabBar_ = bar; }
    SearchMethod method() { return method_; }
    bool loading() { return loading_; }

protected:
    void contextMenuEvent(QContextMenuEvent* event);

private slots:
    void progressStart();
    void progress(int pcount);
    void progressFinished(bool ok);
    void openLink(const QUrl &url);
    void openNewWin();
    void copyHoveredLink(const QString &link, const QString &title,
                         const QString &text);
    void changeFont(const QFont &font);
    void setPopupBrowser(bool);

signals:
    void processRequested(const QString &program, const QStringList &arguments);

private:
    QByteArray encString(const QString &url);
    QString setSearchString(const QString &url, const QByteArray &enc,
                            const QString &query);
    QString directionString(const QString &url);
    QString setDirectionString(const QString &url, const QString &dstr,
                               SearchDirection &direc);

    QString hoveredLink;
    bool loading_;
    bool popupBrowser_;
    SearchMethod method_;
    int progressCount_;
    int tabIndex_;
    QTabBar *tabBar_;
};

#endif // WEBVIEW_H

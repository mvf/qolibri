#ifndef WEBVIEW_H
#define WEBVIEW_H

#include "method.h"

#include <QWebEngineView>

class QTabBar;
struct Query;

class WebView : public QWebEngineView
{
    Q_OBJECT
public:
    explicit WebView(QWidget *parent = 0);
    void zoomIn();
    void zoomOut();
    void setTabIndex(int index) { tabIndex_ = index; }
    void setTabBar(QTabBar *bar) { tabBar_ = bar; }
    SearchMethod method() { return method_; }
    using QWebEngineView::load;
    void load(const QString &url, const Query &query);
    bool loading() { return loading_; }

protected:
    void contextMenuEvent(QContextMenuEvent* event);

private slots:
    void progressStart();
    void progress(int pcount);
    void progressFinished(bool ok);
    void openLink(const QUrl &url);
    void openNewWin();
    void copyHoveredLink(const QString &link);
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
    void changeFontSize(int delta);
    QString hoveredLink;
    bool loading_;
    SearchMethod method_;
    int progressCount_;
    int tabIndex_;
    QTabBar *tabBar_;
};

#endif // WEBVIEW_H

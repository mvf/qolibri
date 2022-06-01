#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <QWebEnginePage>

class QWebEngineView;

class WebPage : public QWebEnginePage
{
    Q_OBJECT
public:
    explicit WebPage(QObject* parent = 0);
    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame);
    QMenu *createContextMenu(const QWebEngineView &view) const;
    void setDelegateLinks(bool enable) { delegateLinks = enable; }
signals:
    void linkClicked(const QUrl &url);
private:
    bool delegateLinks;
};

#endif // WEBPAGE_H

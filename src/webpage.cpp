#include "webpage.h"

#include <QMenu>
#include <QWebEngineContextMenuData>
#include <QWebEngineHistory>

WebPage::WebPage(QObject* parent)
: QWebEnginePage(parent)
, delegateLinks(false)
{
}

bool WebPage::acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame)
{
    Q_UNUSED(isMainFrame);
    if (type == NavigationTypeLinkClicked && delegateLinks) {
        emit linkClicked(url);
        return false;
    }
    return true;
}

// Stripped-down version of QWebEnginePage::createStandardContextMenu to
// have consistent menus across platforms and Qt versions and to only
// offer options that are actually supported
QMenu *WebPage::createContextMenu() const
{
    const QWebEngineContextMenuData &cmdata = contextMenuData();
    if (!cmdata.isValid())
        return 0;

    QWidget *const view = this->view();
    QMenu *const menu = new QMenu(view);

    if (cmdata.selectedText().isEmpty()) {
        QAction *action = new QAction(QIcon::fromTheme(QStringLiteral("go-previous")), tr("&Back"), menu);
        connect(action, SIGNAL(triggered()), view, SLOT(back()));
        action->setEnabled(history()->canGoBack());
        menu->addAction(action);

        action = new QAction(QIcon::fromTheme(QStringLiteral("go-next")), tr("&Forward"), menu);
        connect(action, SIGNAL(triggered()), view, SLOT(forward()));
        action->setEnabled(history()->canGoForward());
        menu->addAction(action);

        action = new QAction(QIcon::fromTheme(QStringLiteral("view-refresh")), tr("&Reload"), menu);
        connect(action, SIGNAL(triggered()), view, SLOT(reload()));
        menu->addAction(action);
    } else {
        menu->addAction(action(Copy));
        menu->addAction(action(Unselect));
    }

    if (!cmdata.linkText().isEmpty()) {
        menu->addAction(action(OpenLinkInNewWindow));
        menu->addAction(action(CopyLinkToClipboard));
    }

    if (cmdata.mediaUrl().isValid()) {
        switch (cmdata.mediaType()) {
        case QWebEngineContextMenuData::MediaTypeImage:
            menu->addAction(action(CopyImageUrlToClipboard));
            menu->addAction(action(CopyImageToClipboard));
            break;
        case QWebEngineContextMenuData::MediaTypeAudio:
        case QWebEngineContextMenuData::MediaTypeVideo:
            menu->addAction(action(CopyMediaUrlToClipboard));
            menu->addAction(action(ToggleMediaPlayPause));
            menu->addAction(action(ToggleMediaLoop));
            menu->addAction(action(ToggleMediaMute));
            break;
        default:
            break;
        }
    } else if (cmdata.mediaType() == QWebEngineContextMenuData::MediaTypeCanvas) {
        menu->addAction(action(CopyImageToClipboard));
    }

    menu->setAttribute(Qt::WA_DeleteOnClose, true);
    return menu;
}

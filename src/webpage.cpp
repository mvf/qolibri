#include "webpage.h"

#include <QMenu>
#if (QT_VERSION < QT_VERSION_CHECK(6, 2, 0))
#include <QWebEngineContextMenuData>
#else
#include <QWebEngineContextMenuRequest>
#endif
#include <QWebEngineHistory>
#include <QWebEngineView>

#include <memory>

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
QMenu *WebPage::createContextMenu(const QWebEngineView &view) const
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 2, 0))
    using QWebEngineContextMenuRequest = QWebEngineContextMenuData;

    const auto &request = contextMenuData();
    if (!request.isValid())
        return nullptr;
#else
    const auto *const pRequest = view.lastContextMenuRequest();
    if (!pRequest)
        return nullptr;

    const auto &request = *pRequest;
#endif

    auto menu = std::make_unique<QMenu>();

    if (request.selectedText().isEmpty()) {
        auto *action = new QAction(QIcon::fromTheme(QStringLiteral("go-previous")), tr("&Back"), menu.get());
        connect(action, &QAction::triggered, &view, &QWebEngineView::back);
        action->setEnabled(history()->canGoBack());
        menu->addAction(action);

        action = new QAction(QIcon::fromTheme(QStringLiteral("go-next")), tr("&Forward"), menu.get());
        connect(action, &QAction::triggered, &view, &QWebEngineView::forward);
        action->setEnabled(history()->canGoForward());
        menu->addAction(action);

        action = new QAction(QIcon::fromTheme(QStringLiteral("view-refresh")), tr("&Reload"), menu.get());
        connect(action, &QAction::triggered, &view, &QWebEngineView::reload);
        menu->addAction(action);
    } else {
        menu->addAction(action(Copy));
        menu->addAction(action(Unselect));
    }

    if (!request.linkText().isEmpty()) {
        menu->addAction(action(OpenLinkInNewWindow));
        menu->addAction(action(CopyLinkToClipboard));
    }

    if (request.mediaUrl().isValid()) {
        switch (request.mediaType()) {
        case QWebEngineContextMenuRequest::MediaTypeImage:
            menu->addAction(action(CopyImageUrlToClipboard));
            menu->addAction(action(CopyImageToClipboard));
            break;
        case QWebEngineContextMenuRequest::MediaTypeAudio:
        case QWebEngineContextMenuRequest::MediaTypeVideo:
            menu->addAction(action(CopyMediaUrlToClipboard));
            menu->addAction(action(ToggleMediaPlayPause));
            menu->addAction(action(ToggleMediaLoop));
            menu->addAction(action(ToggleMediaMute));
            break;
        default:
            break;
        }
    } else if (request.mediaType() == QWebEngineContextMenuRequest::MediaTypeCanvas) {
        menu->addAction(action(CopyImageToClipboard));
    }

    menu->setAttribute(Qt::WA_DeleteOnClose, true);
    return menu.release();
}

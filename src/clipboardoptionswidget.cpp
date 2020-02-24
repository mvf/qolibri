#include "clipboardoptionswidget.h"
#include "ui_clipboardoptionswidget.h"

#include <QApplication>

struct ClipboardOptionsWidget::Impl : Ui::ClipboardOptionsWidget {};

ClipboardOptionsWidget::ClipboardOptionsWidget(int currentIndex, QWidget *parent)
: QWidget(parent)
, d(new Impl)
{
    d->setupUi(this);
    d->stackedWidget->setCurrentIndex(currentIndex);
}

ClipboardOptionsWidget *ClipboardOptionsWidget::maybeCreate(QWidget *parent)
{
    if (auto const clipboard = QApplication::clipboard()) {
        if (clipboard->supportsSelection())
            return new ClipboardOptionsWidget(0, parent);
        else if (clipboard->supportsFindBuffer())
            return new ClipboardOptionsWidget(1, parent);
    }
    return nullptr;
}

ClipboardOptionsWidget::~ClipboardOptionsWidget() = default;

QClipboard::Mode ClipboardOptionsWidget::mode() const
{
    if (d->selectionRadio->isChecked())
        return QClipboard::Selection;
    if (d->findBufferRadio->isChecked())
        return QClipboard::FindBuffer;
    return QClipboard::Clipboard;
}

bool ClipboardOptionsWidget::isRaiseWindowEnabled() const
{
    return d->raiseWindowCheck->isChecked();
}

int ClipboardOptionsWidget::selectionDelay() const
{
    return d->selectionDelayBox->value();
}

void ClipboardOptionsWidget::setMode(QClipboard::Mode mode)
{
    switch (mode) {
    case QClipboard::Clipboard:
        d->clipboardRadio->setChecked(true);
        break;
    case QClipboard::Selection:
        d->selectionRadio->setChecked(true);
        break;
    case QClipboard::FindBuffer:
        d->findBufferRadio->setChecked(true);
        break;
    }
}

void ClipboardOptionsWidget::setRaiseWindowEnabled(bool enabled)
{
    d->raiseWindowCheck->setChecked(enabled);
}

void ClipboardOptionsWidget::setSelectionDelay(int delay)
{
    d->selectionDelayBox->setValue(delay);
}

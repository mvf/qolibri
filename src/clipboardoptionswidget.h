#ifndef CLIPBOARDOPTIONSWIDGET_H
#define CLIPBOARDOPTIONSWIDGET_H

#include <QClipboard>
#include <QWidget>

#include <memory>

class ClipboardOptionsWidget : public QWidget
{
    Q_OBJECT

public:
    static ClipboardOptionsWidget *maybeCreate(QWidget *parent = nullptr);
    ~ClipboardOptionsWidget() override;

    QClipboard::Mode mode() const;
    bool isRaiseWindowEnabled() const;
    int selectionDelay() const;

    void setMode(QClipboard::Mode mode);
    void setRaiseWindowEnabled(bool enabled);
    void setSelectionDelay(int delay);

private:
    ClipboardOptionsWidget(int currentIndex, QWidget *parent);

    struct Impl;
    std::unique_ptr<Impl> d;
};

#endif // CLIPBOARDOPTIONSWIDGET_H

#include "browsefileedit.h"

#include <QBoxLayout>
#include <QCompleter>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QIcon>
#include <QLineEdit>
#include <QPushButton>

BrowseFileEdit::BrowseFileEdit(QWidget *parent)
: QWidget(parent)
, lineEdit(new QLineEdit(this))
{
    lineEdit->setClearButtonEnabled(true);
    QCompleter *const completer = new QCompleter(this);
    QFileSystemModel *const model = new QFileSystemModel(completer);
    model->setRootPath("");
    lineEdit->setCompleter(completer);
    completer->setModel(model);

    QPushButton *const browseBtn = new QPushButton(QIcon(":images/open.png"), QStringLiteral(""), this);
    browseBtn->setToolTip(tr("Browse"));
    connect(browseBtn, SIGNAL(clicked()), SLOT(browse()));

    QLayout *const layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(lineEdit);
    layout->addWidget(browseBtn);
    setLayout(layout);
}

void BrowseFileEdit::setPlaceholderText(const QString &text)
{
    lineEdit->setPlaceholderText(text);
}

void BrowseFileEdit::setFileDialogTitle(const QString &title)
{
    fileDialogTitle = title;
}

QString BrowseFileEdit::text() const
{
    return lineEdit->text();
}

void BrowseFileEdit::setText(const QString &text)
{
    lineEdit->setText(text);
}

void BrowseFileEdit::browse()
{
    QString fileName(QFileDialog::getOpenFileName(this, fileDialogTitle));
    if (!fileName.isEmpty())
        setText(fileName);
}

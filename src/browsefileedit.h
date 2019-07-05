#ifndef BROWSEFILEEDIT_H
#define BROWSEFILEEDIT_H

#include <QString>
#include <QWidget>

class QLineEdit;

class BrowseFileEdit : public QWidget
{
    Q_OBJECT

public:
    explicit BrowseFileEdit(QWidget *parent = 0);
    void setPlaceholderText(const QString &text);
    void setFileDialogTitle(const QString &title);
    QString text() const;

public slots:
    void setText(const QString &text);

private slots:
    void browse();

private:
    QLineEdit *const lineEdit;
    QString fileDialogTitle;
};

#endif // BROWSEFILEEDIT_H

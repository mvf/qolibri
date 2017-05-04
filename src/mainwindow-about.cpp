#include <QMessageBox>

#include "mainwindow.h"
#include "gitversion.h"

void MainWindow::aboutQolibri()
{
    QString msg = tr("<h2>qolibri " GITDESCRIBE
#ifdef GITMODIFIED
                  "*"
#endif
                  "</h2>"
                  "<h3>EPWING Dictionary/Book Viewer</h3>"
                  "<p><a href='" GITREPOURL "'>" GITREPOURL "</a></p>"
                  "Compiled from "
#ifdef GITMODIFIED
                  "modified "
#endif
                  "commit <a href='" GITREPOURL "/commit/" GITHASH "'>" GITHASH "</a> (" GITTIME ").</p>"
                  "<p>Based on <a href='http://github.com/fujii/qolibri'>http://github.com/fujii/qolibri</a>,<br>"
                  "based on <a href='http://qolibri.sourceforge.jp'>http://qolibri.sourceforge.jp</a>.");

    QMessageBox::about(this, "About qolibri", msg );
}

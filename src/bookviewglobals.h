#ifndef BOOKVIEWGLOBALS_H
#define BOOKVIEWGLOBALS_H

// This transitional header contains definitions that were previously in
// bookview.cpp and are shared by classes that were moved out of there.
// It will eventually be refactored away.

class QString;
class QWidget;

extern QWidget *mainWin;

extern const char CutString[];
extern const char IntString[];

#define SJIStoUTF(q_bytearray) \
    QTextCodec::codecForName("Shift-JIS")->toUnicode(q_bytearray)

bool checkStop();
QString toAnchor(const QString &str, int num);

#endif // BOOKVIEWGLOBALS_H

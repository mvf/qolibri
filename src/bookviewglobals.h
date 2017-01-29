#ifndef BOOKVIEWGLOBALS_H
#define BOOKVIEWGLOBALS_H

// This transitional header contains definitions that were previously in
// bookview.cpp and are shared by classes that were moved out of there.
// It will eventually be refactored away.

class QString;

extern const char CutString[];
extern const char IntString[];

bool checkStop();
QString toAnchor(const QString &str, int num);

#endif // BOOKVIEWGLOBALS_H

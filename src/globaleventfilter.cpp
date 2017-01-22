#include "globaleventfilter.h"

#include <QApplication>
#include <QKeyEvent>
#include <QWidget>

bool GlobalEventFilter::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		if (keyEvent->key() == Qt::Key_Backspace
		    && qApp->focusWidget()
		    && !(qApp->focusWidget()->inherits("QLineEdit")
		         || qApp->focusWidget()->metaObject()->className() == QString("QTextEdit")))
		{
//			qWarning("%s", qApp->focusWidget()->metaObject()->className());
			emit focusSearch();
			return true;
		}
	}
	// standard event processing
	return QObject::eventFilter(obj, event);
}

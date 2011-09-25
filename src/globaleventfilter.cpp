#include <QApplication>
#include <QKeyEvent>
#include <QWidget>

#include "globaleventfilter.h"

bool GlobalEventFilter::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		if (keyEvent->key() == Qt::Key_Backspace
		    && qApp->focusWidget()
		    && !qApp->focusWidget()->inherits("QLineEdit"))
		{
			emit(focusSearch());
			return true;
		}
	}
	// standard event processing
	return QObject::eventFilter(obj, event);
}

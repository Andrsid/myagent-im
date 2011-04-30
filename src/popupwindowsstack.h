#ifndef POPUPWINDOWSSTACK_H
#define POPUPWINDOWSSTACK_H

#include <QObject>
#include <QDateTime>

#include "popupwindow.h"

class SystemTrayIcon;

class PopupWindowsStack : public QObject
{
Q_OBJECT
public:
	enum Direction
	{
		Up,
		Down
	};

	PopupWindowsStack(SystemTrayIcon* sysTray);
	~PopupWindowsStack();

public slots:
	void showNewMessage(const QString & from, const QString & to, const QDateTime dateTime);
	void showNewLetter(const QString & from, const QString & subject, const QDateTime dateTime);
	void showLettersUnread(const quint32 cnt);

	void deleteAllWindows();
	void showAllUnclosedWindows();
	void closeAllUnclosedWindows();

signals:
	void mouseEntered();
	void mouseLeaved();

private slots:
	void retranslateWindowsGeometry();
	void windowClosed();

	PopupWindow* newWindow();
	QRect newCoords(quint32 n = 0);

private:
	SystemTrayIcon* m_sysTray;
	QList<PopupWindow*> existingWindows;
	QList<PopupWindow*> shownWindows;
	Direction direction;
};

#endif // POPUPWINDOWSSTACK_H

/***************************************************************************
 *   Copyright (C) 2008 by Alexander Volkov                                *
 *   volkov0aa@gmail.com                                                   *
 *                                                                         *
 *   This file is part of instant messenger MyAgent-IM                     *
 *                                                                         *
 *   MyAgent-IM is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   MyAgent-IM is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef MESSAGEEDITOR_H
#define MESSAGEEDITOR_H

#include <QWidget>
#include <QTextCharFormat>
#include <QPointer>

#include "onlinestatus.h"

class QToolBar;
class MessageEdit;
class QAction;
class EmoticonSelector;
class QToolButton;
class ToolButton;
class QComboBox;
class QLabel;

class Account;
class Contact;

class MessageEditor : public QWidget
{
Q_OBJECT
public:
	MessageEditor(Account* account, Contact* contact, QWidget* parent = 0);
	~MessageEditor();

	void blockInput();
	void unblockInput();
	bool isBlocked();

	QTextDocument* document() const;

public slots:
	void clear();

signals:
	void textChanged();
	void sendPressed();
	void wakeupPressed();

protected:
	virtual void hideEvent(QHideEvent* event);
	bool eventFilter(QObject *obj, QEvent *ev);

private slots:
	void chooseFont();
	void chooseFontColor();
	void chooseBackgroundColor();

	void setCheckSpelling(bool on);

	void setCurrentFont(const QFont& f);
	void setTextColor(const QColor& c);
	void setTextBackgroundColor(const QColor& c);

	void setFontBold(bool b);
	void setFontItalic(bool b);
	void setFontUnderline(bool b);

	void insertEmoticon(const QString& id);

	void updateFormatActions();

	void triggerEmoticonSelector();

	void addFile();
	void fileTransfer(bool checked);

	void slotCurrentCharFormatChanged(const QTextCharFormat& f);

	void checkContactStatus(OnlineStatus status);

	void readSettings();
	void writeSettings();
private:
	QAction* addToolAction(const QIcon& icon, const QObject* receiver, const char* method);
	QIcon toolIcon(const QString& toolName) const;
	void createToolBar();
	
	void mergeMyCharFormat(const QTextCharFormat& fmt);

	void createFileTransferBar();
	QIcon fileTransferIcon(const QString& toolName) const;
	QToolButton* createFileTransferToolButton(const QIcon& icon, const QObject* receiver, const char* method);
private:
	QPointer<Account> m_account;
	QPointer<Contact> m_contact;

	QToolBar* toolBar;
	MessageEdit* messageEdit;

	QAction* spellAction;

	QAction* fontAction;
	QAction* colorAction;
	QAction* bkColorAction;
	
	QAction* boldFontAction;
	QAction* italicFontAction;
	QAction* underlinedFontAction;
	
	QAction* smilesAction;

	ToolButton* wakeupButton;

	EmoticonSelector* emoticonSelector;

	QTextCharFormat lastUserFormat;

	QToolBar* fileTransferBar;
	QComboBox* filesBox;
	QLabel* bytesLabel;
};

#endif
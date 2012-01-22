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
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef MAINMENUBUTTON_H
#define MAINMENUBUTTON_H

#include <QPointer>

#include "gui/buttonwithmenu.h"
#include "onlinestatus.h"

class Account;
class SearchContactsForm;
class ContactListWindow;
class SettingsWindow;
class ChatWindowsManager;
class ContactInfo;
class NewConferenceDialog;

class MainMenuButton : public ButtonWithMenu
{
Q_OBJECT
public:
	MainMenuButton(Account* account, ContactListWindow* w);
	void setChatWindowsManager(ChatWindowsManager* cwm);
	SettingsWindow* getSettingsWindow();

Q_SIGNALS:
	void statusesCountChanged();
	void notificationTypeChanged();

private Q_SLOTS:
	void searchContacts();
	void showSearchResults(quint32 status, bool timeout);
	void showAddContactDialog(const ContactInfo& info);
	void showAddContactDialogEnd();
	void checkOnlineStatus(OnlineStatus);
	void beginSearch();
	void searchMoreContacts();
	void newSearch();
	void addSmsContact();
	void addSmsContactError(QString);
	void addGroup();
	void addGroupError(QString);
	void showSettingsWindow();
	void showAboutDialog();
	void createNewConference();

private:
	QAction* addContactAction;
	QAction* addSmsContactAction;
	QAction* addGroupAction;
	QAction* deleteUserAction;
	QAction* ignoreUserAction;
	QAction* newConferenceAction;
	Account* m_account;
	ContactListWindow* mainWindow;
	QPointer<SearchContactsForm> searchForm;
	QPointer<SettingsWindow> settingsWindow;
	QPointer<NewConferenceDialog> newConferenceWindow;

	ChatWindowsManager* chatWindowsManager;
};

#endif

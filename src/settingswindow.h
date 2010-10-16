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

#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QWidget>
#include <QSettings>
#include "chatwindowsmanager.h"
#include "account.h"
#include "contactlistwindow.h"

class Account;

class QListWidget;
class QStackedWidget;

class QCheckBox;
class QRadioButton;
class QLineEdit;

class SettingsWindow : public QWidget
{
Q_OBJECT
public:
	SettingsWindow(Account* account, ContactListWindow* clw);
	~SettingsWindow();

	void setChatWindowsManager(ChatWindowsManager* cwm);

private slots:
	void saveSettings();

private:
	void createCommonPage();
	void saveCommonSettings();

	void createMessagesPage();
	void saveMessagesSettings();

	void createWindowsPage();
	bool saveWindowsSettings();

	void createAudioPage();
	void saveAudioSettings();

	void createViewPage();
	void saveViewSettings();

	QListWidget* listWidget;
	QStackedWidget* pagesWidget;

	// common page widgets
	QCheckBox* awayCheckBox;
	QLineEdit* minutesEdit;

	//messages page widgets
	QRadioButton* enterButton;
	QRadioButton* doubleEnterButton;
	QRadioButton* ctrlEnterButton;
	QCheckBox* altSButton;

	//windows page widgets
	QCheckBox* tabWindows;

	//audio page widgets
	QCheckBox* enableSounds;

	//view page widgets
	QCheckBox* enableMicroBlog;

	ChatWindowsManager* chatWindowsManager;

	Account* m_account;

	ContactListWindow* m_clw;

};

#endif

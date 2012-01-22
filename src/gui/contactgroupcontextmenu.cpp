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

#include "contactgroupcontextmenu.h"

#include <QDebug>

#include "account.h"
#include "contactlist.h"
#include "contactgroup.h"
#include "inputlinedialog.h"
#include "centeredmessagebox.h"

ContactGroupContextMenu::ContactGroupContextMenu(Account * account, QWidget * parent)
	: QMenu(parent), m_account(account), m_group(0)
{
	renameGroupAction = new QAction(QIcon(":icons/cl_rename_group.png"), tr("Rename group"), this);
	connect(renameGroupAction, SIGNAL(triggered()), SLOT(renameGroup()));
	connect(m_account->contactList(), SIGNAL(renameGroupError(QString)), SLOT(renameGroupError(QString)));
	
	removeGroupAction = new QAction(QIcon(":icons/cl_delete_group.png"), tr("Remove group"), this);
	connect(removeGroupAction, SIGNAL(triggered()), SLOT(removeGroup()));
	connect(m_account->contactList(), SIGNAL(removeGroupOnServerError(QString)), SLOT(removeGroupError(QString)));	

	addAction(renameGroupAction);
	addSeparator();
	addAction(removeGroupAction);

	checkOnlineStatus(m_account->onlineStatus());
	connect(m_account, SIGNAL(onlineStatusChanged(OnlineStatus)), this, SLOT(checkOnlineStatus(OnlineStatus)));
}

void ContactGroupContextMenu::setGroup(ContactGroup* group)
{
	m_group = group;

	checkOnlineStatus(m_account->onlineStatus());
}

void ContactGroupContextMenu::checkOnlineStatus(OnlineStatus status)
{
	if (!m_group)
		return;

	bool connected = status.connected();

	removeGroupAction->setEnabled(connected);
	renameGroupAction->setEnabled(connected);
}

void ContactGroupContextMenu::removeGroup()
{
	qDebug() << "ContactGroupContextMenu::removeGroup";

	if (CenteredMessageBox::question(tr("Remove group"), tr("Are you sure you want to remove group") + ' ' + m_group->name() + '?', QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Ok)
	{
		qDebug() << "real removing";
		m_account->contactList()->removeGroupOnServer(m_group);
	}
}

void ContactGroupContextMenu::removeGroupError(QString error)
{
	qDebug() << "ContactGroupContextMenu::removeGroupError";
	CenteredMessageBox::critical(tr("Removing group error"), error);
}

void ContactGroupContextMenu::renameGroup()
{
	InputLineDialog dialog(tr("Rename group"), QIcon(":icons/cl_rename_group.png"), tr("Enter new name for group %1:").arg(m_group->name()), ".+", m_group->name());
	
	if (dialog.exec() == QDialog::Accepted)
	{
		QString newName = dialog.text();
		m_account->contactList()->renameGroup(m_group, newName);
	}
}

void ContactGroupContextMenu::renameGroupError(QString error)
{
	CenteredMessageBox::critical(tr("Renaming group error"), error);
}

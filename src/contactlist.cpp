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

#include "contactlist.h"

#include <QDebug>

#include <QFile>
#include <QDataStream>

#include "proto.h"
#include "account.h"
#include "contact.h"
#include "taskremovecontact.h"
#include "taskaddcontact.h"
#include "taskaddsmscontact.h"
#include "taskaddgroup.h"
#include "taskremovegroup.h"
#include "taskrenamegroup.h"
#include "tasknewconference.h"

ContactList::ContactList(Account* account)
	: QObject(account), m_account(account)
{
	constructing = false;
	connect(m_account, SIGNAL(onlineStatusChanged(OnlineStatus)), this, SLOT(checkOnlineStatus(OnlineStatus)));
}

ContactList::~ContactList()
{
	qDebug() << "ContactList::~ContactList()";
	save();
	clear();
}

void ContactList::clear()
{
	qDebug() << "--- ContactList::clear() ---";
	qDeleteAll(m_groups);
	m_groups.clear();

	emit groupsCleared();

	qDeleteAll(m_hiddenGroups);
	m_hiddenGroups.clear();

	qDeleteAll(m_contacts);
	m_contacts.clear();
	
	constructing = false;
	emit updated();
}

void ContactList::addGroup(quint32 id, quint32 flags, const QString& name)
{
	qDebug() << "ContactList::addGroup" << constructing;
	ContactGroup* group = new ContactGroup(id, flags, name);
	m_groups.append(group);

	/*if (!constructing) */emit groupAdded(group);
}

Contact* ContactList::addContact(const ContactData& data)
{
	ContactGroup* group;
	uint id = data.group;

	QList<ContactGroup*>::const_iterator it = m_groups.begin();
	while (it != m_groups.end() && (*it)->id() != id) ++it;
	if (it != m_groups.end())
		group = *it;
	else
	{
		group = new ContactGroup(data.group, 0, "");
		m_hiddenGroups.append(group);
	}

	Contact* contact = findContact(data.email, (constructing) ? (tmpContacts) : (m_contacts));
	if (contact)
	{
		qDebug() << "preserving contact" << contact->email();
		((constructing) ? (tmpContacts) : (m_contacts)).removeAll(contact);
		contact->update(data, group);
	}
	else
	{
		qDebug() << "creating new contact for contact" << data.email << data.nick;
		contact = new Contact(data, group, m_account);
	}
	
	if (constructing)
	{
		qDebug() << "adding contact to temporary contacts";
		tmpContacts.append(contact);
	}
	else
	{
		qDebug() << "adding contact to main contacts";
		addContact(contact);
	}

	return contact;
}

void ContactList::addContact(Contact* contact)
{
	m_contacts.append(contact);

	if (!constructing) emit contactAdded(contact);
}

void ContactList::changeContactStatus(OnlineStatus status, QByteArray email)
{
	Contact* contact = findContact(email);
	if (contact)
		contact->changeStatus(status);
}

Contact* ContactList::findContact(const QByteArray & email)
{
	return findContact(email, m_contacts);
}

Contact* ContactList::findContact(const QByteArray& email, QList<Contact*> & list)
{
	if (email == "phone")
		return 0;
	
	QList<Contact*>::iterator it = list.begin();
	
	while (it != list.end() && (*it)->email() != email)
		++it;

	if (it == list.end())
		return 0;
	else
		return *it;
}


Contact* ContactList::findSmsContact(const QString & nickname)
{
	QList<Contact*>::iterator it = m_contacts.begin();
	
	while (it != m_contacts.end())
	{
		bool phone = (*it)->isPhone();
		if (!phone || (*it)->nickname() != nickname)
			++it;
		else
			break;
	}
	
	if (it == m_contacts.end())
		return 0;
	else
		return *it;
}

Contact* ContactList::getContact(const QByteArray& email)
{
	Contact* contact = findContact(email);
	if (contact)
		return contact;
	
	qDebug() << "ContactList::getContact: can't find contact for email " << email;
	// creating temporary contact
	contact = new Contact(ContactData(email), 0, m_account);
	addContact(contact);
	return contact;
}

Contact* ContactList::findContactWithPhone(const QByteArray& phoneNumber)
{
	qDebug() << phoneNumber;

	Contact* c = lastSmsFrom.value(phoneNumber, NULL);
	if (c)
	{
		qDebug() << "Contact found" << c->nickname();
		return c;
	}
	qDebug() << "Contact didn't find";

	QList<Contact*>::iterator it = m_contacts.begin();
	
	while (it != m_contacts.end() && !(*it)->hasPhone(phoneNumber))
		++it;

	if (it != m_contacts.end())
		return *it;

	return 0;
}

void ContactList::contactTyping(QByteArray email)
{
	Contact* contact = findContact(email);
	if (contact)
		contact->contactTyping();
}

void ContactList::beginUpdating()
{
	qDebug() << "ContactList::beginUpdating()";
	qDeleteAll(m_groups);
	m_groups.clear();

	emit groupsCleared();
	
	qDeleteAll(m_hiddenGroups);
	m_hiddenGroups.clear();

	tmpContacts.clear();
	
	constructing = true;
}

void ContactList::endUpdating()
{
	qDebug() << "ContactList::endUpdating()";

	qDeleteAll(m_contacts);
	m_contacts = tmpContacts;
	
	for (int i = 0; i < m_contacts.size(); i++)
		qDebug() << m_contacts.at(i)->email() << (void*)m_contacts.at(i);

	constructing = false;
	emit updated();
}

bool ContactList::removeContactOnServer(Contact* contact)
{
	qDebug() << "ContactList::removeContact " << contact->email();
	if (!contact)
	{
		qDebug() << "removeContact: contact doesn't exist";
		return false;
	}
	
	if (contact->isTemporary() && !contact->isConference())
	{
		m_contacts.removeAll(contact);
		emit contactRemoved(contact);
		//delete contact;
		//emit updated();

		return true;
	}

	Task* task = new Tasks::RemoveContact(contact, m_account->client(), this);
	connect(task, SIGNAL(done(quint32, bool)), this, SLOT(removeContactOnServerEnd(quint32, bool)));
	
	return task->exec();
}

void ContactList::removeContactOnServerEnd(quint32 status, bool timeout)
{
	if (timeout || status != CONTACT_OPER_SUCCESS)
	{
		qDebug() << "checkRemoveContactResult: timeout = " << timeout << ", status = " << status;
		emit removeContactOnServerError(tr("Removing contact failed"));
		return;
	}
	
	Tasks::RemoveContact* task = qobject_cast<Tasks::RemoveContact*>(sender());
	Contact* contact = task->contact();
	m_contacts.removeAll(contact);
	emit contactRemoved(contact);
}

void ContactList::slotContactAuthorized(const QByteArray& email)
{
	qDebug() << "ContactList::slotContactAuthorized";
	Contact* contact = getContact(email);
	if (contact)
		contact->setAuthorized();
	else
		qDebug() << "can't find authorized contact " << email;
}

void ContactList::load()
{
	qDebug() << "ContactList::load()";
	if (m_account->path().isEmpty())
		return;
	
	if (!m_groups.isEmpty() || !m_contacts.isEmpty())
		return;

	QFile file(m_account->path() + "/contactlist");
	if (!file.open(QIODevice::ReadOnly))
	{
		qDebug() << "contactlist file doesn't exist";
		return;
	}
	
	clear();
	
	QDataStream in(&file);
	
	int nGroups;
	in >> nGroups;
	if (in.status() != QDataStream::Ok)
	{
		file.close();
		emit updated();
		return;
	}
	
	for (int i = 0; i < nGroups; i++)
	{
		ContactGroup* group = new ContactGroup(in);
		m_groups.append(group);
		emit groupAdded(group);
	}
	
	if (in.status() != QDataStream::Ok)
	{
		file.close();
		clear();
		return;
	}
	
	while (!in.atEnd())
	{
		Contact* contact = new Contact(m_account);
		contact->load(in);
		if (in.status() == QDataStream::Ok)
		{
			Contact* cont = findContact(contact->email());
			if (cont)
			{
				qDebug() << "Contact" << cont->email() << "already exists!";
				cont = contact;
				delete contact;
			}
			else
				m_contacts.append(contact);

		}
		else
		{
			delete contact;
			break;
		}
	}
	
	if (in.status() != QDataStream::Ok)
		clear();

	emit updated();
	
	file.close();
}

void ContactList::save() const
{
	if (m_account->path().isEmpty())
		return;
	
	QFile file(m_account->path() + "/contactlist");
	if (!file.open(QIODevice::WriteOnly))
	{
		qDebug() << "can't open file to write contactlist";
		return;
	}
	
	QDataStream out(&file);
	
	out << m_groups.size();
	for (int i = 0; i < m_groups.size(); i++)
		out << m_groups.at(i);
	
	for (int i = 0; i < m_contacts.size(); ++i)
		m_contacts.at(i)->save(out);
	
	file.close();
}

void ContactList::checkOnlineStatus(OnlineStatus status)
{
	if (status == OnlineStatus::offline)
		for (int i = 0; i < m_contacts.size(); i++)
			m_contacts[i]->changeStatus(OnlineStatus::offline);
}

ContactGroup* ContactList::group(quint32 id) const
{
	for (int i = 0; i < m_groups.size(); i++)
		if (m_groups.at(i)->id() == id)
			return m_groups.at(i);
	return NULL;
}

bool ContactList::addTemporaryContactToGroup(Contact* contact, quint32 group)
{
	MRIMClient* mc = m_account->client();
	if (mc == NULL)
		return false;

	Task* task = new Tasks::AddContact(group, contact->email(), contact->nickname(), "", mc);
	connect(task, SIGNAL(done(quint32, bool)), this, SLOT(addContactOnServerEnd(quint32, bool)));

	if (!task->exec())
	{
		qDebug() << "Contact::addTemporaryContactToGroup: add contact task returned 0";
		return false;
	}
	return true;
}

bool ContactList::addContactOnServer(quint32 group, const QByteArray & email, const QString & nickname, const QString & authorizationMessage)
{
	Contact* contact = findContact(email);
	if (contact)
		return false;
	
	MRIMClient* mc = m_account->client();
	
	Task* task = new Tasks::AddContact(group, email, nickname, authorizationMessage, mc);
	connect(task, SIGNAL(done(quint32, bool)), this, SLOT(addContactOnServerEnd(quint32, bool)));
	if (!task->exec())
	{
		qDebug() << "add contact task returned 0";
		return false;
	}
	
	return true;
}

void ContactList::addContactOnServerEnd(quint32 status, bool timeout)
{
	if (timeout)
	{
		emit addContactOnServerError(tr("Time is out"));
		return;
	}
	else if (status != CONTACT_OPER_SUCCESS)
	{
		emit addContactOnServerError(tr("Error"));
		return;
	}

	Tasks::AddContact* task = qobject_cast<Tasks::AddContact*>(sender());

	Contact* contact = findContact(task->email());
	if (contact)
	{
		if (contact->isTemporary())
		{
			qDebug() << "addContactOnServerEnd remove temporary contact";
			m_contacts.removeAll(contact);
			//delete contact;
			emit contactRemoved(contact);
		}
		else
		{
			qDebug() << "ContactList::addContactOnServerEnd: strange error: contact added but it is already in contactlist";
			return;
		}
	}

	addContact(task->contactData());
}


bool ContactList::newConferenceOnServer(QString confName, QByteArray owner, QList<QByteArray> members)
{
	qDebug() << "ContactList::newConferenceOnServer" << confName;
	MRIMClient* mc = m_account->client();
	
	Task* task = new Tasks::NewConference(confName, owner, mc, members);
	connect(task, SIGNAL(done(quint32, bool)), this, SLOT(newConferenceOnServerEnd(quint32, bool)));
	if (!task->exec())
	{
		qDebug() << "new conference task returned 0";
		return false;
	}
	
	return true;
}

bool ContactList::addConferenceOnServer(const QByteArray & chat, const QString & confName)
{
	Contact* contact = findContact(chat);
	if (contact)
		return false;

	qDebug() << "ContactList::addConferenceOnServer" << confName << chat;
	MRIMClient* mc = m_account->client();
	
	Task* task = new Tasks::NewConference(confName, chat, mc, QList<QByteArray>());
	connect(task, SIGNAL(done(quint32, bool)), this, SLOT(newConferenceOnServerEnd(quint32, bool)));
	if (!task->exec())
	{
		qDebug() << "new conference task returned 0";
		return false;
	}
	
	return true;
}

void ContactList::newConferenceOnServerEnd(quint32 status, bool timeout)
{
	qDebug() << "ContactList::newConferenceOnServerEnd";
	if (timeout)
	{
		emit newConferenceOnServerError(tr("Time is out"));
		return;
	}
	else if (status != CONTACT_OPER_SUCCESS)
	{
		emit newConferenceOnServerError(tr("Error"));
		return;
	}

	Tasks::NewConference* task = qobject_cast<Tasks::NewConference*>(sender());

	Contact* contact = findContact(task->contactData().email);
	if (contact)
	{
		if (contact->isTemporary())
		{
			qDebug() << "addContactOnServerEnd remove temporary contact";
			m_contacts.removeAll(contact);
			emit contactRemoved(contact);
		}
		else
		{
			qDebug() << "ContactList::addContactOnServerEnd: strange error: contact added but it is already in contactlist";
			return;
		}
	}

	qDebug() << "Conference name:" << task->contactData().nick;
	contact = addContact(task->contactData());
}

bool ContactList::addSmsContactOnServer(const QString & nickname, const QStringList & phones)
{
	Contact* contact = findSmsContact(nickname);
	if (contact)
	{
		emit addSmsContactOnServerError(tr("Contact %1 already exists").arg(nickname));
		return false;
	}
	
	MRIMClient* mc = m_account->client();
	
	Task* task = new Tasks::AddSmsContact(nickname, phones, mc);
	connect(task, SIGNAL(done(quint32, bool)), this, SLOT(addSmsContactOnServerEnd(quint32, bool)));
	if (!task->exec())
	{
		qDebug() << "add sms contact task returned 0";
		return false;
	}
	
	return true;
}

void ContactList::addSmsContactOnServerEnd(quint32 status, bool timeout)
{
	if (timeout)
	{
		emit addSmsContactOnServerError(tr("Time is out"));
		return;
	}
	else if (status != CONTACT_OPER_SUCCESS)
	{
		emit addSmsContactOnServerError(tr("Error"));
		return;
	}

	Tasks::AddSmsContact* task = qobject_cast<Tasks::AddSmsContact*>(sender());
	addContact(task->contactData());

	//emit updated();
}

bool ContactList::addGroupOnServer(const QString& groupName)
{
	for (int i = 0; i < m_groups.size(); i++)
	{
		if (m_groups.at(i)->name() == groupName)
		{
			emit addGroupOnServerError(tr("Group %1 already exists.").arg(groupName));
			return false;
		}
	}
	
	MRIMClient* mc = m_account->client();
	
	Task* task = new Tasks::AddGroup(groupName, mc);
	connect(task, SIGNAL(done(quint32, bool)), this, SLOT(addGroupOnServerEnd(quint32, bool)));
	if (!task->exec())
	{
		emit addGroupOnServerError(tr("Adding group is currently impossible"));
		return false;
	}
	
	return true;
}

void ContactList::addGroupOnServerEnd(quint32 status, bool timeout)
{
	QString error;
	
	if (timeout)
		error = tr("Time is out");
	else if (status == CONTACT_OPER_ERROR)
		error = tr("Incorrect data");
	else if (status == CONTACT_OPER_INTERR)
		error = tr("Internal error");
	else if (status == CONTACT_OPER_INVALID_INFO)
		error = tr("Invalid group name");
	else if (status == CONTACT_OPER_GROUP_LIMIT)
		error = tr("Number of groups exceeded");
	else if (status != CONTACT_OPER_SUCCESS)
		error = tr("Unknown error");
	
	if (!error.isEmpty())
	{
		emit addGroupOnServerError(error);
		return;
	}

	Tasks::AddGroup* task = qobject_cast<Tasks::AddGroup*>(sender());
	addGroup(task->groupId(), CONTACT_FLAG_GROUP, task->groupName());
	//emit updated();
}

bool ContactList::removeGroupOnServer(ContactGroup* group)
{
	if (!group)
	{
		emit removeGroupOnServerError(tr("Unknown group"));
		return false;
	}
	
	qDebug() << "searching for group" << group->name() << (void*)group;
	ContactsIterator it = contactsBegin();
	for (; it != contactsEnd(); it++)
	{
		qDebug() << "current group = " << (*it)->group()->name() << (void*)((*it)->group());
		if ((*it)->group() == group)
			break;
	}

	if (it != contactsEnd())
	{
		emit removeGroupOnServerError(tr("Group is not empty"));
		return false;
	}
	
	MRIMClient* mc = m_account->client();
	Tasks::RemoveGroup* task = new Tasks::RemoveGroup(group, mc);
	connect(task, SIGNAL(done(quint32, bool)), this, SLOT(removeGroupOnServerEnd(quint32, bool)));
	if (!task->exec())
	{
		emit removeGroupOnServerError(tr("Removing group is currently impossible"));
		return false;
	}
	
	return true;
}

void ContactList::removeGroupOnServerEnd(quint32 status, bool timeout)
{
	QString error;
	
	if (timeout)
		error = tr("Time is out");
	else if (status == CONTACT_OPER_ERROR)
		error = tr("Incorrect data");
	else if (status == CONTACT_OPER_INTERR)
		error = tr("Internal error");
	else if (status == CONTACT_OPER_INVALID_INFO)
		error = tr("Invalid group name");
	else if (status == CONTACT_OPER_NO_SUCH_USER)
		error = tr("No such group");
	else if (status != CONTACT_OPER_SUCCESS)
		error = tr("Unknown error");
	
	if (!error.isEmpty())
	{
		emit removeGroupOnServerError(error);
		return;
	}

	Tasks::RemoveGroup* task = qobject_cast<Tasks::RemoveGroup*>(sender());
	
	ContactGroup* group = task->group();
	m_groups.removeAll(group);
	delete group;

	emit groupRemoved(group);
	emit updated();
}


bool ContactList::renameGroup(ContactGroup * group, const QString & newName)
{
	if (!group)
	{
		emit renameGroupError(tr("Unknown group"));
		return false;
	}
	
	MRIMClient* mc = m_account->client();
	Tasks::RenameGroup* task = new Tasks::RenameGroup(group, newName, mc);
	connect(task, SIGNAL(done(quint32, bool)), this, SLOT(renameGroupEnd(quint32, bool)));
	if (!task->exec())
	{
		emit renameGroupError(tr("Renaming group is currently impossible"));
		return false;
	}
	
	return true;
}

void ContactList::renameGroupEnd(quint32 status, bool timeout)
{
	QString error;
	
	if (timeout)
		error = tr("Time is out");
	else if (status == CONTACT_OPER_ERROR)
		error = tr("Incorrect data");
	else if (status == CONTACT_OPER_INTERR)
		error = tr("Internal error");
	else if (status == CONTACT_OPER_INVALID_INFO)
		error = tr("Invalid group name");
	else if (status == CONTACT_OPER_NO_SUCH_USER)
		error = tr("No such group");
	else if (status != CONTACT_OPER_SUCCESS)
		error = tr("Unknown error");
	
	if (!error.isEmpty())
	{
		emit renameGroupError(error);
		return;
	}

	Tasks::RenameGroup* task = qobject_cast<Tasks::RenameGroup*>(sender());
	
	ContactGroup* group = task->group();
	QString name = task->name();
	group->setName(name);

	emit updated();
}

void ContactList::setLastSmsFrom(QByteArray & number, Contact* c)
{
	qDebug() << "setLastSmsFrom" << number.right(11) << c->nickname();
	lastSmsFrom[number.right(11)] = c;
}

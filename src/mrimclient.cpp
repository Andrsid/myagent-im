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

#include <QDebug>

#include <QTextCodec>
#include <QFile>

#include "mrimclient.h"
#include "mrimclientprivate.h"

#include "mrimdatastream.h"
#include "proto.h"
#include "account.h"
#include "contact.h"
#include "contactlist.h"
#include "message.h"
#include "contactdata.h"

using namespace Proto;

MRIMClient::MRIMClient(Account* a)
	: QObject(a)
{
	p = new MRIMClientPrivate(a, this);
}

MRIMClient::~MRIMClient()
{
	qDebug() << "MRIMClient::~MRIMClient()";
}

void MRIMClient::connectToServer(quint32 status)
{
	if (status == STATUS_OFFLINE || p->currentStatus != STATUS_OFFLINE)
		return;
	
	p->newStatus = status;
	p->manualDisconnect = false;
	
	if (!p->gettingAddress)
		p->socket.connectToHost("mrim.mail.ru"/*"194.67.23.154"*/, 2042);

	p->gettingAddress = true;
}

void MRIMClient::disconnectFromServer()
{
	qDebug() << "void MRIMClient::disconnectFromServer()";
	p->manualDisconnect = true;
	p->socket.abort();
}

quint32 MRIMClient::requestContactInfo(QByteArray email)
{
	qDebug() << "requesting info";
	
	QByteArray user = p->getLogin(email);
	QByteArray domain = p->getDomain(email);
	
	QByteArray data;
	MRIMDataStream out(&data, QIODevice::WriteOnly);
	
	out << quint32(MRIM_CS_WP_REQUEST_PARAM_USER);
	out << user;
	out << quint32(MRIM_CS_WP_REQUEST_PARAM_DOMAIN);
	out << domain;
	
	return p->sendPacket(MRIM_CS_WP_REQUEST, data);
}


quint32 MRIMClient::searchContacts(const SearchParams& params)
{
	QList<quint32> keys = params.keys();
	
	if (keys.contains(MRIM_CS_WP_REQUEST_PARAM_USER) ||
		keys.contains(MRIM_CS_WP_REQUEST_PARAM_DOMAIN))
		return 0;
	
	QByteArray data;
	MRIMDataStream out(&data, QIODevice::WriteOnly);
	
	QList<quint32>::const_iterator it = keys.begin();
	for (; it != keys.end(); ++it)
	{
		if (*it != MRIM_CS_WP_REQUEST_PARAM_ONLINE)
		{
			out << *it;
			out << params.value(*it);
		}
	}
	if (keys.contains(MRIM_CS_WP_REQUEST_PARAM_ONLINE))
	{
		out << quint32(MRIM_CS_WP_REQUEST_PARAM_ONLINE);
		out << params.value(MRIM_CS_WP_REQUEST_PARAM_ONLINE);
	}
	
	return p->sendPacket(MRIM_CS_WP_REQUEST, data);
}

void MRIMClient::changeStatus(quint32 status)
{
	qDebug() << "MRIMClient::changeStatus, status = " << status;
	if (status == STATUS_OFFLINE || status == STATUS_UNDETERMINATED)
		return;
	
	QByteArray data;
	MRIMDataStream out(&data, QIODevice::WriteOnly);
	
	out << quint32(status);
	p->sendPacket(MRIM_CS_CHANGE_STATUS, data);
	p->currentStatus = status;
}

quint32 MRIMClient::sendMessage(QByteArray email, const Message* message)
{
	QByteArray baText = p->codec->fromUnicode(message->plainText());
	QByteArray packedRtf = p->packRtf(message->rtfText());
	
	QByteArray data;
	MRIMDataStream out(&data, QIODevice::WriteOnly);

	out << quint32(message->flags());
	out << email;
	out << baText;
	out << packedRtf;

	qDebug() << "MRIMClient::sendMessage" << data.toHex();
	
	return p->sendPacket(MRIM_CS_MESSAGE, data);
}

quint32 MRIMClient::askAuthorization(const QByteArray& email, const QString& message)
{
	qDebug() << "MRIMClient askAuthorization for " << email;
	
	QByteArray data;
	MRIMDataStream out(&data, QIODevice::WriteOnly);
	
	QByteArray packedMessage64 = p->packAuthorizationMessage(message);
	
	out << quint32(MESSAGE_FLAG_NORECV | MESSAGE_FLAG_AUTHORIZE);
	out << email;
	out << packedMessage64;
	out << quint32(0);
	
	qDebug() << data.toHex();
	
	return p->sendPacket(MRIM_CS_MESSAGE, data);
}

quint32 MRIMClient::sendRtfMessage(QByteArray email, QString text, QByteArray rtf)
{
	QByteArray baText = p->codec->fromUnicode(text);
	
	qDebug() << "MRIMClient send message: " << QString::fromAscii(rtf);
	
	QByteArray packedRtf = p->packRtf(rtf);
	
	QByteArray data;
	MRIMDataStream out(&data, QIODevice::WriteOnly);

	out << quint32(MESSAGE_FLAG_RTF);
	out << email;
	out << baText;
	out << packedRtf;

	qDebug() << "MRIMClient::sendRtfMessage" << data.toHex();
	
	return p->sendPacket(MRIM_CS_MESSAGE, data);
}

void MRIMClient::sendTyping(QByteArray email)
{
	qDebug() << "send typing " << email;
	QByteArray data;
	MRIMDataStream out(&data, QIODevice::WriteOnly);
	
	out << quint32(MESSAGE_FLAG_NORECV | MESSAGE_FLAG_NOTIFY);
	out << email;
	out << QByteArray(" ");
	out << QByteArray(" ");
	
	p->sendPacket(MRIM_CS_MESSAGE, data);
}

quint32 MRIMClient::changeContactGroup(quint32 groupID, Contact* c)
{
	qDebug() << "changeContactGroup" << c->nickname() << ", " << groupID;
	qDebug() << c->email() << " " << QString::number(groupID, 16) << "id = " << c->id();
	
	QByteArray data;
	MRIMDataStream out(&data, QIODevice::WriteOnly);
	ContactData contactData = c->contactData();
	contactData.group = groupID;
	contactData.prepareForSending(out);
	
	return p->sendPacket(MRIM_CS_MODIFY_CONTACT, data);
}

quint32 MRIMClient::changeContactPhones(Contact* contact, const QStringList & phones)
{
	qDebug() << "changeContactPhones" << contact->nickname() << ", " << phones;
	
	QByteArray data;
	MRIMDataStream out(&data, QIODevice::WriteOnly);
	ContactData contactData = contact->contactData();
	contactData.phones = phones;
	contactData.prepareForSending(out);
	
	return p->sendPacket(MRIM_CS_MODIFY_CONTACT, data);
}

quint32 MRIMClient::renameContact(const QString& nickname, Contact * contact)
{
	qDebug() << "renameContact" << contact->nickname() << " to " << nickname;
	qDebug() << contact->email() << "id = " << contact->id();

	QByteArray data;
	MRIMDataStream out(&data, QIODevice::WriteOnly);
	ContactData contactData = contact->contactData();
	contactData.nick = nickname;
	contactData.prepareForSending(out);
	
	return p->sendPacket(MRIM_CS_MODIFY_CONTACT, data);
}

quint32 MRIMClient::setVisibility(bool alwaysVisible, bool alwaysInvisible, Contact* contact)
{
	quint32 newFlags = contact->flags();
	
	if (alwaysVisible)
		newFlags |= CONTACT_FLAG_VISIBLE;
	else
		newFlags &= ~CONTACT_FLAG_VISIBLE;
	
	if (alwaysInvisible)
		newFlags |= CONTACT_FLAG_INVISIBLE;
	else
		newFlags &= ~CONTACT_FLAG_INVISIBLE;
	
	if (newFlags == contact->flags())
		return 0;
	
	QByteArray data;
	MRIMDataStream out(&data, QIODevice::WriteOnly);
	ContactData contactData = contact->contactData();
	contactData.flags = newFlags;
	contactData.prepareForSending(out);
	
	return p->sendPacket(MRIM_CS_MODIFY_CONTACT, data);
}

quint32 MRIMClient::addContact(quint32 group, const QString& nickname, const QByteArray& email, const QString& authorizationMessage)
{
	qDebug() << "addContact" << group << ", " << nickname << ", " << email;
	
	QByteArray baNick = p->codec->fromUnicode(nickname);
	
	QByteArray baAuthorizationMessage("");
	if (!authorizationMessage.isEmpty())
		baAuthorizationMessage = p->packAuthorizationMessage(authorizationMessage);
	
	QByteArray data;
	MRIMDataStream out(&data, QIODevice::WriteOnly);
	
	out << quint32(0);
	out << quint32(group);
	out << email;
	out << baNick;
	out << QByteArray(""); //must be empty
	out << baAuthorizationMessage; // packed authorization message
	out << quint32(0); // 0?
	
	return p->sendPacket(MRIM_CS_ADD_CONTACT, data);
}

quint32 MRIMClient::addSmsContact(const QString & nickname, const QStringList & phones)
{
	qDebug() << "addSmsContact" << nickname << ", " << phones;
	
	QByteArray baNick = p->codec->fromUnicode(nickname);
	
	QByteArray baAuthorizationMessage("");
	
	QStringList tmpPhones = phones;
	tmpPhones.replaceInStrings("+", "");
	QByteArray baPhones = p->codec->fromUnicode(tmpPhones.join(","));

	QByteArray data;
	MRIMDataStream out(&data, QIODevice::WriteOnly);
	
	out << quint32(CONTACT_FLAG_SMS);
	out << quint32(SMS_CONTACT_GROUP);
	out << QByteArray("phone");
	out << baNick;
	out << baPhones;
	out << baAuthorizationMessage; // packed authorization message
	out << quint32(0); // 0?
	
	return p->sendPacket(MRIM_CS_ADD_CONTACT, data);
}

quint32 MRIMClient::addGroup(QString& name)
{
	qDebug() << "addGroup" << name;
	
	quint32 nGroups = p->account->contactList()->nGroups();
	nGroups <<= 24;
	
	qDebug() << "nGroups = " << QString::number(nGroups, 16);

	QByteArray baName = p->codec->fromUnicode(name);

	QByteArray data;
	MRIMDataStream out(&data, QIODevice::WriteOnly);
	
	out << quint32(CONTACT_FLAG_GROUP | nGroups);
	out << quint32(0);
	out << baName;
	out << quint32(0);
	out << quint32(0);
	out << quint32(0);
	
	qDebug() << "MRIMClient::addGroup data = " << data.toHex();

	return p->sendPacket(MRIM_CS_ADD_CONTACT, data);
}

quint32 MRIMClient::removeContact(Contact* c)
{
	qDebug() << "removeContact" << c->email();
	qDebug() << c->email() << " id = " << c->id();
	
	QByteArray data;
	MRIMDataStream out(&data, QIODevice::WriteOnly);
	ContactData contactData = c->contactData();
	contactData.flags |= CONTACT_FLAG_REMOVED;
	contactData.prepareForSending(out);
	
	qDebug() << data.toHex();

	return p->sendPacket(MRIM_CS_MODIFY_CONTACT, data);
}

quint32 MRIMClient::removeGroup(ContactGroup * group)
{
	qDebug() << "removeGroup" << group->name();
	
	QByteArray baName = p->codec->fromUnicode(group->name());

	QByteArray data;
	MRIMDataStream out(&data, QIODevice::WriteOnly);
	
	out << quint32(group->id());
	out << quint32(group->flags() | CONTACT_FLAG_REMOVED);
	out << quint32(0);
	out << baName;
	out << quint32(0);
	out << quint32(0);
	
	qDebug() << data.toHex();

	return p->sendPacket(MRIM_CS_MODIFY_CONTACT, data);
}

void MRIMClient::authorizeContact(const QByteArray & email)
{
	QByteArray data;
	MRIMDataStream out(&data, QIODevice::WriteOnly);

	out << email;

	p->sendPacket(MRIM_CS_AUTHORIZE, data);
}

quint32 MRIMClient::getMPOPSession()
{
	return p->sendPacket(MRIM_CS_GET_MPOP_SESSION, "");
}

quint32 MRIMClient::sendSms(QByteArray number, const QString & text)
{
	QByteArray data;
	MRIMDataStream out(&data, QIODevice::WriteOnly);
	
	if (!number.startsWith('+'))
		number.prepend('+');
	QByteArray smsText = p->codec->fromUnicode(text);
	
	out << quint32(0);
	out << number;
	out << smsText;
	
	return p->sendPacket(MRIM_CS_SMS, data);
}

quint32 MRIMClient::renameGroup(ContactGroup* group, QString name)
{
	qDebug() << "renameGroup" << group->name() << "to" << name;
	
	QByteArray baName = p->codec->fromUnicode(name);

	QByteArray data;
	MRIMDataStream out(&data, QIODevice::WriteOnly);
	
	out << quint32(group->id());
	out << quint32(group->flags());
	out << quint32(0);
	out << baName;
	out << quint32(0);
	out << quint32(0);
	
	qDebug() << data.toHex();

	return p->sendPacket(MRIM_CS_MODIFY_CONTACT, data);
}

Account * MRIMClient::account()
{
	return p->account;
}

uint MRIMClient::getPingTime() const
{
	return p->pingTime;
}
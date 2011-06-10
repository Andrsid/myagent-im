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

#ifndef CHATSESSION_H
#define CHATSESSION_H

#include <QObject>
#include <QList>

#include "message.h"
#include "filemessage.h"
#include "tasksendmessage.h"
#include "taskbroadcastmessage.h"

class Account;
class Contact;

class ChatSession : public QObject
{
Q_OBJECT
public:
	ChatSession(Account* account, Contact* contact);
	~ChatSession();

	Account* account() { return m_account; }
	Contact* contact() { return m_contact; }

	typedef QList<QByteArray> ReceiversList;
	typedef QHash<quint32, Message*>::const_iterator MessageIterator;
	typedef QHash<Message*, ReceiversList>::const_iterator BroadcastMessageIterator;
	MessageIterator messagesBegin() const { return messages.begin(); }
	MessageIterator messagesEnd() const { return messages.end(); }
	BroadcastMessageIterator broadcastMessagesBegin() const { return broadcastMessages.begin(); }
	BroadcastMessageIterator broadcastMessagesEnd() const { return broadcastMessages.end(); }

signals:
	void messageAppended(const Message* msg) const;
	void messageDelivered(bool really, Message* msg);
	void smsDelivered(QByteArray phoneNumber, QString text);
	void smsFailed();
	void signalFileReceived(quint32 totalSize, quint32 sessionId, QByteArray filesAnsi, QString filesUtf, QByteArray ips);
	void microblogChanged(QString text, QDateTime mbDateTime);

public slots:	
	void appendMessage(Message* msg, bool addInHash = true);
	void appendBroadcastMessage(Message* msg, ReceiversList rec, bool addInHash = true);
	void slotMicroblogChanged(QString text, QDateTime mbDateTime);
	bool sendMessage(QString plainText, QByteArray rtf);
	bool sendMessage(Message* msg);
	bool broadcastMessage(ReceiversList receivers, QString plainText, QByteArray rtf);
	bool broadcastMessage(Message* msg, ReceiversList receivers);
	bool sendSms(QByteArray number, QString text);
	void sendTyping();
	bool wakeupContact();
	//bool fileTransfer(FileMessage* fmsg);
	void fileReceived(quint32 totalSize, quint32 sessionId, QByteArray filesAnsi, QString filesUtf, QByteArray ips);
	void resendMessage(quint32 id);
	void clearHash();

private slots:
	void slotMessageStatus(quint32 status, bool timeout);
	void slotBroadcastMessageStatus(quint32 status, bool timeout);
	void slotSmsStatus(quint32 status, bool timeout);
	
private:
	Account* m_account;
	Contact* m_contact;
	QHash<quint32, Message*> messages;
	QHash<Message*, ReceiversList> broadcastMessages;
	quint32 numering;
	
	QTime typingTime;
	QTime awakeDelay;
};

#endif

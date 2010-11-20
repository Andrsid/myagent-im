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

#include "mrimclientprivate.h"

#include <QDebug>

#include <QUrl>
#include <QTextCodec>

#include "account.h"
#include "message.h"
#include "filemessage.h"
#include "zlibbytearray.h"
#include "mrimclient.h"
#include "datetime.h"
#include "mrimmime.h"

MRIMClientPrivate::MRIMClientPrivate(Account* a, MRIMClient* parent)
	: QObject(parent), account(a)
{
	q = parent;
	init();
	codec = QTextCodec::codecForName("cp1251");
}

MRIMClientPrivate::~MRIMClientPrivate()
{
}

void MRIMClientPrivate::init()
{
	pingTimer = new QTimer(this);
	pingTime = 30000;

	currentStatus = (OnlineStatus::offline).id();

	sequence = 1;
	headerReceived = false;
	dataSize = 0;

	manualDisconnect = false;

	gettingAddress = false;

	connect(&socket, SIGNAL(connected()), this, SLOT(slotConnectedToServer()));
	connect(&socket, SIGNAL(disconnected()), this, SLOT(slotDisconnectedFromServer()));
	connect(pingTimer, SIGNAL(timeout()), this, SLOT(ping()));
	connect(&socket, SIGNAL(readyRead()), this, SLOT(readData()));
	connect(&socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(slotSocketStateChanged(QAbstractSocket::SocketState)));
	connect(&socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotSocketError(QAbstractSocket::SocketError)));
}

quint32 MRIMClientPrivate::sendPacket(quint32 msgtype, QByteArray data, quint32 protoVersionMinor)
{

/*
For developers! You can use protoVersionMinor for migrating packets version from old to newer.
Default version wrote in proto.h, protocol version at October 2010 = 0x16
*/

	qDebug() << "Send packet";
	QByteArray headerBA;
	MRIMDataStream header(&headerBA, QIODevice::WriteOnly);

	if (sequence == 0) sequence = 1;

	quint32 protoVersion = ((((quint32)(PROTO_VERSION_MAJOR))<<16)|(quint32)(protoVersionMinor));

	header << quint32(CS_MAGIC);
	header << quint32(protoVersion);
	header << quint32(sequence);
	header << quint32(msgtype);
	header << quint32(data.size());
	header << quint32(0);
	header << quint32(0);

	header << quint32(0);
	header << quint32(0);
	header << quint32(0);
	header << quint32(0);

	socket.write(headerBA);
	socket.write(data);

	return sequence++;
}

void MRIMClientPrivate::slotConnectedToServer()
{
	//qDebug() << "MRIMClientPrivate::slotConnectedToServer() currentStatus = " << currentStatus << ", newStatus = " << newStatus << ", gettingAddress = " << gettingAddress;

	if (currentStatus == (OnlineStatus::offline).id() && newStatus != (OnlineStatus::offline).id())
		if (!gettingAddress)
			sendPacket(MRIM_CS_HELLO, QByteArray(), 23);
}

void MRIMClientPrivate::slotDisconnectedFromServer()
{
	//qDebug() << "MRIMClientPrivate slotDisconnectedFromServer, currentStatus = " << currentStatus << ", manualDisconnect = " << manualDisconnect;
	if (currentStatus != (OnlineStatus::offline).id() || !gettingAddress || manualDisconnect)
	{
		pingTimer->stop();
		currentStatus = (OnlineStatus::offline).id();
		gettingAddress = false;
		manualDisconnect = false;
		emit q->disconnectedFromServer();
	}
}

void MRIMClientPrivate::slotSocketStateChanged(QAbstractSocket::SocketState state)
{
	qDebug() << "MRIMClientPrivate::slotSocketStateChanged" << state;
}

void MRIMClientPrivate::slotSocketError(QAbstractSocket::SocketError error)
{
	/*TODO: reconnect on error*/
	qDebug() << "MRIMClientPrivate::slotSocketEror" << error;
}

void MRIMClientPrivate::ping()
{
	sendPacket(MRIM_CS_PING, QByteArray(), 23);
}

void MRIMClientPrivate::readData()
{
	qDebug() << "Read data " << socket.bytesAvailable();

	if (gettingAddress)
	{
		QString address = socket.readLine();
		address = address.trimmed();
		QString host = address.section(':', 0, 0);
		uint port = address.section(':', 1, 1).toUInt();
		socket.disconnectFromHost();
		gettingAddress = false;
		qDebug() << "connecting to " << host << ':' << port;
		socket.connectToHost(host, port);

		return;
	}

	for(;;)
	{
		if (!headerReceived)
		{
			//qDebug() << "sizeof(mrim_packet_header_t) = " << sizeof(mrim_packet_header_t);
			if (socket.bytesAvailable() < sizeof(mrim_packet_header_t))
				break;

			messageHeader = socket.read(sizeof(mrim_packet_header_t));
			mrim_packet_header_t* packetHeader =
				reinterpret_cast<mrim_packet_header_t*>(messageHeader.data());

			qDebug() << "Packet Header:";
			qDebug() << messageHeader.toHex();
			dataSize = packetHeader->dlen;
			if (dataSize == 0)
				qDebug() << "!!! dataSize = 0";

			qDebug() << "dataSize from header = " << dataSize;

			headerReceived = true;
		}

		if (socket.bytesAvailable() < dataSize)
		{
			qDebug() << "socket.bytesAvailable = " << socket.bytesAvailable();
			qDebug() << "low bytes";
			break;
		}

		QByteArray data = socket.read(dataSize);
		processPacket(messageHeader, data);
		headerReceived = false;
	}
}

void MRIMClientPrivate::processPacket(QByteArray header, QByteArray data)
{
	qDebug() << "processPacket:";
	qDebug() << header.toHex();

	mrim_packet_header_t* headerData =
		reinterpret_cast<mrim_packet_header_t*>(header.data());
	quint32 msgtype = headerData->msg;
	quint32 msgseq  = headerData->seq;

	qDebug() << "Message#" << msgtype;
	qDebug() << "Message#" << QString::number(msgtype, 16);
	qDebug() << "Data size = " << data.size();
	switch (msgtype)
	{
		case MRIM_CS_HELLO_ACK:
			processHelloAck(data);
			break;
		case MRIM_CS_LOGIN_ACK:
			processLoginAcknowledged(data);
			break;
		case MRIM_CS_LOGIN_REJ:
			processLoginRejected(data);
			break;
		case MRIM_CS_USER_INFO:
			processUserInfo(data);
			break;
		case MRIM_CS_CONTACT_LIST2:
			processContactList2(data);
			break;
		case MRIM_CS_OFFLINE_MESSAGE_ACK:
			processOfflineMessageAck(data);
			break;
		case MRIM_CS_ANKETA_INFO:
			processAnketaInfo(data, msgseq);
			break;
		case MRIM_CS_USER_STATUS:
			processUserStatus(data);
			break;
		case MRIM_CS_MESSAGE_ACK:
			processMessageAck(data);
			break;
		case MRIM_CS_MESSAGE_STATUS:
			processMessageStatus(data, msgseq);
			break;
		case MRIM_CS_LOGOUT:
			processLogout(data);
			break;
		case MRIM_CS_CONNECTION_PARAMS:
			processConnectionParams(data);
			break;
		case MRIM_CS_MAILBOX_STATUS:
			processMailBoxStatus(data);
			break;
		case MRIM_CS_NEW_MAIL:
			processNewMail(data);
			break;
		case MRIM_CS_MODIFY_CONTACT_ACK:
			processModifyContactAck(data, msgseq);
			break;
		case MRIM_CS_ADD_CONTACT_ACK:
			processAddContactAck(data, msgseq);
			break;
		case MRIM_CS_AUTHORIZE_ACK:
			processAuthorizeAck(data);
			break;
		case MRIM_CS_FILE_TRANSFER:
			processFileTransfer(data);
			break;
		case MRIM_CS_FILE_TRANSFER_ACK:
			processFileTransferAck(data);
			break;
		case MRIM_CS_MPOP_SESSION:
			processMPOPSession(data, msgseq);
			break;
		case MRIM_CS_SMS_ACK:
			processSmsAck(data, msgseq);
			break;
		case MRIM_CS_PROXY:
			processProxy(data, msgseq);
			break;
		case MRIM_CS_PROXY_ACK:
			processProxyAck(data, msgseq);
			break;
		case MRIM_SC_MICROBLOG_TEXT:
			processMicroblogChanged(data);
			break;
		default:
			qDebug() << "unknown message";
	}
}

QByteArray MRIMClientPrivate::packRtf(QByteArray rtf)
{
	QByteArray pack;
	MRIMDataStream packStream(&pack, QIODevice::WriteOnly);

	packStream << quint32(2);
	packStream << rtf;
	packStream << quint32(4);
	packStream << quint32(0x00FFFFFF);

	QString uncompressError;
	QByteArray zip = ZlibByteArray::compress(pack, ZlibByteArray::Zlib, &uncompressError);
	if (!uncompressError.isEmpty())
		qDebug() << uncompressError;

	return zip.toBase64();
}

bool MRIMClientPrivate::unpackRtf(const QByteArray& packedRtf, QByteArray* rtf, quint32* backgroundColor)
{
	QByteArray tmp = QByteArray::fromBase64(packedRtf);
	QString uncompressError;
	QByteArray unc = ZlibByteArray::uncompress(tmp, ZlibByteArray::Zlib, &uncompressError);
	if (!uncompressError.isEmpty())
		qDebug() << uncompressError;

	MRIMDataStream rtfStream(unc);

	quint32 nLines;
	quint32 bcLPSLen;
	rtfStream >> nLines;
	rtfStream >> *rtf;
	rtfStream >> bcLPSLen >> *backgroundColor;
	qDebug() << "messageAck: lines = " << nLines;
	qDebug() << "messageAck: rtfText = " << *rtf;
	qDebug() << "messageAck: bcLPSLen = " << bcLPSLen;
	qDebug() << "messageAck: backColor = " << QString::number(*backgroundColor, 16);

	return true;
}

QByteArray MRIMClientPrivate::packAuthorizationMessage(const QString & message)
{
	QByteArray msg = codec->fromUnicode(message);
	QString nickname = account->nickname();
	QByteArray nick = codec->fromUnicode(nickname);

	QByteArray packedMessage;
	MRIMDataStream pack(&packedMessage, QIODevice::WriteOnly);

	pack << quint32(2);
	pack << nick;
	pack << msg;

	return packedMessage.toBase64();
}

void MRIMClientPrivate::unpackAuthorizationMessage(const QByteArray& message, QString& nickname, QString& unpackedMessage)
{
	QByteArray data = QByteArray::fromBase64(message);
	MRIMDataStream in(data);

	quint32 sz;
	QByteArray nick;
	QByteArray msg;

	in >> sz;
	in >> nick;
	in >> msg;

	nickname = codec->toUnicode(nick);
	unpackedMessage = codec->toUnicode(msg);
}

void MRIMClientPrivate::processHelloAck(QByteArray data)
{
	qDebug() << "MRIM_CS_HELLO_ACK, bytes = " << socket.bytesAvailable();
	MRIMDataStream in(data);

	quint32 pingPeriod;
	in >> pingPeriod;
	qDebug() << "ping period = " << pingPeriod;
	pingTime = pingPeriod*1000;
	pingTimer->start(pingTime);

	QByteArray ba;
	MRIMDataStream out(&ba, QIODevice::WriteOnly);

	out << QByteArray(account->email());
	out << QByteArray(account->password());
	qDebug() << "MRIMClientPrivate: processHelloAck, status = " << currentStatus;

	out << /*quint32(newStatus);*/quint32(0xffffffff);
	out << QByteArray("client=\"magent\" version=\"5.6\" build=\"3278\"");
	out << QByteArray("ru"); //TODO: must depends upon system locale

	out << quint32(16);
	out << quint32(1);
	out << QByteArray("geo-list");
	out << QByteArray("MRA 5.6 (build 3278);");

	quint8 i; //TODO: try to understand these arguments
	for (i = 0; i <= 0x5c; )
	{
		out << quint8(i);

		if (i == 9 || i == 0x1e || i == 0x2c)
			out << quint32(0x01000000);
		else
			out << quint32(0x02000000);

		if (i == 0)
			out << quint32(0x00000cce);
		else if (i == 1)
			out << quint32(4);
		else if (i == 2)
			out << quint32(8);
		else if (i == 3)
			out << quint32(0x7fffffff);
		else if (i == 4 || i == 6 || i == 7 || i == 0x2d || i == 0x2f)
			out << quint32(1);
		else if (i == 9)
			out << QByteArray::fromHex("465d43504049505840745e585c5e184b4c425d47594a");
		else if (i == 0x14)
			out << quint32(0x00000501);
		else if (i == 0x2c)
			out << QByteArray("7af505217f0fe1a344cddd5094cddd0b");
		else
			out << quint32(0);

		if (i == 3 || i == 4 || i == 0x1a || i == 0x54)
			i += 2;
		else if (i == 5)
			i--;
		else if (i == 0x2f)
			i = 0x51;
		else
			i++;
	}

	sendPacket(MRIM_CS_LOGIN3, ba, 21);
}

void MRIMClientPrivate::processUserInfo(QByteArray data)
{
	qDebug() << "MRIM_CS_USER_INFO, bytes = " << data.size();
	MRIMDataStream in(data);

	QString totalMessages, unreadMessages, nick, statusText;
	QByteArray param;
	QString descr;

	do
	{
		in >> param >> descr;

		if (param == "MESSAGES.TOTAL")
		{
			totalMessages = descr;
			account->setTotalMessages(descr);
		}
		else if (param == "MESSAGES.UNREAD")
		{
			unreadMessages = descr;
			account->setUnreadMessages(descr);
		}
		else if (param == "MRIM.NICKNAME")
		{
			nick = descr;
			account->setNickName(descr);
		}
		else if (param == "micblog.status.text")
		{
			statusText = descr;
			account->setStatusText(descr);
		}
		else
			qDebug() << "something strange with user info";

		qDebug() << param << " " << descr;
	} while (in.device()->bytesAvailable());//while (param != "");

	
//		in >> param >> descr;
}

void MRIMClientPrivate::processContactList2(QByteArray data)
{
	qDebug() << "MRIM_CS_CONTACT_LIST2, bytes = " << data.size();

	ContactList* contactList = account->contactList();

	MRIMDataStream in(data);

	quint32 status;
	in >> status;
	qDebug() << "MRIM_CS_CONTACT_LIST2 status = " << status;
	if (status != GET_CONTACTS_OK) return;
	quint32 nGroups;
	in >> nGroups;
	qDebug() << "MRIM_CS_CONTACT_LIST2 ngroups = " << nGroups;
	QByteArray groupMask;
	in >> groupMask;
	qDebug() << "group mask = " << groupMask;
	QByteArray contactsMask;
	in >> contactsMask;
	qDebug() << "contacts mask = " << contactsMask;

	if (!contactsMask.startsWith(ContactData::dataMask()))
	{
		qDebug() << "wrong mask, returning";
		return;
	}

	contactList->beginUpdating();
	for (quint32 groupId = 0; groupId < nGroups; groupId++)
	{
		quint32 groupFlags;
		QString groupName;

		in >> groupFlags;
		in >> groupName;

		qDebug() << QString::number(groupFlags, 16) << " " << groupName;

		int nFlags = groupMask.size();
		quint32 uTmpAttr;
		QByteArray sTmpAttr;

		for (int i = 2; i < nFlags; i++)
		{
			char currFlag = groupMask.at(i);
			if (currFlag == 'u')
			{
				in >> uTmpAttr;
				qDebug() << QString::number(uTmpAttr, 16);
			}
			else if (currFlag == 's')
			{
				in >> sTmpAttr;
				qDebug() << sTmpAttr;
			}
			else
				qDebug() << "!!! strange mask flag: " << currFlag;
		}

		if (!(groupFlags & CONTACT_FLAG_REMOVED))
			contactList->addGroup(groupId, groupFlags, groupName);
	}

	quint32 id = 20;

	while (in.device()->bytesAvailable() != 0)
	{
		ContactData contactData(id, in, contactsMask);
		if (in.status() != QDataStream::Ok)
		{
			qDebug() << "constructing contact data error";
			contactList->clear();
			return;
		}
		id++;
		contactList->addContact(contactData);
	}

	contactList->endUpdating();

	q->changeStatus(newStatus);
}

void MRIMClientPrivate::processAnketaInfo(QByteArray data, quint32 msgseq)
{
	qDebug() << "MRIM_CS_ANKETA_INFO, bytes = " << data.size();
	MRIMDataStream in(data);

	MRIMClient::ContactsInfo info;

	quint32 status;
	in >> status;

	if (status != MRIM_ANKETA_INFO_STATUS_OK)
	{
		qDebug() << "Anketa status = " << status;
		emit q->contactInfoReceived(msgseq, status, info, 0, 0);
		return;
	}

	quint32 numFields, maxRows, serverTime;
	in >> numFields >> maxRows >> serverTime;

	qDebug() << "numFields = " << numFields << ", maxRows = " << maxRows << ", serverTime = " << serverTime;

	QVector<QString> fields;

	qDebug() << "Fields:";
	QByteArray str;
	for (uint i = 0; i < numFields; i++)
	{
		in >> str;
		fields.append(codec->toUnicode(str));
		qDebug() << codec->toUnicode(str);
	}

	qDebug() << "Values:";
	while (!in.atEnd())
		for (uint i = 0; i < numFields; i++)
		{
			in >> str;
			info[fields[i]] << codec->toUnicode(str);
			qDebug() << codec->toUnicode(str);
		}

	emit q->contactInfoReceived(msgseq, status, info, maxRows, serverTime);
}

void MRIMClientPrivate::processLoginAcknowledged(QByteArray data)
{
	qDebug() << "MRIM_CS_LOGIN_ACK, bytes = " << data.size();
	qDebug() << "statuses:" << currentStatus << newStatus;
	currentStatus = newStatus;
	emit q->loginAcknowledged(currentStatus);
}

void MRIMClientPrivate::processLoginRejected(QByteArray data)
{
	qDebug() << "MRIM_CS_LOGIN_REJ, bytes = " << data.size();
	MRIMDataStream in(data);

	QByteArray ba;
	in >> ba;
	QString reason = codec->toUnicode(ba);
	emit q->loginRejected(reason);
}

void MRIMClientPrivate::processLogout(QByteArray data)
{
	qDebug() << "MRIM_CS_LOGOUT, bytes = " << data.size();
	MRIMDataStream in(data);

	quint32 reason;

	in >> reason;
	qDebug() << "logout reason = " << reason;

	emit q->loggedOut(reason);
}

void MRIMClientPrivate::processConnectionParams(QByteArray data)
{
	qDebug() << "MRIM_CS_CONNECTION_PARAMS, bytes = " << data.size();
	MRIMDataStream in(data);

	quint32 pingPeriod;

	in >> pingPeriod;

	qDebug() << "ping period = " << pingPeriod;
	pingTime = pingPeriod*1000;
	pingTimer->setInterval(pingTime);
}

void MRIMClientPrivate::processMailBoxStatus(QByteArray data)
{
	qDebug() << "MRIM_CS_MAILBOX_STATUS, bytes = " << data.size();
	MRIMDataStream in(data);

	quint32 unreadMessages;

	in >> unreadMessages;

	qDebug() << "mailbox status: unreadMessages = " << unreadMessages;

	emit q->newNumberOfUnreadLetters(unreadMessages);
}

void MRIMClientPrivate::processNewMail(QByteArray data)
{
	qDebug() << "MRIM_CS_NEW_MAIL, bytes = " << data.size();
	MRIMDataStream in(data);

	quint32 unreadMessages;

	in >> unreadMessages;
	qDebug() << "mailbox new mail: unreadMessages = " << unreadMessages;

	QByteArray baSender, baSubject;
	quint32 unixTime;
	in >> baSender >> baSubject >> unixTime;

	emit q->newNumberOfUnreadLetters(unreadMessages);
	emit q->newLetter(codec->toUnicode(baSender), codec->toUnicode(baSubject), QDateTime::fromTime_t(unixTime));
}

void MRIMClientPrivate::processUserStatus(QByteArray data)
{
	qDebug() << "MRIM_CS_USER_STATUS, bytes = " << data.size();
	MRIMDataStream in(data);

	quint32 status;
	QString statusDescr;
	QByteArray statusId, email, ver;
	quint32 unk2, unk3;
	in >> status >> statusId >> statusDescr >> unk2 >> email >> unk3 >> ver;
	qDebug() << "status:" << status << statusId << statusDescr;
	qDebug() << "unks:" << unk2 << unk3;
	qDebug() << "email = " << email;

	OnlineStatus userStatus(statusId, statusDescr);
	emit q->contactStatusChanged(userStatus, email);
}

void MRIMClientPrivate::processMessageAck(QByteArray data)
{
	qDebug() << "MRIM_CS_MESSAGE_ACK, bytes = " << data.size();

	MRIMDataStream in(data);

	quint32 msgId, flags, conferenceType;
	QByteArray from, text, confOwner, rtf;
	QString confName;
	in >> msgId >> flags >> from >> text >> rtf;

	qDebug() << from;

	QByteArray(data2);
	in >> data2;
	MRIMDataStream in2(data2);
	in2 >> conferenceType >> confName;
	qDebug() << "confName" << confName;
	in2 >> confOwner;

	qDebug() << "flags = " << QString::number(flags, 16);

	QString plainText;
	plainText = codec->toUnicode(text);

	QByteArray rtfText;
	quint32 backgroundColor = 0x00FFFFFF;

	if (rtf != "")
		if (flags & MESSAGE_FLAG_RTF)
			unpackRtf(rtf, &rtfText, &backgroundColor);

	if (flags & MESSAGE_FLAG_CONFERENCE && rtf == "" && plainText == "" && conferenceType == 5)
	{
		plainText = tr("User %1 left the conference").arg(codec->toUnicode(confOwner));
		flags = flags & 0xffffff7f;
	}

	Message* newMsg = new Message(Message::Incoming, flags, plainText, rtfText, backgroundColor);

	if (flags & MESSAGE_FLAG_CONFERENCE && rtf != "")
		if (!account->contactList()->findContact(from))
			emit q->conferenceAsked(from, confName);

	if (!(flags & MESSAGE_FLAG_NORECV))
	{
		QByteArray replyData;
		MRIMDataStream out(&replyData, QIODevice::WriteOnly);

		if (flags & MESSAGE_FLAG_SMS)
			out << QByteArray("mrim_sms@mail.ru");
		else
			out << from;
		out << msgId;
		sendPacket(MRIM_CS_MESSAGE_RECV, replyData);
	}

	if (flags & MESSAGE_FLAG_NOTIFY)
	{
		qDebug() << "--- contact typing ---";
		emit q->contactTyping(from);
		return;
	}

	if (flags & MESSAGE_FLAG_AUTHORIZE)
	{
		QString nickname, message;
		unpackAuthorizationMessage(text, nickname, message);

		emit q->contactAsksAuthorization(from, nickname, message);

		return;
	}

	if (flags & (MESSAGE_FLAG_SMS | MESSAGE_SMS_DELIVERY_REPORT))
	{
		emit q->messageReceived
		(
			from, new Message(Message::Incoming, flags, plainText, from, 0x00FFFFFF)
		);
		return;
	}

	emit q->messageReceived(from, newMsg);
}

void MRIMClientPrivate::processMessageStatus(QByteArray data, quint32 msgseq)
{
	qDebug() << "MRIM_CS_MESSAGE_STATUS, bytes = " << data.size();
	MRIMDataStream in(data);

	quint32 st;
	in >> st;
	emit q->messageStatus(msgseq, st);
}

void MRIMClientPrivate::processModifyContactAck(QByteArray data, quint32 msgseq)
{
	qDebug() << "MRIM_CS_MODIFY_CONTACT_ACK, bytes = " << data.size();
	MRIMDataStream in(data);

	quint32 status;
	in >> status;
	qDebug() << "seq = " << msgseq << ", status = " << status;
	emit q->contactModified(msgseq, status);
}

void MRIMClientPrivate::processAddContactAck(QByteArray data, quint32 msgseq)
{
	qDebug() << "MRIM_CS_ADD_CONTACT_ACK, bytes = " << data.size();
	MRIMDataStream in(data);

	quint32 status;
	quint32 contactId;
	QByteArray chatAgent;
	in >> status;
	in >> contactId;
	if (!in.atEnd())
		in >> chatAgent;

	qDebug() << "seq = " << msgseq << ", status = " << status << ", id = " << contactId << ", chatAgent = " << chatAgent;

	if (chatAgent != "")
		if (chatAgent.contains("@chat.agent"))
			emit q->conferenceBegan(msgseq, status, contactId, chatAgent);
		else
			emit q->contactAdded(msgseq, status, contactId);
	else
		emit q->contactAdded(msgseq, status, contactId);
}

void MRIMClientPrivate::processAuthorizeAck(QByteArray data)
{
	qDebug() << "MRIM_CS_AUTHORIZE_ACK, bytes = " << data.size();
	MRIMDataStream in(data);

	QByteArray email;
	in >> email;

	emit q->contactAuthorizedMe(email);
}

void MRIMClientPrivate::processOfflineMessageAck(QByteArray data)
{
	qDebug() << "MRIM_CS_OFFLINE_MESSAGE_ACK, bytes = " << data.size();
	MRIMDataStream in(data);

	quint64 uidl;
	QByteArray msg;
	in >> uidl;
	in >> msg;

	MrimMIME mimeMsg(msg);

	/*qDebug() << mimeMsg.from();
	qDebug() << mimeMsg.sender();
	qDebug() << mimeMsg.subject();
	qDebug() << mimeMsg.plainTextCharset();
	qDebug() << mimeMsg.xMrimMultichatType();*/

	qDebug() << mimeMsg.xMrimFlags() << MESSAGE_FLAG_NORECV << (mimeMsg.xMrimFlags() & MESSAGE_FLAG_NORECV);

	QString plainText;
	if (mimeMsg.hasPlainText())
	{
		QTextCodec* codec;
		if (mimeMsg.plainTextCharset() == "" || mimeMsg.xMrimFlags() & MESSAGE_FLAG_CP1251)
			codec = QTextCodec::codecForName("cp1251");
		else
			codec = QTextCodec::codecForName(mimeMsg.plainTextCharset());
		plainText = codec->toUnicode(mimeMsg.plainText());
	}

	QByteArray rtfText;
	quint32 bgColor;
	if (mimeMsg.hasRtfText() && mimeMsg.xMrimFlags() & MESSAGE_FLAG_RTF)
		unpackRtf(mimeMsg.rtfBase64(), &rtfText, &bgColor);

	qDebug() << rtfText;

	if (mimeMsg.xMrimFlags() & MESSAGE_FLAG_CONFERENCE && mimeMsg.xMrimMultichatType() == 5)
		plainText = tr("User %1 left the conference").arg(codec->toUnicode(mimeMsg.sender()));

	Message* newMsg = new Message(Message::Incoming, mimeMsg.xMrimFlags(), plainText, rtfText, bgColor, mimeMsg.dateTime());

	if (mimeMsg.xMrimFlags() & MESSAGE_FLAG_CONFERENCE && mimeMsg.xMrimMultichatType() != 7)
		if (!account->contactList()->findContact(mimeMsg.from()))
			emit q->conferenceAsked(mimeMsg.from(), mimeMsg.subject());

/*	if (!(mimeMsg.xMrimFlags() & MESSAGE_FLAG_NORECV))
	{*/
		qDebug() << "sending reply";

		QByteArray replyData;
		MRIMDataStream out(&replyData, QIODevice::WriteOnly);

		out << uidl;
		sendPacket(MRIM_CS_DELETE_OFFLINE_MESSAGE, replyData);
//	}

	if (mimeMsg.xMrimFlags() & MESSAGE_FLAG_AUTHORIZE)
	{
		QString nickname, message;
		unpackAuthorizationMessage(mimeMsg.rtfBase64(), nickname, message);

		emit q->contactAsksAuthorization(mimeMsg.from(), nickname, message);

		return;
	}

	if (mimeMsg.xMrimFlags() & MESSAGE_FLAG_CONFERENCE && mimeMsg.xMrimMultichatType() == 7)
		return;

	emit q->messageReceived(mimeMsg.from(), newMsg);
}

void MRIMClientPrivate::processFileTransfer(QByteArray data)
{
	qDebug() << "MRIM_CS_FILE_TRANSFER, bytes = " << data.size();
	MRIMDataStream in(data);

	QByteArray email;
	quint32 sessionId;
	quint32 totalSize;
	QByteArray lps1;

	in >> email >> sessionId >> totalSize;
	qDebug() << "email = " << email << ", sessionId = " << sessionId << ", totalSize = " << totalSize;
	in >> lps1;

	qDebug() << "lps1 = " << lps1;
	qDebug() << "lps1.toHex = " << lps1.toHex();

	MRIMDataStream in2(lps1);

	QByteArray filesAnsi;
	QByteArray lps2;
	QByteArray ips;

	in2 >> filesAnsi >> lps2 >> ips;

	MRIMDataStream in3(lps2);

	quint32 unk;
	QString filesUtf;

	in3 >> unk >> filesUtf;

	qDebug() << "filesAnsi = " << filesAnsi;
	qDebug() << "ips = " << ips;
	qDebug() << "unk = " << unk;
	qDebug() << "filesUtf = " << filesUtf;

	FileMessage* fmsg = new FileMessage(FileMessage::Incoming, q->account()->email(), email, totalSize, sessionId, filesAnsi, filesUtf, ips);
	connect(q, SIGNAL(proxy(QByteArray, quint32, quint32, QByteArray, QByteArray, quint32, quint32, quint32, quint32)), fmsg, SLOT(slotProxy(QByteArray, quint32, quint32, QByteArray, QByteArray, quint32, quint32, quint32, quint32)));
	connect(q, SIGNAL(fileTransferAck(quint32, QByteArray, quint32, QByteArray)), fmsg, SLOT(slotFileTransferStatus(quint32, QByteArray, quint32, QByteArray)));
	connect(q, SIGNAL(proxyAck(quint32, QByteArray, quint32, quint32, QByteArray, QByteArray, quint32, quint32, quint32, quint32)), fmsg, SLOT(slotProxyAck(quint32, QByteArray, quint32, quint32, QByteArray, QByteArray, quint32, quint32, quint32, quint32)));

	connect(fmsg, SIGNAL(fileAck(quint32, QByteArray, quint32, QByteArray)), q, SLOT(sendFileAck(quint32, QByteArray, quint32, QByteArray)));
	connect(fmsg, SIGNAL(proxyAck(FileMessage*, quint32, quint32, quint32, quint32, quint32, quint32)), q, SLOT(sendProxyAck(FileMessage*, quint32, quint32, quint32, quint32, quint32, quint32)));

	emit q->fileReceived(fmsg);
}

void MRIMClientPrivate::processFileTransferAck(QByteArray data)
{
	qDebug() << "MRIM_CS_FILE_TRANSFER_ACK, bytes = " << data.size();
	qDebug() << data.toHex();
	MRIMDataStream in(data);

	quint32 status;
	QByteArray email;
	quint32 sessionId;
	QByteArray mirrorIps;

	in >> status >> email >> sessionId >> mirrorIps;

	qDebug() << "status = " << status << ", email = " << email << ", sessionId = " << sessionId << ", mirrorIps = " << mirrorIps;

	emit q->fileTransferAck(status, email, sessionId, mirrorIps);
}

void MRIMClientPrivate::processMPOPSession(QByteArray data, quint32 msgseq)
{
	qDebug() << "MRIM_CS_MPOP_SESSION, bytes = " << data.size();
	MRIMDataStream in(data);

	quint32 status;
	QByteArray session;
	in >> status >> session;

	emit q->receivedMPOPSession(msgseq, status, session);
}

void MRIMClientPrivate::processSmsAck(QByteArray data, quint32 msgseq)
{
	qDebug() << "MRIM_CS_SMS_ACK, bytes = " << data.size();
	MRIMDataStream in(data);

	quint32 status;
	in >> status;

	qDebug() << "sms status = " << QString::number(status, 16);

	emit q->smsAck(msgseq, status);
}

void MRIMClientPrivate::processProxy(QByteArray data, quint32 msgseq)
{
	qDebug() << "MRIM_CS_PROXY, bytes = " << data.size();
	MRIMDataStream in(data);

	qDebug() << data.toHex();

	QByteArray email;
	quint32 idRequest;
	quint32 dataType;

	QByteArray filesAnsi;
	QByteArray proxyIps;
	quint32 sessionId;
	quint32 unk1, unk2, unk3;

	in >> email >> idRequest >> dataType >> filesAnsi >> proxyIps >> sessionId;
	in >> unk1 >> unk2 >> unk3;
	qDebug() << "email =" << email;
	qDebug() << "idRequest =" << idRequest;
	qDebug() << "dataType =" << dataType;
	qDebug() << "userData =" << filesAnsi;

	qDebug() << "lpsIpPort =" << proxyIps;
	qDebug() << "sessionId =" << sessionId;

	qDebug() << unk1 << unk2 << unk3;

	emit q->proxy(email, idRequest, dataType, filesAnsi, proxyIps, sessionId, unk1, unk2, unk3);
}

void MRIMClientPrivate::processProxyAck(QByteArray data, quint32 msgseq)
{
	qDebug() << "MRIM_CS_PROXY_ACK, bytes = " << data.size();
	MRIMDataStream in(data);

	qDebug() << data.toHex();

	quint32 status;
	QByteArray email;
	quint32 idRequest;
	quint32 dataType;
	QByteArray filesAnsi;
	QByteArray ips;
	quint32 sessionId;

	quint32 unk1, unk2, unk3, unk4, unk5, unk6, unk7, unk8;

	in >> status >> email >> idRequest >> dataType >> filesAnsi >> ips >> sessionId;
	in >> unk1 >> unk2 >> unk3 >> unk4 >> unk5 >> unk6 >> unk7 >> unk8;

	qDebug() << status;

	emit q->proxyAck(status, email, idRequest, dataType, filesAnsi, ips, sessionId, unk1, unk2, unk3);
}

void MRIMClientPrivate::processMicroblogChanged(QByteArray data)
{
	qDebug() << "MRIMClientPrivate::processMicroblogChanged";

	MRIMDataStream in(data);

	QString microText;

	in >> microText;

	emit q->microblogChanged(microText);
}

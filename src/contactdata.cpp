#include "contactdata.h"

#include <QDebug>

#include <QTextCodec>
#include <QVariant>

#include "proto.h"

QTextCodec* ContactData::codec = QTextCodec::codecForName("cp1251");

ContactData::ContactData()
{
	status = OnlineStatus::offline;
}

ContactData::ContactData(const QByteArray& email)
{
	id = -1;
	flags = 0;
	group = 0;
	this->email = email;
	nick = email;
	internalFlags = CONTACT_INTFLAG_NOT_AUTHORIZED;
	status = OnlineStatus::offline;
}

ContactData::ContactData(quint32 id, quint32 flags, quint32 group, QByteArray email, QString nick, QStringList phones)
{
	this->id = id;
	this->flags = flags;
	this->group = group;
	this->email = email;
	this->nick = nick;
	this->phones = phones;
	internalFlags = CONTACT_INTFLAG_NOT_AUTHORIZED;
	status = OnlineStatus::offline;
}

ContactData::ContactData(quint32 contactId, MRIMDataStream& stream, const QByteArray& mask)
{
	qDebug() << Q_FUNC_INFO;
	if (!mask.startsWith(dataMask()))
		return;

	id = contactId;

	QByteArray stringQB;

	stream >> flags;
	stream >> group;
	stream >> email;

	stream >> stringQB;
	QTextCodec* c = QTextCodec::codecForName("UTF-16");
	nick = c->toUnicode(stringQB);

	stream >> internalFlags;

	quint32 statusProtocol;
	stream >> statusProtocol;

	status = OnlineStatus::fromProtocolStatus(statusProtocol);
	QByteArray contactPhones;
	stream >> contactPhones;
	//phones = c->toUnicode(contactPhones)/*; QString(contactPhones)*/.split(',', QString::SkipEmptyParts);
	qDebug() << contactPhones.toHex();
	phones = QString(contactPhones).split(',', QString::SkipEmptyParts);

	for (int i = 0; i < phones.size(); i++)
	{
		if (!phones.at(i).startsWith('+'))
			phones[i].prepend('+');
	}

	QByteArray statusId;
	QString statusDescr;

	stream >> statusId;
	stream >> statusDescr;

	if (statusId != "")
	{
		qDebug() << "statusId" << statusId;
		status.setIdStatus(statusId);
		status.setDescr(statusDescr);
	}

	tailMask = mask.right(mask.size() - dataMask().size());

	int nFlags = tailMask.size();
	quint32 uData;
	QByteArray sData;
	
	for (int i = 0; i < nFlags; i++)
	{
		if (tailMask.at(i) == 'u')
		{
			stream >> uData;
			tailData.append(QVariant(uData));
			qDebug() << "u" << uData;
		}
		else if (tailMask.at(i) == 's')
		{
			stream >> sData;
			tailData.append(sData);
			qDebug() << "s" << sData;
		}
		else
			qDebug() << "!!! strange mask flag: " << mask.at(i);
	}

	qDebug() << "ContactData::ContactData " << id << flags << group << email << nick << internalFlags << phones << statusProtocol << statusId << statusDescr;
}

const QByteArray ContactData::dataMask()
{
	return "uussuusss";
}

void ContactData::save(QDataStream & stream) const
{
	stream << id;
	stream << flags;
	stream << group;
	stream << email;
	stream << nick;
	stream << internalFlags;
	stream << phones;
	stream << tailMask;
	stream << tailData;

	qDebug() << "ContactData::save " << id << flags << group << email << nick << internalFlags << phones;
}

void ContactData::load(QDataStream & stream)
{
	stream >> id;
	stream >> flags;
	stream >> group;
	stream >> email;
	stream >> nick;
	stream >> internalFlags;
	status = OnlineStatus::offline;
	stream >> phones;
	stream >> tailMask;
	stream >> tailData;

	qDebug() << "ContactData::load " << id << flags << group << email << nick << internalFlags << phones;
}

void ContactData::prepareForSending(MRIMDataStream & stream) const
{
	QTextCodec* c = QTextCodec::codecForName("UTF-16");
	stream << id;
	stream << flags;
	stream << group;
	stream << email;
	stream << c->fromUnicode(nick);

	QStringList tmp = phones;
	tmp.replaceInStrings("+", "");

	stream << codec->fromUnicode(tmp.join(","));
	for (int i = 0; i < tailMask.size(); i++)
	{
		if (tailMask.at(i) == 'u')
			stream << tailData.at(i).toUInt();
		else if (tailMask.at(i) == 's')
			stream << tailData.at(i).toByteArray();
	}
}

bool ContactData::isConference() const
{
	return email.contains("@chat.agent");
}

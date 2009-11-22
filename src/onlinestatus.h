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

#ifndef ONLINESTATUS_H
#define ONLINESTATUS_H

#include <QCoreApplication>
#include <QIcon>
#include <QVariant>

class OnlineStatus
{
	Q_DECLARE_TR_FUNCTIONS(OnlineStatus)
public:
	enum StatusType
	{
		Unknown,
		Offline,
		Invisible,
		Away,
		Online,
		Connecting,
		Unauthorized
	};
	
	static const OnlineStatus unknown;
	static const OnlineStatus offline;
	static const OnlineStatus invisible;
	static const OnlineStatus away;
	static const OnlineStatus online;
	static const OnlineStatus connecting;
	static const OnlineStatus unauthorized;

	StatusType type() const { return m_type; }
	QString description() const;
	quint32 protocolStatus() const;
	static OnlineStatus fromProtocolStatus(quint32 st);
	QIcon contactListIcon() const;
	QIcon chatWindowIcon() const;
	bool connected() const;
	
	explicit OnlineStatus(StatusType type = Unknown);
	
	bool operator==(OnlineStatus another) { return (m_type == another.m_type); }
	bool operator!=(OnlineStatus another) { return (m_type != another.m_type); }
	operator QVariant() const { return static_cast<int>(m_type); }
	OnlineStatus operator=(const QVariant& v) { m_type = static_cast<StatusType>(v.toInt()); return *this; }
private:
	StatusType m_type;
};

#endif

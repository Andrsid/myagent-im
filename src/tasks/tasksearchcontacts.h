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

#ifndef TASKSEARCHCONTACTS_H
#define TASKSEARCHCONTACTS_H

#include <QList>

#include "simpleblockingtask.h"
#include "protocol/mrim/mrimclient.h"
#include "contactinfo.h"

namespace Tasks
{

class SearchContacts : public SimpleBlockingTask<Tasks::SearchContacts>
{
Q_OBJECT
public:
	SearchContacts(const QByteArray& email, MRIMClient* client, QObject* parent = 0);
	SearchContacts(const MRIMClient::SearchParams& params, MRIMClient* client, QObject* parent = 0);
	
	bool exec();
	QList<ContactInfo> getContactsInfo() const { return contactsInfo; }
	quint32 getMaxRows() const { return m_maxRows; }

private slots:
	void checkResult(quint32 msgseq, quint32 status, MRIMClient::ContactsInfo info, quint32 maxRows, quint32 serverTime);

private:
	QByteArray m_email;
	MRIMClient::SearchParams m_params;
	quint32 m_maxRows;
	QList<ContactInfo> contactsInfo;
};

}

#endif

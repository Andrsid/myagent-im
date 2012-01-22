/***************************************************************************
 *   Copyright (C) 2011 by Dmitry Malakhov <abr_mail@mail.ru>              *
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

#ifndef CONTACTLISTBROADCAST_H
#define CONTACTLISTBROADCAST_H

#include <QListView>

class ContactList;
class ContactListBroadcastModel;

class ContactListBroadcast : public QListView
{
	Q_OBJECT
public:
	ContactListBroadcast(QByteArray contactEmail, ContactList* cl, QWidget* parent = 0);
	~ContactListBroadcast();

	QList<QByteArray> & receivers();

private:
	ContactList* m_contactList;
	ContactListBroadcastModel* broadcastModel;
};

#endif // CONTACTLISTBROADCAST_H

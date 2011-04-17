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

#ifndef TASKBROADCASTMESSAGE_H
#define TASKBROADCASTMESSAGE_H

#include "task.h"

#include <QSet>
#include <QTimer>

class Contact;
class Message;

namespace Tasks
{

class BroadcastMessage : public Task
{
Q_OBJECT
public:
	BroadcastMessage(QList<QByteArray> receivers, Message* m, MRIMClient* client, QObject *parent = 0);
	~BroadcastMessage();

	bool exec();
	Message* getMessage() { return message; }
	QList<QByteArray> getReceivers() { return m_receivers; }
protected slots:
	virtual void timeout();
private slots:
	void checkResult(quint32 msgseq, quint32 status);
private:
	QList<QByteArray> m_receivers;
	Message* message;
	QTimer* tim;
};

}

#endif // TASKBROADCASTMESSAGE_H

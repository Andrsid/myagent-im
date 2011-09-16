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

#ifndef TASKSTASKSENDSMS_H
#define TASKSTASKSENDSMS_H

#include "simpleblockingtask.h"

namespace Tasks
{

class SendSms : public SimpleBlockingTask<Tasks::SendSms>
{
Q_OBJECT
public:
	SendSms(const QByteArray& number, const QString& text, MRIMClient* client, QObject* parent = 0);

	virtual bool exec();

	QByteArray phoneNumber() const { return m_number; }
	QString text() const { return m_text; }

private Q_SLOTS:
	void checkResult(quint32 msgseq, quint32 status);

private:
	QByteArray m_number;
	QString m_text;
};

}

#endif

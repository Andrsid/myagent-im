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
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef TASKSTASKADDGROUP_H
#define TASKSTASKADDGROUP_H

#include "simpleblockingtask.h"

namespace Tasks
{

class AddGroup : public Tasks::SimpleBlockingTask<Tasks::AddGroup>
{
Q_OBJECT
public:
	AddGroup(const QString& groupName, MRIMClient* client, QObject* parent = 0);

	virtual bool exec();

	QString groupName() const { return m_groupName; }
	quint32 groupId() const { return m_groupId; }
private Q_SLOTS:
	void checkResult(quint32 msgseq, quint32 status, quint32 groupId);
private:
	QString m_groupName;
	quint32 m_groupId;
};

}

#endif

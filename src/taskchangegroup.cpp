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

#include "taskchangegroup.h"

#include <QDebug>

#include "mrimclient.h"

namespace Tasks
{

ChangeGroup::ChangeGroup(Contact* c, quint32 g, MRIMClient* client, QObject *parent)
	: SimpleBlockingTask<Tasks::ChangeGroup>(client, parent), contact(c), group(g)
{
}

bool ChangeGroup::exec()
{
	if (!block())
		return false;
	
	connect(mc, SIGNAL(contactModified(quint32, quint32)), this, SLOT(checkResult(quint32, quint32)));
	
	return checkCall(mc->changeContactGroup(group, contact));
}

void ChangeGroup::checkResult(quint32 msgseq, quint32 status)
{
	if (isMyResponse(msgseq))
	{
		emit done(status, false);
		deleteLater();
	}
}

}

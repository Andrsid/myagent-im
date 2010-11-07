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

#include <QDebug>

#include "contactgroup.h"

ContactGroup::ContactGroup(quint32 id, quint32 flags, QString name, bool isConf)
	: m_id(id), m_flags(flags), m_name(name), m_isConf(isConf)
{
}

ContactGroup::ContactGroup(QDataStream& stream)
{
	stream >> m_id >> m_flags >> m_name >> m_isConf;
}

QDataStream & operator <<(QDataStream & stream, ContactGroup* group)
{
	stream << group->id() << group->flags() << group->name() << group->isConferences();
	return stream;
}

QDataStream & operator >>(QDataStream & stream, ContactGroup* group)
{
	stream >> group->m_id >> group->m_flags >> group->m_name >> group->m_isConf;
	return stream;
}

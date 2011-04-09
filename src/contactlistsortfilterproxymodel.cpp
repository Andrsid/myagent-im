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

#include "contactlistsortfilterproxymodel.h"

#include <QDebug>

#include "contactlistmodel.h"
#include "contact.h"
#include "proto.h"

ContactListSortFilterProxyModel::ContactListSortFilterProxyModel(QObject * parent)
	: QSortFilterProxyModel(parent), contactListModel(0)
{
	allowOnlineOnly = false;
	setDynamicSortFilter(true);
}

void ContactListSortFilterProxyModel::setSourceModel(QAbstractItemModel* sourceModel)
{
	contactListModel = qobject_cast<ContactListModel*>(sourceModel);
	connect(contactListModel, SIGNAL(modelRebuilded()), this, SIGNAL(modelRebuilded()));
	connect(contactListModel, SIGNAL(collapseGroups(QList<QModelIndex>)), this, SIGNAL(collapseGroups(QList<QModelIndex>)));
	QSortFilterProxyModel::setSourceModel(sourceModel);
}

bool ContactListSortFilterProxyModel::isGroup(const QModelIndex& index)
{
	return contactListModel->isGroup(mapToSource(index));
}

Contact* ContactListSortFilterProxyModel::contactFromIndex(const QModelIndex& index) const
{
	return contactListModel->contactFromIndex(mapToSource(index));
}

ContactGroup* ContactListSortFilterProxyModel::groupFromIndex(const QModelIndex & index) const
{
	return contactListModel->groupFromIndex(mapToSource(index));
}

void ContactListSortFilterProxyModel::setFilterString(const QString& filterString)
{
	filter = filterString;
	invalidateFilter();
}

bool ContactListSortFilterProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
	Contact* contact1 = contactListModel->contactFromIndex(left);

	if (!contact1)
		return false;

	Contact* contact2 = contactListModel->contactFromIndex(right);

	if (!contact2)
		return false;

	if ((contact1->status().connected() && contact2->status().connected()) || contact1->status() == contact2->status()
	 || (contact1->isIgnored() && contact2->isIgnored()))
	{
		int cmp = contact1->nickname().compare(contact2->nickname(), Qt::CaseInsensitive);
		if (cmp == 0)
			return contact1->email() < contact2->email();
		else
			return cmp < 0;
	}
	else
		return (contact1->status() < contact2->status());
}

bool ContactListSortFilterProxyModel::filterAcceptsColumn(int /*source_column*/, const QModelIndex& /*source_parent*/) const
{
	return true;
}

bool ContactListSortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
	if (filter.isEmpty() && !allowOnlineOnly)
		return true;

	Contact* contact = contactListModel->contactFromIndex(source_parent.child(source_row, 0));

	if (!contact)
		return true;

	if (!allowOnlineOnly || contact->status().connected())
	{
		if (filter.isEmpty())
			return true;

		if (contact->nickname().contains(filter, Qt::CaseInsensitive) ||
		    QString::fromLatin1(contact->email()).contains(filter, Qt::CaseInsensitive))
			return true;
	}

	return false;
}

void ContactListSortFilterProxyModel::allowOnlineOnlyContacts(bool allow)
{
	allowOnlineOnly = allow;
	invalidateFilter();
}

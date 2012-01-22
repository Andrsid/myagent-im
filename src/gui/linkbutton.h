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

#ifndef LINKBUTTON_H
#define LINKBUTTON_H

#include "toolbutton.h"
#include <QUrl>

class LinkButton : public ToolButton
{
Q_OBJECT
public:
	LinkButton(const QString& url, const QIcon& icon, QWidget* parent = 0);
	explicit LinkButton(QAction* action, QWidget* parent = 0);
	
protected:
	virtual void mouseReleaseEvent(QMouseEvent* event);

private Q_SLOTS:
	void copyLinkLocation();

private:
	void init();

private:
	QUrl m_url;
};

#endif

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

#ifndef EMOTICONWIDGET_H
#define EMOTICONWIDGET_H

#include <QWidget>
#include <QBrush>

class EmoticonMovie;

class EmoticonWidget : public QWidget
{
Q_OBJECT
public:
	EmoticonWidget(const QString& emoticonId, QWidget* parent = 0);
	EmoticonWidget(EmoticonMovie* m, QWidget* parent = 0);
	~EmoticonWidget();

	void setDragEnabled(bool on);
	QString emoticonId() const;
	
signals:
	void clicked(QString id);
	void draggingStarted();
protected:
	virtual void showEvent(QShowEvent* event);
	virtual void hideEvent(QHideEvent* event);
	virtual void paintEvent(QPaintEvent* event);
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
private:
	void init();
	void setMovie(EmoticonMovie* m);
	
	EmoticonMovie* movie;

	bool draggable;
	QPoint dragStartPos;
};

#endif

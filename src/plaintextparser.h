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

#ifndef PLAINTEXTPARSER_H
#define PLAINTEXTPARSER_H

#include <QTextCursor>

class QTextDocument;
class QTextCodec;

class PlainTextParser
{
public:
	PlainTextParser();
	~PlainTextParser();

	void parse(const QString& text, QTextDocument* doc);
private:
	void setImage(const char* str);
	void setSmile(const char* str);
	void setUrl(const char* str);
	void setText(const char* str);

	QTextCursor cursor;
	QTextCodec* codec;
};

#endif

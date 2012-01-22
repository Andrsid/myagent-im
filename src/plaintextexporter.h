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

#ifndef PLAINTEXTEXPORTER_H
#define PLAINTEXTEXPORTER_H

#include <QString>

class QTextDocument;
class QTextBlock;
class QTextFragment;

class PlainTextExporter
{
public:
	PlainTextExporter(const QTextDocument* _doc);
	~PlainTextExporter();

	QString toText();
private:
	void processBlock(const QTextBlock &block);
	void processFragment(const QTextFragment &fragment);

	const QTextDocument* doc;
	QString text;
};

#endif

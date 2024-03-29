/*
 * spellchecker.cpp
 *
 * Copyright (C) 2006  Remko Troncon
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * You can also redistribute and/or modify this program under the
 * terms of the Psi License, specified in the accompanied COPYING
 * file, as published by the Psi Project; either dated January 1st,
 * 2005, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "spellchecker.h"

#include <QCoreApplication>

#include "aspellchecker.h"

SpellChecker* SpellChecker::instance() 
{
	if (!instance_) {
		instance_ = new ASpellChecker();
	}
	return instance_;
}

SpellChecker::SpellChecker()
	: QObject(QCoreApplication::instance())
{
}

SpellChecker::~SpellChecker()
{
}

bool SpellChecker::available() const
{
	return false;
}

bool SpellChecker::writable() const
{
	return true;
}

bool SpellChecker::isCorrect(const QString&)
{
	return true;
}

QList<QString> SpellChecker::suggestions(const QString&)
{
	return QList<QString>();
}

bool SpellChecker::add(const QString&)
{
	return false;
}

SpellChecker* SpellChecker::instance_ = NULL;

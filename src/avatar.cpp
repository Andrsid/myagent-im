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

#include "avatar.h"

#include <QDebug>

#include <QDir>
#include <QFile>

#include "datetime.h"

Avatar::Avatar(QObject* parent)
	: QObject(parent)
{
	connect(&m_http, SIGNAL(done(bool)), this, SLOT(httpDone(bool)));
	
	reset();
}

Avatar::~Avatar()
{
	m_http.disconnect(this);
}

void Avatar::reset()
{
	QPixmap::load(emptyAvatarFilename());
	m_state = Null;
	emit updated();
}

void Avatar::setLoading()
{
	QPixmap::load(loadingAvatarFilename());
	m_state = Loading;
	emit updated();
}

void Avatar::setLoaded()
{
	m_state = Loaded;
	emit updated();
}

void Avatar::load(const QString& dirname, const QString& email)
{
	if (m_state != Null)
		return;
	
	m_dirname = dirname;
	m_email = email;
	
	QDir dir(m_dirname);
	QString dateFilename = dir.absoluteFilePath(m_email + ".date");
	QFile dateFile(dateFilename);
	m_fileDateTime = QDateTime();
	if (dateFile.exists() && dateFile.open(QFile::ReadOnly))
	{
		qDebug() << "reading date from datefile";
		QDataStream in(&dateFile);
		in >> m_fileDateTime;
		dateFile.close();
		
		if (m_fileDateTime.isValid())
		{
			qDebug() << "filedatetime is valid";
			qDebug() << "secsTo current date = " << m_fileDateTime.secsTo(QDateTime::currentDateTime());
			if (m_fileDateTime.secsTo(QDateTime::currentDateTime()) < 7200)
			{
				qDebug() << "m_fileDateTime.secsTo(QDateTime::currentDateTime()) < 7200";
				if (QPixmap::load(dir.absoluteFilePath(imageName())))
				{
					setLoaded();
					return;
				}
			}
		}
	}
	
	requestHttpHeader(avatarHref());
}

void Avatar::requestHttpHeader(QUrl url)
{
	qDebug() << "url path = " << url.path();
	if (!url.isValid()) {
		qDebug() << "Error: Invalid URL" << endl;
		return;
	}
	
	m_bRequestingHeader = true;
	m_httpPath = url.path();
	
	setLoading();
	
	m_http.setHost(url.host(), url.port(80));
	m_http.head(m_httpPath);
}

void Avatar::httpDone(bool error)
{
	if (m_bRequestingHeader)
	{
		m_bRequestingHeader = false;
		qDebug() << "header received";
		
		if (m_http.lastResponse().statusCode() == 404)
		{
			qDebug() << "Avatar: error 404";
			reset();
			setLoaded();
			return;
		}
		else
		{
			qDebug() << "no 404, processing header";
			if (!m_fileDateTime.isValid())
			{
				qDebug() << "m_fileDateTime is invalid, requestImage";
				requestImage();
				return;
			}
			else
			{
				qDebug() << "m_fileDateTime is valid";
				QDateTime modificationDateTime = parseRFCDate(m_http.lastResponse().value("Last-Modified"));
				
				if (!modificationDateTime.isValid() || modificationDateTime > m_fileDateTime)
				{
					qDebug() << "modificationDateTime > m_fileDateTime, requestImage";
					requestImage();
				}
				else
				{
					qDebug() << "loading pixmap from file";
					QDir dir(m_dirname);
					if (!QPixmap::load(dir.absoluteFilePath(imageName())))
					{
						qDebug() << "can't load image from file, requestImage";
						requestImage();
					}
					else
					{
						qDebug() << "loaded";
						setLoaded();
					}
				}
				
				return;
			}
		}
	}
	
	qDebug() << "processing body";
	m_buffer.close();
	
	if (error)
	{
		qDebug() << "Error: " << qPrintable(m_http.errorString()) << endl;
		reset();
		return;
	}
	
	QPixmap tmpPixmap;
	if (!tmpPixmap.loadFromData(m_buffer.buffer()))
	{
		qDebug() << "wrong image format";
		reset();
		return;
	}
	
	QPixmap::operator=(tmpPixmap);
	
	QDir dir(m_dirname);
	QFile pixmapFile(dir.absoluteFilePath(imageName()));
	if (pixmapFile.open(QFile::WriteOnly))
	{
		pixmapFile.write(m_buffer.buffer());
		pixmapFile.close();
	}
	
	QFile dateFile(dir.absoluteFilePath(m_email + ".date"));
	if (dateFile.open(QFile::WriteOnly))
	{
		QDataStream out(&dateFile);
		out << QDateTime::currentDateTime();
		dateFile.close();
	}
	
	setLoaded();
}

void Avatar::requestImage()
{
	m_buffer.buffer().clear();
	m_buffer.open(QIODevice::WriteOnly);
	
	m_http.get(m_httpPath, &m_buffer);
}

QString Avatar::avatarBaseHref() const
{
	QString domain = m_email.section('@', 1, 1).section('.', 0, 0);
	QString username = m_email.section('@', 0, 0);
	return "http://obraz.foto.mail.ru/" + domain + "/" + username;
}

QString Avatar::avatarHref() const
{
	return avatarBaseHref() + "/_mrimavatar";
}

QString Avatar::emptyAvatarFilename() const
{
	return ":icons/noavatar.png";
}

QString Avatar::loadingAvatarFilename() const
{
	return ":icons/loadavatar.png";
}

QString Avatar::imageName() const
{
	return m_email + ".jpg";
}

/***************************************************************************
 *   Copyright (C) 2011 by Dmitry Malakhov <abr_mail@mail.ru>              *
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

#include "soundplayer.h"

#include <QDebug>

#include <QMediaPlayer>
#include <QDir>

#include "resourcemanager.h"

SoundPlayer::SoundPlayer(QObject *parent)
	: QObject(parent),
	  currentSound(STAuth),
	  mediaPlayer(0)
{
}

SoundPlayer::~SoundPlayer()
{
}

bool SoundPlayer::playSound(SoundType soundType)
{
	qDebug() << Q_FUNC_INFO << "(\"" + soundDescription(soundType) + ".ogg\")";
	currentSound = soundType;

	QString mediaFile = soundDescription(soundType);

	if (mediaFile.isEmpty())
	{
		qDebug() << Q_FUNC_INFO << "Error: Media file not found";
		deleteLater();
		return false;
	}

	mediaPlayer = new QMediaPlayer(this);

	mediaPlayer->setMedia(QUrl::fromLocalFile(theRM.soundsPath() + QDir::separator() + mediaFile + ".ogg"));

	connect(mediaPlayer, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(mediaStatusChanged()));
	mediaPlayer->play();

	return true;
}

void SoundPlayer::mediaStatusChanged()
{
	switch (mediaPlayer->mediaStatus())
	{
	case QMediaPlayer::InvalidMedia:
	case QMediaPlayer::EndOfMedia:
	{
		deleteLater();
		Q_EMIT finished(currentSound);
	}
		break;
	default:
		break;
	}
}

QString SoundPlayer::soundDescription(SoundType soundType)
{
	switch (soundType)
	{
		case STAuth:
			return "auth";
		case STLetter:
			return "letter";
		case STMessage:
			return "message";
		case STOtprav:
			return "otprav";
		case STRing:
			return "ring";
		case STConference:
			return "conference";
		default:
			return "";
	}
}

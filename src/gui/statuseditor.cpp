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

#include "statuseditor.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFont>
#include <QDateTime>
#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

StatusEditor::StatusEditor(QWidget *parent)
	: QWidget(parent)
{
	setWindowFlags(Qt::Popup);
	QVBoxLayout* layout = new QVBoxLayout;
	QLabel* label1 = new QLabel(tr("Microblog"));
	QLabel* label2 = new QLabel("<font color=#888888>" + QDateTime::currentDateTime().toString("d MMM yy, hh:mm") + "</font>");

	editor = new QLineEdit(this);
	editor->setMaxLength(maxSize);
	connect(editor, SIGNAL(textChanged(const QString&)), SLOT(slotTextChanged(const QString&)));

	QWidget* bottomWidget = new QWidget(this);
	QWidget* twoLabels = new QWidget(bottomWidget);
	sayButton = new QPushButton(tr("Say"));
	sayButton->setEnabled(false);
	connect(sayButton, SIGNAL(clicked(bool)), this, SLOT(slotSendMicrotext()));

	QHBoxLayout* bottomLayout = new QHBoxLayout;
	QVBoxLayout* twoLabelsLayout = new QVBoxLayout;

	symbolsCounter = new QLabel(tr("%n symbol(s) left", "", maxSize));
	QLabel* linkToFriends = new QLabel("<a href=\"#\">" + tr("Checkout news from friends") + "</a>");

	QFont font;
	font.setPointSize(8);
	symbolsCounter->setFont(font);
	linkToFriends->setFont(font);
	label2->setFont(font);

	twoLabelsLayout->addWidget(symbolsCounter);
	twoLabelsLayout->addWidget(linkToFriends);

	twoLabels->setLayout(twoLabelsLayout);

	bottomLayout->addWidget(twoLabels);
	bottomLayout->addWidget(sayButton);

	bottomWidget->setLayout(bottomLayout);

	font.setBold(true);
	label1->setFont(font);

	layout->addWidget(label1);
	layout->addWidget(label2);
	layout->addWidget(editor);
	layout->addWidget(bottomWidget);

	setLayout(layout);
}

StatusEditor::~StatusEditor()
{
}

void StatusEditor::slotTextChanged(const QString& text)
{
	symbolsCounter->setText(tr("%n symbol(s) left", "", maxSize - text.length()));
	if (text.length() == 0)
		sayButton->setEnabled(false);
	else
		sayButton->setEnabled(true);
}

void StatusEditor::slotSendMicrotext()
{
	qDebug() << "StatusEditor::slotSendStatus()";
	Q_EMIT sendMicrotext(editor->text());
	close();
}

void StatusEditor::setStatus(QString text)
{
	editor->setText(text);
}

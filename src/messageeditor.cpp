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

#include "messageeditor.h"

#include <QToolBar>
#include <QAction>
#include <QIcon>
#include <QColorDialog>
#include <QFontDialog>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QKeyEvent>

#include "messageedit.h"
#include "emoticonselector.h"
#include "toolbutton.h"
#include "account.h"
#include "contact.h"

MessageEditor::MessageEditor(Account* account, Contact* contact, QWidget* parent)
	: QWidget(parent), m_account(account), m_contact(contact)
{
	connect(m_contact, SIGNAL(destroyed(Contact*)), this, SLOT(writeSettings()));

	QVBoxLayout* layout = new QVBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	
	createToolBar();
	messageEdit = new MessageEdit;
	messageEdit->installEventFilter(this);
	
	connect(messageEdit, SIGNAL(textChanged()), SIGNAL(textChanged()));
	connect(messageEdit, SIGNAL(cursorPositionChanged()), this, SLOT(updateFormatActions()));

	layout->addWidget(toolBar);
	layout->addWidget(messageEdit);
	setLayout(layout);

	emoticonSelector = new EmoticonSelector(this);
	connect(emoticonSelector, SIGNAL(selected(QString)), SLOT(insertEmoticon(QString)));
	connect(emoticonSelector, SIGNAL(closed()), smilesAction, SLOT(toggle()));

	fileTransferBar = NULL;

	readSettings();

	connect(messageEdit, SIGNAL(currentCharFormatChanged(const QTextCharFormat&)), this, SLOT(slotCurrentCharFormatChanged(const QTextCharFormat &)));

	connect(m_contact, SIGNAL(statusChanged(OnlineStatus)), this, SLOT(checkContactStatus(OnlineStatus)));
	checkContactStatus(m_contact->status());
}

MessageEditor::~MessageEditor()
{
	writeSettings();
}

bool MessageEditor::eventFilter(QObject * obj, QEvent * ev)
{
	if (obj == messageEdit && ev->type() == QEvent::KeyPress)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(ev);
		qDebug() << "MessageEdit::eventFilter, key =" << QString::number(keyEvent->key(), 16) << ", mods =" << QString::number(keyEvent->modifiers(), 16);
		if (keyEvent->key() == Qt::Key_Return)
		{
			QString enterVariant = theRM.settings()->value("Messages/sendOnEnter", "Ctrl+Enter").toString();
			if (enterVariant == "Enter")
			{
				if (!keyEvent->isAutoRepeat() && keyEvent->modifiers() == Qt::NoModifier)
				{
					emit sendPressed();
					return true;
				}
			}
			else if (enterVariant == "Enter+Enter")
			{
				if (keyEvent->isAutoRepeat() && keyEvent->modifiers() == Qt::NoModifier)
				{
					emit sendPressed();
					return true;
				}
			}
			else if (enterVariant == "Ctrl+Enter")
			{
				if (!keyEvent->isAutoRepeat() && keyEvent->modifiers() == Qt::ControlModifier)
				{
					emit sendPressed();
					return true;
				}
			}
		}
		
		if ((keyEvent->key() == Qt::Key_S || keyEvent->key() == 0x42b) && keyEvent->modifiers() == Qt::AltModifier)
		{
			emit sendPressed();
			return true;
		}
	}
	
	return QWidget::eventFilter(obj, ev);
}

QAction* MessageEditor::addToolAction(const QIcon& icon, const QObject * receiver, const char * method)
{
	QAction* action = new QAction(icon, "", this);
	action->setCheckable(true);
	connect(action, SIGNAL(triggered(bool)), receiver, method);
	toolBar->addAction(action);
	toolBar->widgetForAction(action)->setStyleSheet("QToolButton { border: 0px; padding: 0px }");
	
	return action;
}

QIcon MessageEditor::toolIcon(const QString& toolName) const
{
	QIcon icon;
	icon.addPixmap(QPixmap(":/icons/editor/msg_bar_s_" + toolName + "_d.png"), QIcon::Normal, QIcon::Off);
	icon.addPixmap(QPixmap(":/icons/editor/msg_bar_s_" + toolName + "_h.png"), QIcon::Active, QIcon::Off);
	icon.addPixmap(QPixmap(":/icons/editor/msg_bar_s_" + toolName + "_p.png"), QIcon::Normal, QIcon::On);
	icon.addPixmap(QPixmap(":/icons/editor/msg_bar_s_" + toolName + "_ph.png"), QIcon::Active, QIcon::On);
	
	return icon;
}

void MessageEditor::createToolBar()
{
	toolBar = new QToolBar;
	//toolBar->setStyleSheet("QToolBar { border: 0px; }");
	
	spellAction = addToolAction(toolIcon("orfo"), this, SLOT(setCheckSpelling(bool)));

	toolBar->addSeparator();

	fontAction = addToolAction(toolIcon("font"), this, SLOT(chooseFont()));
	colorAction = addToolAction(toolIcon("font_color"), this, SLOT(chooseFontColor()));
	bkColorAction = addToolAction(toolIcon("bkcolor"), this, SLOT(chooseBackgroundColor()));
	
	toolBar->addSeparator();
	
	boldFontAction = addToolAction(toolIcon("bold"), this, SLOT(setFontBold(bool)));
	italicFontAction = addToolAction(toolIcon("italic"), this, SLOT(setFontItalic(bool)));
	underlinedFontAction = addToolAction(toolIcon("underlined"), this, SLOT(setFontUnderline(bool)));
	
	toolBar->addSeparator();
	
	smilesAction = addToolAction(toolIcon("smiles"), this, SLOT(triggerEmoticonSelector()));
	
	toolBar->addSeparator();
	
	QIcon wakeupIcon;
	wakeupIcon.addPixmap(QPixmap(":/icons/editor/wakeup_dis.png"), QIcon::Disabled, QIcon::Off);
	wakeupIcon.addPixmap(QPixmap(":/icons/editor/wakeup_d.png"), QIcon::Normal, QIcon::Off);
	wakeupIcon.addPixmap(QPixmap(":/icons/editor/wakeup_p.png"), QIcon::Normal, QIcon::On);
	wakeupIcon.addPixmap(QPixmap(":/icons/editor/wakeup_h.png"), QIcon::Active, QIcon::Off);
	
	wakeupButton = new ToolButton(wakeupIcon, toolBar);
	connect(wakeupButton, SIGNAL(clicked()), SIGNAL(wakeupPressed()));
	toolBar->addWidget(wakeupButton);
	
	/*editorToolBar->addSeparator();
	
	QAction* fileTransferAction = new QAction(QIcon(":/icons/ft.png"), "", this);
	fileTransferAction->setCheckable(true);
	connect(fileTransferAction, SIGNAL(triggered(bool)), this, SLOT(fileTransfer(bool)));
	editorToolBar->addAction(fileTransferAction);
	*/
}

void MessageEditor::chooseFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok, messageEdit->currentFont(), this);
	if (ok)
		setCurrentFont(font);
	
	fontAction->setChecked(false);
	messageEdit->setFocus(Qt::OtherFocusReason);
}

void MessageEditor::chooseFontColor()
{
	QColor color = QColorDialog::getColor(messageEdit->textColor(), this);
	if (color.isValid())
		setTextColor(color);
	
	colorAction->setChecked(false);
	messageEdit->setFocus(Qt::OtherFocusReason);
}

void MessageEditor::chooseBackgroundColor()
{
	QColor color = QColorDialog::getColor(messageEdit->textBackgroundColor(), this);
	if (color.isValid())
		setTextBackgroundColor(color);
	
	bkColorAction->setChecked(false);
	messageEdit->setFocus(Qt::OtherFocusReason);
}

void MessageEditor::setCurrentFont(const QFont & f)
{
	QTextCharFormat fmt;
	fmt.setFont(f);

	mergeMyCharFormat(fmt);
	updateFormatActions();
}

void MessageEditor::setTextColor(const QColor & c)
{
	QTextCharFormat fmt;
	fmt.setForeground(c);

	mergeMyCharFormat(fmt);
}

void MessageEditor::setTextBackgroundColor(const QColor & c)
{
	QTextCharFormat fmt;
	fmt.setBackground(c);

	mergeMyCharFormat(fmt);
}

void MessageEditor::mergeMyCharFormat(const QTextCharFormat & fmt)
{
	qDebug() << "MessageEditor::mergeMyCharFormat";
	QTextCursor cursor = messageEdit->textCursor();

	cursor.mergeBlockCharFormat(fmt);
	messageEdit->document()->markContentsDirty(cursor.block().position(), cursor.block().length());

	cursor.mergeCharFormat(fmt);

	lastUserFormat = cursor.charFormat();

	messageEdit->setTextCursor(cursor);
}

void MessageEditor::setFontBold(bool b)
{
	QTextCharFormat fmt;
	fmt.setFontWeight(b ? QFont::Bold : QFont::Normal);

	mergeMyCharFormat(fmt);
}

void MessageEditor::setFontItalic(bool b)
{
	QTextCharFormat fmt;
	fmt.setFontItalic(b);

	mergeMyCharFormat(fmt);
}

void MessageEditor::setFontUnderline(bool b)
{
	QTextCharFormat fmt;
	fmt.setFontUnderline(b);

	mergeMyCharFormat(fmt);
}

void MessageEditor::insertEmoticon(const QString & id)
{
	QTextCursor cursor = messageEdit->textCursor();
	QTextCharFormat currentFormat = cursor.charFormat();
	EmoticonFormat fmt(currentFormat, id);
	cursor.insertText(QString(QChar::ObjectReplacementCharacter), fmt);
	messageEdit->setTextCursor(cursor);
	messageEdit->setFocus(Qt::OtherFocusReason);
	messageEdit->setCurrentCharFormat(currentFormat);
	qDebug() << "MessageEditor::insertEmoticon currentFormat = " << currentFormat.objectType() << ", format = " << messageEdit->textCursor().charFormat().objectType();
}

void MessageEditor::readSettings()
{
	if (m_account.isNull() || m_contact.isNull())
	{
		qDebug() << "MessageEditor::readSettings() error: m_account = " << (void*)m_account << ", m_contact = " << (void*)m_contact;
		return;
	}
	
	QSettings* settings = m_account->settings();
	
	if (!settings)
	{
		qDebug() << "MessageEditor::readSettings(): settings don't exist";
		return;
	}
	
	bool checkSpelling = settings->value("MessageEditor/checkSpelling", false).toBool();
	setCheckSpelling(checkSpelling);
	
	QString settingsPrefix = "MessageEditor_" + m_contact->email() + "/";
	QFont font;
	font.fromString(settings->value(settingsPrefix + "font").toString());
	QColor textColor(settings->value(settingsPrefix + "textColor", "#000000").toString());
	QColor backColor(settings->value(settingsPrefix + "backgroundColor", "#ffffff").toString());

	QTextCharFormat fmt;
	fmt.setFont(font);
	fmt.setForeground(textColor);
	fmt.setBackground(backColor);

	mergeMyCharFormat(fmt);
}

void MessageEditor::writeSettings()
{
	if (m_account.isNull() || m_contact.isNull())
	{
		qDebug() << "MessageEditor::writeSettings() error: m_account = " << (void*)m_account << ", m_contact = " << (void*)m_contact;
		return;
	}

	QSettings* settings = m_account->settings();

	if (!settings)
	{
		qDebug() << "MessageEditor::writeSettings(): settings don't exist";
		return;
	}
	
	QString settingsPrefix = "MessageEditor_" + m_contact->email() + "/";
	
	settings->setValue(settingsPrefix + "font", lastUserFormat.font().toString());
	settings->setValue(settingsPrefix + "textColor", lastUserFormat.foreground().color().name());
	if (lastUserFormat.background().style() == Qt::NoBrush)
		settings->remove(settingsPrefix + "backgroundColor");
	else
		settings->setValue(settingsPrefix + "backgroundColor", lastUserFormat.background().color().name());
}

void MessageEditor::blockInput()
{
	messageEdit->setReadOnly(true);
	messageEdit->setBackgroundRole(QPalette::Text);
}

void MessageEditor::unblockInput()
{
	messageEdit->setReadOnly(false);
	messageEdit->setBackgroundRole(QPalette::Base);
}

bool MessageEditor::isBlocked()
{
	return messageEdit->isReadOnly();
}

void MessageEditor::clear()
{
	messageEdit->blockSignals(true);
	messageEdit->clear();
	mergeMyCharFormat(lastUserFormat);
	updateFormatActions();
	messageEdit->blockSignals(false);
}

void MessageEditor::triggerEmoticonSelector()
{
	emoticonSelector->setGeometry(emoticonSelector->geometry());
	emoticonSelector->setVisible(!emoticonSelector->isVisible());
}

void MessageEditor::hideEvent(QHideEvent* /*event*/)
{
	qDebug() << "MessageEditor::hideEvent";
	emoticonSelector->hide();
	smilesAction->setChecked(false);
}

void MessageEditor::updateFormatActions()
{
	if (messageEdit->fontWeight() == QFont::Bold)
		boldFontAction->setChecked(true);
	else
		boldFontAction->setChecked(false);
	
	italicFontAction->setChecked(messageEdit->fontItalic());
	underlinedFontAction->setChecked(messageEdit->fontUnderline());
}

QTextDocument * MessageEditor::document() const
{
	return messageEdit->document();
}

void MessageEditor::createFileTransferBar()
{
	fileTransferBar = new QToolBar;
	
	QLabel* ftLabel = new QLabel(this);
	
	ftLabel->setPixmap(QPixmap(":/icons/editor/msg_p_f_title.png"));
	filesBox = new QComboBox(this);
	filesBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	
	QWidget* helperWidget = new QWidget(this);
	QVBoxLayout* helperLayout = new QVBoxLayout;
	helperLayout->setSpacing(0);
	helperLayout->setContentsMargins(0,0,0,0);
	
	QHBoxLayout* labelsLayout = new QHBoxLayout;
	
	QLabel* label1 = new QLabel(tr("<small>Total size: </small>"));
	label1->setStyleSheet("QLabel { margin : 0px; border : 0px; padding : 0px }");
	bytesLabel = new QLabel("<small>0</small>");
	bytesLabel->setStyleSheet("QLabel { margin : 0px; border : 0px; padding : 0px }");
	QLabel* label2 = new QLabel(tr("<small>&nbsp; bytes</small>"));
	label2->setStyleSheet("QLabel { margin : 0px; border : 0px; padding : 0px }");
	
	labelsLayout->addWidget(label1);
	labelsLayout->addWidget(bytesLabel);
	labelsLayout->addWidget(label2);
	labelsLayout->addStretch();
	
	QHBoxLayout* buttonsLayout = new QHBoxLayout;
	
	QToolButton* plus = createFileTransferToolButton(fileTransferIcon("add"), this, SLOT(addFile()));
	QToolButton* minus = createFileTransferToolButton(fileTransferIcon("delete"), this, SLOT(deleteFile()));
	QToolButton* send = createFileTransferToolButton(fileTransferIcon("send"), this, SLOT(sendFiles()));
	
	buttonsLayout->addWidget(plus);
	buttonsLayout->addWidget(minus);
	buttonsLayout->addSpacing(4);
	buttonsLayout->addWidget(send);
	buttonsLayout->addStretch();
	
	helperLayout->addLayout(labelsLayout);
	helperLayout->addLayout(buttonsLayout);
	
	helperWidget->setLayout(helperLayout);
	
	fileTransferBar->addWidget(ftLabel);
	fileTransferBar->addWidget(filesBox);
	fileTransferBar->addWidget(helperWidget);
}

QIcon MessageEditor::fileTransferIcon(const QString & toolName) const
{
	QIcon icon;
	
	icon.addPixmap(QPixmap(":/icons/editor/msg_p_f_" + toolName + "_dis.png"), QIcon::Disabled, QIcon::Off);
	icon.addPixmap(QPixmap(":/icons/editor/msg_p_f_" + toolName + "_d.png"), QIcon::Normal, QIcon::Off);
	icon.addPixmap(QPixmap(":/icons/editor/msg_p_f_" + toolName + "_h.png"), QIcon::Active, QIcon::Off);
	icon.addPixmap(QPixmap(":/icons/editor/msg_p_f_" + toolName + "_p.png"), QIcon::Active, QIcon::On);
	
	return icon;
}

QToolButton* MessageEditor::createFileTransferToolButton(const QIcon & icon, const QObject * receiver, const char * method)
{
	QSize size = icon.actualSize(QSize(256, 32));
	QAction* action = new QAction(icon, "", this);
	ToolButton* toolButton = new ToolButton(action, this);
	toolButton->setIconSize(size);
	connect(action, SIGNAL(triggered(bool)), receiver, method);
	
	return toolButton;
}

void MessageEditor::addFile()
{
	QStringList files =
		QFileDialog::getOpenFileNames(this, tr("Select files to transfer"));
	
	QStringList::const_iterator it = files.begin();
	for (;it != files.end(); ++it)
	{
		int index = filesBox->findData(*it);
		if (index != -1)
			continue;
		
		QFileInfo finfo(*it);
		filesBox->addItem(finfo.fileName(), *it);
	}
	
	qDebug() << "filesBox.sizeHint = " << filesBox->sizeHint();
	filesBox->resize(filesBox->sizeHint());
}

void MessageEditor::fileTransfer(bool checked)
{
	fileTransferBar->setVisible(checked);
}

void MessageEditor::slotCurrentCharFormatChanged(const QTextCharFormat & f)
{
	qDebug() << "MessageEditor::slotCurrentCharFormatChanged type = " << f.objectType();
}

void MessageEditor::checkContactStatus(OnlineStatus status)
{
	wakeupButton->setEnabled(status.connected());
}

void MessageEditor::setCheckSpelling(bool on)
{
	messageEdit->setCheckSpelling(on);
	if (spellAction->isChecked() != on)
		spellAction->setChecked(on);
	
	QSettings* settings = m_account->settings();
	settings->setValue("MessageEditor/checkSpelling", spellAction->isChecked());
	
}

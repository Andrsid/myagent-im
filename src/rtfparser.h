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

#ifndef RTFPARSER_H
#define RTFPARSER_H

#include <QByteArray>
#include <QStack>
#include <QTextCharFormat>
#include <QTextCursor>
#include "emoticons.h"

class QTextDocument;
class QTextCodec;

struct FontDef
{
	int charset;
	QByteArray name;
};

class RtfParser;

class RtfLevel
{
public:
	RtfLevel();
	RtfLevel(RtfParser* p);
	RtfLevel(const RtfLevel&);
	void setFontTbl() { m_bFontTable = true; }
	void setColors() { m_bColors = true; resetColors(); }
	void setRed(unsigned char val) { setColor(val, &m_nRed); }
	void setGreen(unsigned char val) { setColor(val, &m_nGreen); }
	void setBlue(unsigned char val) { setColor(val, &m_nBlue); }
	void setText(const char* str);
	void setUrl(const char* str);
	void setImage(QString id);
	void setSmile(const char* str);
	void setFont(int nFont);
	//void setEncoding(unsigned nFont);
	void setFontColor(unsigned short color);
	void setBackgroundColor(unsigned short color);
	//void setFontBgColor(unsigned short color);
	void setFontSizeHalfPoints(unsigned short sizeInHalfPoints);
	//void setFontSize(unsigned short sizeInPoints);
	void setBold(bool);
	void setItalic(bool);
	void setUnderline(bool);
	void startParagraph();
	bool isParagraphOpen() const;
	void clearParagraphFormatting();
	//void setParagraphDirLTR();
	//void setParagraphDirRTL();
	//void addLineBreak();
	//void flush();
	void storeCharFormat();
	void restoreCharFormat();

private:
	RtfParser* parser;
	QTextCharFormat previousCharFormat;
	QTextCodec* codec;	

	void init();
	void resetColors() { m_nRed = m_nGreen = m_nBlue = 0; m_bColorInit = false; }
	void setColor(unsigned char val, unsigned char *p)
	{ *p = val; m_bColorInit = true; }

	//enum State { Free, FontTable, ColorTable };
	//State state;
	
// True when parsing the fonts table
	bool m_bFontTable;
// True when parsing the colors table.
	bool m_bColors;

	unsigned char m_nRed;
	unsigned char m_nGreen;
	unsigned char m_nBlue;
	bool m_bColorInit;
	int m_nFont; // 1-based
	int m_nEncoding;
	int m_nFontColor; // 1-based
	int m_nBackgroundColor; // 1-based
	int m_nFontSize;
	int m_nFontBgColor; // 1-based
	QByteArray m_fontName;
	bool m_bBold;
	bool m_bItalic;
	bool m_bUnderline;
};

class RtfParser
{
	friend class RtfLevel;
public:
	RtfParser();
	~RtfParser();
	
	void parse(QByteArray rtf, QTextDocument* doc);
	
private:	
	// Fonts table.
	QVector<FontDef> fonts;
    // Colors table.
	QVector<QColor> colors;
	
	QTextCursor cursor;
	
	RtfLevel curLevel;
	QStack<RtfLevel> levels;
};

#endif

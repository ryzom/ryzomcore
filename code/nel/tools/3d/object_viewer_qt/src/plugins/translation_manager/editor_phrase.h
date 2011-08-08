// Translation Manager Plugin - OVQT Plugin <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Emanuel Costea <cemycc@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef EDITOR_PHRASE_H
#define EDITOR_PHRASE_H

// Qt includes
#include <QtCore/QObject>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QWidget>
#include <QtGui/QMdiArea>
#include <QtGui/QMdiSubWindow>
#include <QtGui/QUndoCommand>
#include <QtGui/QUndoStack>
#include <QtGui/QTextEdit>
#include <QtGui/QSyntaxHighlighter>

// Project includes
#include "translation_manager_editor.h"

namespace Plugin {

class CEditorPhrase : public CEditor
{
	Q_OBJECT
public:
	QTextEdit *text_edit;
public:
    CEditorPhrase(QMdiArea* parent) : CEditor(parent) {}
    CEditorPhrase() : CEditor() {}
    void open(QString filename);
    void save();
    void saveAs(QString filename);
    void activateWindow();
	void closeEvent(QCloseEvent *event);
public Q_SLOTS:
	void contentsChangeNow(int, int, int);
	void docContentsChanged();

};

class CUndoPhraseInsertCommand : public QUndoCommand
{
public:
	CUndoPhraseInsertCommand(int index, const QString &chars, QTextEdit *document, QUndoCommand *parent = 0) : QUndoCommand("Insert characters", parent), 
		m_index(index), 
		m_chars(chars), 
		m_document(document)
	{ }
	
	virtual void redo() 
	{
		QString text = m_document->toPlainText();
		text.insert(m_index, m_chars);
		m_document->clear();
		m_document->setPlainText(text); 
	}

	virtual void undo()
	{
		QString text = m_document->toPlainText();
		text.remove(m_index, m_chars.length());
		m_document->clear();
		m_document->setPlainText(text); 
		m_document->undo();
	}

private:
	int m_index;
	QString m_chars;
	QTextEdit* m_document;

};

class CUndoPhraseRemoveCommand : public QUndoCommand
{
public:
	CUndoPhraseRemoveCommand(int index, int count, QTextEdit *document, QUndoCommand *parent = 0) : QUndoCommand("Remove characters", parent), 
		m_index(index), 
		m_count(count), 
		m_document(document)
	{ }

	virtual void redo()
	{
		QString text = m_document->toPlainText();
		m_removedChars = text.mid(m_index, m_count);
		text.remove(m_index, m_count);
		m_document->clear();
		m_document->setPlainText(text);
	}

	virtual void undo()
	{
		QString text = m_document->toPlainText();
		text.insert(m_index, m_removedChars);
		m_document->clear();
		m_document->setPlainText(text);
	}
private:
	int m_index;
	int m_count;
	QString m_removedChars;
	QTextEdit* m_document;

};

class SyntaxHighlighter : public QSyntaxHighlighter
{
public:
	SyntaxHighlighter(QTextEdit *parent) : QSyntaxHighlighter(parent)
	{ 
     HighlightingRule rule;

     keywordFormat.setForeground(Qt::darkBlue);
     keywordFormat.setFontWeight(QFont::Bold);
     QStringList keywordPatterns;
     keywordPatterns << "\\bchar\\b" << "\\bclass\\b" << "\\bconst\\b"
                     << "\\bdouble\\b" << "\\benum\\b" << "\\bexplicit\\b"
                     << "\\bfriend\\b" << "\\binline\\b" << "\\bint\\b"
                     << "\\blong\\b" << "\\bnamespace\\b" << "\\boperator\\b"
                     << "\\bprivate\\b" << "\\bprotected\\b" << "\\bpublic\\b"
                     << "\\bshort\\b" << "\\bsignals\\b" << "\\bsigned\\b"
                     << "\\bslots\\b" << "\\bstatic\\b" << "\\bstruct\\b"
                     << "\\btemplate\\b" << "\\btypedef\\b" << "\\btypename\\b"
                     << "\\bunion\\b" << "\\bunsigned\\b" << "\\bvirtual\\b"
                     << "\\bvoid\\b" << "\\bvolatile\\b";
     Q_FOREACH(const QString &pattern, keywordPatterns) {
         rule.pattern = QRegExp(pattern);
         rule.format = keywordFormat;
         highlightingRules.append(rule);
     }

     classFormat.setFontWeight(QFont::Bold);
     classFormat.setForeground(Qt::darkMagenta);
     rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
     rule.format = classFormat;
     highlightingRules.append(rule);

     singleLineCommentFormat.setForeground(Qt::red);
     rule.pattern = QRegExp("//[^\n]*");
     rule.format = singleLineCommentFormat;
     highlightingRules.append(rule);

     multiLineCommentFormat.setForeground(Qt::red);

     quotationFormat.setForeground(Qt::darkGreen);
     rule.pattern = QRegExp("\".*\"");
     rule.format = quotationFormat;
     highlightingRules.append(rule);

     functionFormat.setFontItalic(true);
     functionFormat.setForeground(Qt::blue);
     rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
     rule.format = functionFormat;
     highlightingRules.append(rule);

     commentStartExpression = QRegExp("/\\*");
     commentEndExpression = QRegExp("\\*/");	
	}

 void highlightBlock(const QString &text)
 {
     Q_FOREACH(const HighlightingRule &rule, highlightingRules) {
         QRegExp expression(rule.pattern);
         int index = expression.indexIn(text);
         while (index >= 0) {
             int length = expression.matchedLength();
             setFormat(index, length, rule.format);
             index = expression.indexIn(text, index + length);
         }
     }
     setCurrentBlockState(0);

     int startIndex = 0;
     if (previousBlockState() != 1)
         startIndex = commentStartExpression.indexIn(text);

     while (startIndex >= 0) {
         int endIndex = commentEndExpression.indexIn(text, startIndex);
         int commentLength;
         if (endIndex == -1) {
             setCurrentBlockState(1);
             commentLength = text.length() - startIndex;
         } else {
             commentLength = endIndex - startIndex
                             + commentEndExpression.matchedLength();
         }
         setFormat(startIndex, commentLength, multiLineCommentFormat);
         startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
     }
 }

 private:
     struct HighlightingRule
     {
         QRegExp pattern;
         QTextCharFormat format;
     };
     QVector<HighlightingRule> highlightingRules;

     QRegExp commentStartExpression;
     QRegExp commentEndExpression;

     QTextCharFormat keywordFormat;
     QTextCharFormat classFormat;
     QTextCharFormat singleLineCommentFormat;
     QTextCharFormat multiLineCommentFormat;
     QTextCharFormat quotationFormat;
     QTextCharFormat functionFormat;
};

}

#endif	/* EDITOR_PHRASE_H */
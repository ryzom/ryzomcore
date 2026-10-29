// Ryzom Core - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "command_line_edit.h"

#include "icons.h"

#include <QPainter>
#include <QStyle>
#include <QStyleOptionFrame>

namespace {

/// QLineEdit's own margin between the frame contents and the text.
const int s_HorizontalMargin = 2;
/// Room kept between the ghost text and the help at the right.
const int s_HelpGap = 24;

} // namespace

CCommandLineEdit::CCommandLineEdit(CProcessEntry *entry, QWidget *parent)
    : QLineEdit(parent)
    , m_Entry(entry)
{
	connect(entry, &CProcessEntry::commandsChanged, this, &CCommandLineEdit::onCommandsChanged);
	connect(this, &QLineEdit::textChanged, this, &CCommandLineEdit::onTextChanged);
}

void CCommandLineEdit::onCommandsChanged()
{
	m_CompletionIndex = 0;
	update();
}

void CCommandLineEdit::onTextChanged()
{
	m_CompletionIndex = 0;
}

bool CCommandLineEdit::suggestion(CSuggestion &out) const
{
	const QMap<QString, CProcessEntry::CCommand> &commands = m_Entry->commands();
	const QString input = text();
	if (commands.isEmpty() || input.isEmpty())
		return false;
	int space = input.indexOf(' ');
	if (space < 0)
	{
		// the names starting with the word; those matching its case first, each group in name order
		QList<CProcessEntry::CCommand> exact;
		QList<CProcessEntry::CCommand> loose;
		for (const CProcessEntry::CCommand &command : commands)
		{
			if (command.Name.startsWith(input))
				exact.append(command);
			else if (command.Name.startsWith(input, Qt::CaseInsensitive))
				loose.append(command);
		}
		exact.append(loose);
		if (exact.isEmpty())
			return false;
		out.Candidates = exact;
		out.Index = m_CompletionIndex % exact.size();
		out.Naming = true;
		return true;
	}
	const QString word = input.left(space);
	QMap<QString, CProcessEntry::CCommand>::const_iterator it = commands.find(word);
	if (it == commands.end())
	{
		for (it = commands.begin(); it != commands.end(); ++it)
			if (it->Name.compare(word, Qt::CaseInsensitive) == 0)
				break;
		if (it == commands.end())
			return false;
	}
	out.Candidates.clear();
	out.Candidates.append(*it);
	out.Index = 0;
	out.Naming = false;
	return true;
}

bool CCommandLineEdit::focusNextPrevChild(bool next)
{
	// Tab and Shift+Tab reach here before keyPressEvent; with a completion going they are its keys
	CSuggestion found;
	if (suggestion(found) && found.Naming)
	{
		if (next)
		{
			const CProcessEntry::CCommand &command = found.current();
			setText(command.Name + (command.Kind == CProcessEntry::HandlerKind ? "." : " "));
			m_CompletionIndex = 0;
		}
		else if (found.Candidates.size() > 1)
		{
			++m_CompletionIndex;
		}
		update();
		return true;
	}
	return QLineEdit::focusNextPrevChild(next);
}

void CCommandLineEdit::paintEvent(QPaintEvent *event)
{
	QLineEdit::paintEvent(event);

	CSuggestion found;
	if (!suggestion(found))
		return;
	const CProcessEntry::CCommand &command = found.current();

	QStyleOptionFrame option;
	initStyleOption(&option);
	const QRect contents = style()->subElementRect(QStyle::SE_LineEditContents, &option, this).adjusted(s_HorizontalMargin, 0, -s_HorizontalMargin, 0);
	const QFontMetrics metrics = fontMetrics();
	const int textEnd = contents.left() + metrics.horizontalAdvance(text());
	if (textEnd > contents.right())
		return; // the text scrolls; nothing fits after it

	QPainter painter(this);
	painter.setFont(font());
	painter.setPen(Icons::dimColor(palette()));

	// the rest of the name, and the arguments, right after the text
	int ghostEnd = textEnd;
	if (found.Naming)
	{
		QString ghost = command.Name.mid(text().size());
		if (command.Kind == CProcessEntry::HandlerKind)
			ghost += ".";
		else if (!command.Args.isEmpty())
			ghost += " " + command.Args;
		const QRect ghostRect(textEnd, contents.top(), contents.right() - textEnd + 1, contents.height());
		const QString shown = metrics.elidedText(ghost, Qt::ElideRight, ghostRect.width());
		painter.drawText(ghostRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, shown);
		ghostEnd = textEnd + metrics.horizontalAdvance(shown);
	}

	// the help at the right, when there is room for it
	QString help = command.Help;
	if (!found.Naming && !command.Args.isEmpty())
		help = command.Args + (help.isEmpty() ? QString() : ": " + help);
	if (found.Naming && found.Candidates.size() > 1)
		help += QString("  [%1/%2 Shift+Tab]").arg(found.Index + 1).arg(found.Candidates.size());
	const int helpLeft = ghostEnd + s_HelpGap;
	if (help.isEmpty() || helpLeft >= contents.right())
		return;
	const QRect helpRect(helpLeft, contents.top(), contents.right() - helpLeft + 1, contents.height());
	painter.drawText(helpRect, Qt::AlignRight | Qt::AlignVCenter | Qt::TextSingleLine, metrics.elidedText(help, Qt::ElideRight, helpRect.width()));
}

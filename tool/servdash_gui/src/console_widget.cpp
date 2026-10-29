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

#include "console_widget.h"

#include "icons.h"

#include <QFontDatabase>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QTextBlock>
#include <QToolButton>
#include <QVBoxLayout>

namespace {

const int s_MaxBlocks = 5000;
const int s_MaxHistory = 200;

} // namespace

CConsoleWidget::CConsoleWidget(CProcessEntry *entry, QWidget *parent)
    : QWidget(parent)
    , m_Entry(entry)
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setContentsMargins(6, 6, 6, 6);
	layout->setSpacing(4);
	const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);

	if (entry->config().Control)
	{
		QHBoxLayout *top = new QHBoxLayout();
		top->setContentsMargins(0, 0, 0, 0);
		m_Toolbar = new QWidget(this);
		m_ToolbarLayout = new QVBoxLayout(m_Toolbar);
		m_ToolbarLayout->setContentsMargins(0, 0, 0, 0);
		m_ToolbarLayout->setSpacing(3);
		top->addWidget(m_Toolbar, 1);
		m_RawButton = new QToolButton(this);
		m_RawButton->setText("Raw");
		m_RawButton->setToolTip("Show the raw output of the process instead of its console (F9)");
		m_RawButton->setCheckable(true);
		m_RawButton->setAutoRaise(true);
		connect(m_RawButton, &QToolButton::toggled, this, &CConsoleWidget::onRawToggled);
		top->addWidget(m_RawButton, 0, Qt::AlignTop);
		layout->addLayout(top);
	}

	m_Log = new QPlainTextEdit(this);
	m_Log->setReadOnly(true);
	m_Log->setMaximumBlockCount(s_MaxBlocks);
	m_Log->setLineWrapMode(QPlainTextEdit::NoWrap);
	m_Log->setFont(fixedFont);
	layout->addWidget(m_Log, 1);

	m_Input = new QLineEdit(this);
	m_Input->setFont(fixedFont);
	m_Input->setPlaceholderText(entry->config().Control ? tr("command") : tr("stdin"));
	m_Input->installEventFilter(this);
	connect(m_Input, &QLineEdit::returnPressed, this, &CConsoleWidget::onSubmit);
	layout->addWidget(m_Input);

	connect(entry, &CProcessEntry::outputLine, this, &CConsoleWidget::onOutputLine);
	connect(entry, &CProcessEntry::consoleLine, this, &CConsoleWidget::onConsoleLine);
	connect(entry, &CProcessEntry::consoleCleared, this, &CConsoleWidget::onConsoleCleared);
	connect(entry, &CProcessEntry::labelChanged, this, &CConsoleWidget::onLabelChanged);
	connect(entry, &CProcessEntry::labelsReset, this, &CConsoleWidget::onLabelsReset);
	reload();
}

void CConsoleWidget::focusInput()
{
	m_Input->setFocus();
}

bool CConsoleWidget::showsConsole() const
{
	return m_Entry->config().Control && (m_RawButton == nullptr || !m_RawButton->isChecked());
}

void CConsoleWidget::reload()
{
	m_Log->clear();
	const QVector<CProcessEntry::CLine> &lines = showsConsole() ? m_Entry->consoleLines() : m_Entry->outputLines();
	for (const CProcessEntry::CLine &line : lines)
		appendLine(line);
	if (m_Toolbar != nullptr)
	{
		m_Toolbar->setVisible(showsConsole());
		rebuildToolbar();
	}
}

void CConsoleWidget::appendLine(const CProcessEntry::CLine &line)
{
	QScrollBar *bar = m_Log->verticalScrollBar();
	bool atBottom = bar->value() >= bar->maximum();
	QTextCursor cursor(m_Log->document());
	cursor.movePosition(QTextCursor::End);
	QTextCharFormat format;
	format.setForeground(Icons::logColor(line.Color, palette()));
	if (!m_Log->document()->isEmpty())
		cursor.insertBlock();
	cursor.insertText(line.Text, format);
	if (atBottom)
		bar->setValue(bar->maximum());
}

void CConsoleWidget::onOutputLine(CProcessEntry *entry, const CProcessEntry::CLine &line)
{
	Q_UNUSED(entry);
	if (!showsConsole())
		appendLine(line);
}

void CConsoleWidget::onConsoleLine(CProcessEntry *entry, const CProcessEntry::CLine &line)
{
	Q_UNUSED(entry);
	if (showsConsole())
		appendLine(line);
}

void CConsoleWidget::onConsoleCleared()
{
	if (showsConsole())
		m_Log->clear();
}

void CConsoleWidget::onRawToggled(bool raw)
{
	Q_UNUSED(raw);
	reload();
}

// *** toolbar ***

void CConsoleWidget::rebuildToolbar()
{
	if (m_ToolbarLayout == nullptr)
		return;
	for (QWidget *widget : m_LabelWidgets)
		delete widget;
	m_LabelWidgets.clear();

	// as CWinDisplayer::resizeLabels: each row divides its width equally among its own items
	const QStringList &labels = m_Entry->labels();
	QHBoxLayout *rowLayout = nullptr;
	for (int i = 0; i < labels.size(); ++i)
	{
		const QString &label = labels[i];
		if (label.isEmpty())
		{
			rowLayout = nullptr;
			continue;
		}
		if (rowLayout == nullptr)
		{
			QWidget *row = new QWidget(m_Toolbar);
			rowLayout = new QHBoxLayout(row);
			rowLayout->setContentsMargins(0, 0, 0, 0);
			rowLayout->setSpacing(6);
			m_ToolbarLayout->addWidget(row);
			m_LabelWidgets.append(row);
		}
		QWidget *widget;
		if (label.startsWith('@'))
		{
			QString body = label.mid(1);
			int bar = body.indexOf('|');
			QString text = bar >= 0 ? body.left(bar) : body;
			QString command = bar >= 0 ? body.mid(bar + 1) : body;
			QPushButton *button = new QPushButton(text, rowLayout->parentWidget());
			connect(button, &QPushButton::clicked, this, [this, text, command]() { pressButton(text, command); });
			widget = button;
		}
		else
		{
			QLabel *value = new QLabel(label, rowLayout->parentWidget());
			value->setObjectName(QString("label%1").arg(i));
			value->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
			value->setMargin(2);
			value->setTextInteractionFlags(Qt::TextSelectableByMouse);
			widget = value;
		}
		rowLayout->addWidget(widget, 1);
	}
}

void CConsoleWidget::onLabelChanged(CProcessEntry *entry, int index, const QString &value)
{
	Q_UNUSED(entry);
	if (m_Toolbar == nullptr)
		return;
	QLabel *label = m_Toolbar->findChild<QLabel *>(QString("label%1").arg(index));
	if (label != nullptr && !value.isEmpty() && !value.startsWith('@'))
		label->setText(value);
	else
		rebuildToolbar();
}

void CConsoleWidget::onLabelsReset()
{
	rebuildToolbar();
}

void CConsoleWidget::pressButton(const QString &text, const QString &command)
{
	Q_UNUSED(text);
	if (command == "CLEAR")
		m_Entry->clearConsole();
	else
		m_Entry->sendCommand(command);
	m_Input->setFocus();
}

// *** input ***

void CConsoleWidget::onSubmit()
{
	QString text = m_Input->text();
	m_Input->clear();
	m_HistoryPos = -1;
	m_Draft.clear();
	if (text.isEmpty())
		return;
	if (m_History.isEmpty() || m_History.last() != text)
	{
		m_History.append(text);
		if (m_History.size() > s_MaxHistory)
			m_History.removeFirst();
	}
	if (showsConsole())
	{
		if (text == "CLEAR")
			m_Entry->clearConsole();
		else
			m_Entry->sendCommand(text);
	}
	else
	{
		m_Entry->sendInput(text);
	}
}

bool CConsoleWidget::eventFilter(QObject *watched, QEvent *event)
{
	if (watched == m_Input && event->type() == QEvent::KeyPress)
	{
		QKeyEvent *key = static_cast<QKeyEvent *>(event);
		if (key->key() == Qt::Key_Up || key->key() == Qt::Key_Down)
		{
			if (m_History.isEmpty())
				return true;
			if (key->key() == Qt::Key_Up)
			{
				if (m_HistoryPos < 0)
				{
					m_Draft = m_Input->text();
					m_HistoryPos = m_History.size() - 1;
				}
				else if (m_HistoryPos > 0)
				{
					--m_HistoryPos;
				}
				m_Input->setText(m_History[m_HistoryPos]);
			}
			else if (m_HistoryPos >= 0)
			{
				if (m_HistoryPos + 1 < m_History.size())
				{
					++m_HistoryPos;
					m_Input->setText(m_History[m_HistoryPos]);
				}
				else
				{
					m_HistoryPos = -1;
					m_Input->setText(m_Draft);
				}
			}
			return true;
		}
		if (key->key() == Qt::Key_F9 && m_RawButton != nullptr)
		{
			m_RawButton->toggle();
			return true;
		}
	}
	return QWidget::eventFilter(watched, event);
}

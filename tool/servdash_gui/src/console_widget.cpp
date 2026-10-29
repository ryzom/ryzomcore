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
#include "view_settings.h"

#include <QFontDatabase>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QActionGroup>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QShowEvent>
#include <QTextBlock>
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
	// the splitter handle is the gap towards the column; the other sides match the column margins
	layout->setContentsMargins(0, 6, 6, 6);
	layout->setSpacing(4);
	const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);

	if (entry->config().Control)
	{
		m_Toolbar = new QWidget(this);
		m_ToolbarLayout = new QVBoxLayout(m_Toolbar);
		m_ToolbarLayout->setContentsMargins(0, 0, 0, 0);
		m_ToolbarLayout->setSpacing(3);
		layout->addWidget(m_Toolbar);
	}

	m_Log = new QPlainTextEdit(this);
	m_Log->setReadOnly(true);
	m_Log->setMaximumBlockCount(s_MaxBlocks);
	m_Log->setUndoRedoEnabled(false);
	m_Log->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_Log, &QPlainTextEdit::customContextMenuRequested, this, &CConsoleWidget::onLogContextMenu);
	connect(&CViewSettings::instance(), &CViewSettings::changed, this, &CConsoleWidget::onViewSettingsChanged);
	applyWrapMode();
	m_Log->setFont(fixedFont);
	layout->addWidget(m_Log, 1);

	m_Input = new QLineEdit(this);
	m_Input->setFont(fixedFont);
	m_Input->setPlaceholderText(entry->config().Control ? tr("command") : tr("stdin"));
	m_Input->installEventFilter(this);
	connect(m_Input, &QLineEdit::returnPressed, this, &CConsoleWidget::onSubmit);
	layout->addWidget(m_Input);

	connect(entry, &CProcessEntry::lineAdded, this, &CConsoleWidget::onLineAdded);
	connect(entry, &CProcessEntry::linesReset, this, &CConsoleWidget::onLinesReset);
	connect(entry, &CProcessEntry::labelChanged, this, &CConsoleWidget::onLabelChanged);
	connect(entry, &CProcessEntry::labelsReset, this, &CConsoleWidget::onLabelsReset);
	reload();
}

void CConsoleWidget::focusInput()
{
	m_Input->setFocus();
}

bool CConsoleWidget::isConsole() const
{
	return m_Entry->config().Control;
}

void CConsoleWidget::applyWrapMode()
{
	m_Log->setLineWrapMode(CViewSettings::instance().wrap() ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
}

void CConsoleWidget::onViewSettingsChanged()
{
	applyWrapMode();
	// only the pane on screen rebuilds now; a hidden one does when it is shown
	if (isVisible())
		reload();
	else
		m_ReloadPending = true;
}

void CConsoleWidget::showEvent(QShowEvent *event)
{
	QWidget::showEvent(event);
	if (m_ReloadPending)
		reload();
}

void CConsoleWidget::onLogContextMenu(const QPoint &pos)
{
	CViewSettings &settings = CViewSettings::instance();
	QMenu *menu = m_Log->createStandardContextMenu(pos);
	menu->addSeparator();
	QAction *prefix = menu->addAction(tr("Compact log prefixes"));
	prefix->setCheckable(true);
	prefix->setChecked(settings.compactPrefix());
	connect(prefix, &QAction::toggled, &settings, &CViewSettings::setCompactPrefix);
	QAction *wrap = menu->addAction(tr("Wrap lines"));
	wrap->setCheckable(true);
	wrap->setChecked(settings.wrap());
	connect(wrap, &QAction::toggled, &settings, &CViewSettings::setWrap);
	if (CViewSettings::themeSwitchAvailable())
	{
		QMenu *theme = menu->addMenu(tr("Theme"));
		QActionGroup *group = new QActionGroup(theme);
		const struct
		{
			const char *Name;
			CViewSettings::TTheme Theme;
		} themes[3] = { { "System", CViewSettings::System }, { "Light", CViewSettings::Light }, { "Dark", CViewSettings::Dark } };
		for (int i = 0; i < 3; ++i)
		{
			QAction *action = theme->addAction(tr(themes[i].Name));
			action->setCheckable(true);
			action->setChecked(settings.theme() == themes[i].Theme);
			group->addAction(action);
			CViewSettings::TTheme value = themes[i].Theme;
			connect(action, &QAction::triggered, &settings, [&settings, value]() { settings.setTheme(value); });
		}
	}
	menu->exec(m_Log->mapToGlobal(pos));
	delete menu;
}

void CConsoleWidget::reload()
{
	m_ReloadPending = false;
	// one edit, no repaints in between: a rebuild of thousands of lines stays quick
	m_Log->setUpdatesEnabled(false);
	m_Log->clear();
	QTextCursor cursor(m_Log->document());
	cursor.beginEditBlock();
	for (const CProcessEntry::CLine &line : m_Entry->lines())
		insertLine(cursor, line);
	cursor.endEditBlock();
	m_Log->setUpdatesEnabled(true);
	QScrollBar *bar = m_Log->verticalScrollBar();
	bar->setValue(bar->maximum());
	if (m_Toolbar != nullptr)
		rebuildToolbar();
}

void CConsoleWidget::insertLine(QTextCursor &cursor, const CProcessEntry::CLine &line)
{
	const CViewSettings &settings = CViewSettings::instance();
	const QString text = settings.compactPrefix() ? CViewSettings::compactPrefix(line.Text) : line.Text;
	QTextCharFormat format;
	if (line.Raw && !line.Color.isValid() && isConsole())
		format.setForeground(palette().color(QPalette::Mid)); // stdout and stderr beside the console log
	else
		format.setForeground(Icons::logColor(line.Color, palette()));
	cursor.movePosition(QTextCursor::End);
	if (!m_Log->document()->isEmpty())
		cursor.insertBlock();
	// wrapped continuations hang four characters in, so a line's start stays easy to find
	QTextBlockFormat block;
	const qreal indent = QFontMetricsF(m_Log->font()).horizontalAdvance("    ");
	block.setLeftMargin(indent);
	block.setTextIndent(-indent);
	cursor.setBlockFormat(block);
	cursor.insertText(text, format);
}

void CConsoleWidget::appendLine(const CProcessEntry::CLine &line)
{
	QScrollBar *bar = m_Log->verticalScrollBar();
	bool atBottom = bar->value() >= bar->maximum();
	QTextCursor cursor(m_Log->document());
	insertLine(cursor, line);
	if (atBottom)
		bar->setValue(bar->maximum());
}

void CConsoleWidget::onLineAdded(CProcessEntry *entry, const CProcessEntry::CLine &line)
{
	Q_UNUSED(entry);
	appendLine(line);
}

void CConsoleWidget::onLinesReset()
{
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
	if (isConsole())
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
	}
	return QWidget::eventFilter(watched, event);
}

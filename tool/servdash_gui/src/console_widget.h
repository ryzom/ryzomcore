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

#ifndef SERVDASH_CONSOLE_WIDGET_H
#define SERVDASH_CONSOLE_WIDGET_H

#include "process_entry.h"

#include <QStringList>
#include <QWidget>

class QLineEdit;
class QPlainTextEdit;
class QVBoxLayout;

/// The pane of one process. For a NeL service it is the service console as the Win32 window
/// showed it: the toolbar of labels and command buttons in fixed columns, the log with a
/// colour per log type, and the command line with its history. Whatever the process writes
/// to stdout or stderr besides, and the dashboard's own notes, appear in the same log in grey.
/// A plain process shows its output and gets a line for its stdin.
class CConsoleWidget : public QWidget
{
	Q_OBJECT

public:
	explicit CConsoleWidget(CProcessEntry *entry, QWidget *parent = nullptr);

	CProcessEntry *entry() const { return m_Entry; }
	void focusInput();

protected:
	bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
	void onLineAdded(CProcessEntry *entry, const CProcessEntry::CLine &line);
	void onLinesReset();
	void onLabelChanged(CProcessEntry *entry, int index, const QString &value);
	void onLabelsReset();
	void onSubmit();

private:
	bool isConsole() const;
	void reload();
	void appendLine(const CProcessEntry::CLine &line);
	void rebuildToolbar();
	void pressButton(const QString &text, const QString &command);

	CProcessEntry *m_Entry;
	QWidget *m_Toolbar = nullptr;
	QVBoxLayout *m_ToolbarLayout = nullptr;
	QPlainTextEdit *m_Log = nullptr;
	QLineEdit *m_Input = nullptr;
	QList<QWidget *> m_LabelWidgets;
	QStringList m_History;
	int m_HistoryPos = -1;
	QString m_Draft;
};

#endif // SERVDASH_CONSOLE_WIDGET_H

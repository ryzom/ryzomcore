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

#ifndef SERVDASH_PROCESS_ROW_H
#define SERVDASH_PROCESS_ROW_H

#include "process_entry.h"

#include <QWidget>

class QLabel;
class QPushButton;
class QToolButton;

/// One row of the process column: the state light, the title button that selects the
/// process, and the start/stop button; ProcessControl in the Win32 tool.
class CProcessRow : public QWidget
{
	Q_OBJECT

public:
	explicit CProcessRow(CProcessEntry *entry, QWidget *parent = nullptr);

	CProcessEntry *entry() const { return m_Entry; }
	void setSelected(bool selected);

signals:
	void selected(CProcessEntry *entry);

protected:
	void resizeEvent(QResizeEvent *event) override;

private slots:
	void refresh();
	void onStartStop();

private:
	CProcessEntry *m_Entry;
	QLabel *m_Light = nullptr;
	QPushButton *m_Title = nullptr;
	QToolButton *m_StartStop = nullptr;
	bool m_Selected = false;
};

#endif // SERVDASH_PROCESS_ROW_H

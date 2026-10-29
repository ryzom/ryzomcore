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

#ifndef SERVDASH_MAIN_WINDOW_H
#define SERVDASH_MAIN_WINDOW_H

#include "dashboard.h"

#include <QList>
#include <QMainWindow>

class CConsoleWidget;
class CProcessRow;
class QStackedWidget;

/// The dashboard window, laid out as the Win32 servdash: a fixed column on the left with the
/// Launch All and Shutdown All buttons above the process rows, and the pane of the selected
/// process filling the rest. No menu or status bar; the status bar appears only while the
/// window waits for the processes to stop. Closing the window stops every process and waits
/// for them, as the original did; a second close kills what is left.
class CMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	CMainWindow(CDashboard *dashboard, QWidget *parent = nullptr);

protected:
	void closeEvent(QCloseEvent *event) override;

private slots:
	void select(CProcessEntry *entry);
	void refreshTitle();
	void onAllStopped();

private:
	void createPanels();

	CDashboard *m_Dashboard;
	QList<CProcessRow *> m_Rows;
	QList<CConsoleWidget *> m_Panes;
	QStackedWidget *m_Stack = nullptr;
	CProcessEntry *m_Selected = nullptr;
	bool m_CloseRequested = false;
};

#endif // SERVDASH_MAIN_WINDOW_H

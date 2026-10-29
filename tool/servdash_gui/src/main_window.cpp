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

#include "main_window.h"

#include "console_widget.h"
#include "icons.h"
#include "process_row.h"

#include <QCloseEvent>
#include <QPushButton>
#include <QScrollArea>
#include <QSplitter>
#include <QStackedWidget>
#include <QStatusBar>
#include <QVBoxLayout>

CMainWindow::CMainWindow(CDashboard *dashboard, QWidget *parent)
    : QMainWindow(parent)
    , m_Dashboard(dashboard)
{
	setObjectName("CMainWindow");
	createPanels();

	connect(m_Dashboard, &CDashboard::allStopped, this, &CMainWindow::onAllStopped);

	if (!m_Dashboard->entries().isEmpty())
		select(m_Dashboard->entries().first());
	refreshTitle();
	resize(1064, 540);
}

void CMainWindow::createPanels()
{
	QSplitter *splitter = new QSplitter(this);
	splitter->setChildrenCollapsible(false);
	splitter->setHandleWidth(6);

	// the fixed column of the original: the two buttons, then one row per process
	QWidget *column = new QWidget();
	QVBoxLayout *columnLayout = new QVBoxLayout(column);
	columnLayout->setContentsMargins(6, 6, 6, 6);
	columnLayout->setSpacing(4);

	QPushButton *launchAll = new QPushButton(Icons::play(), tr("Launch All"), column);
	launchAll->setToolTip(tr("Start every process of the dashboard in order"));
	connect(launchAll, &QPushButton::clicked, this, [this]() { m_Dashboard->launchAll(); });
	columnLayout->addWidget(launchAll);
	QPushButton *shutdownAll = new QPushButton(Icons::stop(), tr("Shutdown All"), column);
	shutdownAll->setToolTip(tr("Stop every running process"));
	connect(shutdownAll, &QPushButton::clicked, this, [this]() { m_Dashboard->shutdownAll(); });
	columnLayout->addWidget(shutdownAll);
	columnLayout->addSpacing(8);

	m_Stack = new QStackedWidget();
	for (CProcessEntry *entry : m_Dashboard->entries())
	{
		CProcessRow *row = new CProcessRow(entry, column);
		connect(row, &CProcessRow::selected, this, &CMainWindow::select);
		columnLayout->addWidget(row);
		m_Rows.append(row);
		CConsoleWidget *pane = new CConsoleWidget(entry, m_Stack);
		m_Stack->addWidget(pane);
		m_Panes.append(pane);
		connect(entry, &CProcessEntry::titleChanged, this, &CMainWindow::refreshTitle);
	}
	columnLayout->addStretch(1);

	QScrollArea *scroll = new QScrollArea();
	scroll->setWidget(column);
	scroll->setWidgetResizable(true);
	scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scroll->setFrameShape(QFrame::NoFrame);
	scroll->setMinimumWidth(180);
	splitter->addWidget(scroll);
	splitter->addWidget(m_Stack);
	splitter->setStretchFactor(0, 0);
	splitter->setStretchFactor(1, 1);
	splitter->setSizes({ 200, 860 });
	setCentralWidget(splitter);
}

void CMainWindow::select(CProcessEntry *entry)
{
	m_Selected = entry;
	for (int i = 0; i < m_Rows.size(); ++i)
	{
		bool selected = m_Rows[i]->entry() == entry;
		m_Rows[i]->setSelected(selected);
		if (selected)
		{
			m_Stack->setCurrentWidget(m_Panes[i]);
			m_Panes[i]->focusInput();
		}
	}
	refreshTitle();
}

void CMainWindow::refreshTitle()
{
	QString title = m_Dashboard->title();
	if (m_Selected != nullptr)
		title += " - " + m_Selected->title();
	setWindowTitle(title);
}

void CMainWindow::closeEvent(QCloseEvent *event)
{
	// MainWindow_FormClosing: everything stops first, the window goes when nothing runs
	if (!m_Dashboard->anyRunning())
	{
		event->accept();
		return;
	}
	// the status bar exists only for this wait
	if (m_CloseRequested)
	{
		// a second close while stopping: kill what is left
		m_Dashboard->killAll();
		statusBar()->showMessage(tr("Killing the remaining services..."));
	}
	else
	{
		m_CloseRequested = true;
		m_Dashboard->beginClosing();
		statusBar()->showMessage(tr("Stopping all services, the window closes when they are gone (close again to kill)..."));
	}
	event->ignore();
}

void CMainWindow::onAllStopped()
{
	close();
}

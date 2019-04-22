// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2014  Jan BOON (jan.boon@kaetemi.be)
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

#include <nel/misc/types_nl.h>
#include "main_window.h"

// STL includes

// Qt includes
#include <QtGui>
#include <QTreeView>
#include <QDirModel>
#include <QUndoStack>
#include <QScrollArea>
#include <QApplication>
#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QDockWidget>
#include <QToolBar>
#include <QStatusBar>
#include <QStyleFactory>
#include <QMessageBox>

// NeL includes
// #include <nel/misc/debug.h>
#include <nel/misc/i18n.h>
#include <nel/3d/u_driver.h>

// Project includes
#include "../shared_widgets/command_log.h"
#include "panoply_preview.h"

using namespace std;
using namespace NLMISC;

namespace NLTOOLS {

namespace {

QString nli18n(const char *label)
{
	return QString::fromUtf16(CI18N::get(label).c_str());
}

} /* anonymous namespace */

CMainWindow::CMainWindow(const QMap<QString, QSize> &customSizeHints, QWidget *parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags),
	m_PanoplyPreview(NULL), 
	m_CommandLog(NULL), m_CommandLogDock(NULL), 
	m_WidgetsMenu(NULL), m_HelpMenu(NULL), 
	m_AboutAct(NULL)
{
	setObjectName("CMainWindow");
	setWindowTitle(tr("NeL Panoply Preview"));

	createActions();
	createMenus();
	createToolBars();
	createStatusBar();

	m_PanoplyPreview = new CPanoplyPreview(this);
	setCentralWidget(m_PanoplyPreview);
	
	createDockWindows();
}

CMainWindow::~CMainWindow()
{
	
}

void CMainWindow::createActions()
{
	m_AboutAct = new QAction(this);
	connect(m_AboutAct, SIGNAL(triggered()), this, SLOT(about()));

	m_AboutAct->setText(tr("About"));
	m_AboutAct->setStatusTip(tr("About"));
}

void CMainWindow::createMenus()
{
	m_WidgetsMenu = menuBar()->addMenu(QString::null);
	
	m_HelpMenu = menuBar()->addMenu(QString::null);
	m_HelpMenu->addAction(m_AboutAct);

	m_WidgetsMenu->setTitle(tr("Widgets"));
	m_HelpMenu->setTitle(tr("Help"));
}

void CMainWindow::createToolBars()
{
	
}

void CMainWindow::createStatusBar()
{
	statusBar()->showMessage(tr("Ready"));
}

void CMainWindow::createDockWindows()
{
	// CommandLog (Console)
	{
		m_CommandLogDock = new QDockWidget(this);
		m_CommandLogDock->setWindowTitle(tr("Console"));
		m_CommandLogDock->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
		m_CommandLog = new NLQT::CCommandLogDisplayer(m_CommandLogDock);
		m_CommandLogDock->setWidget(m_CommandLog);
		addDockWidget(Qt::BottomDockWidgetArea, m_CommandLogDock);
		m_WidgetsMenu->addAction(m_CommandLogDock->toggleViewAction());
	}
}

void CMainWindow::about()
{
	QMessageBox::about(this, tr("Panoply Preview"), tr("Copyright (C) 2014  Jan BOON (jan.boon@kaetemi.be)"));
}

} /* namespace NLTOOLS */

/* end of file */

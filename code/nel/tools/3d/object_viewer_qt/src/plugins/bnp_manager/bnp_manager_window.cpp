// Object Viewer Qt - BNP Manager Plugin - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Roland Winklmeier <roland.m.winklmeier@googlemail.com>
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

// Project includes
#include "bnp_manager_window.h"
#include "bnp_manager_constants.h"
#include "bnp_dirtree_dialog.h"
#include "bnp_filelist_dialog.h"
#include "bnp_file.h"

#include "../core/icore.h"
#include "../core/menu_manager.h"
#include "../core/core_constants.h"
#include "../../extension_system/iplugin_spec.h"

// NeL includes
#include <nel/misc/debug.h>

// Qt includes
#include <QDebug>
#include <QFileDialog>
#include <QToolBar>
#include <QTableWidget>
#include <QMessageBox>
#include <QSettings>

using namespace std;
using namespace NLMISC;


namespace BNPManager
{

BNPManagerWindow::BNPManagerWindow(QWidget *parent)
    :   QMainWindow(parent)
{
    // add new mainwindow for sheet dockwidgets
	QTableWidget* hideWidget = new QTableWidget(0,0,this);
	setCentralWidget(hideWidget);
	hideWidget->hide();

	// Read the settings
	readSettings();
	
	// create main dialogs and display them
	createDialogs();

	// create actions like open, close, add etc.
	createActions();

	// create a toolbar with icons
	createToolBars();
	
	// this SLOT is triggered if the user activates a bnp files in the 
	// dirtree view
	connect(m_BnpDirTreeDialog, SIGNAL(selectedForm(const QString)), 
			this, SLOT(loadFile(const QString)));

	// not used
    m_undoStack = new QUndoStack(this);
}
// ***************************************************************************
BNPManagerWindow::~BNPManagerWindow()
{
	writeSettings();
}
// ***************************************************************************
void BNPManagerWindow::createDialogs()
{
	// create dialog to list the contents of the specified
	// bnp data file directory
	m_BnpDirTreeDialog = new CBnpDirTreeDialog(tr(m_DataPath.toStdString().c_str()),this);
	addDockWidget(Qt::LeftDockWidgetArea, m_BnpDirTreeDialog);
	m_BnpDirTreeDialog->setVisible(true);
	restoreDockWidget(m_BnpDirTreeDialog);

	// create dialog to list the packed file contents of bnp files on
	// the right hand side
	m_BnpFileListDialog = new BnpFileListDialog(m_DataPath,this);
	addDockWidget(Qt::RightDockWidgetArea, m_BnpFileListDialog);
	m_BnpFileListDialog->setVisible(true);
	restoreDockWidget(m_BnpFileListDialog);
}
// ***************************************************************************
void BNPManagerWindow::createActions()
{
	// open action
	m_openAction = new QAction(tr("&Open..."), this);
	m_openAction->setIcon(QIcon(Core::Constants::ICON_OPEN));
    m_openAction->setStatusTip(tr("Open file"));
	connect(m_openAction, SIGNAL(triggered()), this, SLOT( open() ));

	// close action
    m_closeAction = new QAction(tr("&Close..."), this);
	m_closeAction->setIcon(QIcon(Constants::ICON_CLOSE));
    m_closeAction->setStatusTip(tr("Close the BNP File"));
    connect(m_closeAction, SIGNAL(triggered()), this, SLOT( close() ));

	// add files into the bnp file
    m_addFilesAction = new QAction(tr("&Add..."), this);
	m_addFilesAction->setIcon(QIcon(Constants::ICON_ADD));
    m_addFilesAction->setStatusTip(tr("Add Files to BNP"));
    connect(m_addFilesAction, SIGNAL(triggered()), this, SLOT( addFiles() ));

	// delete files from the bnp file
    m_deleteFilesAction = new QAction(tr("&Delete..."), this);
    m_deleteFilesAction->setIcon(QIcon(Constants::ICON_DELETE));
    m_deleteFilesAction->setStatusTip(tr("Delete Files"));
    connect(m_deleteFilesAction, SIGNAL(triggered()), this, SLOT( deleteFiles() ));

	// unpack selected files into user defined dir
    m_unpackFilesAction = new QAction(tr("&Unpack..."), this);
	m_unpackFilesAction->setIcon(QIcon(Constants::ICON_UNPACK));
    m_unpackFilesAction->setStatusTip(tr("Unpack Files"));
    connect(m_unpackFilesAction, SIGNAL(triggered()), this, SLOT( unpackFiles() ));
}
// ***************************************************************************
void BNPManagerWindow::createToolBars()
{
	m_fileToolBar = addToolBar(tr("&File"));
	m_fileToolBar->addAction(m_openAction);
	m_fileToolBar->addAction(m_closeAction);

	m_toolsBar = addToolBar(tr("&Tools"));
	m_toolsBar->addAction(m_addFilesAction);
	m_toolsBar->addAction(m_deleteFilesAction);
	m_toolsBar->addAction(m_unpackFilesAction);
}
// ***************************************************************************
bool BNPManagerWindow::loadFile(const QString fileName)
{
	m_BnpFileListDialog->loadTable(fileName);
	return true;
}
// ***************************************************************************
void BNPManagerWindow::open()
{
	QString fileName;
	// file dialog to select with file should be opened
	fileName = QFileDialog::getOpenFileName(this,
		tr("Open BNP file"), tr(m_DataPath.toStdString().c_str()), tr("BNP Files (*.bnp)"));

	// check if there is a filename
	if (fileName.isNull())
		return;
    loadFile(fileName);
}
// ***************************************************************************
void BNPManagerWindow::close()
{
    //TODO
}
// ***************************************************************************
void BNPManagerWindow::addFiles()
{
    //TODO
}
// ***************************************************************************
void BNPManagerWindow::deleteFiles()
{
    //TODO
}
// ***************************************************************************
void BNPManagerWindow::unpackFiles()
{
	QFileDialog filedialog(this);
	BNPFileHandle& myBNPFileHandle = BNPFileHandle::getInstance();
	vector<string> selectedrows;

	m_BnpFileListDialog->getSelections(selectedrows);

	// Check if files were selected. If not, inform the user.
	// TODO: Ask the user if nothing was selected, if he wants to unpack all
	// files. This is more like Winzip.
	if (selectedrows.empty())
	{
		QMessageBox::information(this, tr("BNP Manager"),
                                tr("No files were selected to unpack!"),
                                QMessageBox::Ok,
								QMessageBox::Ok);
		return;
	}

	QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
								tr(m_DataPath.toStdString().c_str()),
								QFileDialog::ShowDirsOnly
								| QFileDialog::DontResolveSymlinks);
	
	if (myBNPFileHandle.unpack(dir.toStdString(),selectedrows))
	{
		QMessageBox::information(this, tr("BNP Manager"),
                                tr("All files has been exported successfully."),
                                QMessageBox::Ok,
								QMessageBox::Ok);
	}
}
// ***************************************************************************
void BNPManagerWindow::readSettings()
{
	QSettings *settings = Core::ICore::instance()->settings();

	settings->beginGroup(Core::Constants::DATA_PATH_SECTION);
	m_DataPath = settings->value(Core::Constants::ASSETS_PATH, "w:/database").toString();
	settings->endGroup();
}
// ***************************************************************************
void BNPManagerWindow::writeSettings()
{
}
} // namespace BNPManager

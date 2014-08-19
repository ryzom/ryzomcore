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
#include <nel/misc/path.h>

// STL includes
#include <vector>
#include <string>

// Qt includes
#include <QDebug>
#include <QFileDialog>
#include <QToolBar>
#include <QTableWidget>
#include <QMessageBox>
#include <QSettings>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QUrl>
#include <QEvent>
#include <QInputDialog>

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

	setAcceptDrops(true);

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
	connect(m_BnpDirTreeDialog, SIGNAL(selectedFile(const QString)), 
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
	m_BnpDirTreeDialog = new CBnpDirTreeDialog(tr(m_DataPath.toUtf8().constData()),this);
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
	// new action
	m_newAction = new QAction(tr("&New..."), this);
	m_newAction->setIcon(QIcon(Core::Constants::ICON_NEW));
    m_newAction->setStatusTip(tr("New file"));
	connect(m_newAction, SIGNAL(triggered()), this, SLOT( newFile() ));

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
	m_fileToolBar->addAction(m_newAction);
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
	// Store the filename for later use
	m_openedBNPFile = fileName;
	m_BnpFileListDialog->loadTable(fileName);
	return true;
}
// ***************************************************************************
void BNPManagerWindow::newFile()
{
	// reference to the BNPFileHandle singletone instance
	BNPFileHandle& myBNPFileHandle = BNPFileHandle::getInstance();

	m_openedBNPFile = "";
	m_BnpFileListDialog->clearTable();

	QString filePath = QFileDialog::getSaveFileName(this, tr("Create File"),QDir::currentPath(),
                            tr("BNP File (*.bnp)"));

	if (filePath.isEmpty() )
		return;

	if ( !filePath.endsWith(".bnp", Qt::CaseInsensitive) )
		filePath.append(".bnp");

	m_openedBNPFile = filePath;
	m_BnpFileListDialog->setWindowTitle (filePath);

	myBNPFileHandle.createFile ( filePath.toUtf8().constData() );

}
// ***************************************************************************
void BNPManagerWindow::open()
{
	QString fileName;
	// file dialog to select with file should be opened
	fileName = QFileDialog::getOpenFileName(this,
		tr("Open BNP file"), tr(m_DataPath.toUtf8().constData()), tr("BNP Files (*.bnp)"));

	// Check if filename is empty
	if (fileName.isNull())
		return;

    loadFile(fileName);
}
// ***************************************************************************
void BNPManagerWindow::close()
{
    m_openedBNPFile = "";
	m_BnpFileListDialog->clearTable();
}
// ***************************************************************************
void BNPManagerWindow::addFiles()
{
	// reference to the BNPFileHandle singletone instance
	BNPFileHandle& myBNPFileHandle = BNPFileHandle::getInstance();

	// vector of all current packed filenames
	vector<string> currentFiles;

	// vector of files to add
	vector<string> addFiles;

    // open a file dialog and to add files
	QStringList FileList;

	FileList = QFileDialog::getOpenFileNames(this,tr("Add Files..."),
							QDir::currentPath(), tr("All Files (*.*)") );
	
	// get all current filenames from the opened bnp file
	myBNPFileHandle.fileNames(currentFiles);

	QStringList::iterator it_list = FileList.begin();
	while (it_list != FileList.end() )
	{
		string fileName = CFile::getFilename (it_list->toUtf8().constData() );
		if ( std::find(currentFiles.begin(), currentFiles.end(), fileName ) !=  currentFiles.end() )
		{
			// Ask the user if he wants to override the existing file
			// atm only warn the user and do not override
			QMessageBox::warning(this, tr("BNP Manager"),
                                tr("File is already in the list!"),
                                QMessageBox::Ok,
								QMessageBox::Ok);
		}
		else
		{
			addFiles.push_back( it_list->toUtf8().constData() );
			// log it
			nlinfo("Add file %s", fileName.c_str() );
		}
		it_list++;
	}
	
	if ( !addFiles.empty() )
	{
		myBNPFileHandle.addFiles( addFiles );
	}
	loadFile(m_openedBNPFile);
}
// ***************************************************************************
void BNPManagerWindow::addFiles( QStringList FileList )
{
	// reference to the BNPFileHandle singletone instance
	BNPFileHandle& myBNPFileHandle = BNPFileHandle::getInstance();

	// vector of all current packed filenames
	vector<string> currentFiles;

	// vector of files to add
	vector<string> addFiles;
	
	// get all current filenames from the opened bnp file
	myBNPFileHandle.fileNames(currentFiles);

	QStringList::iterator it_list = FileList.begin();
	while (it_list != FileList.end() )
	{
		string fileName = CFile::getFilename (it_list->toUtf8().constData() );
		if ( std::find(currentFiles.begin(), currentFiles.end(), fileName ) !=  currentFiles.end() )
		{
			// Ask the user if he wants to override the existing file
			// atm only warn the user and do not override
			QMessageBox::warning(this, tr("BNP Manager"),
                                tr("File is already in the list!"),
                                QMessageBox::Ok,
								QMessageBox::Ok);
		}
		else
		{
			addFiles.push_back( it_list->toUtf8().constData() );
			// log it
			nlinfo("Add file %s", fileName.c_str() );
		}
		it_list++;
	}
	
	if ( !addFiles.empty() )
	{
		myBNPFileHandle.addFiles( addFiles );
	}
	loadFile(m_openedBNPFile);
}
// ***************************************************************************
void BNPManagerWindow::deleteFiles()
{
    QFileDialog filedialog(this);
	BNPFileHandle& myBNPFileHandle = BNPFileHandle::getInstance();
	vector<string> selectedRows;

	m_BnpFileListDialog->getSelections(selectedRows);

	// Check if files were selected. If not, inform the user.
	if (selectedRows.empty())
	{
		QMessageBox::information(this, tr("BNP Manager"),
                                tr("No files selected!"),
                                QMessageBox::Ok,
								QMessageBox::Ok);
		return;
	}
	
	myBNPFileHandle.deleteFiles(selectedRows);
	loadFile(m_openedBNPFile);
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
                                tr("No files selected!"),
                                QMessageBox::Ok,
								QMessageBox::Ok);
		return;
	}

	QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
								tr(m_DataPath.toUtf8().constData()),
								QFileDialog::ShowDirsOnly
								| QFileDialog::DontResolveSymlinks);

	// If anything went wrong or the user pressed "cancel"
	if ( dir.isEmpty() )
		return;
	
	if (myBNPFileHandle.unpack(dir.toUtf8().constData(),selectedrows))
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

// ***************************************************************************
void BNPManagerWindow::dragEnterEvent(QDragEnterEvent *event)
{
	// Accept only one file
	// In the future a tabbed FileListDialog would accept more
	if ( event->mimeData()->hasUrls() )
		event->acceptProposedAction();
}
// ***************************************************************************
void BNPManagerWindow::dropEvent(QDropEvent *event)
{
	// reference to the BNPFileHandle singletone instance
	BNPFileHandle& myBNPFileHandle = BNPFileHandle::getInstance();

	// Excraft the local file url from the drop object and fill the table
	const QMimeData *mimeData = event->mimeData();
	QList<QUrl> urlList = mimeData->urls();
	QString filePath;
	QStringList fileList;

	if ( urlList.count() == 1 )
	{
		// If it is a bnp file, open it
		// If it is not a bnp file add it

		filePath = urlList.first().toLocalFile();
		if ( filePath.endsWith(".bnp", Qt::CaseInsensitive) )
		{
			loadFile(filePath);
		}
		else
		{
			if ( m_openedBNPFile == "")
				newFile();
			// Create a QStringList and pass it to addfiles
			fileList.push_back( filePath );
			addFiles( fileList );
			// Reload current bnp
			loadFile(m_openedBNPFile);
		}
	}
	else if ( urlList.count() > 1 )
	{
		// Dont accept any bnp file
		QList<QUrl>::iterator it = urlList.begin();
		while ( it != urlList.end() )
		{
			filePath = it->toLocalFile();
			if ( filePath.endsWith(".bnp") )
			{
				nlwarning("Could not add bnp file %s!", filePath.toUtf8().constData() );
			}
			else
			{
				fileList.push_back( filePath );
			}
			++it;
		}
		if ( m_openedBNPFile == "")
			newFile();
		addFiles( fileList );
		// Reload current bnp
		loadFile(m_openedBNPFile);
	}
}
} // namespace BNPManager

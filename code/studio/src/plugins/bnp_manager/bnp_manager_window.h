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

#ifndef BNP_MANAGER_WINDOW_H
#define BNP_MANAGER_WINDOW_H

// Project includes
//#include "ui_bnp_manager_window.h"

// Qt includes
#include <QtGui/QMainWindow>
#include <QtGui/QLabel>
#include <QtGui/QUndoStack>
#include <QtGui/QTableWidget>


namespace BNPManager
{

class CBnpDirTreeDialog;
class BnpFileListDialog;
class BNPFileHandle;

/**
 * Main window class. Derived from QMainWindow and implements 
 * the basic layout like menue, toolbars and dialogs.
 *
 * \date 2011
 */

class BNPManagerWindow : public QMainWindow
{
    Q_OBJECT

public:

	// Constructor
    BNPManagerWindow(QWidget *parent = 0);

	//Destructor
    ~BNPManagerWindow();


    QUndoStack *m_undoStack;

public Q_SLOTS:

	/**
	 * Create a new file
	 * \return Filename string 
	 */
	void newFile();

	/**
	 * Open a file dialog to choose which file should be opened.
	 */
    void open();

	/**
	 * Load a certain bnp file into the manager
	 * \param Filename
	 * \return true if everything went well
	 */
	bool loadFile(const QString fileName);

	/**
	 * close an opened bnp file and reset all views
	 */
    void close();

	/**
	 * Add files into an opened bnp file.
	 * \param Filelist
	 */
    void addFiles();
	void addFiles( QStringList FileList );

	/**
	 * Unpack the files marked in the filelist dialog into user defined 
	 * directory.
	 * \param TBD
	 * \return true if everything went well
	 */
    void unpackFiles();

	/**
	 * Delete marked files from the bnp file
	 * \param TBD
	 */
    void deleteFiles();

protected:
	void dragEnterEvent (QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);

private:

	/**
	 * Read plugin settings and set the window accordingly
	 */
	void readSettings();

	/**
	 * Write plugin settings
	 */
	void writeSettings();

	/**
	 * Create all plugin dialogs
	 */
	void createDialogs();

	/**
	 * Create all plugin actions
	 */
	void createActions();

	/**
	 * Create the plugin toolbar
	 */
	void createToolBars();

	QToolBar *m_fileToolBar;
	QToolBar *m_toolsBar;

	QAction *m_newAction;
    QAction *m_openAction;
    QAction *m_closeAction;
    QAction *m_addFilesAction;
    QAction *m_unpackFilesAction;
    QAction *m_deleteFilesAction;

	CBnpDirTreeDialog *m_BnpDirTreeDialog;
	BnpFileListDialog *m_BnpFileListDialog;

	QString m_DataPath;
	QString m_openedBNPFile;

}; /* class BNPManagerWindow */

} /* namespace Plugin */

#endif

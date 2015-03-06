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

#ifndef BNP_FILELIST_DIALOG_H
#define BNP_FILELIST_DIALOG_H

// Qt includes
#include <QtGui/QWidget>

// STL includes
#include <vector>
#include <string>

// NeL includes

// Project includes
#include "ui_bnp_filelist_dialog.h"

namespace BNPManager
{

typedef std::vector<std::string> TSelectionList;

class BnpFileListDialog : public QDockWidget
{
	Q_OBJECT

public:

	// Constructor
	BnpFileListDialog(QString bnpPath, QWidget *parent = 0);

	// Destructor
	~BnpFileListDialog();

	/**
	 * Load the bnp file and setup the table view
	 * \param Filename
	 * \return true if everything went well
	 */
	bool loadTable(const QString filePath);

	/**
	 * Set the dimension of the table
	 * \param number of rows
	 */
	void setupTable(int nbrows);

	/**
	 * When BNP files is closed, clear the filelist table
	 */
	void clearTable();

	/**
	 * Fill the files selected in the table view to
	 * unpack them.
	 * \param reference to a vector of filenames.
	 * \return true if everything went well
	 */
	void getSelections(TSelectionList& SelectionList);

private:
	Ui::BnpFileListDialog m_ui;

	// common data path as root folder for the dirtree view
	QString m_DataPath;

};

}

#endif

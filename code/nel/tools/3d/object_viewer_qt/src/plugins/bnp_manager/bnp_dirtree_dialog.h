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

#ifndef BNP_DIRTREE_DIALOG_H
#define BNP_DIRTREE_DIALOG_H

// Qt includes
#include <QtGui/QWidget>

// STL includes

// NeL includes

// Project includes
#include "ui_bnp_dirtree_form.h"

namespace BNPManager
{

class BNPFileSystemModel;
class BNPSortProxyModel;

class CBnpDirTreeDialog : public QDockWidget
{
	Q_OBJECT
public:

	/**
	 * Constructor
	 * \param path to root directory, which should be displayed
	 */
	CBnpDirTreeDialog(QString bnpPath, QWidget *parent = 0);

	/**
	 * Destructor
	 */
	~CBnpDirTreeDialog();

	/**
	 * Change the root path for the dir tree view
	 * \param data path to the new directory
	 */
	void BnpPathChanged(QString);

private:

	Ui::CBnpDirTreeDialog	m_ui;

	// path ro data root directory
	QString					m_DataPath;

	BNPFileSystemModel		*m_dirModel;

	BNPSortProxyModel		*m_proxyModel;

Q_SIGNALS:
	void selectedFile(const QString);

private Q_SLOTS:
	/**
	 * Triggered if the user activates (double klick on windows)
	 * a file name in the dir tree view
	 * \param selected ModelIndex (filename)
	 */
	void fileSelected(QModelIndex index);

	void changeFile(QString file);
};
}
#endif
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
#include "bnp_dirtree_dialog.h"
#include "bnp_filesystem_model.h"
#include "bnp_proxy_model.h"

// Qt includes
#include <QtGui/QWidget>

// NeL includes
#include <nel/misc/debug.h>

namespace BNPManager
{

CBnpDirTreeDialog::CBnpDirTreeDialog(QString bnpPath, QWidget *parent)
	:	QDockWidget(parent),
		m_DataPath(bnpPath)
{
	// Setup the dialog
	m_ui.setupUi(this);

	// Filter settings to only display files with bnp extension.
	// Could be changed to display all files and react according to the extension:
	// Bnp file: opened and displayed
	// all other files: added to the currently opened bnp file
	QStringList filter;
    filter << tr("*.bnp");

	// Setup the directory tree model
	m_dirModel= new BNPFileSystemModel();
	m_proxyModel = new BNPSortProxyModel();
	m_ui.dirTree->setSortingEnabled(true);
	m_dirModel->setRootPath(m_DataPath);
	m_dirModel->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::AllEntries);
	m_dirModel->setNameFilters(filter);
    m_dirModel->setNameFilterDisables(0);

	m_proxyModel->setSourceModel(m_dirModel);

	m_ui.dirTree->setModel(m_proxyModel);

	m_ui.dirTree->setRootIndex( m_proxyModel->mapFromSource (m_dirModel->index(m_DataPath) ) );

	// Trigger if one filename is activated
	// In future drag&drop should be also possible
	connect(m_ui.dirTree, SIGNAL(activated(QModelIndex)),
			this, SLOT(fileSelected(QModelIndex)));
}
// ***************************************************************************
CBnpDirTreeDialog::~CBnpDirTreeDialog()
{

}
// ***************************************************************************
void CBnpDirTreeDialog::fileSelected(QModelIndex index)
{
	QModelIndex source = m_proxyModel->mapToSource(index);
	if (source.isValid() && !m_dirModel->isDir(source))
	{
		// emit the according signal to BNPManagerWindow class
		Q_EMIT selectedFile(m_dirModel->fileInfo(source).filePath());
	}
}
// ***************************************************************************
void CBnpDirTreeDialog::changeFile(QString file)
{

}
// ***************************************************************************
void CBnpDirTreeDialog::BnpPathChanged(QString path)
{

}
// ***************************************************************************
}


